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
#include "Convenience.h"


using namespace SK;
using namespace CPLib;


Game::Game(UserInterface& ui, Network& nw) : ui(ui), network(nw) {
	left = 0;
	right = 1;

	deck.resize(32);
	for (unsigned i = 0; i < deck.size(); i++)
		deck[i] = i;

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
	for (ptrdiff_t i = deck.size() - 1; i > 0; i--)
		swap(deck[i], deck[(ptrdiff_t)(rangen.uniform() * (i + 1))]);
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


void Game::start_dealing(void) {
	shuffle();

	network.command(right, "cutdeck", "");
	
	vector<uchar> lefthand(deck.begin(), deck.begin() + 10);
	vector<uchar> righthand(deck.begin() + 10, deck.begin() + 20);
	hand.assign(deck.begin() + 20, deck.begin() + 30);

	network.command(left, "cards", cards_string(lefthand));
	network.command(right, "cards", cards_string(righthand));

	show_cards(hand);
}


void Game::send_name(void) {
	network.command(left, "name", ui.name->text());
	network.command(right, "name", ui.name->text());
}


bool Game::handle_command(unsigned i, const string& command, const string& data) {
	if (command == "name") {
		leftname = i == left? data: leftname;
		rightname = i == right? data: rightname;
	} else if (command == "seat") {
		left = data == "left"? 1: 0;
		right = data == "right"? 1: 0;
		send_name();
		
	} else if (command == "cards") {
		show_cards(string_cards(data));
	} else if (command == "cutdeck") {
		shuffle();
		network.command(i, "cipherdeck", cards_string(deck));

		vector<uchar> c(deck);
		shuffle();
		vector<uchar> draw;
		for (unsigned j = 0; j < 10 ; j++)
			draw.push_back(c[deck[j]]);
		network.command(i? 0: 1, "secretcards", cards_string(vector<uchar>(deck.begin(), deck.begin() + 10)) + " " + cards_string(draw));
	} else if (command == "cipherdeck") {
		vector<uchar> c(string_cards(data));
		for (unsigned j = 0; j < c.size(); j++)
			c[j] = deck[c[j]];
		network.command(i? 0: 1, "secretdeck", cards_string(c));
	} else if (command == "secretdeck") {
		secretdeck = string_cards(data);
		
	} else if (command == "secretcards") {
		string secret;
		vector<uchar> c(string_cards(ss(data) >> secret));
		secretcards = string_cards(secret);

	} else
		return false;
	return true;
}
