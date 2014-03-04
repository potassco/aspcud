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
#include <test/helpers.h>

//////////////////// Minimizing Selectors /////////////////////// {{{1

BOOST_AUTO_TEST_CASE( test_minimize_solution ) {
    TestDep d(createCrits(false, Criterion::COUNT, Criterion::SOLUTION),
        "package: a\n"
        "version: 1\n"
        "\n"
        "package: b\n"
        "version: 1\n"
        "\n"
        "request: \n"
    );
    BOOST_CHECK(!d.contains("a", 1));
    BOOST_CHECK(!d.contains("b", 1));
}

BOOST_AUTO_TEST_CASE( test_minimize_changed ) {
    TestDep d(createCrits(false, Criterion::COUNT, Criterion::CHANGED),
        "package: a\n"
        "version: 1\n"
        "installed: true\n"
        "\n"
        "package: b\n"
        "version: 1\n"
        "\n"
        "request: \n"
    );
    BOOST_CHECK( d.contains("a", 1));
    BOOST_CHECK(!d.contains("b", 1));
}

BOOST_AUTO_TEST_CASE( test_minimize_new ) {
    TestDep d(createCrits(false, Criterion::COUNT, Criterion::NEW),
        "package: a\n"
        "version: 1\n"
        "installed: true\n"
        "\n"
        "package: a\n"
        "version: 2\n"
        "\n"
        "package: b\n"
        "version: 1\n"
        "\n"
        "request: \n"
    );
    BOOST_CHECK(!d.contains("a", 1));
    BOOST_CHECK(!d.contains("a", 2));
    BOOST_CHECK(!d.contains("b", 1));
}

BOOST_AUTO_TEST_CASE( test_minimize_removed ) {
    TestDep d(createCrits(false, Criterion::COUNT, Criterion::REMOVED),
        "package: a\n"
        "version: 1\n"
        "installed: true\n"
        "\n"
        "package: a\n"
        "version: 2\n"
        "\n"
        "package: b\n"
        "version: 1\n"
        "\n"
        "request: \n"
    );
    BOOST_CHECK( d.contains("a", 1));
    BOOST_CHECK( d.contains("a", 2));
    BOOST_CHECK(!d.contains("b", 1));
}

BOOST_AUTO_TEST_CASE( test_minimize_up ) {
    TestDep d(createCrits(false, Criterion::COUNT, Criterion::UP),
        "package: a\n"
        "version: 1\n"
        "\n"
        "package: a\n"
        "version: 2\n"
        "installed: true\n"
        "\n"
        "package: a\n"
        "version: 3\n"
        "\n"
        "package: a\n"
        "version: 4\n"
        "\n"
        "request: \n"
    );
    BOOST_CHECK(!d.contains("a", 1));
    BOOST_CHECK(!d.contains("a", 2));
    BOOST_CHECK(!d.contains("a", 3));
    BOOST_CHECK(!d.contains("a", 4));
}

BOOST_AUTO_TEST_CASE( test_minimize_down ) {
    TestDep d(createCrits(false, Criterion::COUNT, Criterion::DOWN),
        "package: a\n"
        "version: 1\n"
        "\n"
        "package: a\n"
        "version: 2\n"
        "\n"
        "package: a\n"
        "version: 3\n"
        "installed: true\n"
        "\n"
        "package: a\n"
        "version: 4\n"
        "\n"
        "request: \n"
    );
    BOOST_CHECK(!d.contains("a", 1));
    BOOST_CHECK(!d.contains("a", 2));
    BOOST_CHECK(!d.contains("a", 3));
    BOOST_CHECK(!d.contains("a", 4));
}

BOOST_AUTO_TEST_CASE( test_minimize_installrequest ) {
    TestDep d(createCrits(false, Criterion::COUNT, Criterion::INSTALLREQUEST),
        "package: a\n"
        "version: 1\n"
        "\n"
        "package: a\n"
        "version: 2\n"
        "\n"
        "package: a\n"
        "version: 3\n"
        "\n"
        "package: a\n"
        "version: 4\n"
        "\n"
        "request: \n"
        "install: a > 2\n"
    );
    BOOST_CHECK(!d.contains("a", 1));
    BOOST_CHECK(!d.contains("a", 2));
    BOOST_CHECK( d.contains("a", 3));
    BOOST_CHECK( d.contains("a", 4));
}

BOOST_AUTO_TEST_CASE( test_minimize_upgraderequest ) {
    TestDep d(createCrits(false, Criterion::COUNT, Criterion::UPGRADEREQUEST),
        "package: a\n"
        "version: 1\n"
        "\n"
        "package: a\n"
        "version: 2\n"
        "\n"
        "package: a\n"
        "version: 3\n"
        "\n"
        "package: a\n"
        "version: 4\n"
        "\n"
        "request: \n"
        "upgrade: a > 2\n"
    );
    BOOST_CHECK(!d.contains("a", 1));
    BOOST_CHECK(!d.contains("a", 2));
    BOOST_CHECK( d.contains("a", 3));
    BOOST_CHECK( d.contains("a", 4));
}

