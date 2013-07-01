/*Copyright (C) 2012, 2013 Carsten Paproth

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

#include "BidButton.h"
#include <FL/Fl.H>
#include "Convenience.h"


using namespace SK;
using namespace CPLib;


BidButton::BidButton(int x, int y, int w ,int h, const char* l) : Fl_Button(x, y, w, h, l) {
	for (unsigned i = 2; i <= 18; i++) {
		bidorder.insert(i * 9);
		bidorder.insert(i * 10);
		bidorder.insert(i * 11);
		bidorder.insert(i * 12);
	}
	for (unsigned i = 2; i <= 11; i++)
		bidorder.insert(i * 24);
	bidorder.insert(23);
	bidorder.insert(35);
	bidorder.insert(46);
	bidorder.insert(59);
	
	reset(-1, false);
}


void BidButton::reset(unsigned min, bool b) {
	bidding = b;
	minbid = bidorder.lower_bound(min);
	maxbid = bidorder.find(264);
	bid = minbid;
	if (minbid != bidorder.end())
		copy_label(ss(*bid) << (bidding? " Reizen": " Halten") | c_str);
	else {
		label("Reizen");
		bidding = false;
	}
	redraw();
}


int BidButton::handle(int event) {
	if (bidding && event == FL_MOUSEWHEEL) {
		for (int i = Fl::event_dy(); i < 0 && bid != minbid; i++)
			bid--;
		for (int i = Fl::event_dy(); i > 0 && bid != maxbid; i--)
			bid++;

		copy_label(ss(*bid) << " Reizen" | c_str);
		redraw();

		return 1;
	}

	return Fl_Button::handle(event);
}
