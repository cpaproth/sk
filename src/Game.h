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


namespace SK {

class UserInterface;
class Network;

class Game {
	std::string				leftname;
	std::string				rightname;
	unsigned				left;
	unsigned				right;
	UserInterface&				ui;
	Network&				network;


	Game(const Game&);
	void operator=(const Game&);
public:
	Game(UserInterface&, Network&);
	~Game(void);
	
	void send_name(void);
	bool handle_command(unsigned, const std::string&, const std::string&);

};


}


#endif