BOOST_AUTO_TEST_CASE( test_minimize_request ) {
    TestDep d(createCrits(false, Criterion::COUNT, Criterion::UPGRADEREQUEST),
        "package: a\n"
        "version: 1\n"
        "\n"
        "package: a\n"
        "version: 2\n"
        "\n"
        "package: b\n"
        "version: 1\n"
        "\n"
        "package: b\n"
        "version: 2\n"
        "\n"
        "request: \n"
        "upgrade: b < 2\n"
        "install: a > 1\n"
    );
    BOOST_CHECK(!d.contains("a", 1));
    BOOST_CHECK( d.contains("a", 2));
    BOOST_CHECK( d.contains("b", 1));
    BOOST_CHECK(!d.contains("b", 2));
}

//////////////////// Maximizing Selectors /////////////////////// {{{1

BOOST_AUTO_TEST_CASE( test_maximize_solution ) {
    TestDep d(createCrits(true, Criterion::COUNT, Criterion::SOLUTION),
        "package: a\n"
        "version: 1\n"
        "\n"
        "package: b\n"
        "version: 1\n"
        "\n"
        "request: \n"
    );
    BOOST_CHECK(d.contains("a", 1));
    BOOST_CHECK(d.contains("b", 1));
}

BOOST_AUTO_TEST_CASE( test_maximize_changed ) {
    TestDep d(createCrits(true, Criterion::COUNT, Criterion::CHANGED),
        "package: a\n"
        "version: 1\n"
        "installed: true\n"
        "\n"
        "package: b\n"
        "version: 1\n"
        "\n"
        "request: \n"
    );
    BOOST_CHECK(!d.contains("a", 1));
    BOOST_CHECK( d.contains("b", 1));
}

BOOST_AUTO_TEST_CASE( test_maximize_new ) {
    TestDep d(createCrits(true, Criterion::COUNT, Criterion::NEW),
        "package: a\n"
        "version: 1\n"
        "installed: true\n"
        "\n"
        "package: a\n"
        "version: 2\n"
        "\n"
        "package: b\n"
        "version: 1\n"
        "\n"
        "request: \n"
    );
    BOOST_CHECK(!d.contains("a", 1));
    BOOST_CHECK(!d.contains("a", 2));
    BOOST_CHECK( d.contains("b", 1));
}

BOOST_AUTO_TEST_CASE( test_maximize_removed ) {
    TestDep d(createCrits(true, Criterion::COUNT, Criterion::REMOVED),
        "package: a\n"
        "version: 1\n"
        "installed: true\n"
        "\n"
        "package: a\n"
        "version: 2\n"
        "\n"
        "package: b\n"
        "version: 1\n"
        "\n"
        "request: \n"
    );
    BOOST_CHECK(!d.contains("a", 1));
    BOOST_CHECK(!d.contains("a", 2));
    BOOST_CHECK(!d.contains("b", 1));
}

BOOST_AUTO_TEST_CASE( test_maximize_up ) {
    TestDep d(createCrits(true, Criterion::COUNT, Criterion::UP),
        "package: a\n"
        "version: 1\n"
        "\n"
        "package: a\n"
        "version: 2\n"
        "installed: true\n"
        "\n"
        "package: a\n"
        "version: 3\n"
        "\n"
        "package: a\n"
        "version: 4\n"
        "\n"
        "request: \n"
    );
    BOOST_CHECK(!d.contains("a", 1));
    BOOST_CHECK(!d.contains("a", 2));
    BOOST_CHECK( d.contains("a", 3));
    BOOST_CHECK( d.contains("a", 4));
}

BOOST_AUTO_TEST_CASE( test_maximize_down ) {
    TestDep d(createCrits(true, Criterion::COUNT, Criterion::DOWN),
        "package: a\n"
        "version: 1\n"
        "\n"
        "package: a\n"
        "version: 2\n"
        "\n"
        "package: a\n"
        "version: 3\n"
        "installed: true\n"
        "\n"
        "package: a\n"
        "version: 4\n"
        "\n"
        "request: \n"
    );
    BOOST_CHECK( d.contains("a", 1));
    BOOST_CHECK( d.contains("a", 2));
    BOOST_CHECK(!d.contains("a", 3));
    BOOST_CHECK(!d.contains("a", 4));
}

