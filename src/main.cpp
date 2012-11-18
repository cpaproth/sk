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


using namespace boost;
using namespace CPLib;
using namespace SK;


class UISbuf : public streambuf {
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
	UISbuf(UserInterface& ui) : ui(ui) {
		oldsbuf = cout.rdbuf(this);
	}
	~UISbuf(void) {
		cout.rdbuf(oldsbuf);
	}
};


void handle_command(Network& network, Video& video, unsigned i, const string& command, const string& data) {
	if (command == "peersconnected") {
		if (data == "2") {
			cout << "2 peers connected, the game can start!" << endl;
			network.command(0, "seat", "left");
			network.command(1, "seat", "right");
			video.send_name();
		}
	} else if (!video.handle_command(i, command, data))
		cout << "unknown command: " << command << endl;
}


void start_network(UserInterface& ui, Network& network, Video& video) {
	UIUnlock lock;
	network.start(ui.address->value(), (unsigned short)ui.port->value(), (unsigned)ui.bandwidth->value(),
		bind(&handle_command, ref(network), ref(video), _1, _2, _3));
}


int main(void) {
	try {
		UserInterface	ui;
		UISbuf		sbuf(ui);

		cout << "Skat-Konferenz Copyright (C) 2012 Carsten Paproth." << endl;
		cout << "This program is free software and comes with ABSOLUTELY NO WARRANTY." << endl;
		cout << "Licensed under the terms of GPLv3, see <http://www.gnu.org/licenses/>." << endl << endl;

		Network		network;
		Video		video(ui, network);
		Audio		audio(network);
		UILock		lock;


		audio.restart();

		ui.f["audio restart"] = bind(&Audio::restart, &audio);
		ui.f["audio toggle"] = bind(&Audio::toggle_playmic, &audio);
		ui.f["network stats"] = bind(&Network::stats, &network);
		ui.f["network start"] = bind(&start_network, ref(ui), ref(network), ref(video));
		ui.f["name change"] = bind(&Video::send_name, &video);

		try {
			if (ui.autostart->value())
				start_network(ui, network, video);
		} catch (std::exception& e) {
			cout << "autostarting network failed: " << e.what() << endl;
		}

		while(true)
			try {
				return fltk::run();
			} catch (std::exception& e) {
				cout << "error: " << e.what() << endl;
			}
	} catch (std::exception& e) {
		fltk::alert(ss("initialization error: ") << e.what() | c_str);
	}

	return 1;
}
