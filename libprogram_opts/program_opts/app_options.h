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

#ifndef APP_OPTIONS_H_INCLUDED
#define APP_OPTIONS_H_INCLUDED

#ifdef _MSC_VER
#pragma warning (disable : 4200) // nonstandard extension used : zero-sized array
#pragma once
#endif

#include <string>
#include <utility>
#include "program_options.h"

typedef std::vector<std::string> StringSeq;
struct Messages {
	std::string error;
	StringSeq   warning;
	void clear() { error.clear(); warning.clear(); }
};

/////////////////////////////////////////////////////////////////////////////////////////
// generic options
/////////////////////////////////////////////////////////////////////////////////////////
// Extends group "Basic Options"
struct GenericOptions {
	GenericOptions();
	void initOptions(ProgramOptions::OptionGroup& root, ProgramOptions::OptionGroup& hidden);
	void addDefaults(std::string& def);
	static bool mapLevel(const std::string& s, int& level);
	StringSeq  input;    // positional options (list of files)
	int        verbose;  // verbosity level, default:  0
	bool       help;     // print help and exit
	bool       version;  // print version and exit
	bool       validateOptions(ProgramOptions::OptionValues&, Messages&) { return true; }
};

/////////////////////////////////////////////////////////////////////////////////////////
// Interface for parsing application options
/////////////////////////////////////////////////////////////////////////////////////////
class AppOptions {
public:
	AppOptions() {}
	virtual ~AppOptions() {}
	bool parse(int argc, char** argv, ProgramOptions::PosOption p = 0);
	
	// n warnings and at most one error
	Messages messages;
	
	// Only set if --help is given
	const std::string& getHelp()    const  { return help_;    }
	const std::string& getDefaults()const  { return defaults_; }
	GenericOptions generic;
private:
	AppOptions(const AppOptions&);
	AppOptions& operator=(const AppOptions&);
	virtual void initOptions(ProgramOptions::OptionGroup& root, ProgramOptions::OptionGroup& hidden) = 0;
	virtual void addDefaults(std::string& defaults) = 0;
	virtual bool validateOptions(ProgramOptions::OptionValues&, Messages&) = 0;
	std::string help_;
	std::string defaults_;
};

#endif

