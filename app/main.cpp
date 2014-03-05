//////////////////// Copyright ////////////////////////////////// {{{1

//
// Copyright (c) 2010, Roland Kaminski <kaminski@cs.uni-potsdam.de>
//
// This file is part of aspcud.
//
// aspcud is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// aspcud is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with aspcud.  If not, see <http://www.gnu.org/licenses/>.
//

//////////////////// Preamble /////////////////////////////////// {{{1

#include <iostream>
#include <cudf/version.h>
#include <cudf/parser.h>
#include <cudf/critparser.h>
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

//////////////////// CudfOptions //////////////////////////////// {{{1

class CudfOptions : public AppOptions {
public:
    CudfOptions();

private:
    virtual void initOptions(ProgramOptions::OptionGroup& root, ProgramOptions::OptionGroup& hidden);
    virtual void addDefaults(std::string& defaults);
    virtual bool validateOptions(ProgramOptions::OptionValues&, Messages&);

public:
    Criteria::CritVec crits;
    bool              addAll;
};

//////////////////// CritParser ///////////////////////////////// {{{1

namespace ProgramOptions {
    template <>
    bool parseValue(const std::string& s, Criteria::CritVec& crits, int) {
        std::string lower = toLower(s);
        if (lower == "paranoid") {
            crits.push_back(Criterion());
            crits.back().optimize = false;
            crits.back().measurement = Criterion::COUNT;
            crits.back().selector = Criterion::REMOVED;
            crits.push_back(Criterion());
            crits.back().optimize = false;
            crits.back().measurement = Criterion::COUNT;
            crits.back().selector = Criterion::CHANGED;
        }
        else if (lower == "trendy") {
            crits.push_back(Criterion());
            crits.back().optimize = false;
            crits.back().measurement = Criterion::COUNT;
            crits.back().selector = Criterion::REMOVED;
            crits.push_back(Criterion());
            crits.back().optimize = false;
            crits.back().measurement = Criterion::NOTUPTODATE;
            crits.back().selector = Criterion::SOLUTION;
            crits.push_back(Criterion());
            crits.back().optimize = false;
            crits.back().measurement = Criterion::UNSAT_RECOMMENDS;
            crits.back().selector = Criterion::SOLUTION;
            crits.push_back(Criterion());
            crits.back().optimize = false;
            crits.back().measurement = Criterion::COUNT;
            crits.back().selector = Criterion::NEW;
        }
        else if (lower == "none") { }
        else {
            CritParser p(crits);
            std::istringstream iss(s);
            return p.parse(iss);
        }
        return true;
    }
}

//////////////////// CudfOptions (impl) ///////////////////////// {{{1

CudfOptions::CudfOptions()
    : addAll(false) { }

void CudfOptions::addDefaults(std::string& defaults) {
    defaults += "--criteria=none\n";
}

void CudfOptions::initOptions(ProgramOptions::OptionGroup& root, ProgramOptions::OptionGroup&) {
    using namespace ProgramOptions;
    OptionGroup prepro("Preprocessing Options");
    prepro.addOptions()
        ("criteria,c", storeTo(crits),
            "Preprocess for specific optimization criteria\n"
            "      Default: none\n"
            "      Valid:   none, paranoid, -|+<crit>(,-|+<crit>)*\n"
            "        <crit>: count(<set>) | sum(<set>,<attr>) | unsat_recommends(<set>)\n"
            "              | aligned(<set>,<attr>,<attr>) | notuptodate(<set>)\n"
            "        <attr>: CUDF attribute name\n"
            "        <set> : solution | changed | new | removed | up | down\n"
            "              | installrequest | upgraderequest | request\n"
            "      For backwards compatibility: \n"
            "        new              = count(new)\n"
            "        removed          = count(removed)\n"
            "        changed          = count(changed)\n"
            "        notuptodate      = notuptodate(solution)\n"
            "        unsat_recommends = unsat_recommends(solution)\n"
            "        sum(name)        = sum(name,solution)\n")
        ("addall", bool_switch(&addAll),
            "Disable preprocessing and add all packages\n");

    root.addOptions(prepro);
}

bool CudfOptions::validateOptions(ProgramOptions::OptionValues&, Messages&msg) {
    if (generic.input.size() > 1) {
        msg.error = "at most one file may be given";
        return false;
    }
    return true;
}

bool parsePositional(const std::string&, std::string& out) {
    out = "file";
    return true;
}

//////////////////// main /////////////////////////////////////// {{{1

int main(int argc, char *argv[]) {
    try {
        CudfOptions opts;
        if (!opts.parse(argc, argv, parsePositional)) { throw std::runtime_error( opts.messages.error.c_str() ); }
        if (opts.generic.help) {
            std::cout
                << CUDF_EXECUTABLE << " version " << CUDF_VERSION << "\n\n"
                << "Usage: " << CUDF_EXECUTABLE << " " << CUDF_USAGE << "\n"
                << opts.getHelp() << "\n"
                << "Usage: " << CUDF_EXECUTABLE << " " << CUDF_USAGE << "\n\n"
                << "Default commandline: \n"
                << "  " << CUDF_EXECUTABLE << " " << opts.getDefaults() << std::endl;
            return EXIT_SUCCESS;
        }
        if (opts.generic.version) {
            std::cout
                << CUDF_EXECUTABLE << " " << CUDF_VERSION << "\n\n"
                << "Copyright (C) Roland Kaminski" << "\n"
                << "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n"
                << "Gringo is free software: you are free to change and redistribute it.\n"
                << "There is NO WARRANTY, to the extent permitted by law." << std::endl;
            return EXIT_SUCCESS;
        }
        Dependency d(opts.crits, opts.addAll, opts.generic.verbose > 0);
        Parser p(d);
        if (opts.generic.input.empty() || opts.generic.input.front() == "-") { p.parse(std::cin); }
        else {
            std::ifstream in(opts.generic.input.front().c_str());
            p.parse(in);
        }
        d.closure();
        d.conflicts();
        d.dumpAsFacts(std::cout);
        return EXIT_SUCCESS;
    }
    catch(const std::exception& e) {
        std::cerr << "\nERROR: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
