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

	ui.f["game dealout"] = boost::bind(&Game::dealout_game, this);
	ui.f["game disclose"] = boost::bind(&Game::disclose_hand, this);

	left = 0;
	right = 1;

	deck.resize(32);
	for (unsigned i = 0; i < deck.size(); i++)
		deck[i] = i;

	unsigned secret = 0;
	string(ui.secret->text()).copy((char*)&secret, sizeof(unsigned));
	rangen.seed(secret ^ (unsigned)time(0));
	shuffle();

	reset_game(myself);
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
	playing = false;
	
	hand.clear();
	skat.clear();
	trick.clear();
	tricks.clear();
	lefttricks.clear();
	righttricks.clear();
	playerhand.clear();
	lefthand.clear();
	righthand.clear();
	
	secretdeck.clear();
	secretcards.clear();
	dealtcards.clear();
	drawncards.clear();

	show_info("");
	show_gameinfo(dealer == right? "Vorhand": dealer == left? "Mittelhand": "Hinterhand");
	show_bid(false, -1, false);

	ui.hand->deactivate();
	ui.skat->deactivate();
	ui.announce->label("Spiel ansagen");
	ui.announce->deactivate();
	
	ui.dealout->deactivate();
	ui.disclose->deactivate();
	ui.contrare->value(false);
	ui.contrare->deactivate();
	ui.giveup->value(false);
	ui.giveup->deactivate();

	ui.table->show_cards(hand, skat);
	ui.table->show_trick(trick, 0);
	ui.table->show_disclosed(lefthand, righthand);
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


string Game::game_name(bool select) {
	if (select) {
		gname = ui.diamonds->value()? 24: ui.hearts->value()? 16: ui.spades->value()? 8: ui.clubs->value()? 0: ui.grand->value()? 32: ui.null->value()? 64: 128;
		gextra = ui.ouvert->value()? 15: ui.schwarz->value()? 7: ui.schneider->value()? 3: ui.skat->active()? 1: 0;
	}
	
	string name = gname == 24? "Karo": gname == 16? "Herz": gname == 8? "Pik": gname == 0? "Kreuz": gname == 32? "Grand": gname == 64? "Null": "Null Ouvert";
	name += gextra == 15? " Ouvert": gextra == 7? " Schwarz": gextra == 3? " Schneider": gextra == 1? " Hand": "";
	
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
		ui.announce->copy_label((game_name(true) + " ansagen").c_str());
		ui.announce->redraw();
	}

	sort_hand();
	ui.table->show_cards(hand, skat);
}


bool Game::permit_card(uchar card) {
	if (trick.size() >= 3)
		return false;
	if (trick.size() == 0 && starter != myself)
		return false;
	if (trick.size() == 1 && starter != right)
		return false;
	if (trick.size() == 2 && starter != left)
		return false;
		
	if (trick.size() == 0)
		return true;

	bool trump = gname < 64 && ((trick[0] & 7) == 3 || (trick[0] & 24) == gname);
	unsigned suit = trick[0] & 24;
	bool canserve = false;

	for (unsigned i = 0; i < hand.size(); i++)
		if (trump)
			canserve |= (hand[i] & 7) == 3 || (hand[i] & 24) == gname;
		else
			canserve |= (hand[i] & 24) == suit && (gname > 32 || (hand[i] & 7) != 3);

	if (!canserve)
		return true;
	if (trump && ((card & 7) == 3 || (card & 24) == gname))
		return true;
	if (!trump && (card & 24) == suit && (gname > 32 || (card & 7) != 3))
		return true;

	return false;
}


void Game::check_trick(void) {
	if (trick.size() == 0)
		starter = dealer == myself? left: dealer == left? right: myself;
	else
		ui.table->show_trick(trick, starter == left? 1: starter == right? 2: 0);

	if (trick.size() == 3) {
		map<unsigned, unsigned> priority;

		for (unsigned i = 0; i < trick.size(); i++) {
			uchar suit = trick[i] & 24;
			uchar value = gname > 32? (trick[i] & 7): (trick[i] & 7) == 1? 2: (trick[i] & 7) == 2? 3: (trick[i] & 7) == 4? 1: (trick[i] & 7);

			if (gname < 64 && (trick[i] & 7) == 3)
				priority[trick[i]] = i;
			else
				priority[suit + value + (suit == gname? 32: suit == (trick[0] & 24)? 64: 96)] = i;
		}
		
		if (priority.begin()->second == 1)
			starter = starter == myself? left: starter == left? right: myself;
		else if (priority.begin()->second == 2)
			starter = starter == myself? right: starter == left? myself: left;

		for (unsigned i = 0; i < trick.size(); i++)
			(starter == myself? tricks: starter == left? lefttricks: righttricks).push_back(trick[i]);
			
		trick.clear();
	}

	unsigned pos[] = {myself, right, left, myself, right};
	show_info(starter == pos[trick.size()]? "Spiele eine Karte!": "Warte auf Karte von " + (starter == pos[trick.size() + 2]? leftname: rightname) + '.');
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
			swap (hand[sel], skat[p = p? 0: 1]);
		}

		if (skat[0] == 32 || skat[1] == 32)
			ui.announce->deactivate();
		else
			ui.announce->activate();

		sort_hand();
		ui.table->show_cards(hand, skat);
		
	} else if (playing && sel < hand.size() && permit_card(hand[sel])) {

		trick.push_back(hand[sel]);
		hand.erase(hand.begin() + sel);
		network.command(left, "trick", cards_string(trick));
		network.command(right, "trick", cards_string(trick));

		ui.table->show_cards(hand, skat);
		check_trick();
	}
}


