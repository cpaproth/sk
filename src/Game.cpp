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
	ui.f["name change"] = boost::bind(&Game::send_name, this);
	ui.f["game select"] = boost::bind(&Game::select_game, this);
	ui.f["table event"] = boost::bind(&Game::table_event, this);

	ui.f["game bid"] = boost::bind(&Game::bid_game, this);
	ui.f["game fold"] = boost::bind(&Game::fold_game, this);
	ui.f["skat take"] = boost::bind(&Game::take_skat, this);
	ui.f["game announce"] = boost::bind(&Game::announce_game, this);

	left = 0;
	right = 1;

	deck.resize(32);
	for (unsigned i = 0; i < deck.size(); i++)
		deck[i] = i;

	unsigned secret = 0;
	string(ui.secret->text()).copy((char*)&secret, sizeof(unsigned));
	rangen.seed(secret ^ (unsigned)time(0));
	shuffle();

	reset_game(0);
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


void Game::reset_game(unsigned d) {
	dealer = d;
	
	hand.clear();
	skat.clear();
	trick.clear();
	tricks.clear();
	
	secretdeck.clear();
	secretcards.clear();
	dealtcards.clear();
	drawncards.clear();

	show_info("");
	show_gameinfo(dealer == right? "Vorhand": dealer == left? "Mittelhand": "Hinterhand");
	
	ui.hand->deactivate();
	ui.skat->deactivate();
	ui.announce->label("Spiel ansagen");
	ui.announce->deactivate();

	show_bid(false, -1, false);
}


void Game::show_bid(bool show, unsigned bid, bool bidding) {
	if (show) {
		ui.bid->reset(bid, bidding);
		ui.bid->color(fltk::GREEN);
		if (string(ui.bid->label()) == "Reizen")
			ui.bid->deactivate();
		else
			ui.bid->activate();
		ui.fold->color(fltk::RED);
		ui.fold->activate();
	} else {
		ui.bid->reset(bid, bidding);
		ui.bid->color(fltk::GRAY50);
		ui.bid->deactivate();
		ui.fold->color(fltk::GRAY50);
		ui.fold->deactivate();
	}
}


void Game::show_info(const string& info) {
	ui.info->copy_label(info.c_str());
	ui.info->redraw();
}


void Game::show_gameinfo(const string& info) {
	ui.gameinfo->copy_label(info.c_str());
	ui.gameinfo->redraw();
}


