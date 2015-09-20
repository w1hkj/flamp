// =====================================================================
//
// crc16.h ... crc16 checksum
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
//
// =====================================================================

#ifndef _CCRC16_
#define _CCRC16_

#include <string>
#include <stdio.h>
#include "debug.h"

using namespace std;

class Ccrc16 {
private:
	unsigned int crcval;
	char ss[8];
public:
	Ccrc16() { crcval = 0xFFFF; }
	~Ccrc16() {};
	void reset() { crcval = 0xFFFF;}
	unsigned int val() {return (crcval & 0xFFFF);}
	string sval() {
		snprintf((char *) ss, sizeof(ss), "%04X", val());
		return string(ss);
	}

	void update(unsigned int c) {
		crcval ^= (c & 0xFF);
		for (int i = 0; i < 8; ++i) {
			if (crcval & 1)
				crcval = (crcval >> 1) ^ 0xA001;
			else
				crcval = (crcval >> 1);
		}
	}

	unsigned int crc16(char c) {
		update((unsigned int) c);
		return val();
	}

	unsigned int crc16(char *s, size_t count) {
		reset();
		for (size_t i = 0; i < count; i++)
			update((unsigned int)(s[i] & 0xFF));  // only use lower half of unicode
		return val();
	}

	unsigned int crc16(string s) {
		reset();
		for (size_t i = 0; i < s.length(); i++)
			update((unsigned int)(s[i] & 0xFF));  // only use lower half of unicode
		return val();
	}
	string scrc16(string s) {
		crc16(s);
		return sval();
	}
};

#endif
