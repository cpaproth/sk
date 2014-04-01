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


#include "LogDisplay.h"
#include <FL/Fl.H>
#include <iostream>
#include <climits>
#include <boost/thread/locks.hpp>


using namespace SK;


LogDisplay::LogDisplay(int x, int y, int w ,int h, const char* l) : Fl_Text_Display(x, y, w, h, l) {
	oldsbuf = cout.rdbuf(this);
	buffer(textbuf);
	Fl::add_timeout(0.1, timeout, this);
}


LogDisplay::~LogDisplay(void) {
	Fl::remove_timeout(timeout, this);
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
