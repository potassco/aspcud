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

#pragma once

#include <iostream>
#include <cudf/parser.h>

//////////////////// Helpers //////////////////////////////////// {{{1

struct TestDep {
    TestDep(Criteria::CritVec crits, std::string const &in) 
        : crits(crits)
        , dep(this->crits, false, false)
        , parser(dep) { 
        std::stringstream sin;
        sin.str(in);
        parser.parse(sin);
        dep.closure();
    }
    bool contains(std::string const &name, unsigned version) {
        return dep.test_contains(name, version);
    }
    Criteria::CritVec crits;
    Dependency dep;
    Parser parser;
};

inline Criteria::CritVec createCrits(bool maximize, Criterion::Measurement m, Criterion::Selector f, char const *attr1 = 0, char const *attr2 = 0) {
    Criteria::CritVec crits;
    crits.push_back(Criterion());
    crits.back().optimize    = maximize;
    crits.back().measurement = m;
    crits.back().selector    = f;
    if (attr1) { crits.back().attr1 = attr1; }
    if (attr2) { crits.back().attr2 = attr2; }
    return crits;
}


