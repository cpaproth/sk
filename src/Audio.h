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


#ifndef SK_AUDIO_H
#define SK_AUDIO_H


#include <portaudio.h>
#include <vector>
#include <complex>
#include <boost/dynamic_bitset.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/lockfree/spsc_queue.hpp>


namespace SK {


using namespace std;


class Network;


class Transform {
	vector<complex<float> >	twiddle;
	vector<complex<float> >	factor;
	vector<float>		window;
	vector<complex<float> >	data;
	vector<complex<float> >	tmp;
	vector<float>		tdata;
	vector<float>		fdata;

	void fft(unsigned = 1, unsigned = 0);
public:
	Transform(size_t);

	void mdct();
	void imdct();
	float& t(size_t i) {return tdata[i];}
	float& f(size_t i) {return fdata[i];}
};


class Audio {
	static const unsigned	samplerate = 8000;
	static const size_t	framesize = 256;
	static const size_t	encsize = 80;
	static const unsigned	maxlatency = 20;


	PaStream*				stream;
	Transform				trafo;
	boost::dynamic_bitset<unsigned char>	bits;
	vector<unsigned char>			encbuf;
	vector<vector<unsigned char> >		decbuf;
	Network&				network;
	bool					initerror;
	boost::thread				audiothread;
	boost::lockfree::spsc_queue<short, boost::lockfree::capacity<8 * framesize> >	inbuf;
	boost::lockfree::spsc_queue<short, boost::lockfree::capacity<8 * framesize> >	outbuf;
	boost::atomic<bool>			working;
	boost::atomic<bool>			playmic;
	boost::atomic<bool>			noisegate;
	boost::atomic<bool>			mute;
	unsigned				frame;
	unsigned				rnd;
	int					threshold;
	unsigned				noisecount;
	vector<float>				enctmp;
	vector<float>				dectmp;
	vector<pair<float, unsigned> >		a;
	vector<pair<unsigned char, int> >	v;


	void encode(const short*);
	void decode(short*);

	static int callback(const void*, void*, unsigned long, const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void*);
	void audioworker();

	Audio(const Audio&);
	void operator=(const Audio&);
public:
	Audio(Network&);
	~Audio();
	
	void restart();
	void toggle_playmic();
	void toggle_noisegate();
	void mute_mic(bool);
};


}


#endif