BOOST_AUTO_TEST_CASE( test_maximize_installrequest ) {
    TestDep d(createCrits(true, Criterion::COUNT, Criterion::INSTALLREQUEST),
        "package: a\n"
        "version: 1\n"
        "\n"
        "package: a\n"
        "version: 2\n"
        "\n"
        "package: a\n"
        "version: 3\n"
        "\n"
        "package: a\n"
        "version: 4\n"
        "\n"
        "request: \n"
        "install: a > 2\n"
    );
    BOOST_CHECK(!d.contains("a", 1));
    BOOST_CHECK(!d.contains("a", 2));
    BOOST_CHECK( d.contains("a", 3));
    BOOST_CHECK( d.contains("a", 4));
}

BOOST_AUTO_TEST_CASE( test_maximize_upgraderequest ) {
    TestDep d(createCrits(true, Criterion::COUNT, Criterion::UPGRADEREQUEST),
        "package: a\n"
        "version: 1\n"
        "\n"
        "package: a\n"
        "version: 2\n"
        "\n"
        "package: a\n"
        "version: 3\n"
        "\n"
        "package: a\n"
        "version: 4\n"
        "\n"
        "request: \n"
        "upgrade: a > 2\n"
    );
    BOOST_CHECK(!d.contains("a", 1));
    BOOST_CHECK(!d.contains("a", 2));
    BOOST_CHECK( d.contains("a", 3));
    BOOST_CHECK( d.contains("a", 4));
}

BOOST_AUTO_TEST_CASE( test_maximize_request ) {
    TestDep d(createCrits(true, Criterion::COUNT, Criterion::UPGRADEREQUEST),
        "package: a\n"
        "version: 1\n"
        "\n"
        "package: a\n"
        "version: 2\n"
        "\n"
        "package: b\n"
        "version: 1\n"
        "\n"
        "package: b\n"
        "version: 2\n"
        "\n"
        "request: \n"
        "upgrade: b < 2\n"
        "install: a > 1\n"
    );
    BOOST_CHECK(!d.contains("a", 1));
    BOOST_CHECK( d.contains("a", 2));
    BOOST_CHECK( d.contains("b", 1));
    BOOST_CHECK(!d.contains("b", 2));
}


//////////////////// Minimizing Measurements //////////////////// {{{1

BOOST_AUTO_TEST_CASE( test_minimize_sum ) {
    TestDep d1(createCrits(false, Criterion::SUM, Criterion::SOLUTION, "weight"),
        "preamble: \n"
        "property: weight: int = [0]\n"
        "\n"
        "package: a\n"
        "version: 1\n"
        "weight: 0\n"
        "\n"
        "package: b\n"
        "version: 1\n"
        "weight: -1\n"
        "\n"
        "package: c\n"
        "version: 1\n"
        "weight: 1\n"
        "\n"
        "request: \n"
    );
    BOOST_CHECK(!d1.contains("a", 1));
    BOOST_CHECK( d1.contains("b", 1));
    BOOST_CHECK(!d1.contains("c", 1));
    TestDep d2(createCrits(false, Criterion::SUM, Criterion::REMOVED, "weight"),
        "preamble: \n"
        "property: weight: int = [0]\n"
        "\n"
        "package: a\n"
        "version: 1\n"
        "installed: true\n"
        "weight: 0\n"
        "\n"
        "package: b\n"
        "version: 1\n"
        "installed: true\n"
        "weight: -1\n"
        "\n"
        "package: c\n"
        "version: 1\n"
        "installed: true\n"
        "weight: 1\n"
        "\n"
        "request: \n"
    );
    BOOST_CHECK(!d2.contains("a", 1));
    BOOST_CHECK(!d2.contains("b", 1));
    BOOST_CHECK( d2.contains("c", 1));
}

BOOST_AUTO_TEST_CASE( test_minimize_notuptodate ) {
    TestDep d(createCrits(false, Criterion::NOTUPTODATE, Criterion::REMOVED),
        "package: a\n"
        "version: 1\n"
        "\n"
        "package: a\n"
        "version: 2\n"
        "installed: true\n"
        "\n"
        "package: b\n"
        "version: 1\n"
        "installed: true\n"
        "\n"
        "package: b\n"
        "version: 2\n"
        "\n"
        "request: \n"
    );
    BOOST_CHECK(!d.contains("a", 1));
    BOOST_CHECK(!d.contains("a", 2));
    BOOST_CHECK( d.contains("b", 1));
    BOOST_CHECK( d.contains("b", 2));
}

BOOST_AUTO_TEST_CASE( test_minimize_unsatrecommends ) {
    TestDep d(createCrits(false, Criterion::UNSAT_RECOMMENDS, Criterion::REMOVED),
        "preamble: \n"
        "property: recommends: vpkgformula = [true!]\n"
        "\n"
        "package: a\n"
        "version: 1\n"
        "\n"
        "package: a\n"
        "version: 2\n"
        "installed: true\n"
        "recommends: c\n"
        "\n"
        "package: c\n"
        "version: 1\n"
        "\n"
        "request: \n"
    );
    BOOST_CHECK( d.contains("a", 1));
    BOOST_CHECK( d.contains("a", 2));
    BOOST_CHECK( d.contains("c", 1));
}

