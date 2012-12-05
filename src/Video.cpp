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
	leftname[0] = 0;
	rightname[0] = 0;
	left = 0;
	right = 1;

	ui.leftimage->tooltip(leftname);
	ui.rightimage->tooltip(rightname);

	capture = shared_ptr<VideoCapture>(new VideoCapture(0));

	if (!capture->isOpened() && !capture->open("webcam.avi"))
		throw runtime_error("open webcam failed");
	
	working = true;
	videothread = thread(bind(&Video::worker, this));

	cout << "video capture started" << endl;
	network.add_handler(bind(&Video::handle_command, this, _1, _2, _3));
}


Video::~Video(void) {
	try {
		network.remove_handler();
		ui.midimage->set(0);
		ui.leftimage->set(0);
		ui.rightimage->set(0);
		ui.leftimage->tooltip(0);
		ui.rightimage->tooltip(0);

		cout << "stop video capture" << endl;

		working = false;
		videothread.join();

		cout << "video capture stopped" << endl;
	} catch (...) {}
}


bool Video::handle_command(unsigned i, const string& command, const string& data) {
	if (command == "name") {
		UILock lock;
		if (i == left)
			leftname[("@b;" + data).copy(leftname, namesize - 1)] = 0;
		else if (i == right)
			rightname[("@b;" + data).copy(rightname, namesize - 1)] = 0;
	} else if (command == "seat") {
		left = data == "left"? 1: 0;
		right = data == "right"? 1: 0;
	} else
		return false;
	return true;
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
	try {
		Mat				cap(imageheight, imagewidth, CV_8UC3);
		vector<unsigned char>		encbuf;
		vector<vector<unsigned char> >	decbuf;
		vector<int>			params;

		img = shared_ptr<Mat>(new Mat(imageheight, imagewidth, CV_8UC3, Scalar()));
		limg = shared_ptr<Mat>(new Mat(imageheight, imagewidth, CV_8UC3, Scalar()));
		rimg = shared_ptr<Mat>(new Mat(imageheight, imagewidth, CV_8UC3, Scalar()));
		ui.midimage->set(img.get());
		ui.leftimage->set(limg.get());
		ui.rightimage->set(rimg.get());

		params.push_back(CV_IMWRITE_JPEG_QUALITY);
		params.push_back(25);

		while (working) {
			*capture >> cap;

			if (cap.size().area() == 0) {
				capture->open("webcam.avi");
				*capture >> cap;
			}

			if (cap.size().area() == 0)
				throw runtime_error("empty captured image");
			else {
				UILock lock;
				resize(cap, *img, img->size());
				ui.midimage->redraw();
			}

			imencode(".jpg", *img, encbuf, params);
			network.broadcast(encbuf, decbuf, maxlatency);
			
			if (decbuf.size() > left && decbuf[left].size() > 0) {
				UILock lock;
				*limg = imdecode(Mat(decbuf[left]), 1);
				deblock(*limg);
				ui.leftimage->redraw();
			}
			if (decbuf.size() > right && decbuf[right].size() > 0) {
				UILock lock;
				*rimg = imdecode(Mat(decbuf[right]), 1);
				deblock(*rimg);
				ui.rightimage->redraw();
			}
			
			fltk::awake();
		}
	} catch (std::exception& e) {
		cout << "video failure: " << e.what() << endl;
	}
}
