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

#include <cudf/lexer_impl.hh>
#include <utility>
#include <vector>
#include <stack>
#include <map>
#include <cudf/dependency.hh>
#include <boost/lexical_cast.hpp>

//////////////////// Parser //////////////////////////////////// {{{1

class Parser : public LexerImpl {
public:
    enum RelOp {
        GE  = Cudf::PackageRef::GE,
        LE  = Cudf::PackageRef::LE,
        EQ  = Cudf::PackageRef::EQ,
        NEQ = Cudf::PackageRef::NEQ,
        LT,
        GT
    };
    struct Token {
        uint32_t index;
    };
    struct Type {
        Type(uint32_t type);
        bool intType() const;
        bool stringType() const;

        uint32_t    type;
        Cudf::Value value;
    };

public:
    Parser(Dependency &dep);
    int lex();
    std::string errorToken();
    void syntaxError();
    void parseError();
    void parse(std::istream &sin);
    ~Parser();

    void parseType(uint32_t index);
    int lexString();
    void parseString() {
        lexString_ = true;
    }
    Cudf::Value &addType(uint32_t name, uint32_t type) {
        std::pair<TypeMap::iterator, bool> res = typeMap_.insert(TypeMap::value_type(name, Type(type)));
        if (!res.second) { throw std::runtime_error("duplicate type"); }
        return res.first->second.value;
    }
    bool mapBool(uint32_t index) {
        assert(index == falseStr_ || index == trueStr_);
        return index == trueStr_;
    }
    int32_t mapInt(uint32_t index) {
        return boost::lexical_cast<int32_t>(dep_.string(index));
    }
    void setPkgRef(uint32_t name, uint32_t op = Cudf::PackageRef::GE, uint32_t version = std::numeric_limits<uint32_t>::max()) {
        pkgRef.name    = name;
        pkgRef.version = (version == std::numeric_limits<uint32_t>::max()) ? 0 : boost::lexical_cast<int32_t>(dep_.string(version));
        if (op == GT)
        {
            pkgRef.version++;
            pkgRef.op = Cudf::PackageRef::GE;
        }
        else if (op == LT) {
            pkgRef.version--;
            pkgRef.op = Cudf::PackageRef::LE;
        }
        else { pkgRef.op = op; }
    }
    void pushPkgList() {
        pkgFormula.push_back(Cudf::PkgList());
        std::swap(pkgFormula.back(), pkgList);
    }
    template <class T>
    void setProperty(uint32_t name, T &&value) {
        if (!propMap_.emplace(name, std::forward<T>(value)).second) {
            throw std::runtime_error("duplicate property");
        }
    }
    template <class T>
    void getProp(uint32_t name, T &dst) {
        PropMap::iterator it = propMap_.find(name);
        if (it != propMap_.end()) {
            dst = boost::any_cast<T&>(it->second);
        }
        else {
            TypeMap::iterator it = typeMap_.find(name);
            assert(it != typeMap_.end());
            if (!it->second.value.empty()) {
                dst = boost::any_cast<T&>(it->second.value);
            }
            else { throw std::runtime_error("required attribute missing"); }
        }
    }

    void _checkCrit(uint32_t uid, std::string const &name, bool includeString) {
        TypeMap::iterator it = typeMap_.find(uid);
        if (it != typeMap_.end()) {
            if ((!includeString || !it->second.stringType()) && !it->second.intType() ) {
                throw std::runtime_error(std::string("only integer") + (includeString ? " and string" : "") + " properties are supported in criteria: " + name);
            }
        }
        else {
            throw std::runtime_error("unknown property in criteria: " + name);
        }
    }

