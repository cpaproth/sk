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
	encbuf.reserve(maxfreq - minfreq + splitfreqs * 3);
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


static double log_amp(double amp, double b) {
	return log(1. + amp * b) / log(b + 1.);
}
static double pow_amp(double amp, double b) {
	return (pow(b + 1., amp) - 1.) / b;
}
static unsigned char dbl_uchar(double d) {
	int e;
	double s = frexp(d, &e);
	if (e > 0) return 240;
	if (d == 0. || e < -15) return 15;
	unsigned us = (unsigned)((s - 0.5) * 31. + 0.5);
	return (us << 4) | (-e & 15);
}
static double uchar_dbl(unsigned char c) {
	if (c == 15)
		return 0.;
	return ldexp(0.5 + (c >> 4) / 31., -(c & 15));
}


void Audio::encode(const short* in) {
	static unsigned frame = 0;
	frame++;

	for (unsigned i = 0; i < data.size(); i++)
		data[i] = in[i] / 32768.;
	fft(data, true);

	double minrho = 1., maxrho = 0.;
	vector<double> channels;
	for (unsigned i = minfreq; i < data.size() / 2 && i < maxfreq; i++) {
		double amp = abs(data[i]) / (data.size() / 2);
		channels.push_back(amp);
		minrho = amp < minrho? amp: minrho;
		maxrho = amp > maxrho? amp: maxrho;
	}

	encbuf.clear();
	for (unsigned f = 0; f < splitfreqs; f++) {
		encbuf.push_back((f << 6) | (frame & 63));
		encbuf.push_back(dbl_uchar(minrho));
		encbuf.push_back(dbl_uchar(maxrho - minrho));

		for (unsigned i = f; i < channels.size(); i += splitfreqs) {
			double amp = (channels[i] - minrho) / (maxrho - minrho);
			double rho = log_amp(amp, 100.) * 31.;
			double phi = (arg(data[i + minfreq]) + M_PI) / M_PI * 4.;
			encbuf.push_back(((unsigned)(rho + 0.5) << 3) | ((unsigned)(phi + 0.5) & 7));
		}
	}
}


void Audio::decode(short* out) {
	valarray<double> output(0., framesize);

	for (unsigned k = 0; k < decbuf.size(); k++) {
		data = 0.;
		for (unsigned i = 0; i + 3 < decbuf[k].size();) {
			unsigned f = decbuf[k][i++] >> 6;
			double minrho = uchar_dbl(decbuf[k][i++]);
			double maxrho = minrho + uchar_dbl(decbuf[k][i++]);

			for (unsigned j = minfreq + f; j < data.size() / 2 && j < maxfreq && i < decbuf[k].size(); j += splitfreqs, i++) {
				double amp = pow_amp((decbuf[k][i] >> 3) / 31., 100.);
				double rho = minrho + amp * (maxrho - minrho);
				double phi = (decbuf[k][i] & 7) / 4. * M_PI - M_PI;
				data[j] = 0.5 * polar<double>(rho, phi);
				data[data.size() - j] = conj(data[j]);
			}
		}
		
		fft(data, false);
		for (unsigned i = 0; i < data.size(); i++)
			output[i] += data[i].real();
	}

	for (unsigned i = 0; i < output.size(); i++) {
		output[i] *= i < fade? sin(M_PI / 2. * i / fade): i > output.size() - fade? sin(M_PI / 2. * (output.size() - i) / fade): 1.;
		out[i] = output[i] > 1.? 32767: output[i] < -1.? -32768: (short)(output[i] * 32767.);
	}
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
