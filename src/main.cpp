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
#include "ui.h"
#include "Audio.h"
#include "Video.h"
#include "Game.h"
#include <FL/fl_ask.H>
#include <iostream>
#include "Convenience.h"


using namespace CPLib;
using namespace SK;


void mute_audio(UserInterface& ui, Audio& audio) {
	audio.mute_mic(ui.midimage->get());
}


void connect_network(UserInterface& ui, Network& network) {
	network.connect(ui.address->value(), (unsigned short)ui.port->value(), (bool)ui.server->value(), (unsigned)ui.bandwidth->value());
}


int main() {
	try {
		UILock		lock;
		UserInterface	ui;

		cout << "Skat-Konferenz 1.4.0 beta" << endl;
		cout << "Copyright (C) 2012-2014, 2021 Carsten Paproth." << endl;
		cout << "This program is free software and comes with ABSOLUTELY NO WARRANTY." << endl;
		cout << "Licensed under the terms of GPLv3, see <http://www.gnu.org/licenses/>." << endl << endl;

		Network		network(boost::bind(&Fl::wait, 0.1));
		Video		video(ui, network);
		Audio		audio(network);
		Game		game(ui, network);

		ui.f["audio restart"] = boost::bind(&Audio::restart, &audio);
		ui.f["audio toggle"] = boost::bind(&Audio::toggle_playmic, &audio);
		ui.f["audio mute"] = boost::bind(&mute_audio, boost::ref(ui), boost::ref(audio));
		ui.f["chat message"] = boost::bind(&Video::send_chat, &video);
		ui.f["network stats"] = boost::bind(&Network::stats, &network);
		ui.f["network connect"] = boost::bind(&connect_network, boost::ref(ui), boost::ref(network));

		try {
			audio.restart();
			connect_network(ui, network);
		} catch (exception& e) {
			cout << "start error: " << e.what() << endl;
		}

		while(true)
			try {
				return Fl::run();
			} catch (exception& e) {
				cout << "runtime error: " << e.what() << endl;
			}
	} catch (exception& e) {
		fl_alert("%s", ss("initialization error: ") << e.what() | c_str);
	}

	return 1;
}
