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

#include "Video.h"
#include "Network.h"
#include "ui.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <iostream>
#include "Convenience.h"


using namespace SK;
using namespace CPLib;
using namespace cv;
using namespace boost;


Video::Video(UserInterface& ui, Network& nw) : ui(ui), network(nw) {
	capture = shared_ptr<VideoCapture>(new VideoCapture(0));

	if (!capture->isOpened() && !capture->open("webcam.avi"))
		throw runtime_error("open webcam failed");

	working = true;
	videothread = thread(bind(&Video::worker, this));

	cout << "video capture started" << endl;
}


Video::~Video(void) {
	try {
		working = false;
		videothread.join();

		cout << "video capture stopped" << endl;
	} catch (...) {}
}


unsigned char& Video::pixel(Mat& img, unsigned row, unsigned col, unsigned channel, bool vertical) {
	return vertical? img.at<Vec3b>(row, col)[channel]: img.at<Vec3b>(col, row)[channel];
}


void Video::deblock(Mat& img, unsigned c, bool v) {
	static const int smooththreshold = 80;
	static const int roughthreshold = 60;
	static const int smoothweights[] = {-8, -6, -4, -2, 3, 4, 6, 8};
	static const int roughweights[] = {0, 0, -8, -4, 5, 8, 0, 0};

	for (int i = 0; i < (v? img.rows: img.cols); i++) {
		for (int j = 4; j + 8 < (v? img.cols: img.rows); j += 8) {
			int leftdelta = (int)pixel(img, i, j + 3, c, v) - (int)pixel(img, i, j + 2, c, v);
			int rightdelta = (int)pixel(img, i, j + 4, c, v) - (int)pixel(img, i, j + 5, c, v);
			int delta = (int)pixel(img, i, j + 3, c, v) - (int)pixel(img, i, j + 4, c, v);

			if (abs(leftdelta) + abs(rightdelta) < 5 && abs(delta) < smooththreshold) {
				for (int k = 0; k < 8; k++) {
					int value = (int)pixel(img, i, j + k, c, v) + delta / smoothweights[k];
					between(value, 0, 255);
					pixel(img, i, j + k, c, v) = value;
				}
                        } else if (abs(delta) < roughthreshold) {
				for (int k = 2; k < 6; k++) {
					int value = (int)pixel(img, i, j + k, c, v) + delta / roughweights[k];
					between(value, 0, 255);
					pixel(img, i, j + k, c, v) = value;
				}
			}
		}
	}
}


void Video::deblock(Mat& img) {
	for (unsigned i = 0; i < (unsigned)img.channels(); i++) {
		deblock(img, i, true);
		deblock(img, i, false);
	}
}


void Video::worker(void) {
	Mat			img(imageheight, imagewidth, CV_8UC3);
	Mat			dst(imageheight, imagewidth, CV_8UC3);
	Mat			limg(imageheight, imagewidth, CV_8UC3);
	Mat			rimg(imageheight, imagewidth, CV_8UC3);
	vector<unsigned char>	buf;
	vector<vector<unsigned char> >	rbuf;
	vector<int>		params;

	params.push_back(CV_IMWRITE_JPEG_QUALITY);
	params.push_back(25);

	while (working) {
		//this_thread::sleep(posix_time::milliseconds(100));
		*capture >> img;
		if (img.size().area() == 0) {
			capture->open("webcam.avi");
			continue;
		}
		resize(img, dst, dst.size());
		imencode(".jpg", dst, buf, params);
		
		network.broadcast(buf, rbuf, maxlatency);
		
		if (rbuf.size() > 0 && rbuf[0].size() > 0) {
			limg = imdecode(Mat(rbuf[0]), 1);
			deblock(limg);
		}
		if (rbuf.size() > 1 && rbuf[1].size() > 0) {
			rimg = imdecode(Mat(rbuf[1]), 1);
			deblock(rimg);
		}
		fltk::lock();
		ui.midimage->set(&dst);
		ui.midimage->redraw();
		ui.leftimage->set(&limg);
		ui.leftimage->redraw();
		ui.rightimage->set(&rimg);
		ui.rightimage->redraw();
		fltk::awake();
		fltk::unlock();
	}
}
