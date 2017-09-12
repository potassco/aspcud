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
#include <cudf/version.h>
#include <cudf/parser.h>
#include <cudf/critparser.h>
#include <stdexcept>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/foreach.hpp>
#include <boost/program_options.hpp>

#define foreach BOOST_FOREACH

#define CUDF_EXECUTABLE "cudf2lp"
#define CUDF_USAGE "[options] [file]"

//////////////////// Parse Criteria ///////////////////////// {{{1

void validate(boost::any &result, std::vector<std::string> const &values, Criteria::CritVec *, int) {
    namespace po = boost::program_options;
    po::validators::check_first_occurrence(result);
    result = boost::any(Criteria::CritVec{});
    auto &criteria = boost::any_cast<Criteria::CritVec&>(result);
    auto value = boost::algorithm::to_lower_copy(po::validators::get_single_string(values));
    if (value == "paranoid") {
        criteria.push_back(Criterion());
        criteria.back().optimize = false;
        criteria.back().measurement = Criterion::COUNT;
        criteria.back().selector = Criterion::REMOVED;
        criteria.push_back(Criterion());
        criteria.back().optimize = false;
        criteria.back().measurement = Criterion::COUNT;
        criteria.back().selector = Criterion::CHANGED;
    }
    else if (value == "trendy") {
        criteria.push_back(Criterion());
        criteria.back().optimize = false;
        criteria.back().measurement = Criterion::COUNT;
        criteria.back().selector = Criterion::REMOVED;
        criteria.push_back(Criterion());
        criteria.back().optimize = false;
        criteria.back().measurement = Criterion::NOTUPTODATE;
        criteria.back().selector = Criterion::SOLUTION;
        criteria.push_back(Criterion());
        criteria.back().optimize = false;
        criteria.back().measurement = Criterion::UNSAT_RECOMMENDS;
        criteria.back().selector = Criterion::SOLUTION;
        criteria.push_back(Criterion());
        criteria.back().optimize = false;
        criteria.back().measurement = Criterion::COUNT;
        criteria.back().selector = Criterion::NEW;
    }
    else if (value == "none") { }
    else {
        CritParser p(criteria);
        std::istringstream iss(value);
        if (!p.parse(iss)) {
            throw po::validation_error(po::validation_error::invalid_option_value);
            // error
        }
    }
}

//////////////////// main /////////////////////////////////////// {{{1

void printUsage(boost::program_options::options_description &options) {
    std::cout << "Usage: " << CUDF_EXECUTABLE << " " << CUDF_USAGE << "\n";
    std::cout << options << std::endl;
}

void printVersion() {
    std::cout << CUDF_EXECUTABLE << " version " << CUDF_VERSION << "\n\n";
    std::cout << "License: The MIT License <https://opensource.org/licenses/MIT>" << std::endl;
}

int main(int argc, char *argv[]) {
    try {
        Criteria::CritVec criteria;
        namespace po = boost::program_options;
        po::positional_options_description positional_options;
        positional_options.add("file", 1);
        po::options_description preprocessing_options("Preprocessing Options");
        preprocessing_options.add_options()
            ("criteria,c", po::value<Criteria::CritVec>(&criteria),
                "Preprocess for specific optimization criteria\n"
                "  Default: none\n"
                "  Valid:   none, paranoid, trendy, <list>\n"
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
                "    sum(name)        = sum(name,solution)\n")
            ("addall", "Disable preprocessing and add all packages");
        po::options_description basic_options("Basic Options");
        basic_options.add_options()
            ("help,h", "Print help information and exit")
            ("version,v", "Print version information and exit")
            ("verbose,V", po::value<unsigned>()->default_value(0));
        po::options_description hidden_options;
        hidden_options.add_options()
            ("file,f", po::value<std::string>(), "input file");

        po::options_description cmdline_options, all_options;
        cmdline_options.add(preprocessing_options).add(basic_options);
        all_options.add(cmdline_options).add(hidden_options);

        po::variables_map options;
        po::store(po::command_line_parser(argc, argv).options(all_options).positional(positional_options).run(), options);
        po::notify(options);

        if (options.count("help")) {
            printUsage(cmdline_options);
            return EXIT_SUCCESS;
        }
        if (options.count("version")) {
            printVersion();
            return EXIT_SUCCESS;
        }

        Dependency d(criteria, options.count("addall"), options["verbose"].as<unsigned>());
        Parser p(d);
        std::string file = options.count("file") ? options["file"].as<std::string>() : "-";
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
    catch(const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
