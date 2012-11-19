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

	cards.resize(32);
	for (unsigned i = 0; i < cards.size(); i++)
		cards[i] = i;

	unsigned secret = 0;
	string(ui.secret->text()).copy((char*)&secret, sizeof(unsigned));
	rangen.seed(secret ^ (unsigned)time(0));
	shuffle();
}


Game::~Game(void) {
	try {
	
	} catch (...) {}
}


void Game::shuffle(void) {
	for (ptrdiff_t i = cards.size() - 1; i > 0; i--)
		swap(cards[i], cards[(ptrdiff_t)(rangen.uniform() * (i + 1))]);
}



string Game::cards_string(const vector<uchar>& c) {
	string str(c.size(), 0);
	for (unsigned i = 0; i < c.size(); i++)
		str[i] = c[i] + 33;
	return str;
}


vector<uchar> Game::string_cards(const string& str) {
	vector<uchar> c(str.begin(), str.end());
	for (unsigned i = 0; i < c.size(); i++)
		c[i] -= 33;
	return c;
}


void Game::show_cards(const vector<uchar>& c) {
	UILock lock;
	ui.table->set_cards(c);
	ui.table->redraw();
	fltk::awake();
}


void Game::begin_shuffle(void) {
	shuffle();

	vector<uchar> lefthand(cards.begin(), cards.begin() + 10);
	vector<uchar> righthand(cards.begin() + 10, cards.begin() + 20);
	vector<uchar> myhand(cards.begin() + 20, cards.begin() + 30);

	network.command(left, "cards", cards_string(lefthand));
	network.command(right, "cards", cards_string(righthand));

	show_cards(myhand);
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
		show_cards(string_cards(data));
	} else
		return false;
	return true;
}
