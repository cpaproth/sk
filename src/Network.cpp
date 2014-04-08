/*Copyright (C) 2012-2014 Carsten Paproth

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
using namespace boost;
using namespace boost::asio;


static istream& operator>>(istream& s, ip::udp::endpoint& ep) {
	string a;
	unsigned short p;
	getline(s, a, ':');
	s >> p;
	ep = ip::udp::endpoint(ip::address_v4::from_string(a), p);
	return s;
}


static bool fifo_cmp(const vector<unsigned char>& l, const vector<unsigned char>& r) {
	return abs((int)(l[0] & 63) - (int)(r[0] & 63)) < 32? (l[0] & 63) < (r[0] & 63): (r[0] & 63) < (l[0] & 63);
}


Network::Network(void) : socket(io, ip::udp::v4()), timer(io) {
	msgid = 1;
	bandwidth = maxpeers * minbw;
	mutexbusy = 0;
	ignoredmsg = 0;
	recvbuf.resize(recvsize);
}


Network::~Network(void) {
	try {
		netmutex.lock();
		timer.cancel();
		socket.close();
		netmutex.unlock();
		if (iothread.joinable())
			iothread.join();
	} catch (...) {}
}


void Network::add_handler(handler h) {
	lock_guard<mutex> lock(hdlmutex);
	handlers.push_back(h);
}


void Network::remove_handler(void) {
	lock_guard<mutex> lock(hdlmutex);
	handlers.clear();
}


void Network::connect(const string& address, unsigned short port, unsigned bw) {
	if (!hdlmutex.try_lock())
		return;
	lock_guard<mutex> hlock(hdlmutex, adopt_lock);
	if (netmutex.try_lock()) {
		lock_guard<timed_mutex> nlock(netmutex, adopt_lock);
		timer.cancel();
		socket.close();
	} else {
		return;
	}

	if (iothread.joinable())
		iothread.join();

	lock_guard<timed_mutex> nlock(netmutex);
	peers.clear();
	ignoredpeers.clear();
	socket.open(ip::udp::v4());
	endpoint = udpendpoint();
	io.reset();

	minimal(bandwidth = bw, (unsigned)(maxpeers * minbw));

	if (address.empty()) {
		server = true;
		socket.bind(udpendpoint(ip::udp::v4(), port));
	} else {
		server = false;
		
		ip::udp::resolver resolver(io);
		ip::udp::resolver::query query(ip::udp::v4(), address, ss(port));
		endpoint = *resolver.resolve(query);
		peers.push_back(Peer(endpoint));

		socket.send_to(buffer(recvbuf, 1), endpoint);
	}

	timer.expires_from_now(posix_time::milliseconds(1000 / timerrate));
	timer.async_wait(bind(&Network::deadline, this, _1));
	socket.async_receive_from(buffer(recvbuf), endpoint, bind(&Network::receiver, this, _1, _2));
	iothread = thread(bind(&Network::worker, this));
}


void Network::broadcast(const ucharbuf& send, vector<ucharbuf>& recv, unsigned latency) {
	recv.clear();

	if (!netmutex.timed_lock(posix_time::milliseconds(latency))) {
		mutexbusy++;
		return;
	}
	lock_guard<timed_mutex> lock(netmutex, adopt_lock);

	shared_ptr<ucharbuf> buf(new ucharbuf(send));

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
			socket.async_send_to(buffer(*buf), peers[i].endpoint, bind(&Network::sender, this, buf, _1, _2));
		} else if ((send[0] & 192) == 128) {
			recv.push_back(peers[i].buffer);
			peers[i].buffer.clear();
			if (peers[i].bucket >= buf->size()) {
				socket.async_send_to(buffer(*buf), peers[i].endpoint, bind(&Network::sender, this, buf, _1, _2));
				peers[i].bucket -= buf->size();
			}
		}
	}
}


void Network::command(unsigned i, const string& command, const string& data) {
	lock_guard<timed_mutex> lock(netmutex);
		
	if (i >= peers.size() || !peers[i].connected)
		return;
		
	peers[i].messages.push_back(ss(msgid++) << ' ' << command << ' ' << data);
	
	if (peers[i].messages.size() == 1) {
		shared_ptr<ucharbuf> buf(new ucharbuf(peers[i].messages[0].begin(), peers[i].messages[0].end()));
		socket.async_send_to(buffer(*buf), peers[i].endpoint, bind(&Network::sender, this, buf, _1, _2));
	}
}


void Network::stats(void) {
	lock_guard<timed_mutex> lock(netmutex);
	cout << "known peers: " << peers.size() << ", local port: " << socket.local_endpoint().port() << ", mutex busy: " << mutexbusy << ", messages ignored: " << ignoredmsg << endl;
	for (unsigned i = 0; i < peers.size(); i++)
		cout << "peer " << i << " (" << peers[i].endpoint << ") fifo empty/full/size: " << peers[i].fifoempty << '/' << peers[i].fifofull << '/' << peers[i].fifo.size() << endl;
}


void Network::handle_command(unsigned i, const string& command, const string& data) {
	if (!hdlmutex.try_lock())
		return;
	lock_guard<mutex> lock(hdlmutex, adopt_lock);

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
		shared_ptr<ucharbuf> buf(new ucharbuf(reply.begin(), reply.end()));
		socket.async_send_to(buffer(*buf), endpoint, bind(&Network::sender, this, buf, _1, _2));
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
		peers[i].connected = true;
		if (data.find("from server") != string::npos) {
			peers.resize(1, Peer(udpendpoint()));
		} else if (server && data.find("from peer") != string::npos) {
			for (unsigned j = 0; j < peers.size(); j++) {
				peers[j].connections = 0;
				if (i != j && peers[j].connected) {
					peers[j].messages.push_back(ss(msgid++) << " holepunching " << peers[i].endpoint);
					peers[i].messages.push_back(ss(msgid++) << " holepunching " << peers[j].endpoint);
				}
			}
		} else if (!server && data.find("from peer") != string::npos) {
			peers.front().messages.push_back(ss(msgid++) << " peerconnected " << count_if(peers.begin(), peers.end(), bind(&Peer::connected, _1)));
		}
	} else if (command == "holepunching") {
		udpendpoint ep;
		ss(data) >> ep;
		peers.push_back(Peer(ep));
		if (peers.size() > maxpeers)
			peers.erase(peers.begin() + 1);
		socket.send_to(buffer(recvbuf, 1), ep);
	} else if (command == "peerconnected") {
		ss(data) >> peers[i].connections;
		if (find_if(peers.begin(), peers.end(), bind(&Peer::connections, _1) != peers.size()) == peers.end())
			io.post(bind(&Network::handle_command, this, i, "peersconnected", (string)ss(peers.size())));
	} else {
		io.post(bind(&Network::handle_command, this, i, command, data));
	}
}


void Network::worker(void) {
	try {
		cout << "network connected as " << (server? "server": "client") << endl;
		io.run();
		cout << "network disconnected" << endl;
	} catch (std::exception& e) {
		cout << "network failure: " << e.what() << endl;
	}
}


void Network::receiver(const errorcode& e, size_t n) {
	if (e == error::operation_aborted)
		return;
	lock_guard<timed_mutex> lock(netmutex);


	vector<Peer>::iterator peer = find_if(peers.begin(), peers.end(), bind(&Peer::endpoint, _1) == endpoint);
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
		peer = find_if(peers.begin(), peers.end(), bind(&Peer::idle, _1) > 2 * timerrate);
		if (peer != peers.end()) {
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
		peer->messages.push_back(ss(msgid++) << " hello " << endpoint << " from server");
	}
	if (!e && !server && n == 1 && peer != peers.end() && peer->connections == 0) {
		peer->connections = 1;
		peer->messages.push_back(ss(msgid++) << " hello " << endpoint << " from peer");
	}


	if (e) {
		cout << "receive error: " << e.message() << endl;
	} else if (peer == peers.end()) {
		if (server && n == 1 && ignoredpeers[endpoint]++ % (30 * timerrate) == 0) {
			cout << "ignored peer: " << endpoint << endl;
			socket.send_to(buffer((string)(ss(msgid++) << " busy")), endpoint);
		}
	} else if (n > 1 && (recvbuf[0] & 192) == 0) {
		process_message(peer - peers.begin(), string(recvbuf.begin(), recvbuf.begin() + n));
	} else if (n > 1 && (recvbuf[0] & 192) == 64) {
		list<ucharbuf>::iterator it = lower_bound(peer->fifo.begin(), peer->fifo.end(), recvbuf, fifo_cmp);
		if (peer->fifo.size() == 0 || it != peer->fifo.begin())
			peer->fifo.insert(it, ucharbuf(recvbuf.begin(), recvbuf.begin() + n));
		if (peer->fifo.size() > fifomax) {
			peer->fifofull++;
			while (peer->fifo.size() > 1)
				peer->fifo.pop_front();
		}
	} else if (n > 1 && (recvbuf[0] & 192) == 128) {
		peer->buffer.assign(recvbuf.begin(), recvbuf.begin() + n);
	}

	socket.async_receive_from(buffer(recvbuf), endpoint, bind(&Network::receiver, this, _1, _2));
}


void Network::sender(shared_ptr<ucharbuf>, const errorcode& e, size_t) {
	if (e)
		cout << "send error: " << e.message() << endl;
}


void Network::deadline(const errorcode& e) {
	if (e)
		return;

	lock_guard<timed_mutex> lock(netmutex);


	//static unsigned count = 0;
	//if (!server && ++count % 1 == 0 && (bandwidth & 1) != 0) {
	//	socket.close();
	//	socket.open(ip::udp::v4());
	//	socket.async_receive_from(buffer(recvbuf), endpoint, bind(&Network::receiver, this, _1, _2));
	//}


	for (vector<Peer>::iterator peer = peers.begin(); peer != peers.end(); peer++) {
		if (peer->idle++ > 6 * timerrate) {
			if (server || peer != peers.begin()) {
				cout << "peer " << peer - peers.begin() << " timed out" << endl;
				peer = peers.erase(peer);
				if (peer == peers.end())
					break;
			} else {
				*peer = Peer(peer->endpoint);
				peers.resize(1, Peer(udpendpoint()));
			}
		}

		if (!peer->connected)
			socket.send_to(buffer(recvbuf, 1), peer->endpoint);

		if (peer->messages.size() > 0) {
			shared_ptr<ucharbuf> buf(new ucharbuf(peer->messages[0].begin(), peer->messages[0].end()));
			socket.async_send_to(buffer(*buf), peer->endpoint, bind(&Network::sender, this, buf, _1, _2));
		}
		
		peer->bucket += max(bandwidth / peers.size() - minbw, (size_t)500) / timerrate;
		maximal(peer->bucket, bandwidth);
	}
	
	timer.expires_at(timer.expires_at() + posix_time::milliseconds(1000 / timerrate));
	timer.async_wait(bind(&Network::deadline, this, _1));
}
