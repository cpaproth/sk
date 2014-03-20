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
#include <boost/dynamic_bitset.hpp>


using namespace SK;


#include <complex>
vector<complex<float> > twiddle(512);
vector<complex<float> > factor(512);
vector<float> window(512);

Audio::Audio(Network& nw) : encbuf(encsize + 1), network(nw) {
	for (unsigned i = 0; i < twiddle.size(); i++)
		twiddle[i] = exp(complex<float>(0.f, -(float)M_PI * i / twiddle.size()));
	for (unsigned i = 0; i < factor.size(); i++)
		factor[i] = exp(complex<float>(0.f, -(float)M_PI / factor.size() * (1.f + factor.size() / 2.f) * (i + 0.5f)));
	for (unsigned i = 0; i < window.size(); i++)
		window[i] = sin((float)M_PI / window.size() * (i + 0.5f));

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
		data[i] = window[i] * complex<float>(in[i], -in[in.size() - 1 - i]) * twiddle[i];
	fft(true, data);
	for (unsigned i = 0; i < data.size(); i++)
		data[i] *= factor[2 * i];
	for (unsigned i = 0; i < out.size(); i++)
		out[i] = (i & 1? -data[data.size() - 1 - (i - 1) / 2]: data[i / 2]).real();
}


void imdct(const vector<float>& in, vector<float>& out) {
	out.resize(in.size() * 2);
	vector<complex<float> > data(in.size());
	for (unsigned i = 0; i < data.size(); i++)
		data[i] = in[i] * factor[i];
	fft(true, data);
	for (unsigned i = 0; i < data.size(); i++)
		data[i] *= twiddle[2 * i];
	for (unsigned i = 0; i < out.size(); i++)
		out[i] = window[i] * (i & 1? (i < data.size()? -data[(data.size() - 1 - i) / 2]: data[(3 * data.size() - 1 - i) / 2]): data[i / 2]).real();
}


void Audio::encode(const short* in) {
	static unsigned frame = 0;
	static float tmp[framesize];
	static vector<pair<float, unsigned> > a(framesize);
	static vector<pair<unsigned char, int> > v(framesize);
	static boost::dynamic_bitset<unsigned char> bits(encsize * 8);

	frame++;

	vector<float> input(2 * framesize), output;
	for (unsigned i = 0; i < framesize; i++) {
		input[i] = tmp[i];
		tmp[i] = input[i + framesize] = in[i] / 32768.f;
	}
	mdct(input, output);


	for (unsigned i = 0; i < framesize; i++)
		a[i] = make_pair(output[i] == 0.f? -31.f: log(fabs(output[i]) / framesize * sqrt(2.f)) / log(2.f), i);
	sort(a.begin(), a.end());
	int m = -(int)(a[176].first * 8.f - 0.5f);
	for (unsigned i = 0; i < framesize; i++)
		v[a[i].second] = make_pair((i < 128? 0: i < 226? 5: 6) & (output[a[i].second] < 0? 7: 3), max(0, min(15, (int)(a[i].first - 0.5f) + m / 8)));
	
	bits.set(0, (m & 1) != 0).set(1, (m & 2) != 0).set(2, (m & 4) != 0).set(3, (m & 8) != 0).set(4, (m & 16) != 0).set(5, (m & 32) != 0).set(6, (m & 64) != 0).set(7, (m & 128) != 0);
	for (unsigned i = 0, b = 128, o = 8; i < framesize; i++) {
		if (v[i].first == 0) {
			bits.set(b++, true);
		} else {
			bits.set(b, false).set(b + 1, (v[i].first & 1) != 0).set(b + 2, (v[i].first & 4) != 0);
			b += 3;
		}
		if ((v[i].first & 2) != 0) {
			bits.set(o, (v[i].second & 1) != 0).set(o + 1, (v[i].second & 2) != 0).set(o + 2, (v[i].second & 4) != 0).set(o + 3, (v[i].second & 8) != 0);
			o += 4;
		}
	}

	boost::to_block_range(bits, encbuf.begin() + 1);
}
void Audio::decode(short* out) {
	static float tmp[framesize];
	static boost::dynamic_bitset<unsigned char> bits(encsize * 8);
	static vector<int> a(30);
	static unsigned r = 11113;

	if (decbuf.size() == 0 || decbuf[0].size() != encsize + 1)
		return;

	boost::from_block_range(decbuf[0].begin() + 1, decbuf[0].end(), bits);

	int m = (bits.test(0)? 1: 0) | (bits.test(1)? 2: 0) | (bits.test(2)? 4: 0) | (bits.test(3)? 8: 0) | (bits.test(4)? 16: 0) | (bits.test(5)? 32: 0) | (bits.test(6)? 64: 0) | (bits.test(7)? 128: 0);
	for (unsigned i = 8; i < 128; i += 4)
		a[(i - 8) / 4] = (bits.test(i)? 1: 0) | (bits.test(i + 1)? 2: 0) | (bits.test(i + 2)? 4: 0) | (bits.test(i + 3)? 8: 0);

	vector<float> input(framesize), output;

	for (unsigned i = 0, b = 128, o = 0; i < framesize; i++) {
		if (bits.test(b++))
			input[i] = (((r = r * 75 % 65537) & 8) != 0? -1.f: 1.f) * pow(2.f, -m / 8.f - 3.f) * framesize / sqrt(2.f);
		else if (bits.test(b++))
			input[i] = (bits.test(b++)? -1.f: 1.f) * pow(2.f, -m / 8.f) * framesize / sqrt(2.f);
		else
			input[i] = (bits.test(b++)? -1.f: 1.f) * pow(2.f, a[o++] - m / 8) * framesize / sqrt(2.f);
	}

	imdct(input, output);

	for (unsigned i = 0; i < framesize; i++) {
		output[i] += tmp[i];
		out[i] = output[i] < -128.f? -32768: output[i] > 127.f? 32767: (short)(output[i] * framesize);
		tmp[i] = output[i + framesize];
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
