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

#include "Game.h"
#include "Network.h"
#include "ui.h"


using namespace SK;


Game::Game(UserInterface& ui, Network& nw) : ui(ui), network(nw) {
	left = 0;
	right = 1;

}


Game::~Game(void) {
	try {
	
	} catch (...) {}
}


void Game::send_name(void) {
	network.command(left, "name", ui.name->text());
	network.command(right, "name", ui.name->text());
}


bool Game::handle_command(unsigned i, const string& command, const string& data) {
	if (command == "name") {
		if (i == left)
			leftname = data;
		else if (i == right)
			rightname = data;
	} else if (command == "seat") {
		if (data == "left") {
			left = 1;
			right = 0;
		} else if (data == "right") {
			left = 0;
			right = 1;
		}
		send_name();
	} else
		return false;
	return true;
}
