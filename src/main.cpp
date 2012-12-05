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
#include "Game.h"

#include <fltk/ask.h>
#include <iostream>
#include "Convenience.h"


using namespace CPLib;
using namespace SK;


void connect_network(UserInterface& ui, Network& network) {
	network.connect(ui.address->value(), (unsigned short)ui.port->value(), (unsigned)ui.bandwidth->value());
}


int main(void) {
	try {
		UserInterface	ui;

		cout << "Skat-Konferenz Copyright (C) 2012 Carsten Paproth." << endl;
		cout << "This program is free software and comes with ABSOLUTELY NO WARRANTY." << endl;
		cout << "Licensed under the terms of GPLv3, see <http://www.gnu.org/licenses/>." << endl << endl;

		Network		network;
		Video		video(ui, network);
		Audio		audio(network);
		Game		game(ui, network);


		ui.f["audio restart"] = boost::bind(&Audio::restart, &audio);
		ui.f["audio toggle"] = boost::bind(&Audio::toggle_playmic, &audio);
		ui.f["network stats"] = boost::bind(&Network::stats, &network);
		ui.f["network connect"] = boost::bind(&connect_network, boost::ref(ui), boost::ref(network));
		
		audio.restart();
		try {
			if (ui.autoconnect->value())
				connect_network(ui, network);
		} catch (exception& e) {
			cout << "auto connecting network failed: " << e.what() << endl;
		}

		while(true)
			try {
				UILock lock;
				return fltk::run();
			} catch (exception& e) {
				cout << "error: " << e.what() << endl;
			}
	} catch (exception& e) {
		fltk::alert(ss("initialization error: ") << e.what() | c_str);
	}

	return 1;
}
