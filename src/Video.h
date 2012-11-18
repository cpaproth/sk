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

#ifndef SK_VIDEO_H
#define SK_VIDEO_H


#include <boost/thread/thread.hpp>


namespace cv {
class VideoCapture;
class Mat;
}


namespace SK {

class UserInterface;
class Network;

class Video {
	static const unsigned imagewidth = 320;
	static const unsigned imageheight = 240;
	static const unsigned maxlatency = 200;
	static const size_t namesize = 100;

	boost::shared_ptr<cv::VideoCapture>	capture;
	boost::shared_ptr<cv::Mat>		img;
	boost::shared_ptr<cv::Mat>		limg;
	boost::shared_ptr<cv::Mat>		rimg;
	boost::thread				videothread;
	char					midname[namesize];
	char					leftname[namesize];
	char					rightname[namesize];
	unsigned				left;
	unsigned				right;
	bool					working;
	UserInterface&				ui;
	Network&				network;

	unsigned char& pixel(cv::Mat&, unsigned, unsigned, unsigned, bool);
	void deblock(cv::Mat&, unsigned, bool);
	void deblock(cv::Mat&);

	void worker(void);

	Video(const Video&);
	void operator=(const Video&);
public:
	Video(UserInterface&, Network&);
	~Video(void);
	
	void change_name(void);
	bool handle_command(unsigned, const std::string&, const std::string&);

};


}


#endif
