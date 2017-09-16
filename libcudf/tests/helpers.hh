// {{{ MIT License

// Copyright 2017 Roland Kaminski

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

// }}}
//////////////////// Preamble /////////////////////////////////// {{{1

#pragma once

#include <iostream>
#include <cudf/parser.hh>
#include "catch.hpp"

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


