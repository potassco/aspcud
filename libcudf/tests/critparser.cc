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

#include <boost/test/unit_test.hpp>
#include <cudf/critparser.hh>

//////////////////// Test CritParser //////////////////////////// {{{1

Criteria::CritVec parseCrits(char const *crit) {
    Criteria::CritVec crits;
    CritParser p(crits);
    std::istringstream iss(crit);
    BOOST_CHECK(p.parse(iss));
    return crits;
}

void checkCrit(Criteria::CritVec const &crits, size_t offset, bool maximize, Criterion::Measurement m, Criterion::Selector s, char const *a1 = 0, char const *a2 = 0) {
    BOOST_CHECK(offset < crits.size());
    if (offset < crits.size()) {
        BOOST_CHECK(crits[offset].optimize == maximize);
        BOOST_CHECK(crits[offset].measurement == m);
        BOOST_CHECK(crits[offset].selector == s);
        if (a1) { BOOST_CHECK(crits[offset].attr1 == a1); }
        if (a2) { BOOST_CHECK(crits[offset].attr2 == a2); }
    }
}

BOOST_AUTO_TEST_CASE( test_critparser1 ) {
    Criteria::CritVec crits = parseCrits("-count(new),+count(solution),-count(up),+count(down),-count(removed),-count(installrequest),-count(upgraderequest),+count(request)");
    BOOST_CHECK(crits.size() == 8);
    checkCrit(crits, 0, false, Criterion::COUNT, Criterion::NEW);
    checkCrit(crits, 1, true, Criterion::COUNT, Criterion::SOLUTION);
    checkCrit(crits, 2, false, Criterion::COUNT, Criterion::UP);
    checkCrit(crits, 3, true, Criterion::COUNT, Criterion::DOWN);
    checkCrit(crits, 4, false, Criterion::COUNT, Criterion::REMOVED);
    checkCrit(crits, 5, false, Criterion::COUNT, Criterion::INSTALLREQUEST);
    checkCrit(crits, 6, false, Criterion::COUNT, Criterion::UPGRADEREQUEST);
    checkCrit(crits, 7, true, Criterion::COUNT, Criterion::REQUEST);

}

BOOST_AUTO_TEST_CASE( test_critparser2 ) {
    Criteria::CritVec crits = parseCrits("-count(solution),-sum(solution,version),-aligned(solution,source,version),-notuptodate(solution),-unsat_recommends(solution)");
    BOOST_CHECK(crits.size() == 5);
    checkCrit(crits, 0, false, Criterion::COUNT, Criterion::SOLUTION);
    checkCrit(crits, 1, false, Criterion::SUM, Criterion::SOLUTION, "version");
    checkCrit(crits, 2, false, Criterion::ALIGNED, Criterion::SOLUTION, "source", "version");
    checkCrit(crits, 3, false, Criterion::NOTUPTODATE, Criterion::SOLUTION);
    checkCrit(crits, 4, false, Criterion::UNSAT_RECOMMENDS, Criterion::SOLUTION);
}

BOOST_AUTO_TEST_CASE( test_critparser3 ) {
    Criteria::CritVec crits = parseCrits("-new,-removed,-changed,-notuptodate,-unsat_recommends,-sum(version)");
    BOOST_CHECK(crits.size() == 6);
    checkCrit(crits, 0, false, Criterion::COUNT, Criterion::NEW);
    checkCrit(crits, 1, false, Criterion::COUNT, Criterion::REMOVED);
    checkCrit(crits, 2, false, Criterion::COUNT, Criterion::CHANGED);
    checkCrit(crits, 3, false, Criterion::NOTUPTODATE, Criterion::SOLUTION);
    checkCrit(crits, 4, false, Criterion::UNSAT_RECOMMENDS, Criterion::SOLUTION);
    checkCrit(crits, 5, false, Criterion::SUM, Criterion::SOLUTION, "version");
}

