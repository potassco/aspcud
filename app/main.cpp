//
// Copyright (c) 2010, Roland Kaminski <kaminski@cs.uni-potsdam.de>
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

#include <iostream>
#include <cudf/parser.h>
#include <program_opts/app_options.h>
#include <program_opts/value.h>
#include <stdexcept>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH


#define CUDF_EXECUTABLE "cudf"
#define CUDF_USAGE "[options] [file]"

class CudfOptions : public AppOptions
{
public:
	CudfOptions();

private:
	virtual void initOptions(ProgramOptions::OptionGroup& root, ProgramOptions::OptionGroup& hidden);
	virtual void addDefaults(std::string& defaults);
	virtual bool validateOptions(ProgramOptions::OptionValues&, Messages&);

public:
	Dependency::Criteria criteria;
	bool                 addAll;
};

namespace ProgramOptions
{
	bool setCrit(char sign, int &crit, int prio)
	{
		if     (crit != 0)   { return false; }
		else if(sign == '-') { crit = -prio; }
		else if(sign == '+') { crit =  prio; }
		else                 { return false; }
		return true;
	}

	template <>
	bool parseValue(const std::string& s, Dependency::Criteria& criteria, double)
	{
		std::string lower = toLower(s);
		if(lower == "paranoid")
		{
			criteria.removed = -2;
			criteria.changed = -1;
		}
		else if(lower == "trendy")
		{
			criteria.removed          = -4;
			criteria.notuptodate      = -3;
			criteria.unsat_recommends = -2;
			criteria.newpkg           = -1;
		}
		else if(lower == "none") { }
		else
		{
			std::vector<std::string> strs;
			boost::split(strs, lower, boost::is_any_of(","));
			if(strs.empty()) { return false; }
			int prio = 1;
			foreach(const std::string &tok, strs | boost::adaptors::reversed)
			{
				if(tok.empty()) { return false; }
				std::string sub = tok.substr(1);
				if     (sub == "removed")          { if(!setCrit(tok[0], criteria.removed, prio))          { return false; } }
				else if(sub == "new")              { if(!setCrit(tok[0], criteria.newpkg, prio))           { return false; } }
				else if(sub == "changed")          { if(!setCrit(tok[0], criteria.changed, prio))          { return false; } }
				else if(sub == "notuptodate")      { if(!setCrit(tok[0], criteria.notuptodate, prio))      { return false; } }
				else if(sub == "unsat_recommends") { if(!setCrit(tok[0], criteria.unsat_recommends, prio)) { return false; } }
				else                               { return false; }
				prio++;
			}
		}
		return true;
	}
}

CudfOptions::CudfOptions()
	: addAll(false)
{
}

void CudfOptions::addDefaults(std::string& defaults)
{
	defaults += "--criteria=none\n";
}

void CudfOptions::initOptions(ProgramOptions::OptionGroup& root, ProgramOptions::OptionGroup& hidden)
{
	using namespace ProgramOptions;
	OptionGroup prepro("Preprocessing Options");
	prepro.addOptions()
		("criteria,c", storeTo(criteria),
			"Preprocess for specific optimization criteria\n"
			"      Default: none\n"
			"      Valid:   none, paranoid, trendy, -|+<crit>(,-|+<crit>)*\n"
			"        <crit>: removed, new, changed, notuptodate, or unsat_recommends\n")
		("addall", bool_switch(&addAll),
			"Disable preprocessing and add all packages\n");

	root.addOptions(prepro);
}

bool CudfOptions::validateOptions(ProgramOptions::OptionValues&, Messages&msg)
{
	if(generic.input.size() > 1)
	{
		msg.error = "at most one file may be given";
		return false;
	}
	return true;
}

bool parsePositional(const std::string&, std::string& out)
{
	out = "file";
	return true;
}

int main(int argc, char *argv[])
{
	try
	{
		CudfOptions opts;
		if(!opts.parse(argc, argv, parsePositional)) { throw std::runtime_error( opts.messages.error.c_str() ); }
		if(opts.generic.help)
		{
			std::cout
				<< CUDF_EXECUTABLE << " version " << CUDF_VERSION << "\n\n"
				<< "Usage: " << CUDF_EXECUTABLE << " " << CUDF_USAGE << "\n"
				<< opts.getHelp() << "\n"
				<< "Usage: " << CUDF_EXECUTABLE << " " << CUDF_USAGE << "\n\n"
				<< "Default commandline: \n"
				<< "  " << CUDF_EXECUTABLE << " " << opts.getDefaults() << std::endl;
			return EXIT_SUCCESS;
		}
		if(opts.generic.version)
		{
			std::cout
				<< CUDF_EXECUTABLE << " " << CUDF_VERSION << "\n\n"
				<< "Copyright (C) Roland Kaminski" << "\n"
				<< "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n"
				<< "Gringo is free software: you are free to change and redistribute it.\n"
				<< "There is NO WARRANTY, to the extent permitted by law." << std::endl;
			return EXIT_SUCCESS;
		}
		Dependency d(opts.criteria, opts.generic.verbose > 0);
		Parser p(d);
		if(opts.generic.input.empty() || opts.generic.input.front() == "-") { p.parse(std::cin); }
		else
		{
			std::ifstream in(opts.generic.input.front().c_str());
			p.parse(in);
		}
		d.closure(opts.addAll);
		d.dumpAsFacts(std::cout);
		return EXIT_SUCCESS;
	}
	catch(const std::exception& e)
	{
		std::cerr << "\nERROR: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
