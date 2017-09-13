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

#include <cudf/parser.hh>
#include <stdexcept>
#include "parser_impl.h"
#include <cassert>

void *parserAlloc(void *(*mallocProc)(size_t));
void parserFree(void *p, void (*freeProc)(void*));
void parser(void *yyp, int yymajor, Parser::Token yyminor, Parser *pParser);

//////////////////// Parser //////////////////////////// {{{1

Parser::Type::Type(uint32_t type)
    : type(type) { }

bool Parser::Type::stringType() const {
    return type == PARSER_FEEDBACK_STRING || type == PARSER_FEEDBACK_PKGNAME || type == PARSER_FEEDBACK_ENUM || type == PARSER_FEEDBACK_IDENT;
}

bool Parser::Type::intType() const {
    return
        type == PARSER_FEEDBACK_INT ||
        type == PARSER_FEEDBACK_NAT ||
        type == PARSER_FEEDBACK_POSINT;
}

Parser::Parser(Dependency &dep)
    : dep_(dep)
    , doc_(0)
    , parser_(parserAlloc(malloc))
    , lexString_(false)
    , shiftToken_(0) {
    versionStr_    = dep_.index("version");
    conflictsStr_  = dep_.index("conflicts");
    dependsStr_    = dep_.index("depends");
    recommendsStr_ = dep_.index("recommends");
    providesStr_   = dep_.index("provides");
    keepStr_       = dep_.index("keep");
    installedStr_  = dep_.index("installed");

    installStr_ = dep_.index("install");
    removeStr_  = dep_.index("remove");
    upgradeStr_ = dep_.index("upgrade");

    trueStr_  = dep_.index("true");
    falseStr_ = dep_.index("false");

    packageStr_ = dep_.index("package");
    featureStr_ = dep_.index("feature");
    noneStr_    = dep_.index("none");

    uint32_t wasInstalledStr = dep_.index("was-installed");
    uint32_t emptyStr = dep_.index("");

    // preamble
    addType(dep_.index("property"),        PARSER_FEEDBACK_TYPEDECL) = uint32_t(0);
    addType(dep_.index("univ-checksum"),   PARSER_FEEDBACK_STRING)   = uint32_t(emptyStr);
    addType(dep_.index("status-checksum"), PARSER_FEEDBACK_STRING)   = uint32_t(emptyStr);
    addType(dep_.index("req-checksum"),    PARSER_FEEDBACK_STRING)   = uint32_t(emptyStr);

    // package
    addType(packageStr_,     PARSER_FEEDBACK_PKGNAME);
    addType(versionStr_,     PARSER_FEEDBACK_POSINT);
    addType(dependsStr_,     PARSER_FEEDBACK_VPKGFORMULA) = pkgFormula;
    addType(conflictsStr_,   PARSER_FEEDBACK_VPKGLIST)    = pkgList;
    addType(providesStr_,    PARSER_FEEDBACK_VEQPKGLIST)  = pkgList;
    addType(installedStr_,   PARSER_FEEDBACK_BOOL)        = false;
    addType(wasInstalledStr, PARSER_FEEDBACK_BOOL)        = false;

    /*
    EnumValues values;
    values.insert(packageStr);
    values.insert(featureStr);
    values.insert(versionStr);
    values.insert(noneStr);
    */
    addType(dep_.index("keep"), PARSER_FEEDBACK_ENUM) = uint32_t(noneStr_);

    // request
    addType(installStr_, PARSER_FEEDBACK_VPKGLIST) = pkgList;
    addType(removeStr_,  PARSER_FEEDBACK_VPKGLIST) = pkgList;
    addType(upgradeStr_, PARSER_FEEDBACK_VPKGLIST) = pkgList;
}

void Parser::parseType(uint32_t index) {
    TypeMap::iterator it = typeMap_.find(index);
    if (it != typeMap_.end()) { shiftToken_ = it->second.type; }
    else { syntaxError(); }
}

void Parser::parseError() { }

std::string Parser::errorToken() {
    if(eof()) return "<EOF>";
    else return string();
}

void Parser::syntaxError() {
    throw std::runtime_error("syntax error:" + errorToken());
}

void Parser::parse(std::istream &in) {
    Cudf::Document doc;
    doc_ = &doc;
    reset(&in);
    int token;
    do {
        if (shiftToken_) {
            token       = shiftToken_;
            shiftToken_ = 0;
        }
        else if(lexString_) {
            lexString_ = false;
            token      = lexString();
        }
        else { token = lex(); }
        // std::cerr << "lexed: '" << string() << "' (" << token << ")" << std::endl;
        parser(parser_, token, token_, this);
    }
    while(token != 0);
    dep_.init(*doc_);
    doc_ = 0;
}

Parser::~Parser() {
    parserFree(parser_, free);
}

#include "lexer.hh"