void Game::single_player(void) {
	player = myself;
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

	if (listener == myself)
		single_player();
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

	if (listener == myself)
		ui.dealout->activate();
	else if (type == "Reizen")
		network.command(listener, "fold", ss(bid));
	else
		network.command(listener, "fold", ss(bid + 1));
}


void Game::take_skat(void) {
	if (dealer == myself || dealer == left)
		network.command(right, "dealskat", "");
	else
		network.command(left, "dealskat", "");

	ui.hand->deactivate();
	ui.skat->deactivate();
	select_game();
}


void Game::announce_game(void) {
	playerhand = hand;
	skat.clear();

	ui.table->show_cards(hand, skat);
	playing = true;
	check_trick();
	
	show_gameinfo(game_name(true));
	network.command(left, "announce", ss(gname) << ' ' << gextra);
	network.command(right, "announce", ss(gname) << ' ' << gextra);
	if (gname == 128 || gextra == 15) {
		network.command(left, "disclose", cards_string(hand));
		network.command(right, "disclose", cards_string(hand));
	}

	ui.hand->deactivate();
	ui.skat->deactivate();
	ui.announce->deactivate();
	ui.giveup->activate();
	if (gname != 128 && gextra != 15)
		ui.disclose->activate();
	select_game();
}


void Game::dealout_game(void) {
	reset_game(myself);
	shuffle();

	secretdeck = deck;
	network.command(left, "newdeal", "");
	ui.dealout->deactivate();
}


void Game::disclose_hand(void) {
	network.command(left, "disclose", cards_string(hand));
	network.command(right, "disclose", cards_string(hand));
	ui.disclose->deactivate();
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
		reset_game(myself);
		send_name();
		ui.dealout->activate();

	} else if (command == "name") {
		leftname = i == left? data: leftname;
		rightname = i == right? data: rightname;

	} else if (command == "seat") {
		left = data == "left"? 1: 0;
		right = data == "right"? 1: 0;
		reset_game(myself);
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

	} else if (command == "dealskat" && drawncards.size() == 30 && !playing) {
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
		network.command(i? 0: 1, "bidinfo", data);
	} else if (command == "bidinfo") {
		show_info((i == left? rightname: leftname) + " sagt " + data + " zu " + (i == left? leftname: rightname) + '.');

	} else if (command == "hold") {
		ss(data) >> bid;
		listener = i;
		show_info(ss(i == left? leftname: rightname) << " hält " << bid << '.');
		show_bid(true, bid + 1, true);

	} else if (command == "fold") {
		if (i == dealer && bid == 18 && data == "18") {
			show_info("Spielen für 18 oder Einpassen?");
			listener = myself;
			show_bid(true, bid, false);
		} else if (i == dealer || dealer == myself) {
			single_player();
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
		ss(data) >> gname >> gextra;
		show_gameinfo((player == left? leftname : rightname) + " spielt " + game_name(false) + '.');
		ui.giveup->activate();
		playing = true;
		check_trick();

	} else if (command == "trick") {
		trick = string_cards(data);
		if (i == player && !trick.empty() && playerhand.size() < 10)
			playerhand.push_back(trick.back());
		vector<uchar>& cards = i == left? lefthand: righthand;
		if (!trick.empty())
			cards.resize(remove(cards.begin(), cards.end(), trick.back()) - cards.begin());
		ui.table->show_disclosed(lefthand, righthand);
		check_trick();

	} else if (command == "disclose") {
		vector<uchar> cards = string_cards(data);
		for (unsigned j = 0; j < cards.size() && playerhand.size() < 10; j++)
			playerhand.push_back(cards[j]);
		(i == left? lefthand: righthand) = cards;
		ui.table->show_disclosed(lefthand, righthand);



	} else
		return false;
		
	fltk::awake();
	return true;
}
