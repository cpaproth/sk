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


void Game::shuffle(void) {
	vector<uchar> cards(32);
	for (unsigned i = 0; i < cards.size(); i++)
		cards[i] = i + 33;
	random_shuffle(cards.begin(), cards.end());
	
	network.command(left, "cards", string(cards.begin(), cards.begin() + 10));
	network.command(right, "cards", string(cards.begin() + 10, cards.begin() + 20));
	
	UILock lock;
	ui.table->set_cards(vector<uchar>(cards.begin() + 20, cards.begin() + 30));
	ui.table->redraw();
	fltk::awake();
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
	} else if (command == "cards") {
		UILock lock;
		ui.table->set_cards(vector<uchar>(data.begin(), data.end()));
		ui.table->redraw();
		fltk::awake();
	} else
		return false;
	return true;
}
