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
	bandwidth = 0;
	mutexbusy = 0;
	ignoredmsg = 0;
	recvbuf.resize(recvsize);
}


Network::~Network(void) {
	try {
		lock_guard<timed_mutex> lock(netmutex);
		timer.cancel();
		socket.close();
		if (iothread.joinable())
			iothread.join();
	} catch (...) {}
}


void Network::connect(const string& address, unsigned short port, unsigned bw, handler h) {
	lock_guard<timed_mutex> lock(netmutex);

	if (iothread.joinable()) {
		timer.cancel();
		socket.close();
		iothread.join();
		peers.clear();
		socket.open(ip::udp::v4());
		endpoint = udpendpoint();
		io.reset();
	}

	bandwidth = bw;
	cmdfunc = h;

	if (address.empty()) {
		server = true;
		socket.bind(udpendpoint(ip::udp::v4(), port));
		
	} else {
		server = false;
		endpoint = udpendpoint(ip::address_v4::from_string(address), port);
		peers.push_back(Peer(endpoint));
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

	for (unsigned i = 0; i < peers.size(); i++) {
		if (send.size() == fifosize) {
			if (peers[i].fifo.size() > 0) {
				recv.push_back(peers[i].fifo.front());
				peers[i].fifo.pop_front();
			} else
				peers[i].fifoempty++;
			socket.async_send_to(buffer(*buf), peers[i].endpoint, bind(&Network::sender, this, buf, _1, _2));
		} else {
			recv.push_back(peers[i].buffer);
			if (peers[i].bucket >= send.size()) {
				socket.async_send_to(buffer(*buf), peers[i].endpoint, bind(&Network::sender, this, buf, _1, _2));
				peers[i].bucket -= send.size();
			}
		}
	}
}


void Network::command(unsigned i, const string& command, const string& data) {
	lock_guard<timed_mutex> lock(netmutex);
		
	if (i >= peers.size())
		return;
		
	peers[i].messages.push_back(ss(msgid++) << ' ' << command << ' ' << data);
	
	if (peers[i].messages.back().length() >= fifosize) {
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
	cout << "known peers: " << peers.size() << ", network mutex busy: " << mutexbusy << ", messages ignored: " << ignoredmsg << endl;
	for (unsigned i = 0; i < peers.size(); i++)
		cout << "peer " << i << " fifo empty/full: " << peers[i].fifoempty << '/' << peers[i].fifofull << endl;
}


void Network::processmessage(unsigned i, const string& message) {
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

	if ((command != "hello" && curmsgid <= peers[i].lastmsgid) || (command == "hello" && peers[i].lastmsgid == curmsgid)) {
		ignoredmsg++;
		return;
	}

	peers[i].lastmsgid = curmsgid;
	cout << i << " > " << command << ' ' << data << endl;
	
	if (command == "hello") {
		if (data.find("from server") != string::npos) {
			peers.front().messages.push_back(ss(msgid++) << " hello " << peers.front().endpoint << " from client");
			peers.resize(1, Peer(udpendpoint()));
		} else if (data.find("from peer") != string::npos) {
			peers.front().connections = peers[i].connections = 1;
			peers.front().messages.push_back(ss(msgid++) << " peerconnected " << count_if(peers.begin(), peers.end(), bind(&Peer::connections, _1)));
		}
	} else if (command == "removepeer") {
		udpendpoint ep;
		ss(data) >> ep;
		vector<Peer>::iterator peer = find_if(peers.begin(), peers.end(), bind(&Peer::endpoint, _1) == ep);
		if (peer != peers.end())
			peers.erase(peer);
	} else if (command == "holepunching") {
		udpendpoint ep;
		ss(data) >> ep;
		peers.push_back(Peer(ep));
		peers.back().messages.push_back(ss(msgid++) << " hello " << ep << " from peer");
	} else if (command == "peerconnected") {
		ss(data) >> peers[i].connections;
		if (find_if(peers.begin(), peers.end(), bind(&Peer::connections, _1) != peers.size()) == peers.end())
			io.post(bind(cmdfunc, i, "peersconnected", (string)ss(peers.size())));
	} else
		io.post(bind(cmdfunc, i, command, data));
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
	if (e)
		return;

	lock_guard<timed_mutex> lock(netmutex);
	vector<Peer>::iterator peer = find_if(peers.begin(), peers.end(), bind(&Peer::endpoint, _1) == endpoint);

	if (server) {
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
			peer->lasttime = (double)clock() / CLOCKS_PER_SEC;
	}

	if (peer == peers.end()) {
		//ignore unknown peer
	} else if (n < fifosize) {
		processmessage(peer - peers.begin(), string(recvbuf.begin(), recvbuf.begin() + n));
	} else if (n == fifosize) {
		peer->fifo.push_back(ucharbuf(recvbuf.begin(), recvbuf.begin() + n));
		if (peer->fifo.size() >= fifomax) {
			peer->fifofull++;
			while (peer->fifo.size() > 1)
				peer->fifo.pop_front();
		}
	} else {
		peer->buffer.assign(recvbuf.begin(), recvbuf.begin() + n);
	}

	socket.async_receive_from(buffer(recvbuf), endpoint, bind(&Network::receiver, this, _1, _2));
}


void Network::sender(shared_ptr<ucharbuf>, const errorcode&, size_t) {
}


void Network::deadline(const errorcode& e) {
	if (e)
		return;

	lock_guard<timed_mutex> lock(netmutex);

	if (server) {
		vector<Peer>::iterator peer = find_if(peers.begin(), peers.end(), bind(&Peer::lasttime, _1) < (double)clock() / CLOCKS_PER_SEC - 5.);
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
		
		peers[i].bucket += bandwidth / peers.size() / timerrate;
		if (peers[i].bucket > bandwidth / peers.size())
			peers[i].bucket = bandwidth / peers.size();
	}
	
	timer.expires_at(timer.expires_at() + posix_time::milliseconds(1000 / timerrate));
	timer.async_wait(bind(&Network::deadline, this, _1));
}
