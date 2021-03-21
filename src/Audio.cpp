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


#include "Audio.h"
#include "Network.h"
#include <iostream>


using namespace SK;


Transform::Transform(size_t s) : twiddle(2 * s), factor(2 * s), window(2 * s), data(s), tmp(max<size_t>(4, s / 2)), tdata(2 * s), fdata(s) {
	const float pi = 3.141592654f;
	for (unsigned i = 0; i < 2 * s; i++) {
		twiddle[i] = exp(complex<float>(0.f, -pi * i / twiddle.size()));
		factor[i] = exp(complex<float>(0.f, -pi / factor.size() * (1.f + factor.size() / 2.f) * (i + 0.5f)));
		window[i] = sin(pi / window.size() * (i + 0.5f));
	}
}


void Transform::fft(unsigned g, unsigned o) {
	size_t N = data.size() / g / 2;
	if (N == 2) {
		tmp[0] = data[o] + data[2 * g + o]; tmp[1] = data[o] - data[2 * g + o]; tmp[2] = data[g + o] + data[3 * g + o]; tmp[3] = data[g + o] - data[3 * g + o];
		data[o] = tmp[0] + tmp[2]; data[g + o] = tmp[1] + complex<float>(0.f, -1.f) * tmp[3]; data[2 * g + o] = tmp[0] - tmp[2]; data[3 * g + o] = tmp[1] + complex<float>(0.f, 1.f) * tmp[3];
	} else if (N > 0) {
		fft(2 * g, o);
		fft(2 * g, o + g);
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


void Transform::mdct() {
	for (unsigned i = 0; i < data.size(); i++)
		data[i] = window[i] * complex<float>(tdata[i], -tdata[tdata.size() - 1 - i]) * twiddle[i];
	fft();
	for (unsigned i = 0; i < data.size(); i++)
		data[i] *= factor[2 * i];
	for (unsigned i = 0; i < fdata.size(); i++)
		fdata[i] = (i & 1? -data[data.size() - 1 - (i - 1) / 2]: data[i / 2]).real();
}


void Transform::imdct() {
	for (unsigned i = 0; i < data.size(); i++)
		data[i] = fdata[i] * factor[i];
	fft();
	for (unsigned i = 0; i < data.size(); i++)
		data[i] *= twiddle[2 * i];
	for (unsigned i = 0; i < tdata.size(); i++)
		tdata[i] = window[i] * (i & 1? (i < data.size()? -data[(data.size() - 1 - i) / 2]: data[(3 * data.size() - 1 - i) / 2]): data[i / 2]).real();
}


Audio::Audio(Network& nw) : trafo(framesize), bits(8 * encsize), encbuf(encsize + 1), network(nw) {
	stream = 0;
	playmic = false;
	noisegate = true;
	mute = false;
	frame = 0;
	rnd = 11113;
	threshold = INT_MAX;
	noisecount = 0;

	enctmp.resize(framesize, 0.f);
	dectmp.resize(framesize, 0.f);
	a.resize(framesize);
	v.resize(framesize);

	initerror = Pa_Initialize() != paNoError;
	if (initerror)
		cout << "audio initialization failed" << endl;
}


Audio::~Audio() {
	try {
		if (stream) {
			if (!Pa_IsStreamStopped(stream))
				Pa_StopStream(stream);
			Pa_CloseStream(stream);
		}
		if (!initerror)
			Pa_Terminate();

		cout << "audio stream closed" << endl;
	} catch (...) {}
}


void Audio::restart() {
	if (initerror || (!stream && Pa_OpenDefaultStream(&stream, 1, 1, paInt16, samplerate, framesize, &callback, this) != paNoError))
		throw runtime_error("open audio stream failed");
	if (!Pa_IsStreamStopped(stream)) {
		cout << "audio cpu load: " << Pa_GetStreamCpuLoad(stream) << endl;
		if (Pa_StopStream(stream) == paNoError)
			cout << "audio stream stopped" << endl;
	}
	threshold = INT_MAX;
	noisecount = 0;
	if (Pa_StartStream(stream) == paNoError)
		cout << "audio stream started" << endl;
	else
		throw runtime_error("start audio stream failed");
}


void Audio::toggle_playmic() {
	playmic = !playmic;
}

void Audio::toggle_noisegate() {
	noisegate = !noisegate;
}


void Audio::mute_mic(bool m) {
	mute = m;
}


void Audio::encode(const short* in) {
	int amp = *max_element(in, in + framesize) - *min_element(in, in + framesize);
	threshold *= noisecount > 30? 2: 1;
	noisecount = amp / 2 < threshold || noisecount > 30? 0: noisecount + 1;
	if (amp > 0 && amp < threshold)
		threshold = amp;

	float scale = mute && !playmic? 0.f: (noisegate && amp / 8 < threshold? (amp - threshold) / 7.f / threshold: 1.f) / 32768.f;
	for (unsigned i = 0; i < framesize; i++) {
		trafo.t(i) = enctmp[i];
		enctmp[i] = trafo.t(i + framesize) = in[i] * scale;
	}
	trafo.mdct();

	for (unsigned i = 0; i < framesize; i++)
		a[i] = make_pair(max(-31.f, log(fabs(trafo.f(i)) / framesize * sqrt(2.f)) / log(2.f)), i);
	sort(a.begin(), a.end());

	int m = -(int)(a[176].first * 8.f - 0.5f);
	for (unsigned i = 0; i < framesize; i++)
		v[a[i].second] = make_pair((i < 128? 0: i < 226? 5: 6) & (trafo.f(a[i].second) < 0.f? 7: 3), max(0, min(15, (int)(a[i].first - 0.5f) + m / 8)));
	
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

	encbuf[0] = 64 | ((frame++) & 63);
	boost::to_block_range(bits, encbuf.begin() + 1);
}


void Audio::decode(short* out) {
	fill(&trafo.f(0), &trafo.f(0) + framesize, 0.f);
	for (unsigned i = 0; i < decbuf.size(); i++) {
		if (decbuf[i].size() != encbuf.size())
			continue;

		boost::from_block_range(decbuf[i].begin() + 1, decbuf[i].end(), bits);

		int a[30], m = (bits.test(0)? 1: 0) | (bits.test(1)? 2: 0) | (bits.test(2)? 4: 0) | (bits.test(3)? 8: 0) | (bits.test(4)? 16: 0) | (bits.test(5)? 32: 0) | (bits.test(6)? 64: 0) | (bits.test(7)? 128: 0);
		for (unsigned j = 8; j < 128; j += 4)
			a[(j - 8) / 4] = (bits.test(j)? 1: 0) | (bits.test(j + 1)? 2: 0) | (bits.test(j + 2)? 4: 0) | (bits.test(j + 3)? 8: 0);

		for (unsigned j = 0, b = 128, o = 0; j < framesize; j++) {
			if (bits.test(b++))
				trafo.f(j) += (((rnd = rnd * 75 % 65537) & 8) != 0? -1.f: 1.f) * pow(2.f, -m / 8.f - 3.f) * framesize / sqrt(2.f);
			else if (bits.test(b++))
				trafo.f(j) += (bits.test(b++)? -1.f: 1.f) * pow(2.f, -m / 8.f) * framesize / sqrt(2.f);
			else
				trafo.f(j) += (bits.test(b++)? -1.f: 1.f) * pow(2.f, a[o++] - m / 8) * framesize / sqrt(2.f);
		}
	}

	trafo.imdct();
	for (unsigned i = 0; i < framesize; i++) {
		trafo.t(i) += dectmp[i];
		out[i] = trafo.t(i) < -128.f? -32768: trafo.t(i) > 127.f? 32767: (short)(trafo.t(i) * framesize);
		dectmp[i] = trafo.t(i + framesize);
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
