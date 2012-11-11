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

#include "MathLib.h"

#include <gsl/gsl_randist.h>

#include <bitset>
#include <iomanip>


namespace CPLib {


ostream& operator<<(ostream& s, const vec2d& v) {
	return s << setprecision(15) << v.x << ' ' << v.y;
}
ostream& operator<<(ostream& s, const vec& v) {
	return s << setprecision(15) << v.x << ' ' << v.y << ' ' << v.z;
}
ostream& operator<<(ostream& s, const mat& m) {
	return s << m.row1 << ',' << m.row2 << ',' << m.row3;
}

istream& operator>>(istream& s, vec2d& v) {
	return s >> v.x >> v.y;
}
istream& operator>>(istream& s, vec& v) {
	return s >> v.x >> v.y >> v.z;
}
istream& operator>>(istream& s, mat& m) {
	char c;
	return s >> m.row1 >> c >> m.row2 >> c >> m.row3;
}


double lingauss(double x) {
	if (x > -1. / sqrt(2.))
		return exp(-x * x);
	else
		return (sqrt(2.) * x + 2.) * exp(-1. / 2.);
}
double lingaussd(double x) {
	if (x > -1. / sqrt(2.))
		return -2. * x * exp(-x * x);
	else
		return sqrt(2.) * exp(-1. / 2.);
}


double interpolate(const double& x, const double& x1, const double& y1, const double& x2, const double& y2) {
	if (x2 - x1 == 0.)
		return y1;
	return (y2 - y1) / (x2 - x1) * (x - x1) + y1;
}


double linear(const map<double, double>& values, double x) {
	if (values.size() == 0)
		return 0.;

	map<double, double>::const_iterator it1 = values.lower_bound(x), it2;
	if (it1 == values.end())
		return (--it1)->second;
	if (it1 == values.begin())
		return it1->second;

	it2 = it1--;
	return interpolate(x, it1->first, it1->second, it2->first, it2->second);
}


vec getCircle(double x1, double y1, double x2, double y2, double x3, double y3) {
	vec s1(1., 1., 1.);
	vec s2(x1, x2, x3);
	vec s3(y1, y2, y3);
	double d = s1 * (s2 / s3);
	if (d == 0.)
		return ZEROVEC;
	mat m(s2 / s3, s3 / s1, s1 / s2);
	vec v(x1 * x1 + y1 * y1, x2 * x2 + y2 * y2, x3 * x3 + y3 * y3);
	vec r = m * v / d;
	return vec(r.y / 2., r.z / 2., sqrt(r.x + r.y * r.y / 4. + r.z * r.z / 4.));
}


GSLBase::GSLBase(void) {
	gsl_set_error_handler_off();
}
GSLBase::~GSLBase(void) {
	for (map<void*, void (*)(void*)>::iterator it = guards.begin(); it != guards.end(); it++)
		it->second(it->first);
}


RanGen::RanGen(void) {
	rangen = guard(gsl_rng_alloc(gsl_rng_ranlxd2), &gsl_rng_free);
}
void RanGen::operator=(const RanGen& r) {
	gsl_rng_memcpy(rangen, r.rangen);
}
void RanGen::seed(unsigned s) {
	gsl_rng_set(rangen, s);
}
double RanGen::gaussian(double sigma) {
	return gsl_ran_gaussian(rangen, sigma);
}
double RanGen::uniform(void) {
	return gsl_rng_uniform(rangen);
}


FuncFit::FuncFit(unsigned count, vector<double>& params, unsigned mask) {
	m_size		= params.size();
	m_count		= count;
	m_params	= gsl_vector_view_array(&params[0], m_size);

	bitset<8 * sizeof(unsigned)> ignore(mask);
	m_map.clear();
	for (unsigned i = 0; i < m_size; i++) {
		if (!ignore[i])
			m_map[m_map.size()] = i;
	}
	if (m_map.size() == 0)
		m_map[0] = 0;

	m_size = m_map.size();

	m_param		= guard(gsl_vector_alloc(m_size), &gsl_vector_free);
	m_residue	= guard(gsl_vector_alloc(m_count), &gsl_vector_free);
	m_jacobi	= guard(gsl_matrix_alloc(m_count, m_size), &gsl_matrix_free);
	m_mat		= guard(gsl_matrix_alloc(m_size, m_size), &gsl_matrix_free);
	m_vec		= guard(gsl_vector_alloc(m_size), &gsl_vector_free);
	m_step		= guard(gsl_vector_alloc(m_size), &gsl_vector_free);

	for (unsigned i = 0; i < m_size; i++)
		gsl_vector_set(m_param, i, params[m_map[i]]);
}


FFT::FFT(size_t n) : m_n1(n), m_n2(0), m_n3(0) {
	m_wt1 = guard(gsl_fft_complex_wavetable_alloc(n), &gsl_fft_complex_wavetable_free);
	m_ws1 = guard(gsl_fft_complex_workspace_alloc(n), &gsl_fft_complex_workspace_free);
}
FFT::FFT(size_t n1, size_t n2) : m_n1(n1), m_n2(n2), m_n3(0) {
	m_wt1 = guard(gsl_fft_complex_wavetable_alloc(n1), &gsl_fft_complex_wavetable_free);
	m_ws1 = guard(gsl_fft_complex_workspace_alloc(n1), &gsl_fft_complex_workspace_free);
	m_wt2 = guard(gsl_fft_complex_wavetable_alloc(n2), &gsl_fft_complex_wavetable_free);
	m_ws2 = guard(gsl_fft_complex_workspace_alloc(n2), &gsl_fft_complex_workspace_free);
}
FFT::FFT(size_t n1, size_t n2, size_t n3) : m_n1(n1), m_n2(n2), m_n3(n3) {
	m_wt1 = guard(gsl_fft_complex_wavetable_alloc(n1), &gsl_fft_complex_wavetable_free);
	m_ws1 = guard(gsl_fft_complex_workspace_alloc(n1), &gsl_fft_complex_workspace_free);
	m_wt2 = guard(gsl_fft_complex_wavetable_alloc(n2), &gsl_fft_complex_wavetable_free);
	m_ws2 = guard(gsl_fft_complex_workspace_alloc(n2), &gsl_fft_complex_workspace_free);
	m_wt3 = guard(gsl_fft_complex_wavetable_alloc(n3), &gsl_fft_complex_wavetable_free);
	m_ws3 = guard(gsl_fft_complex_workspace_alloc(n3), &gsl_fft_complex_workspace_free);
}
void FFT::operator()(valarray<complex<double> >& data, bool forward) {
	gsl_fft_direction dir = forward? gsl_fft_forward: gsl_fft_backward;

	if (m_n1 * (m_n2 == 0? 1: m_n2) * (m_n3 == 0? 1: m_n3) > data.size())
		return;

	if (m_n2 == 0) {
		gsl_fft_complex_transform((double*)&data[0], 1, m_n1, m_wt1, m_ws1, dir);
		return;
	}
	if (m_n3 == 0) {
		for (unsigned y = 0; Progress(y, m_n2); y++)
			gsl_fft_complex_transform((double*)&data[y * m_n1], 1, m_n1, m_wt1, m_ws1, dir);
		for (unsigned x = 0; Progress(x, m_n1); x++)
			gsl_fft_complex_transform((double*)&data[x], m_n1, m_n2, m_wt2, m_ws2, dir);
		return;
	}
	for (unsigned z = 0; Progress(z, m_n3); z++)
		for (unsigned y = 0; y < m_n2; y++)
			gsl_fft_complex_transform((double*)&data[z * m_n1 * m_n2 + y * m_n1], 1, m_n1, m_wt1, m_ws1, dir);
	for (unsigned x = 0; Progress(x, m_n1); x++)
		for (unsigned z = 0; z < m_n3; z++)
			gsl_fft_complex_transform((double*)&data[z * m_n1 * m_n2 + x], m_n1, m_n2, m_wt2, m_ws2, dir);
	for (unsigned y = 0; Progress(y, m_n2); y++)
		for (unsigned x = 0; x < m_n1; x++)
			gsl_fft_complex_transform((double*)&data[y * m_n1 + x], m_n1 * m_n2, m_n3, m_wt3, m_ws3, dir);
}


}
