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

#ifndef SK_BIDBUTTON_H
#define SK_BIDBUTTON_H


#include <FL/Fl_Button.H>
#include <set>


namespace SK {


class BidButton : public Fl_Button {
	std::set<unsigned>		bidorder;
	std::set<unsigned>::iterator	minbid;
	std::set<unsigned>::iterator	maxbid;
	std::set<unsigned>::iterator	bid;
	bool				bidding;

	int handle(int);

public:
	BidButton(int, int, int, int, const char* = 0);

	void reset(unsigned, bool);
};


}


#endif
