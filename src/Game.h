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

#ifndef SK_GAME_H
#define SK_GAME_H


#include <boost/thread/thread.hpp>
#include "MathLib.h"


namespace SK {

using namespace std;

class UserInterface;
class Network;

class Game {
	typedef unsigned char uchar;

	CPLib::RanGen	rangen;
	vector<uchar>	deck;
	vector<uchar>	hand;
	
	vector<uchar>	secretdeck;
	vector<uchar>	secretcards;
	vector<uchar>	drawncards;
	
	string		leftname;
	string		rightname;
	unsigned	left;
	unsigned	right;
	UserInterface&	ui;
	Network&	network;
	boost::mutex	gamemutex;


	void shuffle(void);
	string cards_string(const vector<uchar>&);
	vector<uchar> string_cards(const string&);
	void reset_secret(void);

	void show_cards(const vector<uchar>&);
	vector<uchar> deal_cards(unsigned, bool);
	void decipher(void);


	Game(const Game&);
	void operator=(const Game&);
public:
	Game(UserInterface&, Network&);
	~Game(void);
	
	void send_name(void);
	bool handle_command(unsigned, const string&, const string&);

	void start_dealing(void);
};


}


#endif
