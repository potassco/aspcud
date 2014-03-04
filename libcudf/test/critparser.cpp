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

#include <boost/test/unit_test.hpp>
#include <cudf/critparser.h>

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

