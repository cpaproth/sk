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
#include <iostream>
#include <set>


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


void Game::sort_hand(void) {
	UILock lock;
	map<uchar, set<uchar> > suits;
	set<uchar> jacks;
	bool trump = !ui.null->value() && !ui.nullouvert->value();

	for (unsigned i = 0; i < hand.size(); i++) {
		if (trump && (hand[i] & 7) == 3)
			jacks.insert(hand[i]);
		else if (trump && (hand[i] & 7) == 4)
			suits[hand[i] & 24].insert(1);
		else
			suits[hand[i] & 24].insert((hand[i] & 7) * 2);
	}

	size_t size = hand.size();
	hand.assign(jacks.begin(), jacks.end());
	
	uchar nextsuit = suits[0].size() > 0 && suits[8].size() > 0? 0: 16;
	uchar trumpsuit = ui.diamonds->value()? 24: ui.hearts->value()? 16: ui.spades->value()? 8: ui.clubs->value()? 0: 32;
	if (trumpsuit != 32) {
		for (set<uchar>::iterator it = suits[trumpsuit].begin(); it != suits[trumpsuit].end(); it++)
			hand.push_back(trumpsuit + (*it == 1? 4: *it / 2));
		suits.erase(trumpsuit);
		nextsuit = trumpsuit < 16? 16: 0;
	}
	
	while (hand.size() < size) {
		if (suits[nextsuit].size() == 0) {
			nextsuit = (nextsuit + 8) & 24;
			continue;
		}
		for (set<uchar>::iterator it = suits[nextsuit].begin(); it != suits[nextsuit].end(); it++)
			hand.push_back(nextsuit + (*it == 1? 4: *it / 2));
		suits.erase(nextsuit);
		nextsuit = (nextsuit + 16) & 24;
	}
}


void Game::reset_game(void) {
	hand.clear();
	secretdeck.clear();
	secretcards.clear();
	dealtcards.clear();
	drawncards.clear();

	UILock lock;
	ui.game->activate();
	fltk::awake();
}


void Game::show_cards(const vector<uchar>& c) {
	UILock lock;
	ui.table->set_cards(c);
	ui.table->redraw();
	fltk::awake();
}


void Game::start_dealing(void) {
	if (dealer == UINT_MAX)
		return;

	reset_game();
	dealer = UINT_MAX;

	shuffle();

	secretdeck = deck;
	network.command(left, "newdeal", "");
}


void Game::take_skat(void) {
	if (dealer == UINT_MAX || dealer == left)
		network.command(right, "dealskat", "");
	else
		network.command(left, "dealskat", "");
}


void Game::deal_cards(unsigned n, bool cipher) {
	vector<uchar> cards(deck);
	shuffle();

	dealtcards.clear();
	for (unsigned i = 0; dealtcards.size() != n && drawncards.size() < 32; i++) {
		if (find(drawncards.begin(), drawncards.end(), cards[deck[i]]) == drawncards.end()) {
			dealtcards.push_back(cipher? deck[i]: cards[deck[i]]);
			drawncards.push_back(cards[deck[i]]);
		}
	}
}


void Game::decipher(void) {
	if (secretdeck.size() == 0 || secretcards.size() == 0)
		return;

	for (unsigned i = 0; i < secretcards.size(); i++)
		hand.push_back(secretdeck[secretcards[i]]);

	sort_hand();
	show_cards(hand);
	
	secretdeck.clear();
	secretcards.clear();

	if (drawncards.size() == 10) {
		shuffle();
		network.command(right, "cipherdeck", cards_string(deck));
		deal_cards(10, true);
		network.command(left, "secretcards", cards_string(dealtcards) + " " + cards_string(drawncards));

	} else if (drawncards.size() == 20) {
		deal_cards(10, false);
		network.command(left, "secretcards", cards_string(dealtcards));
		network.command(right, "drawncards", cards_string(drawncards));

	} else if (drawncards.size() == 0) {
		secretdeck = deck;
		dealer = left;
	}
}


void Game::send_name(void) {
	network.command(left, "name", ui.name->text());
	network.command(right, "name", ui.name->text());
}


bool Game::handle_command(unsigned i, const string& command, const string& data) {
	if (command == "peersconnected" && data == "2") {
		cout << "2 peers connected, the game can start!" << endl;
		network.command(0, "seat", "left");
		network.command(1, "seat", "right");
		send_name();
		start_dealing();

	} else if (command == "name") {
		leftname = i == left? data: leftname;
		rightname = i == right? data: rightname;

	} else if (command == "seat") {
		left = data == "left"? 1: 0;
		right = data == "right"? 1: 0;
		send_name();


	} else if (command == "newdeal") {
		reset_game();
		dealer = right;
		network.command(left, "cutdeck", "");

	} else if (command == "cutdeck") {
		reset_game();
		dealer = left;
		shuffle();
		network.command(left, "cipherdeck", cards_string(deck));
		deal_cards(10, true);
		network.command(right, "secretcards", cards_string(dealtcards) + " " + cards_string(drawncards));

	} else if (command == "cipherdeck") {
		vector<uchar> cards(string_cards(data));
		for (unsigned j = 0; j < cards.size(); j++)
			cards[j] = deck[cards[j]];
		network.command(i? 0: 1, "secretdeck", cards_string(cards));

	} else if (command == "secretdeck" && i == dealer) {
		secretdeck = string_cards(data);
		decipher();

	} else if (command == "secretcards" && i != dealer) {
		string secret;
		drawncards = string_cards(ss(data) >> secret >> ws);
		secretcards = string_cards(secret);
		decipher();

	} else if (command == "drawncards" && i != dealer) {
		drawncards = string_cards(data);
		if (drawncards.size() == 30)
			network.command(left, "saying", "");

	} else if (command == "dealskat" && drawncards.size() == 30) {
		if (i == dealer) {
			deal_cards(2, false);
			network.command(i, "secretcards", cards_string(dealtcards));
			network.command(i? 0: 1, "drawncards", cards_string(drawncards));
		} else {
			shuffle();
			network.command(dealer, "cipherdeck", cards_string(deck));
			deal_cards(2, true);
			network.command(i, "secretcards", cards_string(dealtcards) + " " + cards_string(drawncards));
		}


	} else
		return false;
	return true;
}
