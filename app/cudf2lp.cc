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

#include <cstdlib>
#include <iostream>
#include <cudf/version.hh>
#include <cudf/parser.hh>
#include <cudf/critparser.hh>
#include <stdexcept>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include "options.hh"

#define CUDF_EXECUTABLE "cudf2lp"
#define CUDF_USAGE "[options] [file]"

//////////////////// Parse Criteria ///////////////////////// {{{1

void option_from_string(char const *value, Criteria::CritVec &target, int) {
    auto lower = boost::algorithm::to_lower_copy(std::string(value));
    if (lower == "paranoid") {
        target.push_back(Criterion());
        target.back().optimize = false;
        target.back().measurement = Criterion::COUNT;
        target.back().selector = Criterion::REMOVED;
        target.push_back(Criterion());
        target.back().optimize = false;
        target.back().measurement = Criterion::COUNT;
        target.back().selector = Criterion::CHANGED;
    }
    else if (lower == "trendy") {
        target.push_back(Criterion());
        target.back().optimize = false;
        target.back().measurement = Criterion::COUNT;
        target.back().selector = Criterion::REMOVED;
        target.push_back(Criterion());
        target.back().optimize = false;
        target.back().measurement = Criterion::NOTUPTODATE;
        target.back().selector = Criterion::SOLUTION;
        target.push_back(Criterion());
        target.back().optimize = false;
        target.back().measurement = Criterion::UNSAT_RECOMMENDS;
        target.back().selector = Criterion::SOLUTION;
        target.push_back(Criterion());
        target.back().optimize = false;
        target.back().measurement = Criterion::COUNT;
        target.back().selector = Criterion::NEW;
    }
    else if (lower == "none") { }
    else {
        CritParser p(target);
        std::istringstream iss(value);
        if (!p.parse(iss)) {
            throw std::runtime_error("invalid criteria");
        }
    }
}

std::string option_to_string(Criteria::CritVec const &target) {
    assert(target.empty());
    return "none";
}

//////////////////// main /////////////////////////////////////// {{{1

void printUsage(Options &a) {
    std::cout << "Usage: " << CUDF_EXECUTABLE << " " << CUDF_USAGE << "\n";
    std::cout << a.description();
}

void printVersion() {
    std::cout << CUDF_EXECUTABLE << " version " << CUDF_VERSION << "\n\n";
    std::cout << "License: The MIT License <https://opensource.org/licenses/MIT>" << std::endl;
}

int main(int argc, char *argv[]) {
    try {
        std::string file = "-";
        bool addall = false, help = false, version = false;
        unsigned verbositiy;
        Criteria::CritVec criteria;
        Options options;
        options.group("Preprocessing Options");
        options.add(criteria, "c,criteria",
            "Preprocess for specific optimization criteria\n"
            "  Accepted values: none, paranoid, trendy, <list>\n"
            "    <list>: <sign><crit>\\(,<sign><crit>\\)*\n"
            "    <sign>: + | -\n"
            "    <crit>: count(<set>) | sum(<set>,<attr>)\n"
            "          | unsat_recommends(<set>)\n"
            "          | aligned(<set>,<attr>,<attr>)\n"
            "          | notuptodate(<set>)\n"
            "    <attr>: CUDF attribute name\n"
            "    <set> : solution | changed | new | removed | up\n"
            "          | down | installrequest | upgraderequest\n"
            "          | request\n"
            "  For backwards compatibility: \n"
            "    new              = count(new)\n"
            "    removed          = count(removed)\n"
            "    changed          = count(changed)\n"
            "    notuptodate      = notuptodate(solution)\n"
            "    unsat_recommends = unsat_recommends(solution)\n"
            "    sum(name)        = sum(name,solution)\n");
        options.add(addall, "a,addall", "Disable preprocessing and add all packages");

        options.group("Basic Options");
        options.add(file, "f,file", "input file", "arg", 1, 0, true);
        options.add(help, "h,help", "Print help information and exit");
        options.add(version, "v,version,v", "Print version information and exit");
        options.add(verbositiy, "V,verbose", "Set verbosity level");

        options.parse(argc, argv);

        if (help) {
            printUsage(options);
            return EXIT_SUCCESS;
        }
        if (version) {
            printVersion();
            return EXIT_SUCCESS;
        }

        Dependency d(criteria, addall, verbositiy);
        Parser p(d);
        if (file == "-") { p.parse(std::cin); }
        else {
            std::ifstream in(file.c_str());
            p.parse(in);
        }
        d.closure();
        d.conflicts();
        d.dumpAsFacts(std::cout);
        return EXIT_SUCCESS;
    }
    catch (OptionsException const &e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        std::cerr << "INFO : " << "try '--help' for usage information" << std::endl;
        return EXIT_FAILURE;
    }
    catch(const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
