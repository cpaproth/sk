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

#include "GlImage.h"
#include <fltk/gl.h>
#include <opencv/cxcore.h>


using namespace SK;


GlImage::Guard::Guard(GlImage* glimg, cv::Mat* img) : glimg(glimg) {
	glimg->set(img);
}


GlImage::Guard::~Guard(void) {
	glimg->set(0);
}


GlImage::GlImage(int x, int y, int w ,int h, const char* l) : GlWindow(x, y, w, h, l) {
	img = 0;
}


void GlImage::set(cv::Mat* cvimg) {
	img = cvimg;
}


void GlImage::draw(void) {
	if (!valid()) {
		ortho();
		glRasterPos2i(0, h());
		glPixelZoom(1.f, -1.f);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	}

	if (img && img->size().area() > 0 && img->elemSize() == 3)
		glDrawPixels(img->cols, img->rows, GL_BGR_EXT, GL_UNSIGNED_BYTE, img->data);
	else
		glClear(GL_COLOR_BUFFER_BIT);
}
