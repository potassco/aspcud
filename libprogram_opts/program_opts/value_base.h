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

#ifndef PROGRAM_OPTIONS_VALUE_BASE_H_INCLUDED
#define PROGRAM_OPTIONS_VALUE_BASE_H_INCLUDED
#ifdef _MSC_VER
#pragma warning (disable : 4786)
#pragma warning (disable : 4503)
#endif
#include <string>
namespace ProgramOptions {

//! base class for values of options
class ValueBase
{
protected:
	ValueBase();
public:
	virtual ~ValueBase() = 0;

	//! returns true if the object already stores a value
	virtual bool hasValue() const = 0;

	//! returns true if the value of the option is implicit
	/*!
	* this property is only meaningful for command line options.
	* An implicit option is a flag, i.e an option that does not
	* expect an explicit value. 
	* Example: --help or --version
	*/
	virtual bool isImplicit() const = 0;

	//! returns true if the value of this option can be composed from multiple source
	virtual bool isComposing() const = 0;

	//! returns true if the value currently holds its default value
	virtual bool isDefaulted() const = 0;

	//! sets the value's default value as value.
	/*!
	* \return true if the default value was set
	*/
	virtual bool applyDefault() = 0;

	//! Parses the given string and stores the result in this value.
	/*!
	* \return 
	* - true if the given string contains a valid value
	* - false otherwise
	*/
	virtual bool parse(const std::string&) = 0;
};

}
#endif
