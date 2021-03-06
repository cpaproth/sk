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


#include "Video.h"
#include "Network.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "ui.h"
#include <iostream>


using namespace SK;
using namespace cv;


Video::Codec::Codec() : a1(-1.586134342f), a2(-0.05298011854f), a3(0.8829110762f), a4(0.4435068522f), k1(0.81289306611596146f), k2(0.61508705245700002f) {
	Y = U = V = tmpY = tmpU = tmpV = vector<float>(imagewidth * imageheight, 0.f);
	frame = part = rndpos = 0;
	while (rndmask.size() < minsize)
		rndmask.push_back(rndmask.size());
	random_shuffle(rndmask.begin(), rndmask.end());
	refresh.resize(minsize, 0);
	for (w = imagewidth, h = imageheight, l = 1; w * h > minsize; w /= 2, h /= 2, l *= 2);
}


void Video::Codec::fcdf97(vector<float>& m, unsigned p, unsigned d, unsigned s) {
	for (unsigned i = 1; i < s - 1; i += 2)
		m[p + i * d] += a1 * (m[p + (i - 1) * d] + m[p + (i + 1) * d]);
	m[p + (s - 1) * d] += 2.f * a1 * m[p + (s - 2) * d];

	for (unsigned i = 2; i < s - 1; i += 2)
		m[p + i * d] += a2 * (m[p + (i - 1) * d] + m[p + (i + 1) * d]);
	m[p] += 2.f * a2 * m[p + d];

	for (unsigned i = 1; i < s - 1; i += 2)
		m[p + i * d] += a3 * (m[p + (i - 1) * d] + m[p + (i + 1) * d]);
	m[p + (s - 1) * d] += 2.f * a3 * m[p + (s - 2) * d];

	for (unsigned i = 2; i < s - 1; i += 2)
		m[p + i * d] += a4 * (m[p + (i - 1) * d] + m[p + (i + 1) * d]);
	m[p] += 2.f * a4 * m[p + d];

	for (unsigned i = 0; i < s / 2; i++) {
		m[p + 2 * i * d] *= k1;
		m[p + (2 * i + 1) * d] *= k2;
	}
}


void Video::Codec::icdf97(vector<float>& m, unsigned p, unsigned d, unsigned s) {
	for (unsigned i = 0; i < s / 2; i++) {
		m[p + 2 * i * d] /= k1;
		m[p + (2 * i + 1) * d] /= k2;
	}

	for (unsigned i = 2; i < s - 1; i += 2)
		m[p + i * d] -= a4 * (m[p + (i - 1) * d] + m[p + (i + 1) * d]);
	m[p] -= 2.f * a4 * m[p + d];

	for (unsigned i = 1; i < s - 1; i += 2)
		m[p + i * d] -= a3 * (m[p + (i - 1) * d] + m[p + (i + 1) * d]);
	m[p + (s - 1) * d] -= 2.f * a3 * m[p + (s - 2) * d];

	for (unsigned i = 2; i < s - 1; i += 2)
		m[p + i * d] -= a2 * (m[p + (i - 1) * d] + m[p + (i + 1) * d]);
	m[p] -= 2.f * a2 * m[p + d];

	for (unsigned i = 1; i < s - 1; i += 2)
		m[p + i * d] -= a1 * (m[p + (i - 1) * d] + m[p + (i + 1) * d]);
	m[p + (s - 1) * d] -= 2.f * a1 * m[p + (s - 2) * d];
}


void Video::Codec::rearrange(const set<unsigned>& mask, vector<float>& C, vector<int>& d, unsigned o, unsigned s, unsigned f, float q, bool fwd) {
	for (set<unsigned>::const_iterator it = mask.begin(); it != mask.end(); it++) {
		unsigned mx = *it % w * l, my = *it / w * l;
		for (unsigned y = my + f / 2 * (1 << s); y < my + l; y += 2 << s) {
			for (unsigned x = mx + f % 2 * (1 << s); x < mx + l; x += 2 << s) {
				if (fwd)
					d[o++] = (int)(C[y * imagewidth + x] / q + (C[y * imagewidth + x] < 0.f? -0.5f: 0.5f));
				else
					C[y * imagewidth + x] = d[o++] * q;
			}
		}
	}
}


