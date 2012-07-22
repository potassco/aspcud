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

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>


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
    struct Crit
    {
        bool optimize;
        unsigned measurement;
        unsigned selector;
        std::string attr1;
        std::string attr2;
    };
}

BOOST_FUSION_ADAPT_STRUCT(
    ProgramOptions::Crit,
    (bool, optimize)
    (unsigned, measurement)
    (unsigned, selector)
    (std::string, attr1)
    (std::string, attr2)
)

namespace ProgramOptions
{
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;

    template <typename Iterator>
    struct CritParser : qi::grammar<Iterator, std::vector<Crit>()>
    {
        CritParser()
            : CritParser::base_type(crits)
        {
            using qi::lit;
            using qi::fail;
            using qi::lexeme;
            using qi::on_error;
            using ascii::char_;
            using boost::spirit::eoi;
            using boost::spirit::_val;
            using boost::phoenix::val;
            using boost::phoenix::construct;

            SELECTOR = 
                lit("solution") [ _val = 0 ] | 
                lit("changed") [ _val = 1 ] |
                lit("new") [ _val = 2 ] |
                lit("removed") [ _val = 3 ] |
                lit("up") [ _val = 4 ] |
                lit("down") [ _val = 5 ];
            SIGN = 
                char_('+') [ _val = true ] |
                char_('-') [ _val = false ];
            COUNT = lit("count") [_val = 0];
            SUM = lit("sum") [_val = 1];
            NOTUPTODATE = lit("notuptodate") [_val = 2];
            UNSAT_RECOMMENDS = lit("unsat_recommends") [_val = 3];
            ALIGNED = lit("aligned") [_val = 4];
            ATTR %= lexeme[char_('a', 'z') >> *char_("[a-z][0-9]\\-")];

            unary %= SIGN >> (COUNT | NOTUPTODATE | UNSAT_RECOMMENDS) >> '(' >> SELECTOR >> ')';
            binary %= SIGN >> SUM >> '(' >> SELECTOR >> ',' >> ATTR >> ')';
            ternary %= SIGN >> ALIGNED >> '(' >> SELECTOR >> ',' >> ATTR >> ',' >> ATTR >> ')';
            crits %= (unary | binary | ternary) > *(',' > (unary | binary | ternary)) > eoi;

            on_error<fail>
            (
                crits,
                std::cerr
                    << val("Error: could not parse: ")
                    << construct<std::string>(qi::_3, qi::_2)
                    << val("\"")
                    << std::endl
            );        
        }
        qi::rule<Iterator, unsigned()> SELECTOR;
        qi::rule<Iterator, bool()> SIGN;
        qi::rule<Iterator, unsigned()> COUNT, SUM, NOTUPTODATE, UNSAT_RECOMMENDS, ALIGNED;
        qi::rule<Iterator, std::string()> ATTR; 

        qi::rule<Iterator, Crit()> unary, binary, ternary;
        qi::rule<Iterator, std::vector<Crit>()> crits;
    };

	template <>
	bool parseValue(const std::string& s, Dependency::Criteria& criteria, double)
	{
		std::string lower = toLower(s);
        std::vector<Crit> crits;
		if(lower == "paranoid")
		{
			criteria.removed = -2;
			criteria.changed = -1;
            crits.push_back(Crit());
            crits.back().optimize = false;
            crits.back().measurement = 0;
            crits.back().selector = 3;
            crits.push_back(Crit());
            crits.back().optimize = false;
            crits.back().measurement = 0;
            crits.back().selector = 1;
		}
		else if(lower == "none") { }
		else if (!qi::parse(lower.begin(), lower.end(), CritParser<std::string::iterator>(), crits))
        {
            return false; 
        }
        std::cerr << "criteria read: " << std::endl;
        foreach(Crit &crit, crits)
        {
            std::cerr << "\t" << (crit.optimize ? "+" : "-");
            switch (crit.measurement)
            {
                case 0: { std::cerr << "count("; break; }
                case 1: { std::cerr << "sum("; break; }
                case 2: { std::cerr << "notuptodate("; break; }
                case 3: { std::cerr << "unsat_recommends("; break; }
                case 4: { std::cerr << "aligned("; break; }
            }
            switch (crit.selector)
            {
                case 0: { std::cerr << "solution"; break; }
                case 1: { std::cerr << "changed"; break; }
                case 2: { std::cerr << "new"; break; }
                case 3: { std::cerr << "removed"; break; }
                case 4: { std::cerr << "up"; break; }
                case 5: { std::cerr << "down"; break; }
            }
            if (!crit.attr1.empty())
            {
                std::cerr << "," << crit.attr1;
            }
            if (!crit.attr2.empty())
            {
                std::cerr << "," << crit.attr2;
            }
            std::cerr << ")" << std::endl;
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
		Dependency d(opts.criteria, opts.addAll, opts.generic.verbose > 0);
		Parser p(d);
		if(opts.generic.input.empty() || opts.generic.input.front() == "-") { p.parse(std::cin); }
		else
		{
			std::ifstream in(opts.generic.input.front().c_str());
			p.parse(in);
		}
		d.closure();
		d.dumpAsFacts(std::cout);
		return EXIT_SUCCESS;
	}
	catch(const std::exception& e)
	{
		std::cerr << "\nERROR: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
