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
parse_string    ::= .          { pParser->parseString(); }
parse_type(res) ::= ident(id). { res.index = id.index; pParser->parseType(id.index); }

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
preamble ::= PREAMBLE parse_string COLONSP STRING NL stanza ssep. { pParser->addPreamble(); }
universe ::= .
universe ::= universe package.
package  ::= PACKAGE COLONSP pkgname(name) NL stanza ssep.  { pParser->addPackage(name.index); }
request  ::= REQUEST parse_string COLONSP STRING NL stanza. { pParser->addRequest(); }

// stanzas
stanza ::= .
stanza ::= stanza COMMENT.
stanza ::= stanza property NL.
property ::= parse_type(name) COLONSP FEEDBACK_BOOL        bool(val).           { pParser->setProperty(name.index, pParser->mapBool(val.index)); }
property ::= parse_type(name) COLONSP FEEDBACK_IDENT       ident(val).          { pParser->setProperty(name.index, uint32_t(val.index)); }
property ::= parse_type(name) COLONSP FEEDBACK_ENUM        ident(val).          { /* name, val */ }
property ::= parse_type(name) COLONSP FEEDBACK_INT         int(val).            { pParser->setProperty(name.index, pParser->mapInt(val.index)); }
property ::= parse_type(name) COLONSP FEEDBACK_NAT         nat(val).            { pParser->setProperty(name.index, pParser->mapInt(val.index)); }
property ::= parse_type(name) COLONSP FEEDBACK_POSINT      posint(val).         { pParser->setProperty(name.index, pParser->mapInt(val.index)); }
property ::= parse_type(name) COLONSP FEEDBACK_PKGNAME     pkgname(val).        { pParser->setProperty(name.index, uint32_t(val.index)); }
property ::= parse_type(name) COLONSP FEEDBACK_TYPEDECL    typedecl(val).       { /* ignore: name, val */ }
property ::= parse_type(name) COLONSP FEEDBACK_VPKG        vpkg.                { pParser->setProperty(name.index, pParser->pkgRef); }
property ::= parse_type(name) COLONSP FEEDBACK_VEQPKG      veqpkg.              { pParser->setProperty(name.index, pParser->pkgRef); }
property ::= parse_type(name) COLONSP FEEDBACK_VPKGFORMULA vpkgformula.         { pParser->setProperty(name.index, pParser->pkgFormula); }
property ::= parse_type(name) COLONSP FEEDBACK_VPKGLIST    vpkglist.            { pParser->setProperty(name.index, pParser->pkgList); }
property ::= parse_type(name) COLONSP FEEDBACK_VEQPKGLIST  veqpkglist.          { pParser->setProperty(name.index, pParser->pkgList); }
property ::= parse_type(name) COLONSP parse_string FEEDBACK_STRING STRING(val). { pParser->setProperty(name.index, uint32_t(val.index)); }

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

equal(res) ::= SPEQUALSP(tok). { res.index = tok.index; }
equal(res) ::= EQUAL(tok).     { res.index = tok.index; }

// complex cudf types
veqpkg ::= pkgname(name).                           { pParser->setPkgRef(name.index); }
veqpkg ::= pkgname(name) SPEQUALSP(op) posint(num). { pParser->setPkgRef(name.index, op.index, num.index); }

vpkg ::= pkgname(name) RELOP(op) posint(num). { pParser->setPkgRef(name.index, op.index, num.index); }
vpkg ::= veqpkg.

orfla ::= vpkg.           { pParser->pkgList.clear(); pParser->pkgList.push_back(pParser->pkgRef); }
orfla ::= orfla BAR vpkg. { pParser->pkgList.push_back(pParser->pkgRef); }

andfla ::= orfla.              { pParser->pkgFormula.clear(); pParser->pushPkgList(); }
andfla ::= andfla comma orfla. { pParser->pushPkgList(); }

vpkgformula ::=	andfla.
vpkgformula ::=	TRUEX.  { pParser->pkgFormula.clear(); }
vpkgformula ::=	FALSEX. { pParser->pkgFormula.clear(); pParser->pkgList.clear(); pParser->pushPkgList(); }

vpkglist  ::= .
vpkglist  ::= nvpkglist.
nvpkglist ::= vpkg.                 { pParser->pkgList.clear(); pParser->pkgList.push_back(pParser->pkgRef); }
nvpkglist ::= nvpkglist comma vpkg. { pParser->pkgList.push_back(pParser->pkgRef); }

veqpkglist  ::= .
veqpkglist  ::= nveqpkglist.
nveqpkglist ::= veqpkg.                   { pParser->pkgList.clear(); pParser->pkgList.push_back(pParser->pkgRef); }
nveqpkglist ::= nveqpkglist comma veqpkg. { pParser->pkgList.push_back(pParser->pkgRef); }

// type declarations
typedecl  ::= .
typedecl  ::= ntypedecl.
ntypedecl ::= typedecl1.
ntypedecl ::= ntypedecl comma typedecl1.

identlist ::= ident(id).                   { pParser->identList.clear(); pParser->identList.push_back(id.index); }
identlist ::= identlist COMMASP ident(id). { pParser->identList.push_back(id.index); }

