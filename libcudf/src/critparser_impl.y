//////////////////// Copyright ///////////////////////// {{{1

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

//////////////////// Preamble ////////////////////////// {{{1

%include {

#include <cassert>
#include "parser_impl.h"
#include "cudf/critparser.h"

}

%name             critparser
%stack_size       0
%parse_failure    { pParser->parseError(); }
%syntax_error     { pParser->syntaxError(); }
%extra_argument   { CritParser *pParser }
%token_type       { CritParser::Token }
%token_destructor { (void)pParser; (void)$$; }
%token_prefix     PARSER_
%start_symbol     crits

%type selector { Criterion::Selector }
%type attr     { std::string const* }
%type crit     { Criterion* }

//////////////////// Grammar /////////////////////////// {{{1

crits ::= ncrits.
crits ::= .

ncrits ::= ncrits COMMA scrit.
ncrits ::= scrit.

scrit ::= SIGN(sign) crit(crit). { crit->optimize = sign.maximize; }

crit(c) ::= COUNT            LPAREN selector(s) RPAREN.                             { c = &pParser->pushCrit(Criterion::COUNT, s); }
crit(c) ::= NOTUPTODATE      LPAREN selector(s) RPAREN.                             { c = &pParser->pushCrit(Criterion::NOTUPTODATE, s); }
crit(c) ::= UNSAT_RECOMMENDS LPAREN selector(s) RPAREN.                             { c = &pParser->pushCrit(Criterion::UNSAT_RECOMMENDS, s); }
crit(c) ::= SUM              LPAREN selector(s) COMMA attr(a) RPAREN.               { c = &pParser->pushCrit(Criterion::SUM, s, a); }
crit(c) ::= ALIGNED          LPAREN selector(s) COMMA attr(g) COMMA attr(v) RPAREN. { c = &pParser->pushCrit(Criterion::ALIGNED, s, g, v); }
// backwards compatibility
crit(c) ::= NEW.                                                                    { c = &pParser->pushCrit(Criterion::COUNT, Criterion::NEW); }
crit(c) ::= REMOVED.                                                                { c = &pParser->pushCrit(Criterion::COUNT, Criterion::REMOVED); }
crit(c) ::= CHANGED.                                                                { c = &pParser->pushCrit(Criterion::COUNT, Criterion::CHANGED); }
crit(c) ::= NOTUPTODATE.                                                            { c = &pParser->pushCrit(Criterion::NOTUPTODATE, Criterion::SOLUTION); }
crit(c) ::= UNSAT_RECOMMENDS.                                                       { c = &pParser->pushCrit(Criterion::UNSAT_RECOMMENDS, Criterion::SOLUTION); }
crit(c) ::= SUM LPAREN attr(a) RPAREN.                                              { c = &pParser->pushCrit(Criterion::SUM, Criterion::SOLUTION, a); }

selector(s) ::= SOLUTION.       { s = Criterion::SOLUTION; }
selector(s) ::= CHANGED.        { s = Criterion::CHANGED; }
selector(s) ::= NEW.            { s = Criterion::NEW; }
selector(s) ::= REMOVED.        { s = Criterion::REMOVED; }
selector(s) ::= UP.             { s = Criterion::UP; }
selector(s) ::= DOWN.           { s = Criterion::DOWN; }
selector(s) ::= INSTALLREQUEST. { s = Criterion::INSTALLREQUEST; }
selector(s) ::= UPGRADEREQUEST. { s = Criterion::UPGRADEREQUEST; }
selector(s) ::= REQUEST.        { s = Criterion::REQUEST; }

attr(r) ::= ATTR(a).          { r = a.string; }
attr(r) ::= SOLUTION.         { r = &pParser->string("solution"); }
attr(r) ::= CHANGED.          { r = &pParser->string("changed"); }
attr(r) ::= NEW.              { r = &pParser->string("new"); }
attr(r) ::= REMOVED.          { r = &pParser->string("removed"); }
attr(r) ::= UP.               { r = &pParser->string("up"); }
attr(r) ::= DOWN.             { r = &pParser->string("down"); }
attr(r) ::= INSTALLREQUEST.   { r = &pParser->string("installrequest"); }
attr(r) ::= UPGRADEREQUEST.   { r = &pParser->string("upgraderequest"); }
attr(r) ::= REQUEST.          { r = &pParser->string("request"); }
attr(r) ::= COUNT.            { r = &pParser->string("count"); }
attr(r) ::= NOTUPTODATE.      { r = &pParser->string("notuptodate"); }
attr(r) ::= UNSAT_RECOMMENDS. { r = &pParser->string("unsat_recommends"); }
attr(r) ::= SUM.              { r = &pParser->string("sum"); }
attr(r) ::= ALIGNED.          { r = &pParser->string("aligned"); }

