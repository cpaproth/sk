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

#include <boost/asio/ip/host_name.hpp>


#include "ui.h"

#include "Convenience.h"

#include "Network.h"
#include "Audio.h"
#include "Video.h"

#include <iostream>

using namespace CPLib;
using namespace SK;




UserInterface* pui;
class mysbuf : public std::streambuf {
	int overflow(int ch) {
		char buf[] = {(char)ch, 0};
		fltk::lock();
		pui->log->append(buf);
		if (buf[0] == '\n') {
			pui->log->scroll(INT_MAX, 0);
			fltk::awake();
		}
		fltk::unlock();
		return ch;
	}
};


Network network;
void newcommand(unsigned i, const string& command, const string& data) {
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

	UserInterface ui;
	pui=&ui;
	mysbuf sbuf;
	streambuf* oldbuf = cout.rdbuf(&sbuf);

	cout << "Skat-Konferenz Copyright (C) 2012 Carsten Paproth." << endl;
	cout << "This program is free software and comes with ABSOLUTELY NO WARRANTY." << endl;
	cout << "Licensed under the terms of GPLv3, see <http://www.gnu.org/licenses/>." << endl << endl;
	
	Video video(ui, network);
	Audio audio(network);


	try {
		pair<string, string> server("fuchsbau", "192.168.0.1");
		if (boost::asio::ip::host_name() == server.first)
			network.start("", 50000, &newcommand);
		else
			network.start(server.second, 50000, &newcommand);
	}catch(exception& e){
		cout<<e.what()<<endl;
	}


	ui.f["Button"] = boost::bind(&Audio::restart, &audio);

	fltk::run();


	cout.rdbuf(oldbuf);


	return 0;
}
