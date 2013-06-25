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

#ifndef SK_LOGDISPLAY_H
#define SK_LOGDISPLAY_H


#include <FL/Fl_Text_Display.H>
#include <boost/thread/mutex.hpp>

namespace SK {

using namespace std;

class LogDisplay : streambuf, public Fl_Text_Display {
	
	Fl_Text_Buffer	textbuf;
	streambuf*	oldsbuf;
	string		strbuf;
	boost::mutex	bufmutex;

	int overflow(int);
	static void timeout(void*);
public:
	LogDisplay(int, int, int, int, const char* = 0);
	~LogDisplay(void);
	

};


}


#endif
