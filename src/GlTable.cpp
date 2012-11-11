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


using namespace SK;


GlTable::GlTable(int x, int y, int w ,int h, const char* l) : GlWindow(x, y, w, h, l) {
	cv::Mat img = cv::imread("cards.png");
	if (!img.size().area())
		throw runtime_error("cards.png not found or invalid");
	width = img.cols;
	height = img.rows;
	mem.resize(width * height * 4);
	for (unsigned y = 0; y < height; y++)
		for (unsigned x = 0; x < width; x++) {
			unsigned mpos = (height - 1 - y) * width * 4 + x * 4;
			unsigned ipos = y * width * 3 + x * 3;
			if (img.data[ipos] == 255 && img.data[ipos + 1] == 0 && img.data[ipos + 2] == 255) {
				mem[mpos] = 128;
				mem[mpos + 1] = 128;
				mem[mpos + 2] = 128;
				mem[mpos + 3] = 0;
			} else {
				mem[mpos] = img.data[ipos + 2];
				mem[mpos + 1] = img.data[ipos + 1];
				mem[mpos + 2] = img.data[ipos];
				mem[mpos + 3] = 255;
			}
		}

	texture = 0;
	selected = 100;
}


void GlTable::drawCard(unsigned c, unsigned r, float x, float y, float a, float sy) {
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


bool GlTable::insideCard(int mx, int my, float x, float y, float a, float sy) {
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

	unsigned vals[] = {10, 29, 24, 7, 6, 5, 1, 0, 19, 3};

	glBegin(GL_QUADS);

	float a = -4.f * 50.f / w() / w();
	float b = -a * w();
	for (unsigned i = 0; i < sizeof(vals) / sizeof(unsigned); i++) {
		float x = 550 - i * 50.f;
		float angle = -0.3f + i * 0.06f;
		float sx = i != selected? 0.f: -50.f * sin(angle);
		float sy = i != selected? 0.f: 50.f * cos(angle);
		drawCard(vals[i] % 8, vals[i] / 8, x + sx, a * x * x + b * x + sy, angle, 200.f);
	}
	
	drawCard(4, 2, 200.f, 330.f, rand() * 0.4f / RAND_MAX - 0.2f, 160.f);
	drawCard(3, 1, 440.f, 330.f, rand() * 0.4f / RAND_MAX - 0.2f, 160.f);
	drawCard(0, 0, 320.f, 300.f, rand() * 0.4f / RAND_MAX - 0.2f, 160.f);
	
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
			for (unsigned i = 0; i < 10; i++) {
				float x = 550 - i * 50.f;
				float angle = -0.3f + i * 0.06f;
				float sx = i != selected? 0.f: -50.f * sin(angle);
				float sy = i != selected? 0.f: 50.f * cos(angle);
				if (insideCard(event_x(), h() - event_y() - 1, x + sx, a * x * x + b * x + sy, angle, 200.f))
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