    void addPreamble() {
        propMap_.clear();
        for (Criterion &crit : dep_.criteria.criteria) {
            switch (crit.measurement) {
                case Criterion::SUM: {
                    _checkCrit(crit.attrUid1, crit.attr1, false);
                    break;
                }
                case Criterion::ALIGNED: {
                    _checkCrit(crit.attrUid1, crit.attr1, true);
                    _checkCrit(crit.attrUid2, crit.attr2, true);
                    break;
                }
                default: { break; }
            }
        }
    }
    void addPackage(uint32_t name) {
        setProperty(packageStr_, name);
        doc_->packages.push_back(Cudf::Package(name));
        Cudf::Package &pkg = doc_->packages.back();
        getProp(versionStr_,    pkg.version);
        getProp(conflictsStr_,  pkg.conflicts);
        getProp(dependsStr_,    pkg.depends);
        if (typeMap_.find(recommendsStr_) != typeMap_.end()) { getProp(recommendsStr_, pkg.recommends); }
        getProp(providesStr_,   pkg.provides);
        getProp(installedStr_,  pkg.installed);

        uint32_t keep;
        getProp(keepStr_, keep);

        if (keep == packageStr_)      { pkg.keep = Cudf::Package::PACKAGE; }
        else if (keep == featureStr_) { pkg.keep = Cudf::Package::FEATURE; }
        else if (keep == versionStr_) { pkg.keep = Cudf::Package::VERSION; }
        else if (keep == noneStr_)    { pkg.keep = Cudf::Package::NONE; }
        else                          { throw std::runtime_error("invalid keep value"); }

        if (!dep_.addAll()) {
            for  (uint32_t name : dep_.criteria.optProps) {
                TypeMap::iterator it = typeMap_.find(name);
                if (it != typeMap_.end()) {
                    if (it->second.intType()) {
                        int32_t value;
                        getProp(name, value);
                        pkg.intProps.insert(Package::IntPropMap::value_type(name, value));
                    }
                    else if (it->second.stringType()) {
                        uint32_t value;
                        getProp(name, value);
                        pkg.stringProps.insert(Package::StringPropMap::value_type(name, value));
                    }
                }
            }
        }
        else {
            for  (TypeMap::value_type &val : typeMap_) {
                if (val.second.intType()) {
                    int32_t value;
                    getProp(val.first, value);
                    pkg.intProps.insert(Package::IntPropMap::value_type(val.first, value));
                }
                else if (val.second.stringType()) {
                    uint32_t value;
                    getProp(val.first, value);
                    pkg.stringProps.insert(Package::StringPropMap::value_type(val.first, value));
                }
            }
        }
        propMap_.clear();
    }
    void addRequest() {
        getProp(installStr_, doc_->request.install);
        getProp(removeStr_,  doc_->request.remove);
        getProp(upgradeStr_, doc_->request.upgrade);
        propMap_.clear();
    }

private:
    typedef boost::unordered_map<uint32_t, Type>        TypeMap;
    typedef boost::unordered_map<uint32_t, Cudf::Value> PropMap;
    typedef std::vector<uint32_t>                       EnumValues;
    typedef std::vector<uint32_t>                       OptPropVec;

    Dependency     &dep_;
    Cudf::Document *doc_;
    void           *parser_;
    Token           token_;
    bool            lexString_;
    uint32_t        shiftToken_;

    TypeMap         typeMap_;
    PropMap         propMap_;

    uint32_t        packageStr_;
    uint32_t        versionStr_;
    uint32_t        conflictsStr_;
    uint32_t        dependsStr_;
    uint32_t        recommendsStr_;
    uint32_t        providesStr_;
    uint32_t        keepStr_;
    uint32_t        installedStr_;
    uint32_t        installStr_;
    uint32_t        removeStr_;
    uint32_t        upgradeStr_;
    uint32_t        featureStr_;
    uint32_t        noneStr_;
    uint32_t        trueStr_;
    uint32_t        falseStr_;

public:
    Cudf::PackageRef pkgRef;
    Cudf::PkgList    pkgList;
    Cudf::PkgFormula pkgFormula;
    EnumValues       identList;
};
