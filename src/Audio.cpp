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

#include "Audio.h"
#include "Network.h"
#include <iostream>


using namespace SK;


bool CPLib::Progress(size_t i, size_t n) {
	return i < n;
}


Audio::Audio(Network& nw) : fft(framesize), network(nw) {
	data.resize(framesize);
	encbuf.resize(encsize);
	if (Pa_Initialize() != paNoError)
		throw runtime_error("audio initialization failed");
	if (Pa_OpenDefaultStream(&stream, 1, 1, paInt16, samplerate, framesize, &callback, this) != paNoError) {
		Pa_Terminate();
		throw runtime_error("open audio stream failed");
	}
	Pa_StartStream(stream);
	cout << "audio started" << endl;
}


Audio::~Audio(void) {
	Pa_StopStream(stream);
	Pa_CloseStream(stream);
	Pa_Terminate();
	cout << "audio stopped" << endl;
}


void Audio::restart(void) {
	cout << "audio cpu load: " << Pa_GetStreamCpuLoad(stream) << endl;
	Pa_StopStream(stream);
	Pa_StartStream(stream);
}


void Audio::encode(short* in) {
	encbuf.assign(encsize, 0);

	for (unsigned i = 0; i < data.size(); i++)
		data[i] = in[i] / 32768.;
	fft(data, true);

	multimap<double, unsigned> greatest;
	for (unsigned i = 1; i < data.size() / 2; i++)
		greatest.insert(pair<double, unsigned>(abs(data[i]) * 2. / data.size(), i));

	map<unsigned, double> channels;
	unsigned i = 0;
	for (multimap<double, unsigned>::reverse_iterator it = greatest.rbegin(); i < 128; it++, i++)
		channels[it->second] = it->first;

	i = 0;
	for (map<unsigned, double>::iterator it = channels.begin(); it != channels.end(); it++, i++) {
		encbuf[it->first >> 3] |= 1 << (it->first & 7);
		encbuf[32 + i] = ((unsigned)(log(1. + it->second * 10000.) / log(10001.) * 15.) << 4)
			| ((unsigned)((arg(data[it->first]) + M_PI) / M_PI * 8.) & 15);
	}
}


void Audio::decode(short* out) {
	if (decbuf.size() == 0)
		return;
	vector<unsigned char> in(decbuf[0]);

	data[0] = data[data.size() / 2] = 0.;
	unsigned i = 0;
	for (unsigned j = 1; j < data.size() / 2; j++) {
		if (in[j >> 3] & (1 << (j & 7)) && i < 128) {
			data[j] = 0.5 * polar<double>((pow(10001., (in[32 + i] >> 4) / 15.) - 1.) / 10000., (in[32 + i] & 15) / 8. * M_PI - M_PI);
			i++;
		} else
			data[j] = polar<double>(0.00005, rand() * 2. * M_PI / RAND_MAX);
		data[data.size() - j] = conj(data[j]);
	}

	fft(data, false);
	for (unsigned i = 0; i < data.size(); i++) {
		if (data[i].real() > 1.)
			out[i] = 32767;
		else if (data[i].real() < -1.)
			out[i] = -32768;
		else
			out[i] = (short)(data[i].real() * 32767.);
	}
}


int Audio::callback(const void* in, void* out, unsigned long size, const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void* data) {
	try {
		if (size != framesize)
			throw runtime_error("wrong frame size");

		Audio& audio = *(Audio*)data;

		audio.encode((short*)in);
		audio.network.broadcast(audio.encbuf, audio.decbuf, maxlatency);
		audio.decode((short*)out);
	} catch (exception& e) {
		cout << "audio failure: " << e.what() << endl;
		return paAbort;
	}

	return paContinue;
}
