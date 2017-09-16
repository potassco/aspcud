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

#include "helpers.hh"

//////////////////// Other ////////////////////////////////////// {{{1

TEST_CASE("other", "[other]") {
    SECTION("test_feature1") {
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
        REQUIRE(d1.contains("a", 1));
        REQUIRE(d1.contains("c", 1));
    }

    SECTION("test_install1") {
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
        REQUIRE(!d1.contains("a", 1));
        REQUIRE( d1.contains("a", 2));
        REQUIRE( d1.contains("a", 3));
    }

    SECTION("test_install2") {
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
        REQUIRE(!d1.contains("a", 1));
        REQUIRE( d1.contains("a", 2));
        REQUIRE( d1.contains("a", 3));
    }

    SECTION("test_install3") {
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
        REQUIRE( d1.contains("a", 1));
        REQUIRE(!d1.contains("a", 2));
        REQUIRE( d1.contains("a", 3));
    }

    SECTION("test_install4") {
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
        REQUIRE( d1.contains("a", 1));
        REQUIRE(!d1.contains("a", 2));
        REQUIRE(!d1.contains("a", 3));
    }

    SECTION("test_install5") {
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
        REQUIRE(!d1.contains("a", 1));
        REQUIRE(!d1.contains("a", 2));
        REQUIRE( d1.contains("a", 3));
    }

    SECTION("test_self_conflict") {
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
        REQUIRE(d1.contains("a", 1));
        REQUIRE(d1.contains("a", 2));
    }

    SECTION("test_upgrade1") {
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
        REQUIRE(!d1.contains("a", 1));
        REQUIRE( d1.contains("a", 2));
        REQUIRE( d1.contains("a", 3));
    }
}