BOOST_AUTO_TEST_CASE( test_minimize_aligned ) {
    TestDep d(createCrits(false, Criterion::ALIGNED, Criterion::REMOVED, "group", "value"),
        "preamble: \n"
        "property: group: string = [\"\"], value: int = [0]\n"
        "\n"
        "package: a\n"
        "version: 1\n"
        "group: a\n"
        "value: 1\n"
        "\n"
        "package: a\n"
        "version: 2\n"
        "installed: true\n"
        "group: a\n"
        "value: 2\n"
        "\n"
        "package: b\n"
        "version: 1\n"
        "installed: true\n"
        "group: b\n"
        "value: 1\n"
        "\n"
        "request: \n"
    );
    BOOST_CHECK( d.contains("a", 1));
    BOOST_CHECK( d.contains("a", 2));
    BOOST_CHECK(!d.contains("b", 1));
}

//////////////////// Maximizing Measurements //////////////////// {{{1

BOOST_AUTO_TEST_CASE( test_maximize_sum ) {
    TestDep d1(createCrits(true, Criterion::SUM, Criterion::SOLUTION, "weight"),
        "preamble: \n"
        "property: weight: int = [0]\n"
        "\n"
        "package: a\n"
        "version: 1\n"
        "weight: 0\n"
        "\n"
        "package: b\n"
        "version: 1\n"
        "weight: -1\n"
        "\n"
        "package: c\n"
        "version: 1\n"
        "weight: 1\n"
        "\n"
        "request: \n"
    );
    BOOST_CHECK(!d1.contains("a", 1));
    BOOST_CHECK(!d1.contains("b", 1));
    BOOST_CHECK( d1.contains("c", 1));
    TestDep d2(createCrits(true, Criterion::SUM, Criterion::REMOVED, "weight"),
        "preamble: \n"
        "property: weight: int = [0]\n"
        "\n"
        "package: a\n"
        "version: 1\n"
        "installed: true\n"
        "weight: 0\n"
        "\n"
        "package: b\n"
        "version: 1\n"
        "installed: true\n"
        "weight: -1\n"
        "\n"
        "package: c\n"
        "version: 1\n"
        "installed: true\n"
        "weight: 1\n"
        "\n"
        "request: \n"
    );
    BOOST_CHECK(!d2.contains("a", 1));
    BOOST_CHECK( d2.contains("b", 1));
    BOOST_CHECK(!d2.contains("c", 1));
}

BOOST_AUTO_TEST_CASE( test_maximize_notuptodate ) {
    TestDep d(createCrits(true, Criterion::NOTUPTODATE, Criterion::SOLUTION),
        "package: a\n"
        "version: 1\n"
        "\n"
        "package: a\n"
        "version: 2\n"
        "\n"
        "request: \n"
    );
    BOOST_CHECK( d.contains("a", 1));
    BOOST_CHECK(!d.contains("a", 2));
}

BOOST_AUTO_TEST_CASE( test_maximize_unsatrecommends ) {
    TestDep d(createCrits(true, Criterion::UNSAT_RECOMMENDS, Criterion::SOLUTION),
        "preamble: \n"
        "property: recommends: vpkgformula = [true!]\n"
        "\n"
        "package: a\n"
        "version: 1\n"
        "\n"
        "package: a\n"
        "version: 2\n"
        "recommends: c\n"
        "\n"
        "package: c\n"
        "version: 1\n"
        "\n"
        "request: \n"
    );
    BOOST_CHECK(!d.contains("a", 1));
    BOOST_CHECK( d.contains("a", 2));
    BOOST_CHECK(!d.contains("c", 1));
}

BOOST_AUTO_TEST_CASE( test_maximize_aligned ) {
    TestDep d(createCrits(true, Criterion::ALIGNED, Criterion::SOLUTION, "group", "value"),
        "preamble: \n"
        "property: group: string = [\"\"], value: int = [0]\n"
        "\n"
        "package: a\n"
        "version: 1\n"
        "group: a\n"
        "value: 1\n"
        "\n"
        "package: a\n"
        "version: 2\n"
        "group: a\n"
        "value: 2\n"
        "\n"
        "package: b\n"
        "version: 1\n"
        "installed: true\n"
        "group: b\n"
        "value: 1\n"
        "\n"
        "request: \n"
    );
    BOOST_CHECK( d.contains("a", 1));
    BOOST_CHECK( d.contains("a", 2));
    BOOST_CHECK(!d.contains("b", 1));
}



