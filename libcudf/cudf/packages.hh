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

#include <stdint.h>
#include <vector>
#include <memory>
#include <limits>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/functional/hash.hpp>
#include <boost/any.hpp>

namespace Cudf {

typedef boost::any Value;


//////////////////// PackageRef ///////////////////////////////// {{{1

struct PackageRef {
    enum RelOp { GE=0, LE, EQ, NEQ };

    uint32_t  name;
    uint32_t  op;
    int32_t   version;

    PackageRef(uint32_t name = 0, RelOp op = GE, int32_t version = 0);
};

typedef std::vector<PackageRef> PkgList;
typedef std::vector<PkgList> PkgFormula;

//////////////////// Package //////////////////////////////////// {{{1

struct Package {
    typedef std::map<uint32_t, int32_t> IntPropMap;
    typedef std::map<uint32_t, uint32_t> StringPropMap;
    enum Keep { VERSION, PACKAGE, FEATURE, NONE };

    Package(uint32_t name = std::numeric_limits<uint32_t>::max(), int32_t version = 0)
        : name(name)
        , version(version)
        , keep(NONE)
        , installed(false) { }

    bool initialized() { return name != std::numeric_limits<uint32_t>::max(); }

    uint32_t      name;
    int32_t       version;
    PkgList       conflicts;   // default empty
    PkgFormula    depends;     // default true
    PkgFormula    recommends;  // default empty
    PkgList       provides;    // default empty
    Keep          keep;        // default NONE
    bool          installed;   // default false
    IntPropMap    intProps;    // default empty
    StringPropMap stringProps; // default empty
};

//////////////////// Request //////////////////////////////////// {{{1

struct Request {
    PkgList install;
    PkgList upgrade;
    PkgList remove;
};

//////////////////// Document /////////////////////////////////// {{{1

struct Document {
    typedef std::vector<Package> Packages;
    Packages packages;
    Request  request;
};

} // namespace Cudf
