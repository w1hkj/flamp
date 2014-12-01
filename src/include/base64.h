// =====================================================================
//
// base64.h
//
// Author: Dave Freese, W1HKJ
// Copyright: 2010
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

using namespace std;

typedef unsigned char byte;

class base64 {
#define LINELEN 64

private:
	bool ateof;
	bool crlf;
	byte dtable[256];
	byte etable[256];
	int linelength;
	size_t iocp;
	size_t iolen;
	string output;
	void init();

public:
	base64(bool t = false) {crlf = t; init(); };
	~base64(){};
	string encode(string in);
	string decode(string in, bool &decode_error);
};
