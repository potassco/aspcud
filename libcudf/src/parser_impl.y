//
// Copyright (c) 2010, Roland Kaminski <kaminski@cs.uni-potsdam.de>
//
// This file is part of aspcud.
//
// gringo is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// gringo is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with gringo.  If not, see <http://www.gnu.org/licenses/>.
//

%include {

#include "cassert"
#include "parser_impl.h"
#include "parser_priv.h"
#include "cudf/parser.h"

}

%name             parser
%stack_size       0
%parse_failure    { pParser->parseError(); }
%syntax_error     { pParser->syntaxError(); }
%extra_argument   { Parser *pParser }
%token_type       { Parser::Token }
%token_destructor { (void)pParser; (void)$$; }
%token_prefix     PARSER_
%start_symbol     cudf

// lexer feedback
parse_string ::= .          { pParser->parseString(); }
parse_type   ::= ident(id). { pParser->parseType(id.index); }

// overall structure
cudf ::= universe request.
cudf ::= preamble universe request.

// flow elements
skip    ::= .
skip    ::= skip COMMENT.
skip    ::= skip NL.
ssep    ::= NL skip.
space   ::= .
space   ::= space SPACE.
colon   ::= space COLON space.
colon   ::= space COLONSP space.
comma   ::= COMMA.
comma   ::= COMMASP.

// document parts
preamble ::= PREAMBLE parse_string COLONSP STRING NL stanza ssep.
universe ::= .
universe ::= universe package.
package  ::= PACKAGE COLONSP pkgname NL stanza ssep.
request  ::= REQUEST parse_string COLONSP STRING NL stanza.

// stanzas
stanza ::= .
stanza ::= stanza COMMENT.
stanza ::= stanza property NL.
property ::= parse_type COLONSP value.
value ::= FEEDBACK_BOOL        bool.
value ::= FEEDBACK_IDENT       ident.
value ::= FEEDBACK_ENUM        ident.
value ::= FEEDBACK_INT         int.
value ::= FEEDBACK_NAT         nat.
value ::= FEEDBACK_POSINT      posint.
value ::= FEEDBACK_PKGNAME     pkgname.
value ::= FEEDBACK_TYPEDECL    typedecl.
value ::= FEEDBACK_VPKG        vpkg.
value ::= FEEDBACK_VEQPKG      veqpkg.
value ::= FEEDBACK_VPKGFORMULA vpkgformula.
value ::= FEEDBACK_VPKGLIST    vpkglist.
value ::= FEEDBACK_VEQPKGLIST  veqpkglist.
value ::= parse_string FEEDBACK_STRING STRING.

// simple cudf types
bool(res) ::= TRUE(tok).  { res.index = tok.index; }
bool(res) ::= FALSE(tok). { res.index = tok.index; }

ident(res) ::= IDENT(tok).            { res.index = tok.index; }
ident(res) ::= PREAMBLE(tok).         { res.index = tok.index; }
ident(res) ::= PACKAGE(tok).          { res.index = tok.index; }
ident(res) ::= REQUEST(tok).          { res.index = tok.index; }
ident(res) ::= TYPE_BOOL(tok).        { res.index = tok.index; }
ident(res) ::= TYPE_INT(tok).         { res.index = tok.index; }
ident(res) ::= TYPE_NAT(tok).         { res.index = tok.index; }
ident(res) ::= TYPE_POSINT(tok).      { res.index = tok.index; }
ident(res) ::= TYPE_STRING(tok).      { res.index = tok.index; }
ident(res) ::= TYPE_PKGNAME(tok).     { res.index = tok.index; }
ident(res) ::= TYPE_IDENT(tok).       { res.index = tok.index; }
ident(res) ::= TYPE_VPKG(tok).        { res.index = tok.index; }
ident(res) ::= TYPE_VEQPKG(tok).      { res.index = tok.index; }
ident(res) ::= TYPE_VPKGFORMULA(tok). { res.index = tok.index; }
ident(res) ::= TYPE_VPKGLIST(tok).    { res.index = tok.index; }
ident(res) ::= TYPE_VEQPKGLIST(tok).  { res.index = tok.index; }
ident(res) ::= TYPE_ENUM(tok).        { res.index = tok.index; }
ident(res) ::= bool(tok).             { res.index = tok.index; }

pkgname(res) ::= PKGNAME(tok). { res.index = tok.index; }
pkgname(res) ::= ident(tok).   { res.index = tok.index; }

posint(res) ::= POSINT(tok). { res.index = tok.index; }

nat(res) ::= NAT(tok).    { res.index = tok.index; }
nat(res) ::= posint(tok). { res.index = tok.index; }

int(res) ::= INT(tok). { res.index = tok.index; }
int(res) ::= nat(tok). { res.index = tok.index; }

relop(res) ::= SPEQUALSP(tok). { res.index = tok.index; }
relop(res) ::= RELOP(tok).     { res.index = tok.index; }

equal(res) ::= SPEQUALSP(tok). { res.index = tok.index; }
equal(res) ::= EQUAL(tok).     { res.index = tok.index; }

// complex cudf types
vpkg ::=	pkgname.
vpkg ::=	pkgname relop posint.

veqpkg ::= pkgname.
veqpkg ::= pkgname SPEQUALSP posint.

orfla ::= vpkg.
orfla ::= orfla BAR vpkg.

andfla ::= orfla.
andfla ::= andfla comma orfla.

vpkgformula ::=	andfla.
vpkgformula ::=	TRUEX.
vpkgformula ::=	FALSEX.

vpkglist  ::= .
vpkglist  ::= nvpkglist.
nvpkglist ::= vpkg.
nvpkglist ::= nvpkglist comma vpkg.

