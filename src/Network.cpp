/*Copyright (C) 2012-2014, 2021 Carsten Paproth

This file is part of Skat-Konferenz.

Skat-Konferenz is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Skat-Konferenz is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Skat-Konferenz.  If not, see <http://www.gnu.org/licenses/>.*/


#include "Network.h"
#include <iostream>
#include "Convenience.h"


using namespace SK;
using namespace CPLib;
using namespace boost::asio;


static istream& operator>>(istream& s, ip::udp::endpoint& ep) {
	boost::system::error_code e;
	string a;
	unsigned short p = 0;
	getline(s, a, ':');
	s >> p;
	ep = ip::udp::endpoint(ip::address_v4::from_string(a, e), p);
	return s;
}


static bool ring_cmp(const vector<unsigned char>& l, const vector<unsigned char>& r) {
	return abs((int)(l[0] & 63) - (int)(r[0] & 63)) < 32? (l[0] & 63) < (r[0] & 63): (r[0] & 63) < (l[0] & 63);
}


Network::Network(boost::function<int()> w) : socket(io, ip::udp::v4()), timer(io), strand(io), wait_to_unlock(w) {
	msgid = 1;
	bandwidth = maxpeers * minbw;
	mutexbusy = 0;
	ignoredmsg = 0;
	recvbuf.resize(recvsize);
}


Network::~Network() {
	try {
		netmutex.lock();
		timer.cancel();
		socket.close();
		io.stop();
		netmutex.unlock();
		while (iothread1.joinable() && !iothread1.timed_join(boost::posix_time::milliseconds(100)))
			wait_to_unlock();
		while (iothread2.joinable() && !iothread2.timed_join(boost::posix_time::milliseconds(100)))
			wait_to_unlock();
	} catch (...) {}
}


void Network::add_handler(handler h) {
	while (!hdlmutex.try_lock())
		wait_to_unlock();
	boost::lock_guard<boost::mutex> lock(hdlmutex, boost::adopt_lock);
	handlers.push_back(h);
}


void Network::remove_handler() {
	while (!hdlmutex.try_lock())
		wait_to_unlock();
	boost::lock_guard<boost::mutex> lock(hdlmutex, boost::adopt_lock);
	handlers.clear();
}


void Network::connect(const string& address, unsigned short port, bool s, unsigned bw) {
	{
		boost::lock_guard<boost::timed_mutex> lock(netmutex);
		timer.cancel();
		socket.close();
		io.stop();
	}
	while (iothread1.joinable() && !iothread1.timed_join(boost::posix_time::milliseconds(100)))
		wait_to_unlock();
	while (iothread2.joinable() && !iothread2.timed_join(boost::posix_time::milliseconds(100)))
		wait_to_unlock();

	boost::lock_guard<boost::timed_mutex> lock(netmutex);
	peers.clear();
	ignoredpeers.clear();
	socket.open(ip::udp::v4());
	endpoint = udpendpoint();
	io.reset();

	minimal(bandwidth = bw, (unsigned)(maxpeers * minbw));

	if (address.empty() && !s)
		return;
	if (!address.empty() && s) {
		boost::system::error_code e;
		ip::udp::socket tmpsocket(io, ip::udp::v4());
		tmpsocket.bind(udpendpoint(ip::udp::v4(), port), e);
		s = e? false: true;
	}

	if (s) {
		server = true;
		socket.bind(udpendpoint(ip::udp::v4(), port));
	} else {
		server = false;
		ip::udp::resolver resolver(io);
		ip::udp::resolver::query query(ip::udp::v4(), address, ss(port));
		endpoint = *resolver.resolve(query);
		peers.push_back(Peer(endpoint));

		socket.send_to(buffer(recvbuf, 1), endpoint);
		ip::udp::socket tmpsocket(io, ip::udp::v4());
		tmpsocket.connect(endpoint);
		localendpoint = udpendpoint(tmpsocket.local_endpoint().address(), socket.local_endpoint().port());
		cout << "connect from " << localendpoint << " to " << endpoint << endl;
	}

	timer.expires_from_now(boost::posix_time::milliseconds(1000 / timerrate));
	timer.async_wait(boost::bind(&Network::deadline, this, _1));
	socket.async_receive_from(buffer(recvbuf), endpoint, boost::bind(&Network::receiver, this, _1, _2));
	iothread1 = boost::thread(boost::bind(&Network::worker, this, 1));
	iothread2 = boost::thread(boost::bind(&Network::worker, this, 2));

	cout << "network 2 threads started " << (server? "server": "peer") << " version " << version << endl;
}


