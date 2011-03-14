//
// Copyright (c) 2006-2007, Benjamin Kaufmann
//
// This file is part of aspcud.
//
// gringo is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// gringo is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with gringo.  If not, see <http://www.gnu.org/licenses/>.
//

#include <program_opts/value.h>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <cctype>
using namespace std;
namespace ProgramOptions { namespace {
	struct ToLower {
		char operator()(char in) const { return (char)std::tolower(static_cast<unsigned char>(in)); }
	};
	struct ToUpper {
		char operator()(char in) const { return (char)std::toupper(static_cast<unsigned char>(in)); }
	};
}

bool parseValue(const std::string& s, bool& b, int)
{
	string copy = toLower(s);
	if (copy.empty() || copy == "true" || copy == "1" || copy == "yes" || copy == "on")
	{
		b = true;
		return true;
	}
	else if (copy == "false" || copy == "0" || copy == "no" || copy == "off")
	{
		b = false;
		return true;
	}
	return false;
}

bool parseValue(const std::string& s, std::string& r, int)
{
	r = s;
	return true;
}

Value<bool>* bool_switch(bool* b)
{
	Value<bool>* nv = value<bool>(b);
	nv->setImplicit();
	return nv;
}

std::string toLower(const std::string& s) {
	std::string ret; ret.reserve(s.size());
	std::transform(s.begin(), s.end(), std::back_inserter(ret), ToLower());
	return ret;
}

std::string toUpper(const std::string& s) {
	std::string ret; ret.reserve(s.size());
	std::transform(s.begin(), s.end(), std::back_inserter(ret), ToUpper());
	return ret;
}


}
