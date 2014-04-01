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


#ifndef SK_VIDEO_H
#define SK_VIDEO_H


#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>


namespace cv {
class VideoCapture;
class Mat;
}
class UserInterface;


namespace SK {


class Network;


class Video {
	static const unsigned imagewidth = 320;
	static const unsigned imageheight = 240;
	static const unsigned minsize = 300;
	static const unsigned maxlatency = 200;

	boost::shared_ptr<cv::VideoCapture>	capture;
	boost::shared_ptr<cv::Mat>		img;
	boost::shared_ptr<cv::Mat>		limg;
	boost::shared_ptr<cv::Mat>		rimg;
	boost::thread				videothread;
	unsigned				left;
	unsigned				right;
	bool					working;
	UserInterface&				ui;
	Network&				network;

	void encode(const cv::Mat&, std::vector<unsigned char>&);
	void decode(const std::vector<unsigned char>&, cv::Mat&);

	void worker(void);

	bool handle_command(unsigned, const std::string&, const std::string&);

	Video(const Video&);
	void operator=(const Video&);
public:
	Video(UserInterface&, Network&);
	~Video(void);

	void send_chat(void);
};


}


#endif