void Video::Codec::denoise(vector<float>& C, float h, const unsigned P, const unsigned K) {
	const unsigned width = imagewidth + 2 * P + 2 * K;
	const unsigned height = imageheight + 2 * P + 2 * K;
	const int f = int(10000.f / (2 * P + 1) / (2 * P + 1) / h / h);
	vector<int> v(width * height), S(v.size(), INT_MIN), u(C.size(), 0), N(C.size(), 0), M(C.size(), 0);

	for (unsigned y = 0; y < height; y++) {
		unsigned ys = y < P + K? P + K - y: y >= imageheight + P + K? 2 * imageheight + P + K - 2 - y: y - P - K;
		for (unsigned x = 0; x < width; x++) {
			unsigned xs = x < P + K? P + K - x: x >= imagewidth + P + K? 2 * imagewidth + P + K - 2 - x: x - P - K;
			v[y * width + x] = (int)C[ys * imagewidth + xs];
		}
	}

	for (int dy = -(int)K; dy <= (int)K; dy++) {
		for (int dx = -(int)K; dx <= (int)K; dx++) {
			if (dy == 0 && dx == 0)
				continue;

			const int delta = dy * width + dx;

			for (unsigned y = K; y < height - K; y++) {
				unsigned p = y * width + K;
				for (unsigned x = K; x < width - K; x++, p++) {
					const int d = v[p] - v[p + delta];
					S[p] = d * d + S[p - 1] + S[p - width] - S[p - width - 1];
				}
			}

			for (unsigned y = 0; y < imageheight; y++) {
				unsigned p = y * imagewidth;
				unsigned q = (P + K + y) * width + P + K;
				for (unsigned x = 0; x < imagewidth; x++, p++, q++) {
					int w = S[q + P * width + P] + S[q - P * width - width - P - 1] - S[q + P * width - P - 1] - S[q - P * width - width + P];
					w = max(10000 - w * f, 0);
					u[p] += w * v[q + delta];
					N[p] += w;
					M[p] = max(M[p], w);
				}
			}
		}
	}

	for (unsigned y = 0; y < imageheight; y++) {
		for (unsigned x = 0; x < imagewidth; x++) {
			unsigned p = y * imagewidth + x;
			if (N[p] + M[p] != 0)
				C[p] = (u[p] + M[p] * C[p]) / (N[p] + M[p]);
		}
	}
}


void Video::Codec::nextframe(const Mat& img, unsigned time) {
	if (time >= refreshtime)
		tmpY = tmpU = tmpV = vector<float>(imagewidth * imageheight, 1000.f);

	diffs.clear();
	frame++;
	part = 0;
	for (unsigned i = 0; i < refresh.size(); i++)
		refresh[i] += rand() % 200 * time / 100 + 1;

	if (img.cols != (int)imagewidth || img.rows != (int)imageheight || img.elemSize() != 3)
		return;

	Y.clear(), U.clear(), V.clear();
	for (unsigned y = 0; y < imageheight; y++) {
		for (unsigned x = 0; x < imagewidth; x++) {
			Y.push_back(Vec3f(0.2f, 0.5f, 0.3f).dot(img.at<Vec3b>(y, x)));
			U.push_back(Y.back() - img.at<Vec3b>(y, x)[2]);
			V.push_back(Y.back() - img.at<Vec3b>(y, x)[0]);
		}
	}

	for (w = imagewidth, h = imageheight, l = 1; w * h > minsize; w /= 2, h /= 2, l *= 2) {
		for (unsigned y = 0; y < h; y++) {
			fcdf97(Y, y * l * imagewidth, l, w);
			fcdf97(U, y * l * imagewidth, l, w);
			fcdf97(V, y * l * imagewidth, l, w);
		}
		for (unsigned x = 0; x < w; x++) {
			fcdf97(Y, x * l, l * imagewidth, h);
			fcdf97(U, x * l, l * imagewidth, h);
			fcdf97(V, x * l, l * imagewidth, h);
		}
	}

	vector<float> ldiffs(minsize, 0.f), ndiffs(minsize, 0.f);
	for (unsigned py = 0; py < h; py++) {
		for (unsigned px = 0; px < w; px++) {
			float bright = 1.f + min(tmpY[py * l * imagewidth + px * l], Y[py * l * imagewidth + px * l]) / 120.f;
			float& diff = ldiffs[py * w + px];
			for (unsigned y = py * l; y < py * l + l; y++) {
				for (unsigned x = px * l; x < px * l + l; x++) {
					float th = bright * (y % 4 == 0 && x % 4 == 0? 2.f: 8.f);
					diff = max(diff, fabs(tmpY[y * imagewidth + x] - Y[y * imagewidth + x]) / th);
					diff = max(diff, fabs(tmpU[y * imagewidth + x] - U[y * imagewidth + x]) / th / 4.f);
					diff = max(diff, fabs(tmpV[y * imagewidth + x] - V[y * imagewidth + x]) / th / 4.f);
				}
			}
			for (unsigned y = py > 0? py - 1: py; y <= py + 1 && y < h; y++)
				for (unsigned x = px > 0? px - 1: px; x <= px + 1 && x < w; x++)
					ndiffs[y * w + x] = max(diff, ndiffs[y * w + x]);
		}
	}

	for (unsigned i = 0; i < ldiffs.size(); i++)
		if (ldiffs[i] > 1.f || (ldiffs[i] > 0.25f && ndiffs[i] > 1.f) || refresh[i] > refreshtime)
			diffs.insert(make_pair(-ldiffs[i], i));

	for (multimap<float, unsigned>::const_iterator it = diffs.begin(); it != diffs.end(); it++) {
		refresh[it->second] = ldiffs[it->second] > 2.f? refreshtime: 0;
		for (unsigned y = it->second / w * l; y < it->second / w * l + l; y++) {
			for (unsigned x = it->second % w * l; x < it->second % w * l + l; x++) {
				tmpY[y * imagewidth + x] = Y[y * imagewidth + x];
				tmpU[y * imagewidth + x] = U[y * imagewidth + x];
				tmpV[y * imagewidth + x] = V[y * imagewidth + x];
			}
		}
	}
}


