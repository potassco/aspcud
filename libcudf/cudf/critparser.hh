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
#include <cudf/dependency.hh>

//////////////////// Parser //////////////////////////////////// {{{1

class CritParser : public LexerImpl {
public:
    union Token {
        bool               maximize;
        std::string const *string;
    };
    using LexerImpl::string;

public:
    CritParser(Criteria::CritVec &crits);
    int lex();
    std::string errorToken();
    void syntaxError();
    void parseError();
    bool parse(std::istream &sin);
    std::string const &string(std::string const &x);
    Criterion &pushCrit(Criterion::Measurement m, Criterion::Selector s, std::string const *a1 = 0, std::string const *a2 = 0);
    ~CritParser();

private:
    typedef std::set<std::string> StringSet;

    StringSet          strings_;
    Criteria::CritVec &crits_;
    Token              token_;
    void              *parser_;
    std::string        errorStr_;
    bool               error_;
};

