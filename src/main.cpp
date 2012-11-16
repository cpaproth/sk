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
#include "Network.h"
#include "Audio.h"
#include "Video.h"

#include <fltk/ask.h>
#include <iostream>
#include "Convenience.h"


using namespace CPLib;
using namespace SK;


class UIsbuf : public streambuf {
	UserInterface&	ui;
	streambuf*	oldsbuf;
	
	int overflow(int c) {
		char buf[] = {(char)c, 0};
		UILock lock;
		ui.log->append(buf);
		if (buf[0] == '\n') {
			ui.log->scroll(INT_MAX, 0);
			fltk::awake();
		}
		return c;
	}
public:
	UIsbuf(UserInterface& ui) : ui(ui) {
		oldsbuf = cout.rdbuf(this);
	}
	~UIsbuf(void) {
		cout.rdbuf(oldsbuf);
	}
};


class Program {
	UserInterface&	ui;
	Network&	network;
	Video&		video;
	Audio&		audio;

	void handle_command(unsigned, const string&, const string&);
public:
	Program(UserInterface& ui, Network& nw, Video& v, Audio& a) : ui(ui), network(nw), video(v), audio(a) {}

	void start_network(void);
};


void Program::start_network(void) {
	UIUnlock lock;
	network.start(ui.address->value(), (unsigned short)ui.port->value(), (unsigned)ui.bandwidth->value(), boost::bind(&Program::handle_command, this, _1, _2, _3));
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
		UserInterface	ui;
		UIsbuf		sbuf(ui);

		cout << "Skat-Konferenz Copyright (C) 2012 Carsten Paproth." << endl;
		cout << "This program is free software and comes with ABSOLUTELY NO WARRANTY." << endl;
		cout << "Licensed under the terms of GPLv3, see <http://www.gnu.org/licenses/>." << endl << endl;

		Network		network;
		Video		video(ui, network);
		Audio		audio(network);
		Program		program(ui, network, video, audio);
		UILock		lock;


		audio.restart();

		ui.f["audio restart"] = boost::bind(&Audio::restart, &audio);
		ui.f["audio toggle"] = boost::bind(&Audio::toggle_playmic, &audio);
		ui.f["network stats"] = boost::bind(&Network::stats, &network);
		ui.f["network start"] = boost::bind(&Program::start_network, &program);

		try {
			if (ui.autostart->value())
				program.start_network();
		} catch (exception& e) {
			cout << "autostarting network failed: " << e.what() << endl;
		}

		while(true)
			try {
				return fltk::run();
			} catch (exception& e) {
				cout << "error: " << e.what() << endl;
			}
	} catch (exception& e) {
		fltk::alert(ss("initialization error: ") << e.what() | c_str);
	}

	return 1;
}
