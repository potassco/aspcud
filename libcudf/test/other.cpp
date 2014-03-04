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

//////////////////// Other ////////////////////////////////////// {{{1

BOOST_AUTO_TEST_CASE( test_feature1 ) {
    TestDep d1(Criteria::CritVec(),
        "package: a\n"
        "version: 1\n"
        "conflicts: a , b\n"
        "provides: b\n"
        "\n"
        "package: c\n"
        "conflicts: c\n"
        "version: 1\n"
        "depends: b\n"
        "\n"
        "request: \n"
        "install: c\n"
    );
    BOOST_CHECK(d1.contains("a", 1));
    BOOST_CHECK(d1.contains("c", 1));
}

BOOST_AUTO_TEST_CASE( test_install1 ) {
    TestDep d1(Criteria::CritVec(),
        "package: a\n"
        "version: 1\n"
        "\n"
        "package: a\n"
        "version: 2\n"
        "\n"
        "package: a\n"
        "version: 3\n"
        "\n"
        "request: \n"
        "install: a > 1\n"
    );
    BOOST_CHECK(!d1.contains("a", 1));
    BOOST_CHECK( d1.contains("a", 2));
    BOOST_CHECK( d1.contains("a", 3));
}

BOOST_AUTO_TEST_CASE( test_install2 ) {
    TestDep d1(Criteria::CritVec(),
        "package: a\n"
        "version: 1\n"
        "\n"
        "package: a\n"
        "version: 2\n"
        "\n"
        "package: a\n"
        "version: 3\n"
        "\n"
        "request: \n"
        "install: a >= 2\n"
    );
    BOOST_CHECK(!d1.contains("a", 1));
    BOOST_CHECK( d1.contains("a", 2));
    BOOST_CHECK( d1.contains("a", 3));
}

BOOST_AUTO_TEST_CASE( test_install3 ) {
    TestDep d1(Criteria::CritVec(),
        "package: a\n"
        "version: 1\n"
        "\n"
        "package: a\n"
        "version: 2\n"
        "\n"
        "package: a\n"
        "version: 3\n"
        "\n"
        "request: \n"
        "install: a != 2\n"
    );
    BOOST_CHECK( d1.contains("a", 1));
    BOOST_CHECK(!d1.contains("a", 2));
    BOOST_CHECK( d1.contains("a", 3));
}

BOOST_AUTO_TEST_CASE( test_install4 ) {
    TestDep d1(Criteria::CritVec(),
        "package: a\n"
        "version: 1\n"
        "\n"
        "package: a\n"
        "version: 2\n"
        "\n"
        "package: a\n"
        "version: 3\n"
        "\n"
        "request: \n"
        "install: a < 2\n"
    );
    BOOST_CHECK( d1.contains("a", 1));
    BOOST_CHECK(!d1.contains("a", 2));
    BOOST_CHECK(!d1.contains("a", 3));
}

BOOST_AUTO_TEST_CASE( test_install5 ) {
    TestDep d1(Criteria::CritVec(),
        "package: a\n"
        "version: 1\n"
        "\n"
        "package: a\n"
        "version: 2\n"
        "\n"
        "package: a\n"
        "version: 3\n"
        "\n"
        "request: \n"
        "install: a = 3\n"
    );
    BOOST_CHECK(!d1.contains("a", 1));
    BOOST_CHECK(!d1.contains("a", 2));
    BOOST_CHECK( d1.contains("a", 3));
}

BOOST_AUTO_TEST_CASE( test_self_conflict ) {
    TestDep d1(Criteria::CritVec(),
        "package: a\n"
        "version: 1\n"
        "conflicts: a\n"
        "\n"
        "package: a\n"
        "version: 2\n"
        "conflicts: a\n"
        "\n"
        "request: \n"
        "install: a\n"
    );
    BOOST_CHECK(d1.contains("a", 1));
    BOOST_CHECK(d1.contains("a", 2));
}

BOOST_AUTO_TEST_CASE( test_upgrade1 ) {
    TestDep d1(Criteria::CritVec(),
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
        "request: upgrade-request\n"
        "upgrade: a\n"
    );
    BOOST_CHECK(!d1.contains("a", 1));
    BOOST_CHECK( d1.contains("a", 2));
    BOOST_CHECK( d1.contains("a", 3));
}

