#!/bin/bash

cd "$(dirname "$0")"/..

mkdir -p build/debug
cmake -H. -Bbuild/debug
make -C build/debug

mkdir -p libcudf/gen/src
cp build/debug/libcudf/src/{critlexer.hh,critparser_impl.cc,critparser_impl.h,critparser_impl.y,lexer.hh,parser_impl.cc,parser_impl.h,parser_impl.y} libcudf/gen/src
