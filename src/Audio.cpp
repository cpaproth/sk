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

#include "Audio.h"
#include "Network.h"
#include <iostream>


using namespace SK;


bool CPLib::Progress(size_t i, size_t n) {
	return i < n;
}

vector<complex<float> > twiddle(512);
vector<complex<float> > factor(512);
vector<float> window(512);

Audio::Audio(Network& nw) : fft(framesize), network(nw) {
	for (unsigned i = 0; i < twiddle.size(); i++)
		twiddle[i] = exp(complex<float>(0.f, -(float)M_PI * i / twiddle.size()));
	for (unsigned i = 0; i < factor.size(); i++)
		factor[i] = exp(complex<float>(0.f, -(float)M_PI / factor.size() * (1.f + factor.size() / 2.f) * (i + 0.5f)));
	for (unsigned i = 0; i < window.size(); i++)
		window[i] = sin((float)M_PI / window.size() * (i + 0.5f));

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

bool testflag = false;
void Audio::toggle_playmic(void) {
	playmic = !playmic;
	testflag = !testflag;
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


void fft(bool d, vector<complex<float> >& data, unsigned g = 1, unsigned o = 0) {
	static complex<float> tmp[256];
	size_t N = data.size() / g / 2;
	if (N == 2) {
		tmp[0] = data[o] + data[2 * g + o]; tmp[1] = data[o] - data[2 * g + o]; tmp[2] = data[g + o] + data[3 * g + o]; tmp[3] = data[g + o] - data[3 * g + o];
		data[o] = tmp[0] + tmp[2]; data[g + o] = tmp[1] + complex<float>(0.f, -1.f) * tmp[3]; data[2 * g + o] = tmp[0] - tmp[2]; data[3 * g + o] = tmp[1] + complex<float>(0.f, 1.f) * tmp[3];
	} else if (N > 0) {
		fft(d, data, 2 * g, o);
		fft(d, data, 2 * g, o + g);
		for (unsigned i = 0; i < N; i++) {
			data[g * i + o] = data[2 * g * i + o];
			//tmp[i] = data[2 * g * i + o + g] * exp(complex<float>(0.f, (d? -1.f: 1.f) * (float)M_PI * i / N));
			tmp[i] = data[2 * g * i + o + g] * twiddle[4 * g * i];
		}
		for (unsigned i = 0; i < N; i++) {
			data[g * (i + N) + o] = data[g * i + o] - tmp[i];
			data[g * i + o] += tmp[i];
		}
	}
}


void mdct(const vector<float>& in, vector<float>& out) {
	out.resize(in.size() / 2);
	vector<complex<float> > data(out.size());
	for (unsigned i = 0; i < data.size(); i++)
		//data[i] = sin((float)M_PI / in.size() * (i + 0.5f)) * complex<float>(in[i], -in[in.size() - 1 - i]) * exp(complex<float>(0.f, -(float)M_PI / in.size() * i));
		data[i] = window[i] * complex<float>(in[i], -in[in.size() - 1 - i]) * twiddle[i];
	fft(true, data);
	for (unsigned i = 0; i < data.size(); i++)
		//data[i] *= exp(complex<float>(0.f, -(float)M_PI / out.size() * (0.5f + out.size() / 2.f) * (2.f * i + 0.5f)));
		data[i] *= factor[2 * i];
	for (unsigned i = 0; i < out.size(); i++)
		out[i] = (i & 1? -data[data.size() - 1 - (i - 1) / 2]: data[i / 2]).real();
}


void imdct(const vector<float>& in, vector<float>& out) {
	out.resize(in.size() * 2);
	vector<complex<float> > data(in.size());
	for (unsigned i = 0; i < data.size(); i++)
		//data[i] = in[i] * exp(complex<float>(0.f, -(float)M_PI / in.size() * (0.5f + in.size() / 2.f) * (i + 0.5f)));
		data[i] = in[i] * factor[i];
	fft(true, data);
	for (unsigned i = 0; i < data.size(); i++)
		//data[i] *= exp(complex<float>(0.f, -(float)M_PI / in.size() * i));
		data[i] *= twiddle[2 * i];
	for (unsigned i = 0; i < out.size(); i++)
		//out[i] = sin((float)M_PI / out.size() * (i + 0.5f)) * (i & 1? (i < data.size()? -data[(data.size() - 1 - i) / 2].real(): data[(3 * data.size() - 1 - i) / 2].real()): data[i / 2].real());
		out[i] = window[i] * (i & 1? (i < data.size()? -data[(data.size() - 1 - i) / 2]: data[(3 * data.size() - 1 - i) / 2]): data[i / 2]).real();
}


#include <boost/dynamic_bitset.hpp>
vector<unsigned char> enc(80);

vector<float> encbuf(256);
void encode0(const short* in) {
	static float tmp[256];
	vector<float> input(512), output;
	for (unsigned i = 0; i < 256; i++) {
		input[i] = tmp[i];
		tmp[i] = input[i + 256] = in[i] / 32768.f;
	}
	mdct(input, output);


	vector<float> amps;
	for (unsigned i = 0; i < output.size(); i++)
		amps.push_back(fabs(output[i]));
	sort(amps.begin(), amps.end());
	for (unsigned i = 0; i < 256; i++) {
		float o = fabs(output[i]), s = output[i] < 0.f? -1.f: 1.f;
		int a = (int)(log(o / 256.f * sqrt(2.f)) / log(2.f) - 0.5f);
		if (o < amps[128])
			encbuf[i] = 0;
		else if (o < amps[226])
			encbuf[i] = s * amps[176];
		else
			encbuf[i] = s * pow(2.f, a < -15? -15: a) * 256.f / sqrt(2.f);
	}


	static vector<pair<float, unsigned> > a(256);
	static vector<pair<unsigned char, int> > v(256);
	static boost::dynamic_bitset<unsigned char> bits(enc.size() * 8);

	for (unsigned i = 0; i < output.size(); i++)
		a[i] = make_pair(log(fabs(output[i]) / 256.f * sqrt(2.f)) / log(2.f) - 0.5f, i);
	sort(a.begin(), a.end());

	for (unsigned i = 0; i < 256; i++)
		v[a[i].second] = make_pair((i < 128? 0: i < 226? 5: 6) & (output[a[i].second] < 0? 7: 3), min(15, -(int)a[i].first));

	int amin = min(15, -(int)a[64].first + (int)a[226].first), amid = min(15, -(int)a[176].first + (int)a[226].first);
	bits.set(0, (amin & 1) != 0).set(1, (amin & 2) != 0).set(2, (amin & 4) != 0).set(3, (amin & 8) != 0);
	bits.set(4, (amid & 1) != 0).set(5, (amid & 2) != 0).set(6, (amid & 4) != 0).set(7, (amid & 8) != 0);
	for (unsigned i = 0, b = 128, o = 8; i < 256; i++) {
		if (v[i].first == 0) {
			bits.set(b++, false);
		} else {
			bits.set(b, true).set(b + 1, (v[i].first & 4) != 0).set(b + 2, (v[i].first & 1) != 0);
			b += 3;
		}
		if ((v[i].first & 2) != 0) {
			bits.set(o, (v[i].second & 1) != 0).set(o + 1, (v[i].second & 2) != 0).set(o + 2, (v[i].second & 4) != 0).set(o + 3, (v[i].second & 8) != 0);
			o += 4;
		}
	}

	boost::to_block_range(bits, enc.begin());


	for (unsigned i = 0; i < 256 && testflag; i++) {
		//encbuf[i] = output[i];
	}
}
void decode0(short* out) {
	static boost::dynamic_bitset<unsigned char> bits(enc.size() * 8);
	static vector<int> a(32);

	boost::from_block_range(enc.begin(), enc.end(), bits);

	a[0] = (bits.test(0)? 1: 0) | (bits.test(1)? 2: 0) | (bits.test(2)? 4: 0) | (bits.test(3)? 8: 0);
	a[1] = (bits.test(4)? 1: 0) | (bits.test(5)? 2: 0) | (bits.test(6)? 4: 0) | (bits.test(7)? 8: 0);



	static float tmp[256];
	vector<float> input(256), output;
	for (unsigned i = 0; i < 256; i++)
		input[i] = encbuf[i];
	imdct(input, output);
	for (unsigned i = 0; i < 256; i++) {
		//out[i] = (short)((output[i] + tmp[i]) * 32768.f / 256 * 2);
		output[i] += tmp[i];
		out[i] = output[i] < -128.f? -32768: output[i] > 127.f? 32767: (short)(output[i] * 256.f);
		tmp[i] = output[i + 256];
	}
}

int Audio::callback(const void* in, void* out, unsigned long size, const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void* data) {
	try {
		if (size != framesize)
			throw runtime_error("wrong frame size");

		Audio& a = *(Audio*)data;

		encode0((short*)in);
		//a.encode((short*)in);
		if (!a.playmic) {
			a.network.broadcast(a.encbuf, a.decbuf, maxlatency);
		} else {
			a.decbuf.clear();
			a.decbuf.push_back(a.encbuf);
		}
		//a.decode((short*)out);
		decode0((short*)out);

	} catch (exception& e) {
		cout << "audio failure: " << e.what() << endl;
		return paAbort;
	}

	return paContinue;
}
