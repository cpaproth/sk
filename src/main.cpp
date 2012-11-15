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


#include "ui.h"

#include "Convenience.h"

#include "Network.h"
#include "Audio.h"
#include "Video.h"

#include <iostream>

using namespace CPLib;
using namespace SK;


class UIsbuf : public streambuf {
	UserInterface&	ui;
	streambuf*	oldsbuf;
	
	int overflow(int);
public:
	UIsbuf(UserInterface&);
	~UIsbuf(void);
};


UIsbuf::UIsbuf(UserInterface& ui) : ui(ui) {
	UILock lock;
	oldsbuf = cout.rdbuf(this);
}
UIsbuf::~UIsbuf(void) {
	UILock lock;
	cout.rdbuf(oldsbuf);
}
int UIsbuf::overflow(int c) {
	char buf[] = {(char)c, 0};
	UILock lock;
	ui.log->append(buf);
	if (buf[0] == '\n') {
		ui.log->scroll(INT_MAX, 0);
		fltk::awake();
	}
	return c;
}




class Program {
	UserInterface	ui;
	UIsbuf		sbuf;
	Network		network;
	Video		video;
	Audio		audio;

	void start_network(void);
	void handle_command(unsigned, const string&, const string&);
public:
	Program(void);
};


Program::Program(void) : sbuf(ui), video(ui, network), audio(network) {
	cout << "Skat-Konferenz Copyright (C) 2012 Carsten Paproth." << endl;
	cout << "This program is free software and comes with ABSOLUTELY NO WARRANTY." << endl;
	cout << "Licensed under the terms of GPLv3, see <http://www.gnu.org/licenses/>." << endl << endl;

	ui.f["audio restart"] = boost::bind(&Audio::restart, &audio);
	ui.f["audio toggle"] = boost::bind(&Audio::toggle_playmic, &audio);
	ui.f["network stats"] = boost::bind(&Network::stats, &network);
	ui.f["network start"] = boost::bind(&Program::start_network, this);

	if (ui.autostart->value())
		start_network();
}


void Program::start_network(void) {
	fltk::unlock();
	network.start(ui.address->value(), (unsigned short)ui.port->value(), (unsigned)ui.bandwidth->value(), boost::bind(&Program::handle_command, this, _1, _2, _3));
	fltk::lock();
}


void Program::handle_command(unsigned i, const string& command, const string& data) {
	if (command == "peersconnected") {
		cout << "juhu " << data << endl;
		if (data == "2")
			network.command(0, "roundtrip", "0");
	} else if (command == "roundtrip") {
		unsigned trip;
		ss(data) >> trip;
		if (trip < 30)
			network.command((i+1)%2, "roundtrip", ss(++trip));
	} else
		cout << "unknown command: " << command << endl;
}


int main(void) {
	try {
		Program program;

		while(true)
			try {
				return fltk::run();
			} catch (exception& e) {
				cout << "error: " << e.what() << endl;
			}
	} catch (exception& e) {
		cout << "error: " << e.what() << endl;
	}

	return 1;

}
