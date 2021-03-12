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


#ifndef SK_VIDEO_H
#define SK_VIDEO_H


#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/atomic.hpp>
#include <vector>
#include <set>


namespace cv {
class VideoCapture;
class Mat;
}
class UserInterface;


namespace SK {


using namespace std;


class Network;


class Video {
	static const unsigned imagewidth = 320;
	static const unsigned imageheight = 240;
	static const unsigned minsize = 300;
	static const unsigned maxlatency = 200;


	class Codec {
		const float		a1, a2, a3, a4, k1, k2;
		vector<float>		Y, U, V, tmpY, tmpU, tmpV;
		set<unsigned>		mask;
		vector<unsigned>	rndmask;
		unsigned		frame, rndnext, w, h, l;

		void fcdf97(vector<float>&, unsigned, unsigned, unsigned);
		void icdf97(vector<float>&, unsigned, unsigned, unsigned);
		void rearrange(const set<unsigned>&, vector<float>&, vector<int>&, unsigned, unsigned, unsigned, float, bool);
		void denoise(vector<float>&, float, unsigned, unsigned);
	public:
		Codec();
		void encode(const cv::Mat&, vector<unsigned char>&, bool);
		void decode(const vector<unsigned char>&, cv::Mat&, bool);
	};


	boost::shared_ptr<cv::VideoCapture>	capture;
	boost::shared_ptr<cv::Mat>		img;
	boost::shared_ptr<cv::Mat>		limg;
	boost::shared_ptr<cv::Mat>		rimg;
	boost::thread				videothread;
	boost::thread				codecthread;
	unsigned				left;
	unsigned				right;
	boost::atomic<bool>			working;
	boost::atomic<unsigned>			capframe;
	UserInterface&				ui;
	Network&				network;

	void encode(const cv::Mat&, vector<unsigned char>&);
	void decode(const vector<unsigned char>&, cv::Mat&);

	void worker();
	void coder();

	bool handle_command(unsigned, const string&, const string&);

	Video(const Video&);
	void operator=(const Video&);
public:
	Video(UserInterface&, Network&);
	~Video();

	void send_chat();
};


}


#endif
