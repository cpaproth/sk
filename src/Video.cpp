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


void Video::encode(const Mat& img, vector<unsigned char>& enc) {
	if (img.cols != (int)imagewidth || img.rows != (int)imageheight || img.elemSize() != 3)
		return;

	enc.assign(1, 128);

	vector<float> Y, U, V, W, Z;
	for (unsigned y = 0; y < imageheight; y++) {
		for (unsigned x = 0; x < imagewidth; x++) {
			Y.push_back(0.3f * (float)img.at<Vec3b>(y, x)[2] + 0.5f * (float)img.at<Vec3b>(y, x)[1] + 0.2f * (float)img.at<Vec3b>(y, x)[0]);
			U.push_back(Y.back() - (float)img.at<Vec3b>(y, x)[2]);
			V.push_back(Y.back() - (float)img.at<Vec3b>(y, x)[0]);
		}
	}

	W.resize(Y.size());
	for (unsigned w = imagewidth / 2, h = imageheight / 2; w * h >= minsize; w /= 2, h /= 2) {
		for (unsigned y = 0; y < h; y++) {
			for (unsigned x = 0; x < w; x++) {
				unsigned pw = y * w + x, py = 4 * y * w + 2 * x, s = w * h;
				float x00 = Y[py] / 4.f, x10 = Y[py + 1] / 4.f, x01 = Y[py + 2 * w] / 4.f, x11 = Y[py + 2 * w + 1] / 4.f;
				W[pw] = x00 + x10 + x01 + x11;
				W[pw + s] = x00 + x10 - x01 - x11;
				W[pw + 2 * s] = x00 - x10 + x01 - x11;
				W[pw + 3 * s] = x00 - x10 - x01 + x11;
			}
		}
		if (w == imagewidth / 2)
			Y.swap(W);
		else
			copy(W.begin(), W.begin() + 4 * w * h, Y.begin());
	}

	W.clear();
	for (unsigned y = 0; y + 7 < imageheight; y += 8) {
		for (unsigned x = 0; x + 7 < imagewidth; x += 8) {
			W.push_back(0.f);
			Z.push_back(0.f);
			for (unsigned j = 0; j < 8; j++) {
				for (unsigned i = 0; i < 8; i++) {
					W.back() += U[(y + j) * imagewidth + x + i];
					Z.back() += V[(y + j) * imagewidth + x + i];
				}
			}
		}
	}


	vector<int> data;
	for (unsigned i = 0; i < W.size(); i++)
		data.push_back((int)(W[i] / 256.f));
	for (unsigned i = 0; i < Z.size(); i++)
		data.push_back((int)(Z[i] / 256.f));
	for (unsigned i = 0; i < Y.size(); i++)
		data.push_back((int)(Y[i] / (i < minsize? 2.f: i < 4 * minsize? 1.f: i < 16 * minsize? 2.f: i < 64 * minsize? 4.f: 10.f)));
	for (unsigned i = 9 * minsize - 1; i > 0; i--)
		data[i] -= data[i - 1];

	unsigned size = 0;
	for (unsigned i = 0; i < data.size(); i++) {
		if (data[i] == 0) {
			int z = 2;
			for (; i + 1 < data.size() && data[i + 1] == 0; i++)
				z++;
			for (; z >= 2; z >>= 1)
				data[size++] = (z & 1) * 139;
		} else if (data[i] > -140 && data[i] < 139) {
			data[size++] = data[i];
		} else
			return;
	}
	data.resize(size);


	vector<unsigned> mem1(280, 0), mem2(280, 0);
	unsigned *freqs = &mem1[mem1.size() / 2], *starts = &mem2[mem2.size() / 2];
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
	if (img.cols != (int)imagewidth || img.rows != (int)imageheight || img.elemSize() != 3)
		return;

	vector<unsigned> mem1, mem2(280, 0);
	unsigned size = 0, p = 1;
	for (; p + 4 < enc.size() && mem1.size() < mem2.size(); size += mem1.back(), p++) {
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
	if (mem1.size() != mem2.size() || mem1[0] != 0)
		return;
	unsigned *freqs = &mem1[mem1.size() / 2], *starts = &mem2[mem2.size() / 2];
	for (int i = 0; starts[i] < size; i = i < 0? -i: -i - 1)
		starts[i < 0? -i: -i - 1] = starts[i] + freqs[i];


	map<unsigned, int> ranges;
	for (int i = 0; ranges.size() == 0 || ranges.rbegin()->first < size; i = i < 0? -i: -i - 1) {
		if (freqs[i] != 0)
			ranges[starts[i] + freqs[i]] = i;
	}
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
		if (dec[i] == 0 || dec[i] == 139) {
			int z = dec[i] & 1, b = 1;
			for (; i + 1 < dec.size() && (dec[i + 1] == 0 || dec[i + 1] == 139); i++, b++)
				z |= (dec[i + 1] & 1) << b;
			for (z += (1 << b) - 2; z >= 0; z--)
				data.push_back(0);
		} else {
			data.push_back(dec[i]);
		}
	}
	if (data.size() < 264 * minsize)
		return;


	vector<float> Y, U, V, W;
	for (unsigned i = 1; i < 9 * minsize; i++)
		data[i] += data[i - 1];
	for (unsigned i = 0; i < 4 * minsize; i++)
		U.push_back(4.f * (data[i] + (data[i] < 0? -0.5f: data[i] > 0? 0.5f: 0.f)));
	for (unsigned i = 4 * minsize; i < 8 * minsize; i++)
		V.push_back(4.f * (data[i] + (data[i] < 0? -0.5f: data[i] > 0? 0.5f: 0.f)));
	for (unsigned i = 8 * minsize; i < data.size(); i++)
		Y.push_back((i < 9 * minsize? 2.f: i < 12 * minsize? 1.f: i < 24 * minsize? 2.f: i < 72 * minsize? 4.f: 10.f) * (data[i] + (data[i] < 0? -0.5f: data[i] > 0? 0.5f: 0.f)));


	W.resize(Y.size());
	for (unsigned w = imagewidth / 16, h = imageheight / 16; w <= imagewidth / 2; w *= 2, h *= 2) {
		for (unsigned y = 0; y < h; y++) {
			for (unsigned x = 0; x < w; x++) {
				unsigned py = y * w + x, pw = 4 * y * w + 2 * x, s = w * h;
				float x00 = Y[py], x10 = Y[py + s], x01 = Y[py + 2 * s], x11 = Y[py + 3 * s];
				W[pw] = x00 + x10 + x01 + x11;
				W[pw + 1] = x00 + x10 - x01 - x11;
				W[pw + 2 * w] = x00 - x10 + x01 - x11;
				W[pw + 2 * w + 1] = x00 - x10 - x01 + x11;
			}
		}
		copy(W.begin(), W.begin() + 4 * w * h, Y.begin());
		float th = w == imagewidth / 2? 500.f: 25.f, w0 = w == imagewidth / 2? 0.5625f: 0.8f, w1 = sqrt(w0) - w0, w2 = 1.f - 2.f * sqrt(w0) + w0;
		for (unsigned y = 0; y < h; y++) {
			for (unsigned x = 0; x < w; x++) {
				unsigned wi = 2 * w, pw = 2 * y * wi + 2 * x;
				if (W[pw] != W[pw + 1] || W[pw] != W[pw + wi] || W[pw] != W[pw + wi + 1])
					continue;
				Y[pw] = Y[pw + 1] = Y[pw + wi] = Y[pw + wi + 1] = w0 * W[pw];
				Y[pw] += w1 * W[x > 0 && fabs(W[pw - 1] - W[pw]) < th? pw - 1: pw];
				Y[pw] += w1 * W[y > 0 && fabs(W[pw - wi] - W[pw]) < th? pw - wi: pw];
				Y[pw] += w2 * W[x > 0 && y > 0 && fabs(W[pw - wi - 1] - W[pw]) < th? pw - wi - 1: pw];
				Y[pw + 1] += w1 * W[x + 1 < w && fabs(W[pw + 2] - W[pw]) < th? pw + 2: pw];
				Y[pw + 1] += w1 * W[y > 0 && fabs(W[pw - wi + 1] - W[pw]) < th? pw - wi + 1: pw];
				Y[pw + 1] += w2 * W[x + 1 < w && y > 0 && fabs(W[pw - wi + 2] - W[pw]) < th? pw - wi + 2: pw];
				Y[pw + wi] += w1 * W[x > 0 && fabs(W[pw + wi - 1] - W[pw]) < th? pw + wi - 1: pw];
				Y[pw + wi] += w1 * W[y + 1 < h && fabs(W[pw + 2 * wi] - W[pw]) < th? pw + 2 * wi: pw];
				Y[pw + wi] += w2 * W[x > 0 && y + 1 < h && fabs(W[pw + 2 * wi - 1] - W[pw]) < th? pw + 2 * wi - 1: pw];
				Y[pw + wi + 1] += w1 * W[x + 1 < w && fabs(W[pw + wi + 2] - W[pw]) < th? pw + wi + 2: pw];
				Y[pw + wi + 1] += w1 * W[y + 1 < h && fabs(W[pw + 2 * wi + 1] - W[pw]) < th? pw + 2 * wi + 1: pw];
				Y[pw + wi + 1] += w2 * W[x + 1 < w && y + 1 < h && fabs(W[pw + 2 * wi + 2] - W[pw]) < th? pw + 2 * wi + 2: pw];
			}
		}
	}

	for (unsigned y = 0; y < imageheight; y++) {
		for (unsigned x = 0; x < imagewidth; x++) {
			unsigned mx = x < 4? 0: (x - 4) / 8;
			unsigned my = y < 4? 0: (y - 4) / 8;
			float wx = x < 4? 0.f: (0.5f + (x - 4) % 8) / 8;
			float wy = y < 4? 0.f: (0.5f + (y - 4) % 8) / 8;
			unsigned w = imagewidth / 8, h = imageheight / 8, py = y * imagewidth + x, p = my * w + mx;
			float u = (1.f - wx) * (1.f - wy) * U[p] + wx * (1.f - wy) * U[mx + 1 < w? p + 1: p] + (1.f - wx) * wy * U[my + 1 < h? p + w: p] + wx * wy * U[mx + 1 < w && my + 1 < h? p + w + 1: p];
			float v = (1.f - wx) * (1.f - wy) * V[p] + wx * (1.f - wy) * V[mx + 1 < w? p + 1: p] + (1.f - wx) * wy * V[my + 1 < h? p + w: p] + wx * wy * V[mx + 1 < w && my + 1 < h? p + w + 1: p];
			float r = Y[py] - u, g = Y[py] + 0.6f * u + 0.4f * v, b = Y[py] - v;
			
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

		img = shared_ptr<Mat>(new Mat(imageheight, imagewidth, CV_8UC3, Scalar()));
		limg = shared_ptr<Mat>(new Mat(imageheight, imagewidth, CV_8UC3, Scalar()));
		rimg = shared_ptr<Mat>(new Mat(imageheight, imagewidth, CV_8UC3, Scalar()));
		ui.midimage->set(img.get());
		ui.leftimage->set(limg.get());
		ui.rightimage->set(rimg.get());

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
			encode(*img, encbuf);
			network.broadcast(encbuf, decbuf, maxlatency);

			UILock lock;
			if (decbuf.size() > left) {
				decode(decbuf[left], *limg);
				ui.leftimage->redraw();
			}
			if (decbuf.size() > right) {
				decode(decbuf[right], *rimg);
				ui.rightimage->redraw();
			}
			Fl::awake();
		}
	} catch (std::exception& e) {
		cout << "video failure: " << e.what() << endl;
	}
}
