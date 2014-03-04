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

#pragma once

#include <cudf/lexer_impl.h>
#include <cudf/dependency.h>

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