typedecl1 ::= ident(id) colon TYPE_ENUM space LBRAC identlist RBRAC.                              { pParser->addType(id.index, PARSER_FEEDBACK_ENUM); }
typedecl1 ::= ident(id) colon TYPE_ENUM space LBRAC identlist RBRAC equal LBRAC ident(val) RBRAC. { pParser->addType(id.index, PARSER_FEEDBACK_ENUM) = uint32_t(val.index); }
typedecl1 ::= ident(id) colon TYPE_BOOL.                                                          { pParser->addType(id.index, PARSER_FEEDBACK_BOOL); }
typedecl1 ::= ident(id) colon TYPE_BOOL equal LBRAC bool(val) RBRAC.                              { pParser->addType(id.index, PARSER_FEEDBACK_BOOL) = pParser->mapBool(val.index); }
typedecl1 ::= ident(id) colon TYPE_INT.                                                           { pParser->addType(id.index, PARSER_FEEDBACK_INT); }
typedecl1 ::= ident(id) colon TYPE_INT equal LBRAC int(val) RBRAC.                                { pParser->addType(id.index, PARSER_FEEDBACK_INT) = pParser->mapInt(val.index); }
typedecl1 ::= ident(id) colon TYPE_NAT.                                                           { pParser->addType(id.index, PARSER_FEEDBACK_NAT); }
typedecl1 ::= ident(id) colon TYPE_NAT equal LBRAC nat(val) RBRAC.                                { pParser->addType(id.index, PARSER_FEEDBACK_NAT) = pParser->mapInt(val.index); }
typedecl1 ::= ident(id) colon TYPE_POSINT.                                                        { pParser->addType(id.index, PARSER_FEEDBACK_POSINT); }
typedecl1 ::= ident(id) colon TYPE_POSINT equal LBRAC posint(val) RBRAC.                          { pParser->addType(id.index, PARSER_FEEDBACK_POSINT) = pParser->mapInt(val.index); }
typedecl1 ::= ident(id) colon TYPE_STRING.                                                        { pParser->addType(id.index, PARSER_FEEDBACK_STRING); }
typedecl1 ::= ident(id) colon TYPE_STRING equal LBRAC QUOTED(val) RBRAC.                          { pParser->addType(id.index, PARSER_FEEDBACK_STRING) = uint32_t(val.index); }
typedecl1 ::= ident(id) colon TYPE_PKGNAME.                                                       { pParser->addType(id.index, PARSER_FEEDBACK_PKGNAME); }
typedecl1 ::= ident(id) colon TYPE_PKGNAME equal LBRAC pkgname(val) RBRAC.                        { pParser->addType(id.index, PARSER_FEEDBACK_PKGNAME) = uint32_t(val.index); }
typedecl1 ::= ident(id) colon TYPE_IDENT.                                                         { pParser->addType(id.index, PARSER_FEEDBACK_IDENT); }
typedecl1 ::= ident(id) colon TYPE_IDENT equal LBRAC ident(val) RBRAC.                            { pParser->addType(id.index, PARSER_FEEDBACK_IDENT) = uint32_t(val.index); }
typedecl1 ::= ident(id) colon TYPE_VPKG.                                                          { pParser->addType(id.index, PARSER_FEEDBACK_VPKG); }
typedecl1 ::= ident(id) colon TYPE_VPKG equal LBRAC vpkg RBRAC.                                   { pParser->addType(id.index, PARSER_FEEDBACK_VPKG) = pParser->pkgRef; }
typedecl1 ::= ident(id) colon TYPE_VEQPKG.                                                        { pParser->addType(id.index, PARSER_FEEDBACK_VEQPKG); }
typedecl1 ::= ident(id) colon TYPE_VEQPKG equal LBRAC veqpkg RBRAC.                               { pParser->addType(id.index, PARSER_FEEDBACK_VEQPKG) = pParser->pkgRef; }
typedecl1 ::= ident(id) colon TYPE_VPKGFORMULA.                                                   { pParser->addType(id.index, PARSER_FEEDBACK_VPKGFORMULA); }
typedecl1 ::= ident(id) colon TYPE_VPKGFORMULA equal LBRAC vpkgformula RBRAC.                     { Cudf::Value &val = pParser->addType(id.index, PARSER_FEEDBACK_VPKGFORMULA) = Cudf::PkgFormula(); std::swap(pParser->pkgFormula, boost::any_cast<Cudf::PkgFormula&>(val)); }
typedecl1 ::= ident(id) colon TYPE_VPKGLIST.                                                      { pParser->addType(id.index, PARSER_FEEDBACK_VPKGLIST); }
typedecl1 ::= ident(id) colon TYPE_VPKGLIST equal LBRAC vpkglist RBRAC.                           { Cudf::Value &val = pParser->addType(id.index, PARSER_FEEDBACK_VPKGLIST) = Cudf::PkgList(); std::swap(pParser->pkgList, boost::any_cast<Cudf::PkgList&>(val)); }
typedecl1 ::= ident(id) colon TYPE_VEQPKGLIST.                                                    { pParser->addType(id.index, PARSER_FEEDBACK_VEQPKGLIST); }
typedecl1 ::= ident(id) colon TYPE_VEQPKGLIST equal LBRAC veqpkglist RBRAC.                       { Cudf::Value &val = pParser->addType(id.index, PARSER_FEEDBACK_VEQPKGLIST) = Cudf::PkgList(); std::swap(pParser->pkgList, boost::any_cast<Cudf::PkgList&>(val)); }
