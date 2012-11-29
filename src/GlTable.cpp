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

#include "GlTable.h"
#include <fltk/gl.h>
#include <fltk/events.h>
#include <opencv/highgui.h>
#include <stdexcept>
#include <fltk/xpmImage.h>
#include "../images/cards.xpm"


using namespace SK;


GlTable::GlTable(int x, int y, int w ,int h, const char* l) : GlWindow(x, y, w, h, l) {
	uchar* data;
	unsigned depth;
	cv::Mat pngimg = cv::imread("cards.png");
	fltk::xpmImage xpmimg(cards_xpm);

	if (pngimg.size().area() != 0) {
		width = pngimg.cols;
		height = pngimg.rows;
		data = pngimg.data;
		depth = pngimg.elemSize();
	} else {
		xpmimg.fetch();
		width = xpmimg.buffer_width();
		height = xpmimg.buffer_height();
		data = xpmimg.buffer();
		depth = xpmimg.buffer_depth();
	}

	if (width == 0 || height == 0 || depth < 3)
		throw runtime_error("loading card images failed");

	mem.resize(width * height * 4);
	for (unsigned y = 0; y < height; y++)
		for (unsigned x = 0; x < width; x++) {
			unsigned mpos = (height - 1 - y) * width * 4 + x * 4;
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

	texture = 0;
	selected = UINT_MAX;
	mx = -1;
	my = -1;
	pushed = false;
}


void GlTable::show_cards(const vector<uchar>& h, const vector<uchar>& s) {
	hand = h;
	skat = s;
	selected = UINT_MAX;
	redraw();
}


void GlTable::show_trick(const vector<uchar>& t, unsigned s) {
	trick = t;
	start = s;
	redraw();
}


void GlTable::show_disclosed(const vector<uchar>& l, const vector<uchar>& r) {
	lefthand = l;
	righthand = r;
	redraw();
}


unsigned GlTable::selection(void) {
	return selected;
}


void GlTable::get_position(unsigned i, size_t s, float& x, float& y, float& a) {
	x = w() / 2.f + (0.5f + i - s / 2.f) * (90.f - 4.f * s);
	y = -4.f * 45.f / w() / w() * x * (x - w());
	a = -(0.5f + i - s / 2.f) * 0.06f;
	
	x += pushed && i == selected? -50.f * sin(a): 0.f;
	y += pushed && i == selected? 50.f * cos(a): 0.f;
}


void GlTable::draw_card(uchar card, float x, float y, float a, float sy) {
	unsigned c = card % 8;
	unsigned r = card / 8;
	float sx = sy / height * width / 4.f;
	sy /= 2.f;

	glTexCoord2f(c * 0.125f, r * 0.25f);
	glVertex2f(x - cos(a) * sx + sin(a) * sy, y - sin(a) * sx - cos(a) * sy);

	glTexCoord2f((c + 1) * 0.125f, r * 0.25f);
	glVertex2f(x + cos(a) * sx + sin(a) * sy, y + sin(a) * sx - cos(a) * sy);

	glTexCoord2f((c + 1) * 0.125f, (r + 1) * 0.25f);
	glVertex2f(x + cos(a) * sx - sin(a) * sy, y + sin(a) * sx + cos(a) * sy);

	glTexCoord2f(c * 0.125f, (r + 1) * 0.25f);
	glVertex2f(x - cos(a) * sx - sin(a) * sy, y - sin(a) * sx + cos(a) * sy);
}


bool GlTable::inside_card(int mx, int my, float x, float y, float a, float sy) {
	float sx = sy / height * width / 4.f;
	sy /= 2.f;

	float cx = cos(a) * (mx - x) + sin(a) * (my - y);
	float cy = -sin(a) * (mx - x) + cos(a) * (my - y);

	return cx > -sx && cx < sx && cy > -sy && cy < sy;
}


void GlTable::draw(void) {
	if (!valid()) {
		ortho();
		glDeleteTextures(1, &texture);
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);		
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &mem[0]);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glClearColor(0.831f, 0.816f, 0.784f, 1.f);
	}

	glClear(GL_COLOR_BUFFER_BIT);


	glBegin(GL_QUADS);

	for (unsigned i = 0; i < hand.size(); i++) {
		float x, y, a;
		get_position(i, hand.size(), x, y, a);
		draw_card(hand[i], x, y, a, 200.f);
	}

	if (skat.size() > 0 && skat[0] < 32)
		draw_card(skat[0], 250.f, 300.f, 0.1f, pushed && selected == 100? 200.f: 160.f);
	if (skat.size() > 1 && skat[1] < 32)
		draw_card(skat[1], 390.f, 300.f, -0.1f, pushed && selected == 101? 200.f: 160.f);
	
	if (selected == UINT_MAX || selected < 200 || !pushed) {
		for (unsigned i = 0; i < trick.size(); i++) {
			if ((i + start) % 3 == 0)
				draw_card(trick[i], 330.f, 290.f, 0.05f, 160.f);
			else if ((i + start) % 3 == 1)
				draw_card(trick[i], 270.f, 320.f, 0.2f, 160.f);
			else 
				draw_card(trick[i], 370.f, 330.f, -0.25f, 160.f);
		}

		for (unsigned i = 0; i < lefthand.size(); i++) {
			float a = 0.785f + (0.5f + i - lefthand.size() / 2.f) * 0.13f;
			draw_card(lefthand[i], sin(a) * 120.f, h() - cos(a) * 120.f, -3.142f + a, 120.f);
		}
		for (unsigned i = 0; i < righthand.size(); i++) {
			float a = 0.785f + (0.5f + i - righthand.size() / 2.f) * 0.13f;
			draw_card(righthand[i], w() - cos(a) * 120.f, h() - sin(a) * 120.f, -4.712f + a, 120.f);
		}

	} else {
		vector<uchar>& cards = selected < 300? lefthand: righthand;
		for (unsigned i = 0; i < cards.size(); i++) {
			float x, y, a;
			get_position(i, cards.size(), x, y, a);
			draw_card(cards[i], x, h() - 80.f - y, -a, 160.f);
		}
	}
	
	glEnd();
}


