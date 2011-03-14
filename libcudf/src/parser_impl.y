// Copyright (c) 2010, Roland Kaminski <kaminski@cs.uni-potsdam.de>
//
// This file is part of gringo.
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

%include {

#include "cassert"
#include "parser_impl.h"
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
%start_symbol     start

start ::= preamble packages request.

// ======== Cudf Type Parsing ========

// string types
pkgname(r) ::= PKGNAME(v). { r.index = v.index; }
pkgname(r) ::= ident(v).   { r.index = v.index; }
pkgname(r) ::= POSINT(v).  { r.index = v.index; }

ident(r) ::= IDENT(v). { r.index = v.index; }
ident(r) ::= bool(v).  { r.index = v.index; }
ident(r) ::= keep(v).  { r.index = v.index; }

// versioned packages
vpkg ::= veqpkg.
vpkg ::= pkgname(n) RELOP(r) POSINT(v). { pParser->addPkg(n.index, r.index, v.index); }

veqpkg ::= pkgname(n).                    { pParser->addPkg(n.index); }
veqpkg ::= pkgname(n) EQUAL(r) POSINT(v). { pParser->addPkg(n.index, r.index, v.index); }

// conjunctive normal form over versioned packages
vpkgformula ::= and_formula.
vpkgformula ::= TRUEX.  { pParser->setConstant(true); }
vpkgformula ::= FALSEX. { pParser->setConstant(false); }

and_formula ::= or_formula.                   { pParser->addClause(); }
and_formula ::= and_formula COMMA or_formula. { pParser->addClause(); }

or_formula  ::= vpkg.                { pParser->addToClause(); }
or_formula  ::= or_formula BAR vpkg. { pParser->addToClause(); }

// lists of versioned packages
vpkglist  ::= .
vpkglist  ::= nvpkglist.

nvpkglist ::= vpkg.                 { pParser->addToList(); }
nvpkglist ::= nvpkglist COMMA vpkg. { pParser->addToList(); }

veqpkglist  ::= .
veqpkglist  ::= nveqpkglist.

nveqpkglist ::= veqpkg.                 { pParser->addToList(); }
nveqpkglist ::= nveqpkglist COMMA vpkg. { pParser->addToList(); }

bool(b) ::= IDENT_TRUE(i).  { b.index = i.index; }
bool(b) ::= IDENT_FALSE(i). { b.index = i.index; }

keep(k) ::= IDENT_VERSION(i). { k.index = i.index; }
keep(k) ::= IDENT_PACKAGE(i). { k.index = i.index; }
keep(k) ::= IDENT_FEATURE(i). { k.index = i.index; }
keep(k) ::= IDENT_NONE(i).    { k.index = i.index; }

// ======== Parse Generic Property Lines ========

properties ::= .
properties ::= properties property NL.

property ::= ATTRIBUTE    IGNORE.
property ::= VERSION      POSINT(v).   { pParser->setVersion(v.index); }
property ::= DEPENDS      vpkgformula. { pParser->setDepends(); }
property ::= CONFLICTS    vpkglist.    { pParser->setConflicts(); }
property ::= INSTALL      vpkglist.    { pParser->setInstall(); }
property ::= REMOVE       vpkglist.    { pParser->setRemove(); }
property ::= UPGRADE      vpkglist.    { pParser->setUpgrade(); }
property ::= PROVIDES     veqpkglist.  { pParser->setProvides(); }
property ::= INSTALLED    bool(b).     { pParser->setInstalled(b.index); }
property ::= KEEP         keep(e).     { pParser->setKeep(e.index); }
property ::= RECOMMENDS   vpkgformula. { pParser->setRecommends(); }

// ======== Preamble Parsing ========

preamble ::= .
preamble ::= nl PREAMBLE IGNORE NL properties.

// ======== Package Parsing ========

packages ::= .
packages ::= package packages.

package_ ::= PACKAGE pkgname(n) NL. { pParser->newPackage(n.index); }
package  ::= package_ properties.

// ======== Request Parsing ========

request_ ::= REQUEST IGNORE NL. { pParser->newRequest(); }
request  ::= request_ properties.

nl ::= .
nl ::= NL.
