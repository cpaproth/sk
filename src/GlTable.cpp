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
	selected = 100;
}


void GlTable::set_cards(const vector<uchar>& c) {
	cards = c;
}


void GlTable::draw_card(unsigned c, unsigned r, float x, float y, float a, float sy) {
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

	//return cx > -sx && cx < sx && cy > -sy && cy < sy;
	return cx > -sx && cx < sx;
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

	float a = -4.f * 50.f / w() / w();
	float b = -a * w();
	for (unsigned i = 0; i < cards.size(); i++) {
		float x = 100 + i * 50.f;
		float angle = 0.3f - i * 0.06f;
		float sx = i != selected? 0.f: -50.f * sin(angle);
		float sy = i != selected? 0.f: 50.f * cos(angle);
		draw_card((cards[i] - 33) % 8, (cards[i] - 33) / 8, x + sx, a * x * x + b * x + sy, angle, 200.f);
	}
	
	draw_card(4, 2, 200.f, 330.f, rand() * 0.4f / RAND_MAX - 0.2f, 160.f);
	draw_card(3, 1, 440.f, 330.f, rand() * 0.4f / RAND_MAX - 0.2f, 160.f);
	draw_card(0, 0, 320.f, 300.f, rand() * 0.4f / RAND_MAX - 0.2f, 160.f);
	
	glEnd();
}


int GlTable::handle(int event) {
	using namespace fltk;
	
	float a = -4.f * 50.f / w() / w();
	float b = -a * w();
	unsigned sel = 100;

	switch(event) {
	case ENTER:
		return 1;
	case LEAVE:
		selected = 100;
		redraw();
		return 1;
	case MOVE:
		if (h() - event_y() - 1 < a * event_x() * event_x() + b * event_x() + 150.f)
			for (unsigned i = 0; i < cards.size(); i++) {
				float x = 100 + i * 50.f;
				float angle = 0.3f - i * 0.06f;
				float sx = i != selected? 0.f: -50.f * sin(angle);
				float sy = i != selected? 0.f: 50.f * cos(angle);
				if (inside_card(event_x(), h() - event_y() - 1, x + sx, a * x * x + b * x + sy, angle, 200.f))
					sel = i;
			}
		if (sel != selected) {
			selected = sel;
			redraw();
		}
		return 1;
	case PUSH:
		return 1;
	}

	return GlWindow::handle(event);
}