bool Video::Codec::encode(vector<unsigned char>& enc) {
	part++;
	if (diffs.size() == 0 && part > 1) {
		enc.assign(1, 128 | (frame & 63));
		enc.push_back(part);
		return true;
	}

	multimap<float, unsigned>::iterator diff;
	unsigned blocks = diffs.size() > 50? 20: 9, lastblocks = blocks;
	do {

		mask.clear();
		for (diff = diffs.begin(); diff != diffs.end() && mask.size() < blocks; diff++)
			mask.insert(diff->second);
		while ((part == 1 && mask.size() < min(9u, blocks)) || mask.size() % 2 != blocks % 2)
			mask.insert(rndmask[rndpos++ % minsize]);


		vector<int> data(minsize + 264 * mask.size(), 0);
		for (set<unsigned>::const_iterator it = mask.begin(); it != mask.end(); it++)
			data[*it] = 1;

		float q = mask.size() % 2 == 0? 3.f: 1.f;
		for (unsigned f = 0; f < 4; f++) {
			rearrange(mask, Y, data, minsize + 3 * f * mask.size(), 3, f, 1.f, true);
			rearrange(mask, U, data, minsize + (3 * f + 1) * mask.size(), 3, f, 4.f, true);
			rearrange(mask, V, data, minsize + (3 * f + 2) * mask.size(), 3, f, 4.f, true);
		}
		for (unsigned f = 1; f < 4; f++)
			rearrange(mask, Y, data, minsize + ((f - 1) * 4 + 12) * mask.size(), 2, f, 1.f * q, true);
		for (unsigned f = 1; f < 4; f++)
			rearrange(mask, Y, data, minsize + ((f - 1) * 16 + 24) * mask.size(), 1, f, 2.f * q, true);
		for (unsigned f = 1; f < 4; f++)
			rearrange(mask, Y, data, minsize + ((f - 1) * 64 + 72) * mask.size(), 0, f, 4.f * q, true);
		for (unsigned i = minsize + 3 * mask.size() - 1; i > minsize; i--)
			data[i] -= data[i - 1];


		unsigned size = 0;
		for (unsigned i = 0; i < data.size(); i++) {
			if (data[i] == 0) {
				int z = 2;
				for (; i + 1 < data.size() && data[i + 1] == 0; i++)
					z++;
				for (; z >= 2; z >>= 1)
					data[size++] = z & 1;
			} else {
				data[size++] = data[i] < 0? data[i] * -2: data[i] * 2 + 1;
			}
		}
		data.resize(size);


		vector<unsigned> freqs(*max_element(data.begin(), data.end()) + 1, 0), starts(freqs.size() + 1, 0);
		for (unsigned i = 0; i < size; i++)
			freqs[data[i]]++;
		for (int i = 0; starts[i] < size; i++)
			starts[i + 1] = starts[i] + freqs[i];

		enc.assign(1, 128 | (frame & 63));
		for (unsigned i = 0; i < freqs.size(); i++) {
			if (freqs[i] == 0) {
				enc.push_back(1);
				for (; i + 1 < freqs.size() && freqs[i + 1] == 0 && enc.back() < 63; i++)
					enc.back()++;
			} else if (freqs[i] <= 128) {
				enc.push_back((freqs[i] - 1) | 128);
			} else {
				unsigned freq = freqs[i] - 129;
				enc.push_back((freq & 63) | 64);
				for (freq >>= 6; freq > 0 || (enc.back() & 128) == 0; freq >>= 7)
					enc.push_back((freq & 127) | (freq > 127? 0: 128));
			}
		}
		enc.push_back(0);

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

		if (blocks < 3 || (enc.size() <= maxpacket && (diff == diffs.end() || blocks + 2 == lastblocks)))
			break;
	} while (lastblocks = blocks, blocks += enc.size() > maxpacket? -2: (maxpacket - enc.size()) * mask.size() / 2 / enc.size() * 2, enc.size() > maxpacket || blocks != lastblocks);

	diffs.erase(diffs.begin(), diff);

	return false;
}


