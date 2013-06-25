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

#ifndef SK_GLTABLE_H
#define SK_GLTABLE_H


#include <FL/Fl_Gl_Window.H>
#include <vector>


namespace SK {

using namespace std;

class GlTable : public Fl_Gl_Window {

	unsigned		texture;
	unsigned		width;
	unsigned		height;
	unsigned		potwidth;
	unsigned		potheight;
	vector<uchar>		mem;
	vector<uchar>		hand;
	vector<uchar>		skat;
	vector<uchar>		trick;
	unsigned		start;
	vector<uchar>		lefthand;
	vector<uchar>		righthand;
	
	unsigned		selected;
	bool			pushed;
	Fl_Color		bgcolor;

	void get_position(unsigned, size_t, float&, float&, float&);
	void draw_card(uchar, float, float, float, float);
	bool inside_card(int, int, float, float, float, float);

	void draw(void);
	int handle(int);

public:
	GlTable(int, int, int, int, const char* = 0);
	
	void set_bgcolor(Fl_Color);
	void show_cards(const vector<uchar>&, const vector<uchar>&);
	void show_trick(const vector<uchar>&, unsigned);
	void show_disclosed(const vector<uchar>&, const vector<uchar>&);
	
	unsigned selection(void);

};


}


#endif
