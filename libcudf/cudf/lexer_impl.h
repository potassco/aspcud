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

#pragma once

#include <stdint.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <list>
#include <cstddef>

#define YYCTYPE   char
#define YYCURSOR  state().cursor_
#define YYLIMIT   state().limit_
#define YYMARKER  state().marker_
#define YYFILL(n) {state().fill(n);}

class LexerImpl
{
private:
	class State
	{
	public:
		State() :
			in_(&std::cin),
			bufmin_(65536), bufsize_(0), buffer_(0),
			start_(0), offset_(0), cursor_(0),
			limit_(0), marker_(0), eof_(0),
			line_(1)
		{
		}
		void fill(size_t n)
		{
			if(eof_) return;
			if(start_ > buffer_)
			{
				size_t shift = start_ - buffer_;
				memmove(buffer_, start_, limit_ - start_);
				start_  = buffer_;
				offset_-= shift;
				marker_-= shift;
				limit_ -= shift;
				cursor_-= shift;
			}
repeat:
			size_t inc = n < bufmin_ ? bufmin_ : n;
			if(bufsize_ < inc + (limit_ - buffer_))
			{
				bufsize_ = inc + (limit_ - buffer_);
				char *buf = (char*)realloc(buffer_, bufsize_ * sizeof(char));
				start_  = buf + (start_ - buffer_);
				cursor_ = buf + (cursor_ - buffer_);
				limit_  = buf + (limit_ - buffer_);
				marker_ = buf + (marker_ - buffer_);
				offset_ = buf + (offset_ - buffer_);
				buffer_ = buf;

			}
			in_->read(limit_, inc);
			char *i = limit_, *start = limit_;
			limit_ += in_->gcount();
			for(char *j = i; j < limit_; j++)
			{
				if(*j == '\n' && j + 1 < limit_ && *(j + 1) == ' ')
				{
					// TODO: at this point line numbers have to be adjusted
					j++;
				}
				else { *i++ = *j; }
			}
			limit_ = i;
			if(in_->gcount() < (std::streamsize)inc)
			{
				eof_ = limit_;
				*eof_++ = 0;
			}
			else
			{
				if(*(limit_ - 1) == '\n' && in_->peek() == ' ') { in_->get(); limit_--; }
				if(limit_ - start < (std::ptrdiff_t)n)          { goto repeat; }
			}
		}
		void step()
		{
			offset_ = cursor_;
			line_++;
		}
		void start() { start_ = cursor_; }
		void unget() { cursor_--; }
		void reset(std::istream *in)
		{
			in_     = in;
			start_  = buffer_;
			offset_ = buffer_;
			cursor_ = buffer_;
			limit_  = buffer_;
			marker_ = buffer_;
			eof_    = 0;
			line_   = 1;
		}
		~State() { if(buffer_) free(buffer_); }

	public:
		std::istream *in_;
		size_t bufmin_;
		size_t bufsize_;
		char *buffer_;
		char *start_;
		char *offset_;
		char *cursor_;
		char *limit_;
		char *marker_;
		char *eof_;
		int line_;
	};
protected:
	LexerImpl() : states_(1) { }
	void start() { state().start(); }
	void unget() { state().unget(); }
	bool eof() const { return state().cursor_ == state().eof_; }
	void reset(std::istream *in) { state().reset(in); }
	std::string &string(uint32_t start = 0, uint32_t end = 0)
	{
		string_.assign(state().start_ + start, state().cursor_ - end);
		return string_;
	}
	void step() { state().step(); }
	int integer()
	{
		int r = 0;
		int s = 0;
		if(*state().start_ == '-') s = 1;
		for(char *i = state().start_ + s; i != state().cursor_; i++)
		{
			r *= 10;
			r += *i - '0';
		}
		return s ? -r : r;
	}
	void push() { states_.push_back(State()); }
	void pop() { states_.pop_back(); }
	const State &state() const { return states_.back(); }
	State &state() { return states_.back(); }
public:
	int line() { return state().line_; }
	int column() { return state().start_ - state().offset_ + 1; }
private:
	std::string string_;
	std::list<State> states_;
};