void Video::Codec::decode(const vector<unsigned char>& enc) {
	if (enc.size() < 6)
		return;

	vector<unsigned> freqs;
	unsigned size = 0, p = 1;
	for (; p < enc.size() && enc[p] != 0; size += freqs.back(), p++) {
		if ((enc[p] & 192) == 0) {
			for (unsigned i = enc[p]; i > 0; i--)
				freqs.push_back(0);
		} else if ((enc[p] & 128) == 128) {
			freqs.push_back((enc[p] & 127) + 1);
		} else {
			unsigned freq = enc[p++] & 63, s = 6;
			for (; p + 1 < enc.size() && (enc[p] & 128) == 0; s += 7, p++)
				freq += (enc[p] & 127) << s;
			freqs.push_back(freq + ((enc[p] & 127) << s) + 129);
		}
	}
	if ((p += 1) + 3 >= enc.size())
		return;
	vector<unsigned> starts(freqs.size() + 1, 0);
	for (int i = 0; starts[i] < size; i++)
		starts[i + 1] = starts[i] + freqs[i];

	map<unsigned, int> ranges;
	for (int i = 0; ranges.size() == 0 || ranges.rbegin()->first < size; i++) {
		if (freqs[i] != 0)
			ranges[starts[i] + freqs[i]] = i;
	}
	vector<int> dec, data;
	unsigned low = 0, range = UINT_MAX;
	unsigned code = (enc[p] << 24) | (enc[p + 1] << 16) | (enc[p + 2] << 8) | enc[p + 3];
	for (p += 4; dec.size() < size && range >= 400000 && range >= size;) {
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
		if (dec[i] == 0 || dec[i] == 1) {
			int z = dec[i], b = 1;
			for (; i + 1 < dec.size() && (dec[i + 1] == 0 || dec[i + 1] == 1); i++, b++)
				z |= dec[i + 1] << b;
			for (z += (1 << b) - 2; z >= 0; z--)
				data.push_back(0);
		} else {
			data.push_back((dec[i] & 1) == 0? dec[i] / -2: (dec[i] - 1) / 2);
		}
	}


	mask.clear();
	for (unsigned i = 0; i < minsize && i < data.size(); i++)
		if (data[i] == 1)
			mask.insert(i);
	if (data.size() != minsize + 264 * mask.size())
		return;

	for (unsigned i = minsize + 1; i < minsize + 3 * mask.size(); i++)
		data[i] += data[i - 1];

	float q = mask.size() % 2 == 0? 3.f: 1.f;
	for (unsigned f = 0; f < 4; f++) {
		rearrange(mask, tmpY, data, minsize + 3 * f * mask.size(), 3, f, 1.f, false);
		rearrange(mask, tmpU, data, minsize + (3 * f + 1) * mask.size(), 3, f, 4.f, false);
		rearrange(mask, tmpV, data, minsize + (3 * f + 2) * mask.size(), 3, f, 4.f, false);
	}
	for (unsigned f = 1; f < 4; f++)
		rearrange(mask, tmpY, data, minsize + ((f - 1) * 4 + 12) * mask.size(), 2, f, 1.f * q, false);
	for (unsigned f = 1; f < 4; f++)
		rearrange(mask, tmpY, data, minsize + ((f - 1) * 16 + 24) * mask.size(), 1, f, 2.f * q, false);
	for (unsigned f = 1; f < 4; f++)
		rearrange(mask, tmpY, data, minsize + ((f - 1) * 64 + 72) * mask.size(), 0, f, 4.f * q, false);
}


