// =====================================================================
//
// file_io.h
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

#ifndef FILE_IO_H
#define FILE_IO_H

#include <string.h>
#include <string>
#include "socket.h"

#ifdef WIN32
#  include <winsock2.h>
#else
#  include <arpa/inet.h>
#endif

//using namespace std;

enum {NONE, BASE64, BASE128, BASE256};

extern Socket *tcpip;
extern Address *localaddr;
extern bool bConnected;

extern void compress_maybe(std::string& input, int encode_with, bool try_compress = true);
extern void decompress_maybe(std::string& input);
extern void send_via_fldigi(std::string tosend);
extern void connect_to_fldigi(void *);

extern void rx_extract_reset();
extern int  rx_fldigi(std::string &);
extern int  rx_fldigi(char *buffer, int limit);

extern bool binary(std::string &);
extern bool c_binary(int c);
extern bool isPlainText(std::string &_buffer);
extern bool isPlainText(char *_buffer, size_t count);

#endif
