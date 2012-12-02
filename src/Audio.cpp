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
	encbuf.reserve(encodesize);
	stream = 0;
	playmic = false;

	if (Pa_Initialize() != paNoError)
		throw runtime_error("audio initialization failed");
}


Audio::~Audio(void) {
	try {
		if (stream) {
			if (!Pa_IsStreamStopped(stream))
				Pa_StopStream(stream);
			Pa_CloseStream(stream);
		}
		Pa_Terminate();

		cout << "audio stream closed" << endl;
	} catch (...) {}
}


void Audio::restart(void) {
	if (!stream && Pa_OpenDefaultStream(&stream, 1, 1, paInt16, samplerate, framesize, &callback, this) != paNoError)
		throw runtime_error("open audio stream failed");
	if (!Pa_IsStreamStopped(stream)) {
		cout << "audio cpu load: " << Pa_GetStreamCpuLoad(stream) << endl;
		if (Pa_StopStream(stream) == paNoError)
			cout << "audio stream stopped" << endl;
	}
	if (Pa_StartStream(stream) == paNoError)
		cout << "audio stream started" << endl;
}


void Audio::toggle_playmic(void) {
	playmic = !playmic;
}


double Audio::logamp(double amp, double b) {
	return log(1. + amp * b) / log(b + 1.);
}


double Audio::powamp(double amp, double b) {
	return (pow(b + 1., amp) - 1.) / b;
}


void Audio::encode(const short* in) {
	for (unsigned i = 0; i < data.size(); i++)
		data[i] = in[i] / 32768.;
	fft(data, true);

	multimap<double, unsigned> greatest;
	for (unsigned i = 1; i < data.size() / 2; i++)
		greatest.insert(pair<double, unsigned>(abs(data[i]) / (data.size() / 2), i));
		
	double minrho = 0., maxrho = greatest.rbegin()->first;
	map<unsigned, double> channels;
	for (multimap<double, unsigned>::reverse_iterator it = greatest.rbegin(); enchead + channels.size() < encodesize; it++, minrho = it->first)
		channels[it->second] = it->first;

	encbuf.assign(enchead, 0);
	encbuf[enchead - 2] = (unsigned)(logamp(minrho, 10000.) * 255. + 0.5);
	encbuf[enchead - 1] = (unsigned)(logamp(maxrho - minrho, 100.) * 255. + 0.5);

	for (map<unsigned, double>::iterator it = channels.begin(); it != channels.end(); it++) {
		encbuf[it->first >> 3] |= 1 << (it->first & 7);
		double amp = (it->second - minrho) / (maxrho - minrho);
		double rho = logamp(amp, 1000.) * 31.;
		double phi = (arg(data[it->first]) + M_PI) / M_PI * 4.;
		encbuf.push_back(((unsigned)(rho + 0.5) << 3) | ((unsigned)(phi + 0.5) & 7));
	}
}


void Audio::decode(short* out) {
	valarray<double> output(0., framesize);

	for (unsigned k = 0; k < decbuf.size() && enchead < decbuf[k].size(); k++) {
		data[0] = data[data.size() / 2] = 0.;
		
		double minrho = powamp(decbuf[k][enchead - 2] / 255., 10000.);
		double maxrho = minrho + powamp(decbuf[k][enchead - 1] / 255., 100.);

		for (unsigned j = 1, i = 0; j < data.size() / 2; j++) {
			if (decbuf[k][j >> 3] & (1 << (j & 7)) && enchead + i < decbuf[k].size()) {
				double amp = powamp((decbuf[k][enchead + i] >> 3) / 31., 1000.);
				double rho = minrho + amp * (maxrho - minrho);
				double phi = (decbuf[k][enchead + i] & 7) / 4. * M_PI - M_PI;
				data[j] = 0.5 * polar<double>(rho, phi);
				i++;
			} else
				data[j] = 0.5 * polar<double>(minrho * rangen.uniform(), 2. * M_PI * rangen.uniform());
			data[data.size() - j] = conj(data[j]);
		}

		fft(data, false);
		for (unsigned i = 0; i < data.size(); i++)
			output[i] += data[i].real();
	}

	for (unsigned i = 0; i < output.size(); i++)
		out[i] = output[i] > 1.? 32767: output[i] < -1.? -32768: (short)(output[i] * 32767.);
}


int Audio::callback(const void* in, void* out, unsigned long size, const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void* data) {
	try {
		if (size != framesize)
			throw runtime_error("wrong frame size");

		Audio& a = *(Audio*)data;

		a.encode((short*)in);
		if (!a.playmic) {
			a.network.broadcast(a.encbuf, a.decbuf, maxlatency);
		} else {
			a.decbuf.clear();
			a.decbuf.push_back(a.encbuf);
		}
		a.decode((short*)out);

	} catch (exception& e) {
		cout << "audio failure: " << e.what() << endl;
		return paAbort;
	}

	return paContinue;
}