void Video::Codec::showframe(Mat& img, unsigned quality) {
	if (img.cols != (int)imagewidth || img.rows != (int)imageheight || img.elemSize() != 3)
		return;

	Y = tmpY, U = tmpU, V = tmpV;
	for (unsigned w = imagewidth / 8, h = imageheight / 8, l = 8; w <= imagewidth; w *= 2, h *= 2, l /= 2) {
		for (unsigned y = 0; y < h; y++) {
			icdf97(Y, y * l * imagewidth, l, w);
			icdf97(U, y * l * imagewidth, l, w);
			icdf97(V, y * l * imagewidth, l, w);
		}
		for (unsigned x = 0; x < w; x++) {
			icdf97(Y, x * l, l * imagewidth, h);
			icdf97(U, x * l, l * imagewidth, h);
			icdf97(V, x * l, l * imagewidth, h);
		}
	}

	denoise(Y, 5.f, 3, quality);

	UILock lock;
	for (unsigned i = 0; i < Y.size(); i++)
		img.at<Vec3b>(i / imagewidth, i % imagewidth) = Vec3f(Y[i] - V[i], Y[i] + 0.6f * U[i] + 0.4f * V[i], Y[i] - U[i]);
}


Video::Video(UserInterface& ui, Network& nw) : ui(ui), network(nw) {
	left = 0;
	right = 1;

	img = boost::shared_ptr<Mat>(new Mat(imageheight, imagewidth, CV_8UC3, Scalar()));
	limg = boost::shared_ptr<Mat>(new Mat(imageheight, imagewidth, CV_8UC3, Scalar()));
	rimg = boost::shared_ptr<Mat>(new Mat(imageheight, imagewidth, CV_8UC3, Scalar()));

	capture = boost::shared_ptr<VideoCapture>(new VideoCapture(0));

	reset = false;
	working = true;
	capturethread = boost::thread(boost::bind(&Video::captureworker, this));
	encodethread = boost::thread(boost::bind(&Video::encodeworker, this));
	decodethread = boost::thread(boost::bind(&Video::decodeworker, this));

	if (capture->isOpened())
		cout << "video capture started" << endl;
	else
		cout << "open webcam failed" << endl;

	network.add_handler(boost::bind(&Video::handle_command, this, _1, _2, _3));

	ui.midimage->set(img.get());
	ui.leftimage->set(limg.get());
	ui.rightimage->set(rimg.get());
}


Video::~Video() {
	try {
		network.remove_handler();
		ui.midimage->set(0);
		ui.leftimage->set(0);
		ui.rightimage->set(0);

		working = false;
		while (capturethread.joinable() && !capturethread.timed_join(boost::posix_time::milliseconds(100)))
			Fl::wait(0.1);
		while (encodethread.joinable() && !encodethread.timed_join(boost::posix_time::milliseconds(100)))
			Fl::wait(0.1);
		while (decodethread.joinable() && !decodethread.timed_join(boost::posix_time::milliseconds(100)))
			Fl::wait(0.1);

	} catch (...) {}
}


void Video::send_chat() {
	string msg(ui.chat->value(), 0, maxpacket);
	network.command(left, "chat", msg);
	network.command(right, "chat", msg);
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
	} else if (command == "newpeer") {
		reset = true;
	} else
		return false;
	return true;
}


