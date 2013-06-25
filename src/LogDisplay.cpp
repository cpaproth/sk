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

#include "LogDisplay.h"
#include <FL/Fl.h>
#include <iostream>
#include <climits>


using namespace SK;


LogDisplay::LogDisplay(int x, int y, int w ,int h, const char* l) : Fl_Text_Display(x, y, w, h, l) {
	Fl::add_timeout(0.1, timeout, this);
	oldsbuf = cout.rdbuf(this);
	buffer(textbuf);
}


LogDisplay::~LogDisplay(void) {
	buffer(0);
	cout.rdbuf(oldsbuf);
}


int LogDisplay::overflow(int c) {
	boost::lock_guard<boost::mutex> lock(bufmutex);
	strbuf.append(1, (char)c);
	return c;
}


void LogDisplay::timeout(void* v) {
	LogDisplay* l = (LogDisplay*)v;

	boost::lock_guard<boost::mutex> lock(l->bufmutex);
	if (l->strbuf.length() > 0) {
		l->textbuf.append(l->strbuf.c_str());
		l->strbuf.clear();
		l->scroll(INT_MAX, 0);
		l->redraw();
	}

	Fl::repeat_timeout(0.1, timeout, v);
}


int LogDisplay::handle(int event) {
	//using namespace fltk;

	//if (event == TIMEOUT) {
	if (event == FL_PUSH) {
		boost::lock_guard<boost::mutex> lock(bufmutex);
		if (strbuf.length() > 0) {
			textbuf.append(strbuf.c_str());
			strbuf.clear();
			scroll(INT_MAX, 0);
			redraw();
		}

		//repeat_timeout(0.1f);
		return 1;
	}

	return Fl_Text_Display::handle(event);
}
