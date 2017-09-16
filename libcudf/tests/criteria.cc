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

//////////////////// Minimizing Selectors /////////////////////// {{{1

TEST_CASE("criteria", "[criteria]") {
    SECTION("test_minimize_solution") {
        TestDep d(createCrits(false, Criterion::COUNT, Criterion::SOLUTION),
            "package: a\n"
            "version: 1\n"
            "\n"
            "package: b\n"
            "version: 1\n"
            "\n"
            "request: \n"
        );
        REQUIRE(!d.contains("a", 1));
        REQUIRE(!d.contains("b", 1));
    }

    SECTION("test_minimize_changed") {
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
        REQUIRE( d.contains("a", 1));
        REQUIRE(!d.contains("b", 1));
    }

    SECTION("test_minimize_new") {
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
        REQUIRE(!d.contains("a", 1));
        REQUIRE(!d.contains("a", 2));
        REQUIRE(!d.contains("b", 1));
    }

    SECTION("test_minimize_removed") {
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
        REQUIRE( d.contains("a", 1));
        REQUIRE( d.contains("a", 2));
        REQUIRE(!d.contains("b", 1));
    }

    SECTION("test_minimize_up") {
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
        REQUIRE(!d.contains("a", 1));
        REQUIRE(!d.contains("a", 2));
        REQUIRE(!d.contains("a", 3));
        REQUIRE(!d.contains("a", 4));
    }

    SECTION("test_minimize_down") {
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
        REQUIRE(!d.contains("a", 1));
        REQUIRE(!d.contains("a", 2));
        REQUIRE(!d.contains("a", 3));
        REQUIRE(!d.contains("a", 4));
    }

    SECTION("test_minimize_installrequest") {
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
        REQUIRE(!d.contains("a", 1));
        REQUIRE(!d.contains("a", 2));
        REQUIRE( d.contains("a", 3));
        REQUIRE( d.contains("a", 4));
    }

    SECTION("test_minimize_upgraderequest") {
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
        REQUIRE(!d.contains("a", 1));
        REQUIRE(!d.contains("a", 2));
        REQUIRE( d.contains("a", 3));
        REQUIRE( d.contains("a", 4));
    }

    SECTION("test_minimize_request") {
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
        REQUIRE(!d.contains("a", 1));
        REQUIRE( d.contains("a", 2));
        REQUIRE( d.contains("b", 1));
        REQUIRE(!d.contains("b", 2));
    }

//////////////////// Maximizing Selectors /////////////////////// {{{1

    SECTION("test_maximize_solution") {
        TestDep d(createCrits(true, Criterion::COUNT, Criterion::SOLUTION),
            "package: a\n"
            "version: 1\n"
            "\n"
            "package: b\n"
            "version: 1\n"
            "\n"
            "request: \n"
        );
        REQUIRE(d.contains("a", 1));
        REQUIRE(d.contains("b", 1));
    }

    SECTION("test_maximize_changed") {
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
        REQUIRE(!d.contains("a", 1));
        REQUIRE( d.contains("b", 1));
    }

    SECTION("test_maximize_new") {
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
        REQUIRE(!d.contains("a", 1));
        REQUIRE(!d.contains("a", 2));
        REQUIRE( d.contains("b", 1));
    }

    SECTION("test_maximize_removed") {
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
        REQUIRE(!d.contains("a", 1));
        REQUIRE(!d.contains("a", 2));
        REQUIRE(!d.contains("b", 1));
    }

    SECTION("test_maximize_up") {
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
        REQUIRE(!d.contains("a", 1));
        REQUIRE(!d.contains("a", 2));
        REQUIRE( d.contains("a", 3));
        REQUIRE( d.contains("a", 4));
    }

    SECTION("test_maximize_down") {
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
        REQUIRE( d.contains("a", 1));
        REQUIRE( d.contains("a", 2));
        REQUIRE(!d.contains("a", 3));
        REQUIRE(!d.contains("a", 4));
    }

    SECTION("test_maximize_installrequest") {
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
        REQUIRE(!d.contains("a", 1));
        REQUIRE(!d.contains("a", 2));
        REQUIRE( d.contains("a", 3));
        REQUIRE( d.contains("a", 4));
    }

    SECTION("test_maximize_upgraderequest") {
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
        REQUIRE(!d.contains("a", 1));
        REQUIRE(!d.contains("a", 2));
        REQUIRE( d.contains("a", 3));
        REQUIRE( d.contains("a", 4));
    }

    SECTION("test_maximize_request") {
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
        REQUIRE(!d.contains("a", 1));
        REQUIRE( d.contains("a", 2));
        REQUIRE( d.contains("b", 1));
        REQUIRE(!d.contains("b", 2));
    }


//////////////////// Minimizing Measurements //////////////////// {{{1

    SECTION("test_minimize_sum") {
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
        REQUIRE(!d1.contains("a", 1));
        REQUIRE( d1.contains("b", 1));
        REQUIRE(!d1.contains("c", 1));
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
        REQUIRE(!d2.contains("a", 1));
        REQUIRE(!d2.contains("b", 1));
        REQUIRE( d2.contains("c", 1));
    }

    SECTION("test_minimize_notuptodate") {
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
        REQUIRE(!d.contains("a", 1));
        REQUIRE(!d.contains("a", 2));
        REQUIRE( d.contains("b", 1));
        REQUIRE( d.contains("b", 2));
    }

    SECTION("test_minimize_unsatrecommends") {
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
        REQUIRE( d.contains("a", 1));
        REQUIRE( d.contains("a", 2));
        REQUIRE( d.contains("c", 1));
    }

    SECTION("test_minimize_aligned") {
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
        REQUIRE( d.contains("a", 1));
        REQUIRE( d.contains("a", 2));
        REQUIRE(!d.contains("b", 1));
    }

//////////////////// Maximizing Measurements //////////////////// {{{1

    SECTION("test_maximize_sum") {
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
        REQUIRE(!d1.contains("a", 1));
        REQUIRE(!d1.contains("b", 1));
        REQUIRE( d1.contains("c", 1));
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
        REQUIRE(!d2.contains("a", 1));
        REQUIRE( d2.contains("b", 1));
        REQUIRE(!d2.contains("c", 1));
    }

    SECTION("test_maximize_notuptodate") {
        TestDep d(createCrits(true, Criterion::NOTUPTODATE, Criterion::SOLUTION),
            "package: a\n"
            "version: 1\n"
            "\n"
            "package: a\n"
            "version: 2\n"
            "\n"
            "request: \n"
        );
        REQUIRE( d.contains("a", 1));
        REQUIRE(!d.contains("a", 2));
    }

    SECTION("test_maximize_unsatrecommends") {
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
        REQUIRE(!d.contains("a", 1));
        REQUIRE( d.contains("a", 2));
        REQUIRE(!d.contains("c", 1));
    }

    SECTION("test_maximize_aligned") {
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
        REQUIRE( d.contains("a", 1));
        REQUIRE( d.contains("a", 2));
        REQUIRE(!d.contains("b", 1));
    }

}