void Video::captureworker() {
	try {
		Mat cap(imageheight, imagewidth, CV_8UC3);
		Mat still(imageheight, imagewidth, CV_8UC3, Scalar());
		
		unsigned secret = 0;
		UILock(), string(ui.secret->value()).copy((char*)&secret, sizeof(unsigned));
		srand(secret ^ (unsigned)time(0));
		circle(still, Point(imagewidth / 2, imageheight * 19 / 16), imageheight / 2, Scalar(20 + rand() % 150, 20 + rand() % 150, 20 + rand() % 150), -1, 16);
		circle(still, Point(imagewidth / 2, imageheight / 2), imageheight / 4, Scalar(20 + rand() % 150, 20 + rand() % 150, 20 + rand() % 150), -1, 16);

		boost::posix_time::ptime t = boost::posix_time::microsec_clock::local_time();
		while (working) {
			do {
				boost::this_thread::sleep(boost::posix_time::milliseconds(10));
				if (UILock(), ui.mainwnd->visible())
					*capture >> cap;
			} while ((boost::posix_time::microsec_clock::local_time() - t).total_milliseconds() < 40);
			t = boost::posix_time::microsec_clock::local_time();

			//if (cap.size().area() == 0) capture->open("webcam.avi"), *capture >> cap;

			bool pause = (UILock(), ui.midimage->get());
			if (cap.size().area() == 0 || pause) {
				still.copyTo(cap);
				if (!pause) {
					circle(cap, Point(imagewidth / 2 - imageheight / 10, imageheight / 2), imageheight / 20, Scalar(190, 190, 190), -1, 16);
					circle(cap, Point(imagewidth / 2 + imageheight / 10, imageheight / 2), imageheight / 20, Scalar(190, 190, 190), -1, 16);
				}
			}

			int arcols = min<int>(cap.cols, cap.rows * imagewidth / imageheight);
			int arrows = min<int>(cap.rows, cap.cols * imageheight / imagewidth);
			UILock lock;
			resize(cap(Rect((cap.cols - arcols) / 2, (cap.rows - arrows) / 2, arcols, arrows)), *img, img->size());
			ui.midimage->redraw();
			Fl::awake();
		}

		cout << "video capture stopped" << endl;

	} catch (std::exception& e) {
		cout << "video capture failure: " << e.what() << endl;
	}
}


void Video::encodeworker() {
	try {
		Codec encoder;
		bool update = true, finish = true;
		vector<unsigned char> encbuf;
		vector<vector<unsigned char> > decbuf;
		
		boost::posix_time::ptime t = boost::posix_time::microsec_clock::local_time();
		while (working) {
			if (reset || (finish && update)) {
				unsigned d = (boost::posix_time::microsec_clock::local_time() - t).total_milliseconds();
				boost::this_thread::sleep(boost::posix_time::milliseconds(d < 30? 40 - d: 10));
				t = boost::posix_time::microsec_clock::local_time();
				UILock lock;
				encoder.nextframe(*img, reset.exchange(false)? refreshtime: d + (d < 30? 40 - d: 10));
				finish = false;
			}

			if (update)
				finish = encoder.encode(encbuf);
			else
				boost::this_thread::sleep(boost::posix_time::milliseconds(10));

			if (reset) {
				update = true;
				continue;
			}

			update = network.broadcast(encbuf, decbuf, maxlatency);

			boost::lock_guard<boost::mutex> lock(bufmutex);
			if (buf.size() == 0)
				swap(decbuf, buf);
			else
				buf.insert(buf.end(), decbuf.begin(), decbuf.end());
		}

		cout << "video encode stopped" << endl;

	} catch (std::exception& e) {
		cout << "video encode failure: " << e.what() << endl;
	}
}


void Video::decodeworker() {
	try {
		Codec decoder0, decoder1;
		set<unsigned> show;
		vector<vector<unsigned char> > decbuf;

		while (working) {
			unsigned quality = (UILock(), ui.quality->value());

			for (unsigned i = 0; i < decbuf.size(); i++) {
				(decbuf[i][0] == 0? decoder0: decoder1).decode(decbuf[i]);
				show.insert(decbuf[i][0]);
			}

			while (show.size() > 0) {
				(*show.begin() == 0? decoder0: decoder1).showframe(*show.begin() == left? *limg: *rimg, quality);
				UILock lock;
				(*show.begin() == left? ui.leftimage: ui.rightimage)->redraw();
				Fl::awake();
				show.erase(show.begin());
			}

			boost::this_thread::sleep(boost::posix_time::milliseconds(10));

			decbuf.clear();
			boost::lock_guard<boost::mutex> lock(bufmutex);
			swap(decbuf, buf);
		}

		cout << "video decode stopped" << endl;

	} catch (std::exception& e) {
		cout << "video decode failure: " << e.what() << endl;
	}
}