void Game::sort_hand(void) {
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


string Game::game_name(void) {
	string name = ui.diamonds->value()? "Karo": ui.hearts->value()? "Herz": ui.spades->value()? "Pik": ui.clubs->value()? "Kreuz": 
			ui.grand->value()? "Grand": ui.null->value()? "Null": "Null Ouvert";
	name += ui.ouvert->value()? " Ouvert": ui.schwarz->value()? " Schwarz": ui.schneider->value()? " Schneider": ui.skat->active()? " Hand": "";
	return name;
}


void Game::send_name(void) {
	network.command(left, "name", ui.name->text());
	network.command(right, "name", ui.name->text());
}


void Game::select_game(void) {
	if (ui.null->value() || ui.nullouvert->value())
		ui.hand->deactivate();
	else if (ui.skat->active())
		ui.hand->activate();

	if (!ui.hand->active()) {
		ui.ouvert->value(false);
		ui.schwarz->value(false);
		ui.schneider->value(false);
	}

	if (ui.announce->active() || skat.size() > 0) {
		ui.announce->copy_label((game_name() + " ansagen").c_str());
		ui.announce->redraw();
	}

	sort_hand();
	ui.table->show_cards(hand, skat);
}


void Game::table_event(void) {
	unsigned sel = ui.table->selection();
	
	if (skat.size() > 1) {
		if (sel >= 100 && sel <= 101) {
			hand.push_back(skat[sel - 100]);
			skat[sel - 100] = 32;
		} else if (sel < hand.size() && skat[0] == 32) {
			skat[0] = hand[sel];
			hand.erase(hand.begin() + sel);
		} else if (sel < hand.size() && skat[1] == 32) {
			skat[1] = hand[sel];
			hand.erase(hand.begin() + sel);
		} else if (sel < hand.size()) {
			static unsigned p = 1;
			swap (hand[sel], skat[p = p == 1? 0: 1]);
		}

		if (skat[0] == 32 || skat[1] == 32)
			ui.announce->deactivate();
		else
			ui.announce->activate();

		sort_hand();
		ui.table->show_cards(hand, skat);
	}
}


void Game::choose_game(void) {
	player = UINT_MAX;
	show_info(ss("Spiel für ") << bid << " erhalten.");
	network.command(left, "bidvalue", ss(bid));
	network.command(right, "bidvalue", ss(bid));

	ui.skat->activate();
	ui.announce->activate();
	select_game();
}


void Game::bid_game(void) {
	string type = ss(ui.bid->label()) >> bid >> ws;
	show_bid(false, bid, type == "Reizen");
	show_info("");

	if (listener == UINT_MAX)
		choose_game();
	else if (type == "Reizen")
		network.command(listener, "bid", ss(bid));
	else
		network.command(listener, "hold", ss(bid));
}


void Game::fold_game(void) {
	string type = ss(ui.bid->label()) >> bid >> ws;
	bid = bid == 0? 264: bid;
	show_bid(false, -1, false);
	show_info("");

	if (listener == UINT_MAX)
		start_dealing();
	else if (type == "Reizen")
		network.command(listener, "fold", ss(bid));
	else
		network.command(listener, "fold", ss(bid + 1));
}


void Game::take_skat(void) {
	if (dealer == UINT_MAX || dealer == left)
		network.command(right, "dealskat", "");
	else
		network.command(left, "dealskat", "");

	ui.hand->deactivate();
	ui.skat->deactivate();
	select_game();
}


void Game::announce_game(void) {
	for (unsigned i = 0; i < skat.size(); i++)
		tricks.push_back(skat[i]);
	skat.clear();

	starter = dealer == UINT_MAX? left: dealer == left? right: UINT_MAX;
	show_info(starter == UINT_MAX? "Spiele eine Karte!": "Warte auf Karte von " + (starter == left? leftname: rightname) + '.'); 
	show_gameinfo(game_name());
	network.command(left, "announce", game_name());
	network.command(right, "announce", game_name());

	ui.hand->deactivate();
	ui.skat->deactivate();
	ui.announce->deactivate();
	ui.table->show_cards(hand, skat);
}


void Game::start_dealing(void) {
	if (dealer == UINT_MAX)
		return;

	reset_game(UINT_MAX);
	shuffle();

	secretdeck = deck;
	network.command(left, "newdeal", "");
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


void Game::decipher_cards(void) {
	if (secretdeck.size() == 0 || secretcards.size() == 0)
		return;

	for (unsigned i = 0; i < secretcards.size(); i++)
		(secretcards.size() == 2? skat: hand).push_back(secretdeck[secretcards[i]]);

	sort_hand();
	ui.table->show_cards(hand, skat);
	
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

	} else if (drawncards.size() == 0)
		secretdeck = deck;
}


bool Game::handle_command(unsigned i, const string& command, const string& data) {
	UILock lock;
	
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
		reset_game(right);
		network.command(left, "cutdeck", "");

	} else if (command == "cutdeck") {
		reset_game(left);
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
		decipher_cards();

	} else if (command == "secretcards" && i != dealer) {
		string secret;
		drawncards = string_cards(ss(data) >> secret >> ws);
		secretcards = string_cards(secret);
		decipher_cards();

	} else if (command == "drawncards" && i != dealer) {
		drawncards = string_cards(data);
		if (drawncards.size() == 30) {
			show_info("Warte auf Gebot von " + leftname + '.');
			network.command(left, "bidme", ss(bid = 18));
		}

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


	} else if (command == "bidme") {
		ss(data) >> bid;
		listener = i;
		show_info("Reize " + (i == left? leftname: rightname) + '!');
		show_bid(true, bid, true);

	} else if (command == "bid") {
		ss(data) >> bid;
		listener = i;
		show_info(ss(i == left? leftname: rightname) << " sagt " << bid << '!');
		show_bid(true, bid, false);

	} else if (command == "hold") {
		ss(data) >> bid;
		listener = i;
		show_info(ss(i == left? leftname: rightname) << " hält " << bid << '.');
		show_bid(true, bid + 1, true);

	} else if (command == "fold") {
		if (i == dealer && bid == 18 && data == "18") {
			show_info("Spielen für 18 oder Einpassen?");
			listener = UINT_MAX;
			show_bid(true, bid, false);
		} else if (i == dealer || dealer == UINT_MAX) {
			choose_game();
		} else {
			show_info(ss(i == left? leftname: rightname) << " passt bei " << bid << '.');
			network.command(dealer, "bidme", data);
		}


	} else if (command == "bidvalue") {
		ss(data) >> bid;
		player = i;
		show_info("Warte auf Spielansage.");
		show_gameinfo(ss(i == left? leftname: rightname) << " spielt für " << bid << '.');
		
	} else if (command == "announce") {
		starter = dealer == UINT_MAX? left: dealer == left? right: UINT_MAX;
		show_info(starter == UINT_MAX? "Spiele eine Karte!": "Warte auf Karte von " + (starter == left? leftname: rightname) + '.'); 
		show_gameinfo((player == left? leftname : rightname) + " spielt " + data + '.');


	} else
		return false;
		
	fltk::awake();
	return true;
}