void Network::send_buffer(unsigned i, boost::shared_ptr<ucharbuf> buf) {
	if (server || !peers[i].relayed) {
		socket.async_send_to(buffer(*buf), peers[i].endpoint, boost::bind(&Network::sender, this, buf, _1, _2));
	} else if ((buf->at(0) & 192) == 0) {
		buf->push_back(buf->at(0));
		buf->at(0) = 192;
		socket.async_send_to(buffer(*buf), peers.front().endpoint, boost::bind(&Network::sender, this, buf, _1, _2));
	}
}


bool Network::broadcast(const ucharbuf& send, vector<ucharbuf>& recv, unsigned latency) {
	recv.clear();

	if (!netmutex.timed_lock(boost::posix_time::milliseconds(latency))) {
		mutexbusy++;
		return false;
	}
	boost::lock_guard<boost::timed_mutex> lock(netmutex, boost::adopt_lock);

	boost::shared_ptr<ucharbuf> buf(new ucharbuf(send));

	bool sent = false;
	vector<Peer>::iterator minpeer = min_element(peers.begin(), peers.end(), boost::bind(&Peer::bucket, _1) < boost::bind(&Peer::bucket, _2));
	unsigned minbucket = minpeer != peers.end()? minpeer->bucket: 0;
	for (unsigned i = 0; i < peers.size() && send.size() > 0; i++) {
		if (!peers[i].connected)
			continue;
		if ((send[0] & 192) == 64) {
			if (peers[i].fifo.size() > 0) {
				recv.push_back(peers[i].fifo.front());
				peers[i].fifo.pop_front();
			} else {
				peers[i].fifoempty++;
			}
			send_buffer(i, buf);
			sent = true;
		} else if ((send[0] & 192) == 128) {
			while (peers[i].buffer.size() > 0) {
				recv.push_back(peers[i].buffer.front());
				recv.back()[0] = i;
				peers[i].buffer.pop_front();
			}

			if (minbucket >= buf->size()) {
				peers[i].bucket -= buf->size();
				send_buffer(i, buf);
				sent = true;
			}
		}
	}
	return sent;
}


void Network::command(unsigned i, const string& command, const string& data) {
	boost::lock_guard<boost::timed_mutex> lock(netmutex);
		
	if (i >= peers.size() || !peers[i].connected)
		return;
		
	peers[i].messages.push_back(ss(msgid++) << ' ' << command << ' ' << data);
	
	if (peers[i].messages.size() == 1) {
		boost::shared_ptr<ucharbuf> buf(new ucharbuf(peers[i].messages[0].begin(), peers[i].messages[0].end()));
		send_buffer(i, buf);
	}
}


void Network::stats() {
	boost::lock_guard<boost::timed_mutex> lock(netmutex);
	cout << "known peers: " << peers.size() << ", local port: " << socket.local_endpoint().port() << ", mutex busy: " << mutexbusy << ", messages ignored: " << ignoredmsg << endl;
	for (unsigned i = 0; i < peers.size(); i++)
		cout << "peer " << i << " (" << peers[i].endpoint << (peers[i].relayed? ", relayed": "") << ") fifo empty/full/size: " << peers[i].fifoempty << '/' << peers[i].fifofull << '/' << peers[i].fifo.size() << endl;
}


void Network::handle_command(unsigned i, const string& command, const string& data) {
	boost::lock_guard<boost::mutex> lock(hdlmutex);

	bool handled = false;
	for (unsigned j = 0; j < handlers.size(); j++)
		handled |= handlers[j](i, command, data);
	
	if (!handled)
		cout << "unhandled command: " << command << endl;
}


