/*Copyright (C) 2012-2014, 2021 Carsten Paproth

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


#include "GlTable.h"
#include <FL/gl.h>
#include <FL/Fl.H>
#include <opencv2/highgui/highgui.hpp>
#include <stdexcept>
#include <map>
#include "../images/cards.xpm"
#include "Convenience.h"


using namespace SK;


GlTable::GlTable(int x, int y, int w ,int h, const char* l) : Fl_Gl_Window(x, y, w, h, l) {
	uchar* data = 0;
	unsigned depth = 0;
	cv::Mat pngimg = cv::imread("cards.png");
	vector<uchar> xpmimg;
	size_t c, d, s = sizeof(cards_xpm) / sizeof(char*);


	CPLib::ss(s > 0? cards_xpm[0]: "0 0 0 0") >> width >> height >> c >> d;

	if (pngimg.size().area() != 0) {
		width = pngimg.cols;
		height = pngimg.rows;
		data = pngimg.data;
		depth = pngimg.elemSize();
	} else if (d == 1 && s == height + c + 1) {
		depth = 3;
		xpmimg.resize(width * height * depth);
		data = &xpmimg[0];

		map<char, unsigned> colors;
		for (unsigned i = 1; i <= c; i++) {
			string color(cards_xpm[i]);
			if (color.length() > 5)
				CPLib::ss(color.substr(5)) >> hex >> colors[color[0]];
		}

		for (unsigned y = 0; y < height; y++) {
			string row(cards_xpm[y + c + 1]);
			for (unsigned x = 0; x < width && x < row.length(); x++) {
				data[(y * width + x) * depth] = colors[row[x]];
				data[(y * width + x) * depth + 1] = colors[row[x]] >> 8;
				data[(y * width + x) * depth + 2] = colors[row[x]] >> 16;
			}
		}
	}


	if (width == 0 || height == 0 || depth < 3 || data == 0)
		throw runtime_error("loading card images failed");

	for (potwidth = 1; potwidth < width; potwidth <<= 1);
	for (potheight = 1; potheight < height; potheight <<= 1);

	mem.resize(potwidth * potheight * 4);
	for (unsigned y = 0; y < height; y++) {
		for (unsigned x = 0; x < width; x++) {
			unsigned mpos = (height - 1 - y) * potwidth * 4 + x * 4;
			unsigned ipos = y * width * depth + x * depth;
			if (data[ipos] == 255 && data[ipos + 1] == 0 && data[ipos + 2] == 255) {
				mem[mpos] = 128;
				mem[mpos + 1] = 128;
				mem[mpos + 2] = 128;
				mem[mpos + 3] = 0;
			} else {
				mem[mpos] = data[ipos + 2];
				mem[mpos + 1] = data[ipos + 1];
				mem[mpos + 2] = data[ipos];
				mem[mpos + 3] = 255;
			}
		}
	}

	texture = 0;
	selected = UINT_MAX;
	pushed = false;
	bgcolor = FL_GRAY;
}


void GlTable::set_bgcolor(Fl_Color color) {
	bgcolor = color == 0? FL_GRAY: color;
	redraw();
}


void GlTable::show_cards(const vector<uchar>& h, const vector<uchar>& s) {
	hand = h;
	skat = s;
	selected = UINT_MAX;
	lasttrick.clear();
	redraw();
}


void GlTable::show_trick(const vector<uchar>& t, unsigned s) {
	trick = t;
	start = s;
	if (t.size() == 3) {
		lasttrick = t;
		laststart = s;
	}
	redraw();
}


void GlTable::show_disclosed(const vector<uchar>& l, const vector<uchar>& r) {
	lefthand = l;
	righthand = r;
	redraw();
}


unsigned GlTable::selection() {
	return selected;
}


void GlTable::get_position(unsigned i, size_t s, float& x, float& y, float& a) {
	x = (0.5f + i - s / 2.f) * (90.f - 4.f * s);
	y = -45.f / 102400.f * (x + 320.f) * (x - 320.f);
	a = -(0.5f + i - s / 2.f) * 0.06f;

	x += w() / 2.f;
	x += pushed && i == selected? -50.f * sin(a): 0.f;
	y += pushed && i == selected? 50.f * cos(a): 0.f;
}


void GlTable::draw_card(uchar card, float x, float y, float a, float sy) {
	unsigned c = card % 8;
	unsigned r = card / 8;
	float sx = sy / height * width / 4.f;
	sy /= 2.f;
	float cw = 0.125f * width / potwidth;
	float ch = 0.25f * height / potheight;

	glTexCoord2f(c * cw, r * ch);
	glVertex2f(x - cos(a) * sx + sin(a) * sy, y - sin(a) * sx - cos(a) * sy);

	glTexCoord2f((c + 1) * cw, r * ch);
	glVertex2f(x + cos(a) * sx + sin(a) * sy, y + sin(a) * sx - cos(a) * sy);

	glTexCoord2f((c + 1) * cw, (r + 1) * ch);
	glVertex2f(x + cos(a) * sx - sin(a) * sy, y + sin(a) * sx + cos(a) * sy);

	glTexCoord2f(c * cw, (r + 1) * ch);
	glVertex2f(x - cos(a) * sx - sin(a) * sy, y - sin(a) * sx + cos(a) * sy);
}


bool GlTable::inside_card(int mx, int my, float x, float y, float a, float sy) {
	float sx = sy / height * width / 4.f;
	sy /= 2.f;

	float cx = cos(a) * (mx - x) + sin(a) * (my - y);
	float cy = -sin(a) * (mx - x) + cos(a) * (my - y);

	return cx > -sx && cx < sx && cy > -sy && cy < sy;
}


void GlTable::draw() {
	if (!valid()) {
		glDeleteTextures(1, &texture);
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, potwidth, potheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &mem[0]);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	ortho();

	unsigned char r, g, b;
	Fl::get_color(bgcolor, r, g, b);
	glClearColor(r / 255.f, g / 255.f, b / 255.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);


	glBegin(GL_QUADS);

	for (unsigned i = 0; i < hand.size(); i++) {
		float x, y, a;
		get_position(i, hand.size(), x, y, a);
		draw_card(hand[i], x, y, a, 200.f);
	}

	if (skat.size() > 0 && skat[0] < 32)
		draw_card(skat[0], w() / 2.f - 70.f, 300.f, 0.1f, pushed && selected == 100? 200.f: 160.f);
	if (skat.size() > 1 && skat[1] < 32)
		draw_card(skat[1], w() / 2.f + 70.f, 300.f, -0.1f, pushed && selected == 101? 200.f: 160.f);

	if (selected == UINT_MAX || selected < 200 || !pushed) {
		for (unsigned i = 0; i < trick.size(); i++) {
			if ((i + start) % 3 == 0)
				draw_card(trick[i], w() / 2.f + 10.f, 290.f, 0.05f, 160.f);
			else if ((i + start) % 3 == 1)
				draw_card(trick[i], w() / 2.f - 50.f, 320.f, 0.2f, 160.f);
			else
				draw_card(trick[i], w() / 2.f + 50.f, 330.f, -0.25f, 160.f);
		}

		for (unsigned i = 0; i < lefthand.size(); i++) {
			float a = 0.785f + (0.5f + i - lefthand.size() / 2.f) * 0.13f;
			draw_card(lefthand[i], w() / 2.f - 320.f + sin(a) * 120.f, h() - cos(a) * 120.f, -3.142f + a, 120.f);
		}
		for (unsigned i = 0; i < righthand.size(); i++) {
			float a = 0.785f + (0.5f + i - righthand.size() / 2.f) * 0.13f;
			draw_card(righthand[i], w() / 2.f + 320.f - cos(a) * 120.f, h() - sin(a) * 120.f, -4.712f + a, 120.f);
		}

	} else if (selected < 400) {
		vector<uchar>& cards = selected < 300? lefthand: righthand;
		for (unsigned i = 0; i < cards.size(); i++) {
			float x, y, a;
			get_position(i, cards.size(), x, y, a);
			draw_card(cards[i], x, h() - 80.f - y, -a, 160.f);
		}

	} else {
		for (unsigned i = 0; i < lasttrick.size(); i++) {
			if ((i + laststart) % 3 == 0)
				draw_card(lasttrick[i], w() / 2.f + 10.f, 290.f, 0.05f, 180.f);
			else if ((i + laststart) % 3 == 1)
				draw_card(lasttrick[i], w() / 2.f - 50.f, 320.f, 0.2f, 180.f);
			else
				draw_card(lasttrick[i], w() / 2.f + 50.f, 330.f, -0.25f, 180.f);
		}
	}

	glEnd();
}


int GlTable::handle(int event) {
	switch(event) {
	case FL_ENTER:
		return 1;
	case FL_LEAVE:
		selected = UINT_MAX;
		redraw();
		return 1;
	case FL_PUSH:
		pushed = true;
		redraw();
	case FL_DRAG:
	case FL_MOVE: {
		unsigned sel = UINT_MAX;
		int mx = Fl::event_x();
		int my = Fl::event_y();

		for (unsigned i = 0; i < hand.size(); i++) {
			float x, y, a;
			get_position(i, hand.size(), x, y, a);
			if (inside_card(mx, h() - my - 1, x, y, a, 200.f))
				sel = i;
		}

		if (skat.size() > 0 && skat[0] < 32 && inside_card(mx, h() - my - 1, w() / 2.f - 70.f, 300.f, 0.1f, pushed && selected == 100? 200.f: 160.f))
			sel = 100;
		if (skat.size() > 1 && skat[1] < 32 && inside_card(mx, h() - my - 1, w() / 2.f + 70.f, 300.f, -0.1f, pushed && selected == 101? 200.f: 160.f))
			sel = 101;

		for (unsigned i = 0; i < trick.size() && lasttrick.size() == 3; i++) {
			if ((i + start) % 3 == 0 && inside_card(mx, h() - my - 1, w() / 2.f + 10.f, 290.f, 0.05f, 160.f))
				sel = 400;
			else if ((i + start) % 3 == 1 && inside_card(mx, h() - my - 1, w() / 2.f - 50.f, 320.f, 0.2f, 160.f))
				sel = 401;
			else if (inside_card(mx, h() - my - 1, w() / 2.f + 50.f, 330.f, -0.25f, 160.f))
				sel = 402;
		}

		for (unsigned i = 0; i < lefthand.size(); i++) {
			float a = 0.785f + (0.5f + i - lefthand.size() / 2.f) * 0.13f;
			if (inside_card(mx, h() - my - 1, w() / 2.f - 320.f + sin(a) * 120.f, h() - cos(a) * 120.f, -3.142f + a, 120.f))
				sel = 200 + i;
		}
		for (unsigned i = 0; i < righthand.size(); i++) {
			float a = 0.785f + (0.5f + i - righthand.size() / 2.f) * 0.13f;
			if (inside_card(mx, h() - my - 1, w() / 2.f + 320.f - cos(a) * 120.f, h() - sin(a) * 120.f, -4.712f + a, 120.f))
				sel = 300 + i;
		}

		if (sel != selected) {
			selected = sel;
			redraw();
		}
		return 1;}
	case FL_RELEASE:
		pushed = false;
		if (selected != UINT_MAX) {
			do_callback();
			selected = UINT_MAX;
			redraw();
		}
		return 1;
	}

	return Fl_Gl_Window::handle(event);
}
