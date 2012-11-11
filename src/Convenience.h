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

#ifndef CPLIB_CONVENIENCE_H
#define CPLIB_CONVENIENCE_H

#include <sstream>

namespace CPLib {

using namespace std;


template<class T> void minimal(T& val, const T& mi) {
	if (val < mi)
		val = mi;
}
template<class T> void maximal(T& val, const T& ma) {
	if (val > ma)
		val = ma;
}
template<class T> void between(T& val, const T& mi, const T& ma) {
	if (val < mi)
		val = mi;
	if (val > ma)
		val = ma;
}


const struct c_str_t {} c_str = {};
class ss {
	stringstream	ioss;
	string		str;
public:
	template<class T> ss(const T& v) {
		ioss << v;
	}
	template<class T> ss& operator<<(const T& v) {
		ioss << v;
		return *this;
	}
	template<class T> ss& operator>>(T& v) {
		ioss >> v;
		return *this;
	}
	ss& operator<<(ostream& (*)(ostream&));
	ss& operator>>(istream& (*)(istream&));
	const char* operator|(const c_str_t&);
	operator string(void);
};


}


#endif