void Network::process_message(unsigned i, const string& message) {
	string id, command, data;
	data = ss(message) >> id >> command >> ws;

	if (command == "busy") {
		cout << "all seats of the server are occupied" << endl;
		return;
	}

	if (command == "reply") {
		if (peers[i].messages.size() > 0 && peers[i].messages[0].compare(0, id.length() + 1, id + ' ') == 0) {
			cout << i << " < " << string(ss(peers[i].messages[0]) >> id >> ws) << endl;
			peers[i].messages.pop_front();
		} else {
			ignoredmsg++;
		}
		return;
	} else {
		string reply = id + " reply";
		boost::shared_ptr<ucharbuf> buf(new ucharbuf(reply.begin(), reply.end()));
		send_buffer(i, buf);
	}

	unsigned curmsgid;
	ss(id) >> curmsgid;

	if (curmsgid <= peers[i].lastmsgid) {
		ignoredmsg++;
		return;
	}

	peers[i].lastmsgid = curmsgid;
	cout << i << " > " << command << ' ' << data << endl;
	
	if (command == "hello") {
		string str, type;
		unsigned v = 0;
		ss(data) >> str >> str >> type >> str >> v >> ws >> peers[i].altendpoint;
		if (v == version) {
			peers[i].connected = true;
			strand.post(boost::bind(&Network::handle_command, this, i, "newpeer", ""));
		} else {
			cout << "peer " << i << " incompatible version " << v << endl;
		}

		if (type == "server") {
			peers[i].connections = 0;
			peers.resize(1, Peer(udpendpoint()));
		} else if (server && type == "peer") {
			for (unsigned j = 0; j < peers.size(); j++) {
				peers[j].connections = 0;
				if (i != j && peers[j].connected) {
					peers[j].messages.push_back(ss(msgid++) << " holepunching " << peers[i].endpoint << ' ' << peers[i].altendpoint);
					peers[i].messages.push_back(ss(msgid++) << " holepunching " << peers[j].endpoint << ' ' << peers[j].altendpoint);
				}
			}
		} else if (!server && type == "peer") {
			peers.front().messages.push_back(ss(msgid++) << " peerconnected " << count_if(peers.begin(), peers.end(), boost::bind(&Peer::connected, _1)));
		}
	} else if (command == "holepunching") {
		peers.push_back(Peer(udpendpoint()));
		ss(data) >> peers.back().endpoint >> ws >> peers.back().altendpoint;
		peers.back().connections = 0;
		if (peers.size() > maxpeers)
			peers.erase(peers.begin() + 1);
	} else if (command == "peerconnected") {
		ss(data) >> peers[i].connections;
		if (find_if(peers.begin(), peers.end(), boost::bind(&Peer::connections, _1) != peers.size()) == peers.end())
			strand.post(boost::bind(&Network::handle_command, this, i, "peersconnected", (string)ss(peers.size())));
	} else {
		strand.post(boost::bind(&Network::handle_command, this, i, command, data));
	}
}


void Network::worker(size_t i) {
	try {
		io.run();
		boost::lock_guard<boost::timed_mutex> lock(netmutex);
		cout << "network thread " << i << " stopped" << endl;
	} catch (std::exception& e) {
		cout << "network failure: " << e.what() << endl;
	}
}


void Network::receiver(const errorcode& e, size_t n) {
	if (e == error::operation_aborted)
		return;
	boost::lock_guard<boost::timed_mutex> lock(netmutex);

	if (endpoint == localendpoint)
		return socket.async_receive_from(buffer(recvbuf), endpoint, boost::bind(&Network::receiver, this, _1, _2));


	vector<Peer>::iterator peer = find_if(peers.begin(), peers.end(), boost::bind(&Peer::endpoint, _1) == endpoint);
	if (!e && peer == peers.end() && (!server || n > 1)) {
		for (vector<Peer>::iterator it = peers.begin(); it != peers.end(); it++)
			peer = (it->weakport || it->idle > 2 * timerrate) && it->endpoint.address() == endpoint.address()? it: peer;
		if (peer != peers.end()) {
			if (!peer->weakport)
				cout << "warning: peer " << peer - peers.begin() << " changed port" << endl;
			peer->endpoint = endpoint;
			peer->weakport = true;
		}
	}
	if (!e && peer == peers.end() && !server) {
		peer = find_if(peers.begin(), peers.end(), boost::bind(&Peer::idle, _1) > 2 * timerrate);
		if (peer != peers.end()) {
			if (peer->altendpoint != endpoint)
				cout << "warning: peer " << peer - peers.begin() << " changed address" << endl;
			peer->endpoint = endpoint;
		}
	}
	if (!e && peer != peers.end())
		peer->idle = 0;

	if (!e && server && n == 1 && peer == peers.end() && peers.size() < maxpeers) {
		peers.push_back(Peer(endpoint));
		peer = peers.end() - 1;
		cout << "new peer " << peer - peers.begin() << ": " << endpoint << endl;
		peer->messages.push_back(ss(msgid++) << " hello " << endpoint << " from server version " << version);
	}
	if (!e && !server && n == 1 && peer != peers.end() && peer->connections == 0) {
		peer->connections = 1;
		peer->messages.push_back(ss(msgid++) << " hello " << endpoint << " from peer version " << version << ' ' << localendpoint);
	}

	if (e) {
		if (e != error::connection_reset && e != error::connection_refused)
			cout << "receive error: " << e.message() << endl;
	} else if (peer == peers.end()) {
		if (server && n == 1 && ignoredpeers[endpoint]++ % (30 * timerrate) == 0) {
			cout << "ignored peer: " << endpoint << endl;
			string reply = ss(msgid++) << " busy";
			boost::shared_ptr<ucharbuf> buf(new ucharbuf(reply.begin(), reply.end()));
			socket.async_send_to(buffer(*buf), endpoint, boost::bind(&Network::sender, this, buf, _1, _2));
		}
	} else if (n > 1 && (recvbuf[0] & 192) == 0) {
		process_message(peer - peers.begin(), string(recvbuf.begin(), recvbuf.begin() + n));
	} else if (n > 1 && (recvbuf[0] & 192) == 64) {
		list<ucharbuf>::iterator it = lower_bound(peer->fifo.begin(), peer->fifo.end(), recvbuf, ring_cmp);
		if (it == peer->fifo.end() || (it != peer->fifo.begin() && ring_cmp(recvbuf, *it)))
			peer->fifo.insert(it, ucharbuf(recvbuf.begin(), recvbuf.begin() + n));
		if (peer->fifo.size() > fifomax) {
			peer->fifofull++;
			while (peer->fifo.size() > 1)
				peer->fifo.pop_front();
		}
	} else if (n > 1 && (recvbuf[0] & 192) == 128) {
		list<ucharbuf>::iterator it = lower_bound(peer->buffer.begin(), peer->buffer.end(), recvbuf, ring_cmp);
		if (it == peer->buffer.end() || ring_cmp(recvbuf, *it))
			peer->buffer.insert(it, ucharbuf(recvbuf.begin(), recvbuf.begin() + n));
		while (peer->buffer.size() > 3 * fifomax)
			peer->buffer.pop_front();
	} else if (n > 1 && (recvbuf[0] & 192) == 192 && server) {
		peer->relayed = true;
	} else if (n > 1 && (recvbuf[0] & 192) == 192 && !server && peers.size() > 1) {
		peers[1].relayed = true;
		recvbuf[0] = recvbuf[n - 1];
		endpoint = peers[1].endpoint;
		return io.post(boost::bind(&Network::receiver, this, e, n - 1));
	}


	if (!e && peer != peers.end() && server && n > 1 && (recvbuf[0] & 192) != 0 && peer->relayed && peers.size() > 1) {
		boost::shared_ptr<ucharbuf> buf(new ucharbuf(recvbuf.begin(), recvbuf.begin() + n));
		if ((recvbuf[0] & 192) != 192)
			buf->push_back(buf->at(0));
		buf->at(0) = 192;
		unsigned p = peer == peers.begin()? 1: 0;
		if ((buf->back() & 192) != 128 || peers[p].bucket >= buf->size()) {
			peers[p].bucket -= min(peers[p].bucket, (unsigned)buf->size());
			socket.async_send_to(buffer(*buf), peers[p].endpoint, boost::bind(&Network::sender, this, buf, _1, _2));
		}
	}


	socket.async_receive_from(buffer(recvbuf), endpoint, boost::bind(&Network::receiver, this, _1, _2));
}


