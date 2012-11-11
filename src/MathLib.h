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

#ifndef CPLIB_MATHLIB_H
#define CPLIB_MATHLIB_H


#include <gsl/gsl_math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_fft_complex.h>

#include <valarray>
#include <map>
#include <vector>
#include <complex>
#include <stdexcept>


namespace CPLib {

bool Progress(size_t, size_t);

using namespace std;

struct vec2d {
	double	x;
	double	y;

	vec2d(void) : x(0.), y(0.) {}
	vec2d(double x, double y) : x(x), y(y) {}
	vec2d operator+(const vec2d& v) const {
		return vec2d(x + v.x, y + v.y);
	}
	vec2d operator-(const vec2d& v) const {
		return vec2d(x - v.x, y - v.y);
	}
	double operator*(const vec2d& v) const {
		return x * v.x + y * v.y;
	}
	double operator!() const {
		return sqrt(x * x + y * y);
	}
	vec2d operator*(const double& v) const {
		return vec2d(x * v, y * v);
	}
	vec2d operator/(const double& v) const {
		return vec2d(x / v, y / v);
	}
	double& operator[](unsigned index) {
		return (&x)[index];
	}
	operator double*(void) {
		return &x;
	}
};


struct vec {
	double	x;
	double	y;
	double	z;