int GlTable::handle(int event) {
	using namespace fltk;
	
	unsigned sel = UINT_MAX;

	switch(event) {
	case ENTER:
		return 1;
	case LEAVE:
		selected = UINT_MAX;
		redraw();
		return 1;
	case PUSH:
		pushed = true;
		redraw();
	case DRAG:
	case MOVE:
		mx = event_x();
		my = event_y();

		for (unsigned i = 0; i < hand.size(); i++) {
			float x, y, a;
			get_position(i, hand.size(), x, y, a);
			if (inside_card(mx, h() - my - 1, x, y, a, 200.f))
				sel = i;
		}

		if (skat.size() > 0 && skat[0] < 32 && inside_card(mx, h() - my - 1, 250.f, 300.f, 0.1f, pushed && selected == 100? 200.f: 160.f))
			sel = 100;
		if (skat.size() > 1 && skat[1] < 32 && inside_card(mx, h() - my - 1, 390.f, 300.f, -0.1f, pushed && selected == 101? 200.f: 160.f))
			sel = 101;

		for (unsigned i = 0; i < lefthand.size(); i++) {
			float a = 0.785f + (0.5f + i - lefthand.size() / 2.f) * 0.13f;
			if (inside_card(mx, h() - my - 1, sin(a) * 120.f, h() - cos(a) * 120.f, -3.142f + a, 120.f))
				sel = 200 + i;
		}
		for (unsigned i = 0; i < righthand.size(); i++) {
			float a = 0.785f + (0.5f + i - righthand.size() / 2.f) * 0.13f;
			if (inside_card(mx, h() - my - 1, w() - cos(a) * 120.f, h() - sin(a) * 120.f, -4.712f + a, 120.f))
				sel = 300 + i;
		}

		if (sel != selected) {
			selected = sel;
			redraw();
		}
		return 1;
	case RELEASE:
		pushed = false;
		if (selected != UINT_MAX) {
			do_callback();
			selected = UINT_MAX;
			redraw();
		}
		return 1;
	}

	return GlWindow::handle(event);
}
