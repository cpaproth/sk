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

#include "LogDisplay.h"
#include <fltk/events.h>
#include <iostream>
#include <climits>


using namespace SK;


LogDisplay::LogDisplay(int x, int y, int w ,int h, const char* l) : TextDisplay(x, y, w, h, l) {
	add_timeout(0.1f);
	oldsbuf = cout.rdbuf(this);
}


LogDisplay::~LogDisplay(void) {
	cout.rdbuf(oldsbuf);
}


int LogDisplay::overflow(int c) {
	boost::lock_guard<boost::mutex> lock(bufmutex);
	buffer.append(1, (char)c);
	return c;
}


int LogDisplay::handle(int event) {
	using namespace fltk;
	
	if (event == TIMEOUT) {
		boost::lock_guard<boost::mutex> lock(bufmutex);
		if (buffer.length() > 0) {
			append(buffer.c_str());
			buffer.clear();
			scroll(INT_MAX, 0);
			redraw();
		}

		repeat_timeout(0.1f);
		return 1;
	}

	return TextDisplay::handle(event);
}
