/*Copyright (C) 2012-2014 Carsten Paproth

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

#ifndef SK_GAME_H
#define SK_GAME_H


#include "MathLib.h"

class UserInterface;

namespace SK {

using namespace std;

class Network;

class Game {
	static const unsigned myself = UINT_MAX;

	typedef unsigned char uchar;

	CPLib::RanGen	rangen;
	vector<uchar>	deck;
	vector<uchar>	hand;
	vector<uchar>	skat;
	vector<uchar>	trick;
	vector<uchar>	tricks;
	vector<uchar>	lefttricks;
	vector<uchar>	righttricks;
	vector<uchar>	playerhand;
	vector<uchar>	lefthand;
	vector<uchar>	righthand;
	
	vector<uchar>	secretdeck;
	vector<uchar>	secretcards;
	vector<uchar>	dealtcards;
	vector<uchar>	drawncards;
	unsigned	dealer;
	unsigned	listener;
	unsigned	starter;
	unsigned	player;
	unsigned	quitter;
	unsigned	bid;
	unsigned	gname;
	unsigned	gextra;
	int		gtips;
	bool		playing;
	bool		givingup;

	string		header;
	vector<uchar>	rounds;
	unsigned	row;
	unsigned	contrare;
	unsigned	rules;
	unsigned	leftrules;
	unsigned	rightrules;
	int		scores;
	int		leftscores;
	int		rightscores;

	string		leftname;
	string		rightname;
	unsigned	left;
	unsigned	right;
	UserInterface&	ui;
	Network&	network;


	bool rule(unsigned);
	void shuffle(void);
	string cards_string(const vector<uchar>&);
	vector<uchar> string_cards(const string&);

	void reset_game(unsigned);
	void reset_round(void);
	void show_bid(bool, unsigned, bool);
	void show_contrare(const char*, bool, bool);
	void show_info(const string&);
	void show_gameinfo(const string&);
	void sort_hand(void);
	string game_name(bool);

	void check_rules(void);
	void send_rules(void);
	void send_name(void);
	void select_game(void);
	bool permit_card(uchar);
	void game_over(void);
	void check_trick(void);
	void table_event(void);

	void single_player(void);
	void junk_player(void);
	void bid_game(void);
	void fold_game(void);
	void take_skat(void);
	void announce_game(void);

	void dealout_game(void);
	void disclose_hand(void);
	void giveup_game(void);
	void contrare_game(void);

	void deal_cards(unsigned, bool);
	void decipher_cards(void);

	bool handle_command(unsigned, const string&, const string&);

	Game(const Game&);
	void operator=(const Game&);
public:
	Game(UserInterface&, Network&);
	~Game(void);

};


}


#endif
