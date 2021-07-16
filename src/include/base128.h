// =====================================================================
//
// base128.h
//
// Author: Dave Freese, W1HKJ
// Copyright: 2012
//
// This file is part of FLAMP.
//
// This is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// =====================================================================

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <string>

typedef unsigned char t_byte;

class base128 {
#define LINELEN 64

private:
	bool ateof;
	int linelength;
	size_t iocp;
	size_t iolen;
	std::string output;

	void addlf(std::string &);
	void escape(std::string &, bool encode = true);
	void init();
	void remlf(std::string &);

public:
	base128() { init(); };
	~base128() {};
	std::string encode(std::string &in);
	std::string decode(std::string &in, bool &decode_error);
};

