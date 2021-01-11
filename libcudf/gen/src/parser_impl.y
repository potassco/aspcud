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

%include {

#include <cassert>
#include "parser_impl.h"
#include "cudf/parser.hh"

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

//////////////////// Grammar /////////////////////////// {{{1

// lexer feedback
parse_string    ::= .                 { pParser->parseString(); }
parse_type(res) ::= nonkey_ident(id). { res.index = id.index; pParser->parseType(id.index); }

// flow elements
nnl ::= NL.
nnl ::= nnl NL.

nl ::= .
nl ::= nnl.

// overall structure
cudf ::= nl preamble universe request.

// document parts
preamble ::= PREAMBLE parse_string COLONSP STRING nnl stanza. { pParser->addPreamble(); }
preamble ::= .                                                { pParser->addPreamble(); }
universe ::= .
universe ::= universe package.
package  ::= PACKAGE COLONSP pkgname(name) nnl stanza.       { pParser->addPackage(name.index); }
request  ::= REQUEST parse_string COLONSP STRING nnl stanza. { pParser->addRequest(); }

// stanzas
stanza   ::= .
stanza   ::= stanza property nnl.
property ::= parse_type(name) COLONSP FEEDBACK_BOOL        bool(val).           { pParser->setProperty(name.index, pParser->mapBool(val.index)); }
property ::= parse_type(name) COLONSP FEEDBACK_IDENT       ident(val).          { pParser->setProperty(name.index, uint32_t(val.index)); }
property ::= parse_type(name) COLONSP FEEDBACK_ENUM        ident(val).          { pParser->setProperty(name.index, uint32_t(val.index)); }
property ::= parse_type(name) COLONSP FEEDBACK_INT         int(val).            { pParser->setProperty(name.index, pParser->mapInt(val.index)); }
property ::= parse_type(name) COLONSP FEEDBACK_NAT         nat(val).            { pParser->setProperty(name.index, pParser->mapInt(val.index)); }
property ::= parse_type(name) COLONSP FEEDBACK_POSINT      posint(val).         { pParser->setProperty(name.index, pParser->mapInt(val.index)); }
property ::= parse_type(name) COLONSP FEEDBACK_PKGNAME     pkgname(val).        { pParser->setProperty(name.index, uint32_t(val.index)); }
property ::= parse_type(name) COLONSP FEEDBACK_TYPEDECL    typedecl(val).       { /* ignore: name, val */ }
property ::= parse_type(name) COLONSP FEEDBACK_VPKG        vpkg.                { pParser->setProperty(name.index, std::move(pParser->pkgRef)); }
property ::= parse_type(name) COLONSP FEEDBACK_VEQPKG      veqpkg.              { pParser->setProperty(name.index, std::move(pParser->pkgRef)); }
property ::= parse_type(name) COLONSP FEEDBACK_VPKGFORMULA vpkgformula.         { pParser->setProperty(name.index, std::move(pParser->pkgFormula)); }
property ::= parse_type(name) COLONSP FEEDBACK_VPKGLIST    vpkglist.            { pParser->setProperty(name.index, std::move(pParser->pkgList)); }
property ::= parse_type(name) COLONSP FEEDBACK_VEQPKGLIST  veqpkglist.          { pParser->setProperty(name.index, std::move(pParser->pkgList)); }
property ::= parse_type(name) COLONSP parse_string FEEDBACK_STRING STRING(val). { pParser->setProperty(name.index, uint32_t(val.index)); }

// simple cudf types
bool(res) ::= TRUE(tok).  { res.index = tok.index; }
bool(res) ::= FALSE(tok). { res.index = tok.index; }

nonkey_ident(res) ::= IDENT(tok).            { res.index = tok.index; }
nonkey_ident(res) ::= TYPE_BOOL(tok).        { res.index = tok.index; }
nonkey_ident(res) ::= TYPE_INT(tok).         { res.index = tok.index; }
nonkey_ident(res) ::= TYPE_NAT(tok).         { res.index = tok.index; }
nonkey_ident(res) ::= TYPE_POSINT(tok).      { res.index = tok.index; }
nonkey_ident(res) ::= TYPE_STRING(tok).      { res.index = tok.index; }
nonkey_ident(res) ::= TYPE_PKGNAME(tok).     { res.index = tok.index; }
nonkey_ident(res) ::= TYPE_IDENT(tok).       { res.index = tok.index; }
nonkey_ident(res) ::= TYPE_VPKG(tok).        { res.index = tok.index; }
nonkey_ident(res) ::= TYPE_VEQPKG(tok).      { res.index = tok.index; }
nonkey_ident(res) ::= TYPE_VPKGFORMULA(tok). { res.index = tok.index; }
nonkey_ident(res) ::= TYPE_VPKGLIST(tok).    { res.index = tok.index; }
nonkey_ident(res) ::= TYPE_VEQPKGLIST(tok).  { res.index = tok.index; }
nonkey_ident(res) ::= TYPE_ENUM(tok).        { res.index = tok.index; }
nonkey_ident(res) ::= bool(tok).             { res.index = tok.index; }
nonkey_ident(res) ::= int(tok).              { res.index = tok.index; }

ident(res) ::= REQUEST(tok).      { res.index = tok.index; }
ident(res) ::= PREAMBLE(tok).     { res.index = tok.index; }
ident(res) ::= PACKAGE(tok).      { res.index = tok.index; }
ident(res) ::= nonkey_ident(tok). { res.index = tok.index; }

pkgname(res) ::= PKGNAME(tok). { res.index = tok.index; }
pkgname(res) ::= ident(tok).   { res.index = tok.index; }

posint(res) ::= POSINT(tok). { res.index = tok.index; }

nat(res) ::= NAT(tok).    { res.index = tok.index; }
nat(res) ::= posint(tok). { res.index = tok.index; }

int(res) ::= INT(tok). { res.index = tok.index; }
int(res) ::= nat(tok). { res.index = tok.index; }

// complex cudf types
veqpkg ::= pkgname(name).                       { pParser->setPkgRef(name.index); }
veqpkg ::= pkgname(name) EQUAL(op) posint(num). { pParser->setPkgRef(name.index, op.index, num.index); }

vpkg ::= pkgname(name) RELOP(op) posint(num). { pParser->setPkgRef(name.index, op.index, num.index); }
vpkg ::= veqpkg.

orfla ::= vpkg.           { pParser->pkgList.clear(); pParser->pkgList.push_back(pParser->pkgRef); }
orfla ::= orfla BAR vpkg. { pParser->pkgList.push_back(pParser->pkgRef); }

andfla ::= orfla.              { pParser->pkgFormula.clear(); pParser->pushPkgList(); }
andfla ::= andfla COMMA orfla. { pParser->pushPkgList(); }

vpkgformula ::= andfla.
vpkgformula ::= TRUEX.  { pParser->pkgFormula.clear(); }
vpkgformula ::= FALSEX. { pParser->pkgFormula.clear(); pParser->pkgList.clear(); pParser->pushPkgList(); }

vpkglist  ::= .
vpkglist  ::= nvpkglist.
nvpkglist ::= vpkg.                 { pParser->pkgList.clear(); pParser->pkgList.push_back(pParser->pkgRef); }
nvpkglist ::= nvpkglist COMMA vpkg. { pParser->pkgList.push_back(pParser->pkgRef); }

veqpkglist  ::= .
veqpkglist  ::= nveqpkglist.
nveqpkglist ::= veqpkg.                   { pParser->pkgList.clear(); pParser->pkgList.push_back(pParser->pkgRef); }
nveqpkglist ::= nveqpkglist COMMA veqpkg. { pParser->pkgList.push_back(pParser->pkgRef); }

// type declarations
typedecl  ::= .
typedecl  ::= ntypedecl.
ntypedecl ::= typedecl1.
ntypedecl ::= ntypedecl COMMA typedecl1.

identlist ::= ident(id).                 { pParser->identList.clear(); pParser->identList.push_back(id.index); }
identlist ::= identlist COMMA ident(id). { pParser->identList.push_back(id.index); }

colon ::= COLON.
colon ::= COLONSP.

typedecl1 ::= ident(id) colon TYPE_ENUM LBRAC identlist RBRAC.                              { pParser->addType(id.index, PARSER_FEEDBACK_ENUM); }
typedecl1 ::= ident(id) colon TYPE_ENUM LBRAC identlist RBRAC EQUAL LBRAC ident(val) RBRAC. { pParser->addType(id.index, PARSER_FEEDBACK_ENUM) = uint32_t(val.index); }
typedecl1 ::= ident(id) colon TYPE_BOOL.                                                    { pParser->addType(id.index, PARSER_FEEDBACK_BOOL); }
typedecl1 ::= ident(id) colon TYPE_BOOL EQUAL LBRAC bool(val) RBRAC.                        { pParser->addType(id.index, PARSER_FEEDBACK_BOOL) = pParser->mapBool(val.index); }
typedecl1 ::= ident(id) colon TYPE_INT.                                                     { pParser->addType(id.index, PARSER_FEEDBACK_INT); }
typedecl1 ::= ident(id) colon TYPE_INT EQUAL LBRAC int(val) RBRAC.                          { pParser->addType(id.index, PARSER_FEEDBACK_INT) = pParser->mapInt(val.index); }
typedecl1 ::= ident(id) colon TYPE_NAT.                                                     { pParser->addType(id.index, PARSER_FEEDBACK_NAT); }
typedecl1 ::= ident(id) colon TYPE_NAT EQUAL LBRAC nat(val) RBRAC.                          { pParser->addType(id.index, PARSER_FEEDBACK_NAT) = pParser->mapInt(val.index); }
typedecl1 ::= ident(id) colon TYPE_POSINT.                                                  { pParser->addType(id.index, PARSER_FEEDBACK_POSINT); }
typedecl1 ::= ident(id) colon TYPE_POSINT EQUAL LBRAC posint(val) RBRAC.                    { pParser->addType(id.index, PARSER_FEEDBACK_POSINT) = pParser->mapInt(val.index); }
typedecl1 ::= ident(id) colon TYPE_STRING.                                                  { pParser->addType(id.index, PARSER_FEEDBACK_STRING); }
typedecl1 ::= ident(id) colon TYPE_STRING EQUAL LBRAC QUOTED(val) RBRAC.                    { pParser->addType(id.index, PARSER_FEEDBACK_STRING) = uint32_t(val.index); }
typedecl1 ::= ident(id) colon TYPE_PKGNAME.                                                 { pParser->addType(id.index, PARSER_FEEDBACK_PKGNAME); }
typedecl1 ::= ident(id) colon TYPE_PKGNAME EQUAL LBRAC pkgname(val) RBRAC.                  { pParser->addType(id.index, PARSER_FEEDBACK_PKGNAME) = uint32_t(val.index); }
typedecl1 ::= ident(id) colon TYPE_IDENT.                                                   { pParser->addType(id.index, PARSER_FEEDBACK_IDENT); }
typedecl1 ::= ident(id) colon TYPE_IDENT EQUAL LBRAC ident(val) RBRAC.                      { pParser->addType(id.index, PARSER_FEEDBACK_IDENT) = uint32_t(val.index); }
typedecl1 ::= ident(id) colon TYPE_VPKG.                                                    { pParser->addType(id.index, PARSER_FEEDBACK_VPKG); }
typedecl1 ::= ident(id) colon TYPE_VPKG EQUAL LBRAC vpkg RBRAC.                             { pParser->addType(id.index, PARSER_FEEDBACK_VPKG) = pParser->pkgRef; }
typedecl1 ::= ident(id) colon TYPE_VEQPKG.                                                  { pParser->addType(id.index, PARSER_FEEDBACK_VEQPKG); }
typedecl1 ::= ident(id) colon TYPE_VEQPKG EQUAL LBRAC veqpkg RBRAC.                         { pParser->addType(id.index, PARSER_FEEDBACK_VEQPKG) = pParser->pkgRef; }
typedecl1 ::= ident(id) colon TYPE_VPKGFORMULA.                                             { pParser->addType(id.index, PARSER_FEEDBACK_VPKGFORMULA); }
typedecl1 ::= ident(id) colon TYPE_VPKGFORMULA EQUAL LBRAC vpkgformula RBRAC.               { Cudf::Value &val = pParser->addType(id.index, PARSER_FEEDBACK_VPKGFORMULA) = Cudf::PkgFormula(); std::swap(pParser->pkgFormula, boost::any_cast<Cudf::PkgFormula&>(val)); }
typedecl1 ::= ident(id) colon TYPE_VPKGLIST.                                                { pParser->addType(id.index, PARSER_FEEDBACK_VPKGLIST); }
typedecl1 ::= ident(id) colon TYPE_VPKGLIST EQUAL LBRAC vpkglist RBRAC.                     { Cudf::Value &val = pParser->addType(id.index, PARSER_FEEDBACK_VPKGLIST) = Cudf::PkgList(); std::swap(pParser->pkgList, boost::any_cast<Cudf::PkgList&>(val)); }
typedecl1 ::= ident(id) colon TYPE_VEQPKGLIST.                                              { pParser->addType(id.index, PARSER_FEEDBACK_VEQPKGLIST); }
typedecl1 ::= ident(id) colon TYPE_VEQPKGLIST EQUAL LBRAC veqpkglist RBRAC.                 { Cudf::Value &val = pParser->addType(id.index, PARSER_FEEDBACK_VEQPKGLIST) = Cudf::PkgList(); std::swap(pParser->pkgList, boost::any_cast<Cudf::PkgList&>(val)); }
