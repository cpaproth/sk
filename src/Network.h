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

#ifndef SK_NETWORK_H
#define SK_NETWORK_H

#include <boost/asio/ip/udp.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/thread/thread.hpp>
#include <boost/function/function3.hpp>

#include <vector>
#include <deque>
#include <string>


namespace SK {

using namespace std;


class Network {
	static const size_t maxpeers = 2;
	static const size_t fifosize = 160;
	static const size_t recvsize = 20000;
	static const unsigned short serverport = 34588;
	static const unsigned timerrate = 4;

	
	typedef vector<unsigned char> ucharbuf;
	typedef boost::function<void(unsigned, const string&, const string&)> handler;
	typedef boost::asio::ip::udp::endpoint udpendpoint;
	typedef boost::system::error_code errorcode;


	struct Peer {
		udpendpoint	endpoint;
		deque<ucharbuf>	fifo;
		ucharbuf	buffer;
		deque<string>	messages;
		double		lasttime;
		unsigned	lastmsgid;
		unsigned	bucket;
		unsigned	connections;
		Peer(const udpendpoint& ep) : endpoint(ep), lastmsgid(-1), bucket(0), connections(0) {}
	};
	
	bool				server;
	unsigned			msgid;
	unsigned			bandwidth;
	handler				cmdfunc;
	boost::asio::io_service		io;
	boost::asio::ip::udp::socket	socket;
	udpendpoint			endpoint;
	boost::asio::deadline_timer	timer;
	ucharbuf			recvbuf;
	vector<Peer>			peers;
	boost::thread			iothread;
	boost::timed_mutex		netmutex;


	void processmessage(unsigned, const string&);

	void worker(void);
	void receiver(const errorcode&, size_t);
	void sender(boost::shared_ptr<ucharbuf>, const errorcode&, size_t);
	void deadline(const errorcode&);

	Network(const Network&);
	void operator=(const Network&);
public:
	Network(void);
	~Network(void);

	void start(const string&, unsigned, handler);
	void broadcast(const ucharbuf&, vector<ucharbuf>&, unsigned);
	void command(unsigned, const string&, const string&);
	
	
	unsigned toobig, toosmall, nolock;

};


}


#endif
