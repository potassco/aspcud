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
//////////////////// Preamble ////////////////////////// {{{1

#include <cudf/critparser.hh>
#include <stdexcept>
#include "critparser_impl.h"
#include <cassert>

void *critparserAlloc(void *(*mallocProc)(size_t));
void critparserFree(void *p, void (*freeProc)(void*));
void critparser(void *yyp, int yymajor, CritParser::Token yyminor, CritParser *pParser);

//////////////////// CritParser //////////////////////////// {{{1

CritParser::CritParser(Criteria::CritVec &crits)
    : crits_(crits)
    , parser_(critparserAlloc(malloc))
    , error_(false) { }

void CritParser::parseError() {
    error_ = true;
}

std::string CritParser::errorToken() {
    if(eof()) return "<EOF>";
    else return string();
}

void CritParser::syntaxError() {
    error_    = true;
    errorStr_ = "syntax error:" + errorToken();
}

bool CritParser::parse(std::istream &in) {
    reset(&in);
    for (int token = lex(); token != 0 && !error_; token = lex()) {
        if (!error_) {
            //std::cerr << "lexed: '" << string() << "' (" << token << ")" << std::endl;
            critparser(parser_, token, token_, this);
        }
    }
    if (!error_) { critparser(parser_, 0, token_, this); }
    return !error_;
}

Criterion &CritParser::pushCrit(Criterion::Measurement m, Criterion::Selector s, std::string const *a1, std::string const *a2) {
    crits_.push_back(Criterion());
    crits_.back().optimize    = false;
    crits_.back().measurement = m;
    crits_.back().selector    = s;
    if (a1) { crits_.back().attr1 = *a1; }
    if (a2) { crits_.back().attr2 = *a2; }
    return crits_.back();
}

std::string const &CritParser::string(std::string const &x) {
    return *strings_.insert(x).first;
}

CritParser::~CritParser() {
    critparserFree(parser_, free);
}

#include "critlexer.hh"