veqpkglist  ::= .
veqpkglist  ::= nveqpkglist.
nveqpkglist ::= veqpkg.
nveqpkglist ::= nveqpkglist comma veqpkg.

// type declarations
typedecl  ::= .
typedecl  ::= ntypedecl.
ntypedecl ::= typedecl1.
ntypedecl ::= ntypedecl comma typedecl1.

identlist ::= ident.
identlist ::= identlist COMMASP ident.

typedecl1 ::= ident(id) colon TYPE_ENUM space LBRAC identlist RBRAC.                                   { pParser->addType(id.index, new Cudf::RequiredValue<PARSER_FEEDBACK_ENUM>()); }
typedecl1 ::= ident(id) colon TYPE_ENUM space LBRAC identlist(lst) RBRAC equal LBRAC ident(val) RBRAC. { /* id lst val */ }
typedecl1 ::= ident(id) colon TYPE_BOOL.                                                               { pParser->addType(id.index, new Cudf::RequiredValue<PARSER_FEEDBACK_BOOL>()); }
typedecl1 ::= ident(id) colon TYPE_BOOL equal LBRAC bool(val) RBRAC.                                   { pParser->addType(id.index, new Cudf::OptionalValue<PARSER_FEEDBACK_BOOL, bool>(pParser->mapBool(val.index))); }
typedecl1 ::= ident(id) colon TYPE_INT.                                                                { pParser->addType(id.index, new Cudf::RequiredValue<PARSER_FEEDBACK_INT>()); }
typedecl1 ::= ident(id) colon TYPE_INT equal LBRAC int(val) RBRAC.                                     { pParser->addType(id.index, new Cudf::OptionalValue<PARSER_FEEDBACK_INT, int32_t>(pParser->mapInt(val.index))); }
typedecl1 ::= ident(id) colon TYPE_NAT.                                                                { pParser->addType(id.index, new Cudf::RequiredValue<PARSER_FEEDBACK_NAT>()); }
typedecl1 ::= ident(id) colon TYPE_NAT equal LBRAC nat(val) RBRAC.                                     { pParser->addType(id.index, new Cudf::OptionalValue<PARSER_FEEDBACK_NAT, int32_t>(pParser->mapInt(val.index))); }
typedecl1 ::= ident(id) colon TYPE_POSINT.                                                             { pParser->addType(id.index, new Cudf::RequiredValue<PARSER_FEEDBACK_POSINT>()); }
typedecl1 ::= ident(id) colon TYPE_POSINT equal LBRAC posint(val) RBRAC.                               { pParser->addType(id.index, new Cudf::OptionalValue<PARSER_FEEDBACK_POSINT, int32_t>(pParser->mapInt(val.index))); }
typedecl1 ::= ident(id) colon TYPE_STRING.                                                             { pParser->addType(id.index, new Cudf::RequiredValue<PARSER_FEEDBACK_STRING>()); }
typedecl1 ::= ident(id) colon TYPE_STRING equal LBRAC QUOTED(val) RBRAC.                               { pParser->addType(id.index, new Cudf::OptionalValue<PARSER_FEEDBACK_STRING, uint32_t>(val.index)); }
typedecl1 ::= ident(id) colon TYPE_PKGNAME.                                                            { pParser->addType(id.index, new Cudf::RequiredValue<PARSER_FEEDBACK_PKGNAME>()); }
typedecl1 ::= ident(id) colon TYPE_PKGNAME equal LBRAC pkgname(val) RBRAC.                             { pParser->addType(id.index, new Cudf::OptionalValue<PARSER_FEEDBACK_PKGNAME, uint32_t>(val.index)); }
typedecl1 ::= ident(id) colon TYPE_IDENT.                                                              { pParser->addType(id.index, new Cudf::RequiredValue<PARSER_FEEDBACK_IDENT>()); }
typedecl1 ::= ident(id) colon TYPE_IDENT equal LBRAC ident(val) RBRAC.                                 { pParser->addType(id.index, new Cudf::OptionalValue<PARSER_FEEDBACK_IDENT, uint32_t>(val.index)); }
typedecl1 ::= ident(id) colon TYPE_VPKG.                                                               { pParser->addType(id.index, new Cudf::RequiredValue<PARSER_FEEDBACK_VPKG>()); }
typedecl1 ::= ident colon TYPE_VPKG equal LBRAC vpkg RBRAC.
typedecl1 ::= ident(id) colon TYPE_VEQPKG.                                                             { pParser->addType(id.index, new Cudf::RequiredValue<PARSER_FEEDBACK_VEQPKG>()); }
typedecl1 ::= ident colon TYPE_VEQPKG equal LBRAC veqpkg RBRAC.
typedecl1 ::= ident(id) colon TYPE_VPKGFORMULA.                                                        { pParser->addType(id.index, new Cudf::RequiredValue<PARSER_FEEDBACK_VPKGFORMULA>()); }
typedecl1 ::= ident colon TYPE_VPKGFORMULA equal LBRAC vpkgformula RBRAC.
typedecl1 ::= ident(id) colon TYPE_VPKGLIST.                                                           { pParser->addType(id.index, new Cudf::RequiredValue<PARSER_FEEDBACK_VPKGLIST>()); }
typedecl1 ::= ident colon TYPE_VPKGLIST equal LBRAC vpkglist RBRAC.
typedecl1 ::= ident(id) colon TYPE_VEQPKGLIST.                                                         { pParser->addType(id.index, new Cudf::RequiredValue<PARSER_FEEDBACK_VEQPKGLIST>()); }
typedecl1 ::= ident colon TYPE_VEQPKGLIST equal LBRAC veqpkglist RBRAC.