void Network::sender(boost::shared_ptr<ucharbuf>, const errorcode& e, size_t) {
	if (e)
		cout << "send error: " << e.message() << endl;
}


void Network::deadline(const errorcode& e) {
	if (e == error::operation_aborted)
		return;
	if (e)
		throw runtime_error(e.message());

	boost::lock_guard<boost::timed_mutex> lock(netmutex);

	for (vector<Peer>::iterator peer = peers.begin(); peer != peers.end(); peer++) {
		if (peer->idle++ > 8 * timerrate) {
			if (!server && peer != peers.begin() && !peer->connected && !peer->relayed) {
				peer->relayed = true;
				peer->idle = 0;
			} else if (server || peer != peers.begin()) {
				cout << "peer " << peer - peers.begin() << " timed out" << endl;
				peer = peers.erase(peer);
				if (peer == peers.end())
					break;
			} else {
				cout << "no response from server" << endl;
				*peer = Peer(peer->endpoint);
				peers.resize(1, Peer(udpendpoint()));
			}
		}

		boost::shared_ptr<ucharbuf> beacon(new ucharbuf(1, 0));
		if (!peer->connected || peer->bucket >= bandwidth)
			send_buffer(peer - peers.begin(), beacon);
		if (!server && peer != peers.begin() && !peer->connected && peer->idle > 4 * timerrate) {
			for (unsigned short p = 1; p < 100; p++)
				socket.async_send_to(buffer(*beacon, 1), udpendpoint(peer->endpoint.address(), peer->endpoint.port() + p), boost::bind(&Network::sender, this, beacon, _1, _2));
			if (peer->altendpoint != udpendpoint())
				socket.async_send_to(buffer(*beacon, 1), peer->altendpoint, boost::bind(&Network::sender, this, beacon, _1, _2));
		}

		if (peer->messages.size() > 0) {
			boost::shared_ptr<ucharbuf> buf(new ucharbuf(peer->messages[0].begin(), peer->messages[0].end()));
			send_buffer(peer - peers.begin(), buf);
		}
		
		peer->bucket += max(bandwidth / peers.size() - minbw, (size_t)500) / timerrate;
		maximal(peer->bucket, bandwidth);
	}
	
	timer.expires_at(timer.expires_at() + boost::posix_time::milliseconds(1000 / timerrate));
	timer.async_wait(boost::bind(&Network::deadline, this, _1));
}
