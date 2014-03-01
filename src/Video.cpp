/*Copyright (C) 2012-2014 Carsten Paproth

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
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "ui.h"
#include <iostream>
#include "Convenience.h"


using namespace SK;
using namespace CPLib;
using namespace cv;
using namespace boost;


Video::Video(UserInterface& ui, Network& nw) : ui(ui), network(nw) {
	left = 0;
	right = 1;

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

		cout << "stop video capture" << endl;

		working = false;
		videothread.join();

		cout << "video capture stopped" << endl;
	} catch (...) {}
}


void Video::send_chat(void) {
	network.command(left, "chat", ui.chat->value());
	network.command(right, "chat", ui.chat->value());
	ui.chat->value("");
	ui.leftimage->set("");
	ui.rightimage->set("");
}


bool Video::handle_command(unsigned i, const string& command, const string& data) {
	UILock lock;
	if (command == "name") {
		(i == left? ui.leftimage: ui.rightimage)->copy_tooltip(data.c_str());
		(i == left? ui.leftimage: ui.rightimage)->set(data);
	} else if (command == "seat") {
		left = data == "left"? 1: 0;
		right = data == "right"? 1: 0;
	} else if (command == "chat") {
		(i == left? ui.leftimage: ui.rightimage)->set(data);
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


void Video::encode(const Mat& img, vector<unsigned char>& enc) {
	unsigned ratio = 16;
	enc.clear();
	if (img.cols != (int)imagewidth || img.rows != (int)imageheight || img.elemSize() != 3)
		return;

	vector<float> Y, U, V, W, Z;

	for (unsigned y = 0; y < imageheight; y++) {
		for (unsigned x = 0; x < imagewidth; x++) {
			Y.push_back(0.3f * (float)img.at<Vec3b>(y, x)[2] + 0.5f * (float)img.at<Vec3b>(y, x)[1] + 0.2f * (float)img.at<Vec3b>(y, x)[0]);
			U.push_back(Y.back() - (float)img.at<Vec3b>(y, x)[2]);
			V.push_back(Y.back() - (float)img.at<Vec3b>(y, x)[0]);
		}
	}

	W.resize(Y.size());
	float q = 0.4f * 3.5f * 3.5f * 3.5f;
	for (unsigned w = imagewidth / 2, h = imageheight / 2; w >= imagewidth / ratio; w /= 2, h /= 2, q /= 3.5f) {
		for (unsigned y = 0; y < h; y++) {
			for (unsigned x = 0; x < w; x++) {
				unsigned pw = y * w + x, py = 4 * y * w + 2 * x, s = w * h;
				float x00 = Y[py] / 4.f, x10 = Y[py + 1] / 4.f, x01 = Y[py + 2 * w] / 4.f, x11 = Y[py + 2 * w + 1] / 4.f;
				W[pw] = x00 + x10 + x01 + x11;
				W[pw + s] = (x00 + x10 - x01 - x11) / q;
				W[pw + 2 * s] = (x00 - x10 + x01 - x11) / q;
				W[pw + 3 * s] = (x00 - x10 - x01 + x11) / q;
			}
		}
		if (w == imagewidth / 2)
			Y.swap(W);
		else
			copy(W.begin(), W.begin() + 4 * w * h, Y.begin());
	}

	W.clear();
	for (unsigned y = 0; y + ratio < imageheight + 1; y += ratio) {
		for (unsigned x = 0; x + ratio < imagewidth + 1; x += ratio) {
			W.push_back(0.f);
			Z.push_back(0.f);
			for (unsigned j = 0; j < ratio; j++) {
				for (unsigned i = 0; i < ratio; i++) {
					W.back() += U[(y + j) * imagewidth + x + i];
					Z.back() += V[(y + j) * imagewidth + x + i];
				}
			}
		}
	}


	vector<int> data;
	for (unsigned i = 0; i < W.size(); i++)
		data.push_back(roundtoeven(W[i] / ratio / ratio / 4.f));
	for (unsigned i = 0; i < Z.size(); i++)
		data.push_back(roundtoeven(Z[i] / ratio / ratio / 4.f));
	for (unsigned i = 0; i < Y.size(); i++)
		data.push_back(roundtoeven(Y[i] / 2.5f));
	for (unsigned i = 3 * imagewidth * imageheight / ratio / ratio - 1; i > 0; i--)
		data[i] -= data[i - 1];

	unsigned size = 0;
	for (unsigned i = 0; i < data.size(); i++) {
		if (data[i] == 0) {
			int z = 2;
			for (; i + 1 < data.size() && data[i + 1] == 0; i++)
				z++;
			for (; z >= 2; z >>= 1)
				data[size++] = (z & 1) * 127;
		} else if (data[i] > -128 && data[i] < 127) {
			data[size++] = data[i];
		} else
			cout << "out of range " << i << " " << data[i] << endl;
	}
	data.resize(size);


	vector<unsigned> mem1(256, 0), mem2(256, 0);
	unsigned *freqs = &mem1[128], *starts = &mem2[128];
	for (unsigned i = 0; i < size; i++)
		freqs[data[i]]++;
	for (int i = 0; starts[i] < size; i = i < 0? -i: -i - 1)
		starts[i < 0? -i: -i - 1] = starts[i] + freqs[i];
	for (unsigned i = 0; i < mem1.size(); i++) {
		if (mem1[i] == 0) {
			enc.push_back(0);
			for (; i + 1 < mem1.size() && mem1[i + 1] == 0 && enc.back() < 63; i++)
				enc.back()++;
		} else if (mem1[i] <= 128) {
			enc.push_back((mem1[i] - 1) | 128);
		} else {
			enc.push_back((((mem1[i] - 129) >> 8) & 63) | 64);
			enc.push_back((mem1[i] - 129) & 255);
		}
	}


	unsigned low = 0, range = UINT_MAX;
	for (unsigned i = 0; i < size; i++) {
		range /= size;
		low += range * starts[data[i]];
		range *= freqs[data[i]];

		while (range < 400000) {
			enc.push_back(low >> 24);
			low <<= 8;
			range <<= 8;
			if (UINT_MAX - low < range)
				range = UINT_MAX - low;
		}
	}
	enc.push_back(low >> 24); enc.push_back((low >> 16) & 255); enc.push_back((low >> 8) & 255); enc.push_back(low & 255);
}


void Video::decode(const vector<unsigned char>& enc, Mat& img) {
	unsigned ratio = 16;
	if (img.cols != (int)imagewidth || img.rows != (int)imageheight || img.elemSize() != 3)
		return;
	if (enc.size() < 4)
		return;

	vector<unsigned> mem1, mem2(256, 0);
	unsigned size = 0, p = 0;
	for (; p < enc.size() && mem1.size() < 256; size += mem1.back(), p++) {
		if ((enc[p] & 192) == 0) {
			for (unsigned i = enc[p] + 1; i > 0; i--)
				mem1.push_back(0);
		} else if ((enc[p] & 128) == 128) {
			mem1.push_back((enc[p] & 127) + 1);
		} else {
			mem1.push_back((enc[p++] & 63) << 8);
			mem1.back() += enc[p] + 129;
		}
	}
	if (mem1.size() < 256)
		return;
	unsigned *freqs = &mem1[128], *starts = &mem2[128];
	for (int i = 0; starts[i] < size; i = i < 0? -i: -i - 1)
		starts[i < 0? -i: -i - 1] = starts[i] + freqs[i];


	map<unsigned, int> ranges;
	for (int i = 0; ranges.size() == 0 || ranges.rbegin()->first < size; i = i < 0? -i: -i - 1)
		if (freqs[i] != 0) ranges[starts[i] + freqs[i]] = i;
	vector<int> dec, data;
	unsigned low = 0, range = UINT_MAX;
	unsigned code = (enc[p] << 24) | (enc[p + 1] << 16) | (enc[p + 2] << 8) | enc[p + 3];
	for (p += 4; dec.size() < size && range >= 400000;) {
		range /= size;
		dec.push_back(ranges.upper_bound((code - low) / range % size)->second);
		low += range * starts[dec.back()];
		range *= freqs[dec.back()];

		while (range < 400000) {
			if (p >= enc.size())
				break;

			code = (code << 8) | enc[p++];
			low <<= 8;
			range <<= 8;
			if (UINT_MAX - low < range)
				range = UINT_MAX - low;
		}
	}


	for (unsigned i = 0; i < dec.size(); i++) {
		if (dec[i] == 0 || dec[i] == 127) {
			int z = dec[i] & 1, b = 1;
			for (; i + 1 < dec.size() && (dec[i + 1] == 0 || dec[i + 1] == 127); i++, b++)
				z |= (dec[i + 1] & 1) << b;
			for (z += (1 << b) - 2; z >= 0; z--)
				data.push_back(0);
		} else {
			data.push_back(dec[i]);
		}
	}
	if (data.size() < imagewidth * imageheight + 2 * imagewidth * imageheight / ratio / ratio)
		return;


	vector<float> Y, U, V, W;
	for (unsigned i = 1; i < 3 * imagewidth * imageheight / ratio / ratio; i++)
		data[i] += data[i - 1];
	for (unsigned i = 0; i < imagewidth * imageheight / ratio / ratio; i++)
		U.push_back(4.f *  data[i]);
	for (unsigned i = imagewidth * imageheight / ratio / ratio; i < 2 * imagewidth * imageheight / ratio / ratio; i++)
		V.push_back(4.f * data[i]);
	for (unsigned i = 2 * imagewidth * imageheight / ratio / ratio; i < data.size(); i++)
		Y.push_back(2.5f * data[i]);


	W.resize(Y.size());
	float q = 0.4f;
	for (unsigned w = imagewidth / ratio, h = imageheight / ratio; w <= imagewidth / 2; w *= 2, h *= 2, q *= 3.5f) {
		for (unsigned y = 0; y < h; y++) {
			for (unsigned x = 0; x < w; x++) {
				unsigned py = y * w + x, pw = 4 * y * w + 2 * x, s = w * h;
				float x00 = Y[py], x10 = Y[py + s] * q, x01 = Y[py + 2 * s] * q, x11 = Y[py + 3 * s] * q;
				if (x10 == 0.f && x01 == 0.f && x11 == 0.f) {
					float cx = x > 0 && fabs(Y[py - 1] - x00) < 30.f? Y[py - 1]: x00;
					float cy = y > 0 && fabs(Y[py - w] - x00) < 30.f? Y[py - w]: x00;
					float cw = x + 1 < w && fabs(Y[py + 1] - x00) < 30.f? Y[py + 1]: x00;
					float ch = y + 1 < h && fabs(Y[py + w] - x00) < 30.f? Y[py + w]: x00;
					W[pw] = 0.6f * x00 + 0.2f * cx + 0.2f * cy;
					W[pw + 1] = 0.6f * x00 + 0.2f * cw + 0.2f * cy;
					W[pw + 2 * w] = 0.6f * x00 + 0.2f * cx + 0.2f * ch;
					W[pw + 2 * w + 1] = 0.6f * x00 + 0.2f * cw + 0.2f * ch;
				} else {
					W[pw] = x00 + x10 + x01 + x11;
					W[pw + 1] = x00 + x10 - x01 - x11;
					W[pw + 2 * w] = x00 - x10 + x01 - x11;
					W[pw + 2 * w + 1] = x00 - x10 - x01 + x11;
				}
			}
		}
		if (w == imagewidth / 2)
			Y.swap(W);
		else
			copy(W.begin(), W.begin() + 4 * w * h, Y.begin());
	}

	for (unsigned y = 0; y < imageheight; y++) {
		for (unsigned x = 0; x < imagewidth; x++) {
			unsigned mx = x < ratio / 2? 0: (x - ratio / 2) / ratio;
			unsigned my = y < ratio / 2? 0: (y - ratio / 2) / ratio;
			float wx = x < ratio / 2? 0.f: (0.5f + (x - ratio / 2) % ratio) / ratio;
			float wy = y < ratio / 2? 0.f: (0.5f + (y - ratio / 2) % ratio) / ratio;
			unsigned w = imagewidth / ratio, h = imageheight / ratio, py = y * imagewidth + x, p = my * w + mx;
			float u = (1.f - wx) * (1.f - wy) * U[p] + wx * (1.f - wy) * U[mx + 1 < w? p + 1: p] + (1.f - wx) * wy * U[my + 1 < h? p + w: p] + wx * wy * U[mx + 1 < w && my + 1 < h? p + w + 1: p];
			float v = (1.f - wx) * (1.f - wy) * V[p] + wx * (1.f - wy) * V[mx + 1 < w? p + 1: p] + (1.f - wx) * wy * V[my + 1 < h? p + w: p] + wx * wy * V[mx + 1 < w && my + 1 < h? p + w + 1: p];
			float l = 0.5f * Y[py] + 0.125f * Y[x > 0? py - 1: py] + 0.125f * Y[x + 1 < imagewidth? py + 1: py] + 0.125f * Y[y > 0? py - imagewidth: py] + 0.125f * Y[y + 1 < imageheight? py + imagewidth: py];
			//float r = Y[py] - u, g = Y[py] + 0.6f * u + 0.4f * v, b = Y[py] - v;
			float r = l - u, g = l + 0.6f * u + 0.4f * v, b = l - v;
			
			between(r, 0.f, 255.f);
			between(g, 0.f, 255.f);
			between(b, 0.f, 255.f);
			img.at<Vec3b>(y, x)[0] = (unsigned char)b;
			img.at<Vec3b>(y, x)[1] = (unsigned char)g;
			img.at<Vec3b>(y, x)[2] = (unsigned char)r;
		}
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

//capture->open("webcam.avi");
		while (working) {
			this_thread::sleep(posix_time::milliseconds(10));
			*capture >> cap;

			if (cap.size().area() == 0) {
				capture->open("webcam.avi");
				*capture >> cap;
			}

			if (cap.size().area() == 0) {
				throw runtime_error("empty captured image");
			} else {
				UILock lock;
				resize(cap, *img, img->size());
				ui.midimage->redraw();
			}
			//imencode(".jpg", *img, encbuf, params);
			encode(*img, encbuf);
			network.broadcast(encbuf, decbuf, maxlatency);
			
			
			UILock lock;
			decode(encbuf, *limg);
			ui.leftimage->redraw();
			size_t tmp = encbuf.size();
			imencode(".jpg", *img, encbuf, params);
			*rimg = imdecode(Mat(encbuf), 1);
			deblock(*rimg);
			//if (tmp > encbuf.size() || tmp < encbuf.size() / 2)
				cout << "jpg " << encbuf.size() << " wav " << tmp << endl;
			ui.rightimage->redraw();
			
			if (decbuf.size() > left && decbuf[left].size() > 0) {
				//*limg = imdecode(Mat(decbuf[left]), 1);
				//deblock(*limg);
				decode(decbuf[left], *limg);
				ui.leftimage->redraw();
			}
			if (decbuf.size() > right && decbuf[right].size() > 0) {
				//*rimg = imdecode(Mat(decbuf[right]), 1);
				//deblock(*rimg);
				decode(decbuf[right], *rimg);
				ui.rightimage->redraw();
			}
			Fl::awake();
		}
	} catch (std::exception& e) {
		cout << "video failure: " << e.what() << endl;
	}
}