	vec(void) : x(0.), y(0.), z(0.) {}
	vec(double x, double y, double z) : x(x), y(y), z(z) {}
	vec(double* m) : x(m[0]), y(m[1]), z(m[2]) {}
	vec(const vec2d& v) : x(v.x), y(v.y), z(0.) {}
	void copy(double* m, unsigned s = 1) {
		m[0] = x;
		m[s] = y;
		m[2 * s] = z;
	}
	vec operator+(const vec& v) const {
		return vec(x + v.x, y + v.y, z + v.z);
	}
	vec operator-(const vec& v) const {
		return vec(x - v.x, y - v.y, z - v.z);
	}
	double operator*(const vec& v) const {
		return x * v.x + y * v.y + z * v.z;
	}
	vec operator/(const vec& v) const {
		return vec(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
	double operator!() const {
		return sqrt(x * x + y * y + z * z);
	}
	vec operator*(const double& v) const {
		return vec(x * v, y * v, z * v);
	}
	vec operator/(const double& v) const {
		return vec(x / v, y / v, z / v);
	}
	double& operator[](unsigned index) {
		return (&x)[index];
	}
	operator double*(void) {
		return &x;
	}
};


struct mat {
	vec	row1;
	vec	row2;
	vec	row3;

	mat(void) {
		row1 = row2 = row3 = vec(0., 0., 0.);
	}
	mat(const vec& v1, const vec& v2, const vec& v3) : row1(v1), row2(v2), row3(v3) {}
	mat(const vec& viewdir, const vec& viewup) {
		row3 = viewdir / !viewdir * -1.;
		row1 = viewup / row3;
		row1 = row1 / !row1;
		row2 = row3 / row1;
		row2 = row2 / !row2;
	}
	mat(double x, double y, double z, double w) {
		row1 = vec(x * x - y * y - z * z + w * w, 2. * (x * y - z * w), 2. * (x * z + y * w));
		row2 = vec(2. * (x * y + z * w), -x * x + y * y - z * z + w * w, 2. * (y * z - x * w));
		row3 = vec(2. * (x * z - y * w), 2. * (y * z + x * w), -x * x - y * y + z * z + w * w);
	}
	mat(const vec& aa) {
		double angle = !aa;
		double x = angle == 0.? 1.: aa.x / angle;
		double y = angle == 0.? 1.: aa.y / angle;
		double z = angle == 0.? 1.: aa.z / angle;
		double c = cos(angle / 180. * M_PI);
		double s = sin(angle / 180. * M_PI);
		row1 = vec(c + x * x * (1. - c), x * y * (1. - c) - z * s, x * z * (1. - c) + y * s);
		row2 = vec(y * x * (1. - c) + z * s, c + y * y * (1. - c), y * z * (1. - c) - x * s);
		row3 = vec(z * x * (1. - c) - y * s, z * y * (1. - c) + x * s, c + z * z * (1. - c));
	}
	void copy(double* m, unsigned sm = 3, unsigned sv = 1) {
		row1.copy(m, sv);
		row2.copy(m + sm, sv);
		row3.copy(m + 2 * sm, sv);
	}
	mat transpose(void) const {
		return mat(vec(row1.x, row2.x, row3.x), vec(row1.y, row2.y, row3.y), vec(row1.z, row2.z, row3.z));
	}
	mat operator+(const mat& m) const {
		return mat(row1 + m.row1, row2 + m.row2, row3 + m.row3);
	}
	mat operator-(const mat& m) const {
		return mat(row1 - m.row1, row2 - m.row2, row3 - m.row3);
	}
	vec operator*(const vec& v) const {
		return vec(row1 * v, row2 * v, row3 * v);
	}
	mat operator*(const mat& m) const {
		mat tmp(m.transpose());
		return mat(tmp * row1, tmp * row2, tmp * row3);
	}
	double operator!() const {
		return (row1 / row2) * row3;
	}
	mat operator*(const double& v) const {
		return mat(row1 * v, row2 * v, row3 * v);
	}
	mat operator/(const double& v) const {
		return mat(row1 / v, row2 / v, row3 / v);
	}
	double& operator()(unsigned row, unsigned col) {
		return (&row1.x)[3 * row + col];
	}
	operator double*(void) {
		return &row1.x;
	}
};


static const vec2d ZEROVEC2D(0., 0.);
static const vec2d NORMXVEC2D(1., 0.);
static const vec2d NORMYVEC2D(0., 1.);

static const vec ZEROVEC(0., 0., 0.);
static const vec NORMXVEC(1., 0., 0.);
static const vec NORMYVEC(0., 1., 0.);
static const vec NORMZVEC(0., 0., 1.);

static const mat ZEROMAT(ZEROVEC, ZEROVEC, ZEROVEC);
static const mat UNITMAT(NORMXVEC, NORMYVEC, NORMZVEC);

ostream& operator<<(ostream&, const vec2d&);
ostream& operator<<(ostream&, const vec&);
ostream& operator<<(ostream&, const mat&);

istream& operator>>(istream&, vec2d&);
istream& operator>>(istream&, vec&);
istream& operator>>(istream&, mat&);


double lingauss(double);
double lingaussd(double);
double interpolate(const double&, const double&, const double&, const double&, const double&);

template<class T> double bilinear(const valarray<T>& mem, unsigned w, unsigned h, unsigned x, unsigned y, double wx, double wy) {
	double m0 = mem[y * w + x];
	double m1 = x + 1 < w? mem[y * w + x + 1]: m0;
	double m2 = y + 1 < h? mem[(y + 1) * w + x]: m0;
	double m3 = x + 1 < w && y + 1 < h? mem[(y + 1) * w + x + 1]: m0;
	return (1. - wx) * (1. - wy) * m0 + wx * (1. - wy) * m1 + (1. - wx) * wy * m2 + wx * wy * m3;
}

double linear(const map<double, double>&, double);
vec getCircle(double, double, double, double, double, double);


class GSLBase {
	map<void*, void (*)(void*)> guards;
	GSLBase(const GSLBase&);
	void operator=(const GSLBase&);
public:
	GSLBase(void);
	~GSLBase(void);
	template<class T> T* guard(T* v, void (*f)(T*)) {
		if (!v)
			throw runtime_error("GSL not enough memory");
		guards[v] = (void (*)(void*))f;
		return v;
	}
};


class RanGen : GSLBase {
	gsl_rng* rangen;
public:
	RanGen(void);
	void operator=(const RanGen&);
	void seed(unsigned);
	double gaussian(double);
	double uniform(void);
};


class FuncFit : GSLBase {
	gsl_vector_view		m_params;
	gsl_vector*		m_param;
	gsl_vector*		m_residue;
	gsl_matrix*		m_jacobi;
	gsl_matrix*		m_mat;
	gsl_vector*		m_vec;
	gsl_vector*		m_step;
	unsigned		m_count;
	size_t			m_size;
	map<size_t, unsigned>	m_map;
public:
	FuncFit(unsigned, vector<double>&, unsigned);
	template<class T, class F> void operator()(double* v, T* object, F f) {
		double min, max, norm;

		max = 1.;
		for (unsigned steps = 0; Progress(steps, 100); steps++) {
			for (unsigned i = 0; i < m_size; i++)
				gsl_vector_set(&m_params.vector, m_map[i], gsl_vector_get(m_param, i));

			for (unsigned i = 0; i < m_count; i++) {
				gsl_vector_set(m_residue, i, object->function(f, i, 0) - v[i]);

				for (unsigned j = 0; j < m_size; j++)
					gsl_matrix_set(m_jacobi, i, j, object->function(f, i, m_map[j] + 1));
			}

			gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1., m_jacobi, m_jacobi, 0., m_mat);
			gsl_blas_dgemv(CblasTrans, 1., m_jacobi, m_residue, 0., m_vec);
			gsl_linalg_HH_solve(m_mat, m_vec, m_step);

			norm = gsl_blas_dnrm2(m_param);
			if (norm == 0.)
				norm = 1.;
			min = gsl_blas_dnrm2(m_step) / norm;

			if (min > max) {
				gsl_blas_daxpy( -0.1, m_step, m_param);
				max /= 1.1;
			} else {
				gsl_blas_daxpy( -1., m_step, m_param);
				max *= 1.1;
			}

			if (min < 0.00001)
				break;
  		}
	}
};


class FFT : GSLBase {
	size_t				m_n1;
	size_t				m_n2;
	size_t				m_n3;
	gsl_fft_complex_wavetable*	m_wt1;
	gsl_fft_complex_wavetable*	m_wt2;
	gsl_fft_complex_wavetable*	m_wt3;
	gsl_fft_complex_workspace*	m_ws1;
	gsl_fft_complex_workspace*	m_ws2;
	gsl_fft_complex_workspace*	m_ws3;
public:
	FFT(size_t);
	FFT(size_t, size_t);
	FFT(size_t, size_t, size_t);
	void operator()(valarray<complex<double> >&, bool);
};


}

#endif
