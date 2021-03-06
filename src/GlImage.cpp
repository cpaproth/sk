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


#include "GlImage.h"
#include <FL/gl.h>
#include <opencv2/opencv.hpp>


using namespace SK;


GlImage::GlImage(int x, int y, int w ,int h, const char* l) : Fl_Gl_Window(x, y, w, h, l) {
	img = 0;
	mode = 0;
}


void GlImage::set(cv::Mat* cvimg) {
	img = cvimg;
}


void GlImage::set(const std::string& s) {
	str = s;
	redraw();
}


void GlImage::set() {
	mode = 1;
}


bool GlImage::get() {
	return mode == 3;
}


void GlImage::draw() {
	if (!valid()) {
		ortho();
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	glPixelZoom(1.f, -1.f);
	glRasterPos2i(0, h());
	if (img && img->size().area() > 0 && img->elemSize() == 3) {
		if (img->cols == w() && img->rows == h()) {
			glDrawPixels(img->cols, img->rows, GL_BGR_EXT, GL_UNSIGNED_BYTE, img->data);
		} else {
			cv::Mat res(h() * 4 / 3 < w()? w() * 3 / 4: h(), w() * 3 / 4 < h()? h() * 4 / 3: w(), CV_8UC3);
			cv::resize(*img, res, res.size());
			glBitmap(0, 0, 0.f, 0.f, -0.5f * (res.cols - w()), 0.5f * (res.rows - h()), 0);
			glDrawPixels(res.cols, res.rows, GL_BGR_EXT, GL_UNSIGNED_BYTE, res.data);
		}
	} else {
		glClear(GL_COLOR_BUFFER_BIT);
	}

	if (mode > 1) {
		glColor4f(0.f, 0.f, 0.f, 0.5f);
		glEnable(GL_BLEND);
		glRectf(0.f, 0.f, (float)w(), (float)h());
		glDisable(GL_BLEND);
		glColor3f(1.f, 1.f, 1.f);
		if (mode == 2) {
			glRectf(w() / 2.f - 15.f, h() / 2.f - 15.f, w() / 2.f - 5.f, h() / 2.f + 15.f);
			glRectf(w() / 2.f + 5.f, h() / 2.f - 15.f, w() / 2.f + 15.f, h() / 2.f + 15.f);
		} else {
			glBegin(GL_TRIANGLES);
			glVertex3f(w() / 2.f - 15.f, h() / 2.f - 15.f, 0.f);
			glVertex3f(w() / 2.f - 15.f, h() / 2.f + 15.f, 0.f);
			glVertex3f(w() / 2.f + 15.f, h() / 2.f, 0.f);
			glEnd();
		}
	}

	if (!str.empty()) {
		gl_font(FL_HELVETICA, 12);
		glColor4f(0.f, 0.f, 0.f, 0.3f);
		glEnable(GL_BLEND);
		glRectf(0.f, 10.f, (float)w(), 10.f + gl_height());
		glDisable(GL_BLEND);
		glColor3f(1.f, 1.f, 1.f);
		gl_draw(str.c_str(), std::max(0.f, (w() - (float)gl_width(str.c_str())) / 2.f), 10.f + gl_descent());
	}
}


int GlImage::handle(int event) {
	switch(event) {
	case FL_ENTER:
		if (mode == 1) {
			mode = 2;
			redraw();
			return 1;
		}
		break;
	case FL_LEAVE:
		if (mode == 2) {
			mode = 1;
			redraw();
			return 1;
		}
		break;
	case FL_PUSH:
		if (mode > 1) {
			mode = mode == 2? 3: 2;
			do_callback();
			redraw();
			return 1;
		}
		break;
	}

	return Fl_Gl_Window::handle(event);
}
