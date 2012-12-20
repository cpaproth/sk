/*Copyright (C) 2012 Carsten Paproth

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
#include <ctime>
#include "Convenience.h"


using namespace SK;
using namespace CPLib;
using namespace boost;
using namespace boost::asio;


istream& operator>>(istream& s, ip::udp::endpoint& ep) {
	string a;
	unsigned short p;
	getline(s, a, ':');
	s >> p;
	ep = ip::udp::endpoint(ip::address_v4::from_string(a), p);
	return s;
}


Network::Network(void) : socket(io, ip::udp::v4()), timer(io) {
	msgid = 0;
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
	erase_header(*buf);

	for (unsigned i = 0; i < peers.size(); i++) {
		if (send.size() == fifosize) {
			if (peers[i].fifo.size() > 0) {
				recv.push_back(peers[i].fifo.front());
				peers[i].fifo.pop_front();
			} else {
				peers[i].fifoempty++;
			}
			for (size_t s = 0; s < fifosize; s += splitsize)
				socket.async_send_to(buffer(&(*buf)[s], splitsize), peers[i].endpoint, bind(&Network::sender, this, buf, _1, _2));
		} else {
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
		
	if (i >= peers.size())
		return;
		
	peers[i].messages.push_back(ss(msgid++) << ' ' << command << ' ' << data);
	
	if (peers[i].messages.back().length() >= splitsize) {
		peers[i].messages.pop_back();
		return;
	}
	
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


void Network::insert_header(unsigned i) {
	ucharbuf& b = peers[i].buffer;
	if (b.size() > 1 && b[0] == 255 && b[1] == 216 && peers[i].header.size() == 0) {
		for (unsigned j = 0; j + 1 < b.size() && !(b[j] == 255 && b[j + 1] == 218); j++)
			peers[i].header.push_back(b[j]);
		peers[i].messages.push_back(ss(msgid++) << " knownheader " << peers[i].header.size());
	} else if (b.size() > 1 && b[0] == 255 && b[1] == 218) {
		b.insert(b.begin(), peers[i].header.begin(), peers[i].header.end());
	}
}


void Network::erase_header(ucharbuf& b) {
	if (b.size() > fifosize && b[0] == 255 && b[1] == 216 && find_if(peers.begin(), peers.end(), bind(&Peer::known, _1) == false) == peers.end()) {
		unsigned i;
		for (i = 0; i + 1 < b.size() && !(b[i] == 255 && b[i + 1] == 218); i++);
		b.erase(b.begin(), b.begin() + i);
	}
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

	if (command == "reply") {
		if (peers[i].messages.size() > 0 && peers[i].messages[0].compare(0, id.length() + 1, id + ' ') == 0) {
			cout << i << " < " << string(ss(peers[i].messages[0]) >> id >> ws) << endl;
			peers[i].messages.pop_front();
		} else
			ignoredmsg++;
		return;
	} else {
		string reply = id + " reply";
		shared_ptr<ucharbuf> buf(new ucharbuf(reply.begin(), reply.end()));
		socket.async_send_to(buffer(*buf), endpoint, bind(&Network::sender, this, buf, _1, _2));
	}

	unsigned curmsgid;
	ss(id) >> curmsgid;

	if ((command != "hello" && curmsgid <= peers[i].lastmsgid) || (command == "hello" && peers[i].lastmsgid == curmsgid && curmsgid != 0)) {
		ignoredmsg++;
		return;
	}

	peers[i].lastmsgid = curmsgid;
	cout << i << " > " << command << ' ' << data << endl;
	
	if (command == "hello") {
		if (data.find("from server") != string::npos) {
			peers.front().messages.push_back(ss((msgid = 1)++) << " hello " << peers.front().endpoint << " from client");
			peers.front().header.clear();
			peers.front().known = false;
			peers.resize(1, Peer(udpendpoint()));
		} else if (data.find("from peer") != string::npos) {
			peers.front().connections = peers[i].connections = 1;
			peers.front().messages.push_back(ss(msgid++) << " peerconnected " << count_if(peers.begin(), peers.end(), bind(&Peer::connections, _1)));
		}
	} else if (command == "knownheader") {
		peers[i].known = true;
	} else if (command == "removepeer" || command == "holepunching") {
		udpendpoint ep;
		ss(data) >> ep;
		vector<Peer>::iterator peer = find_if(peers.begin(), peers.end(), bind(&Peer::endpoint, _1) == ep);
		if (peer != peers.end())
			peers.erase(peer);
			
		if (command == "holepunching" && peers.size() < maxpeers) {
			peers.push_back(Peer(ep));
			peers.back().messages.push_back(ss(msgid++) << " hello " << ep << " from peer");

			socket.send_to(buffer(recvbuf, 1), ep);
		}
	} else if (command == "peerconnected") {
		ss(data) >> peers[i].connections;
		if (find_if(peers.begin(), peers.end(), bind(&Peer::connections, _1) != peers.size()) == peers.end())
			io.post(bind(&Network::handle_command, this, i, "peersconnected", (string)ss(peers.size())));
	} else
		io.post(bind(&Network::handle_command, this, i, command, data));
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


static int cmp_pos(unsigned char a, unsigned char b) {
	a &= 63;
	b &= 63;
	if (a < b && b - a < 32)
		return -1;
	if (a < b)
		return 1;
	if (a > b && a - b < 32)
		return 1;
	if (a > b)
		return -1;
	return 0;
}


void Network::receiver(const errorcode& e, size_t n) {
	if (e == error::operation_aborted)
		return;

	lock_guard<timed_mutex> lock(netmutex);
	vector<Peer>::iterator peer = find_if(peers.begin(), peers.end(), bind(&Peer::endpoint, _1) == endpoint);

	if (!e && server) {
		if (peer == peers.end() && peers.size() < maxpeers) {
			peers.push_back(Peer(endpoint));
			peer = peers.end() - 1;
			cout << "new peer " << peer - peers.begin() << ": " << endpoint << endl;
			peer->messages.push_back(ss(msgid++) << " hello " << endpoint << " from server");
			for (vector<Peer>::iterator it = peers.begin(); it != peers.end(); it++)
				if (it != peer) {
					it->messages.push_back(ss(msgid++) << " holepunching " << peer->endpoint);
					peer->messages.push_back(ss(msgid++) << " holepunching " << it->endpoint);
				}
		}
		if (peer != peers.end())
			peer->lasttime = (size_t)time(0);
	}

	if (e || n <= 1 || peer == peers.end()) {
		if (e)
			cout << "receive error: " << e.message() << endl;
	} else if (n < splitsize) {
		process_message(peer - peers.begin(), string(recvbuf.begin(), recvbuf.begin() + n));
	} else if (n == splitsize) {
		if (peer->fifo.size() == 0 || cmp_pos(peer->fifo.back().front(), recvbuf[0]) < 0) {
			peer->fifo.push_back(ucharbuf(recvbuf.begin(), recvbuf.begin() + n));
		} else if (cmp_pos(peer->fifo.front().front(), recvbuf[0]) <= 0) {
			for (list<ucharbuf>::iterator it = peer->fifo.begin(); it != peer->fifo.end();) {
				if (cmp_pos(it->front(), recvbuf[0]) == 0) {
					it->resize(it->size() + n);
					copy(recvbuf.begin(), recvbuf.begin() + n, it->begin() + it->size() - n);
					break;
				} else if (cmp_pos((++it)->front(), recvbuf[0]) > 0) {
					peer->fifo.insert(it, ucharbuf(recvbuf.begin(), recvbuf.begin() + n));
					break;
				}
			}
		}

		if (peer->fifo.size() > fifomax) {
			peer->fifofull++;
			while (peer->fifo.size() > 1)
				peer->fifo.pop_front();
		}
	} else {
		peer->buffer.assign(recvbuf.begin(), recvbuf.begin() + n);
		insert_header(peer - peers.begin());
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

	if (server) {
		vector<Peer>::iterator peer = find_if(peers.begin(), peers.end(), bind(&Peer::lasttime, _1) < time(0) - 5);
		if (peer != peers.end()) {
			cout << "peer " << peer - peers.begin() << " timed out" << endl;
			for (vector<Peer>::iterator it = peers.begin(); it != peers.end(); it++) {
				it->connections = 0;
				it->messages.push_back(ss(msgid++) << " removepeer " << peer->endpoint);
			}
			peers.erase(peer);
		}
	}

	for (unsigned i = 0; i < peers.size(); i++) {
		if (peers[i].messages.size() > 0) {
			shared_ptr<ucharbuf> buf(new ucharbuf(peers[i].messages[0].begin(), peers[i].messages[0].end()));
			socket.async_send_to(buffer(*buf), peers[i].endpoint, bind(&Network::sender, this, buf, _1, _2));
		}
		
		peers[i].bucket += max(bandwidth / peers.size() - minbw, (size_t)500) / timerrate;
		maximal(peers[i].bucket, bandwidth);
	}
	
	timer.expires_at(timer.expires_at() + posix_time::milliseconds(1000 / timerrate));
	timer.async_wait(bind(&Network::deadline, this, _1));
}
