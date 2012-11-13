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

#ifndef SK_AUDIO_H
#define SK_AUDIO_H


#include <portaudio.h>
#include "MathLib.h"
#include <boost/thread/mutex.hpp>


namespace SK {

using namespace std;


class Network;


class sbool {
	bool		value;
	boost::mutex	mutex;
public:
	sbool(void) : value(false) {}
	void set(bool b) {
		boost::lock_guard<boost::mutex> lock(mutex);
		value = b;
	}
	bool get(void) {
		boost::lock_guard<boost::mutex> lock(mutex);
		return value;
	}
	operator bool(void) {
		if (mutex.try_lock()) {
			boost::lock_guard<boost::mutex> lock(mutex, boost::adopt_lock);
			return value;
		}
		return false;
	}
};


class Audio {
	static const unsigned samplerate = 8000;
	static const size_t framesize = 512;
	static const size_t encodesize = 160;
	static const unsigned maxlatency = 20;

	PaStream*			stream;
	CPLib::FFT			fft;
	valarray<complex<double> >	data;
	vector<unsigned char>		encbuf;
	vector<vector<unsigned char> >	decbuf;
	Network&			network;
	sbool				mic;


	void encode(short*);
	void decode(short*);

	static int callback(const void*, void*, unsigned long, const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void*);

	Audio(const Audio&);
	void operator=(const Audio&);
public:
	Audio(Network&);
	~Audio(void);
	
	void restart(void);
	void toggle_dest(void);

};


}


#endif
