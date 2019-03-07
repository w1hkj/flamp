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
#include <sys/time.h>

#include "socket.h"


#ifdef WIN32
#  include <winsock2.h>
#else
#  include <arpa/inet.h>
#endif

//using namespace std;

enum {NONE, BASE64, BASE128, BASE256};

extern Address *localaddr;
extern bool bConnected;
extern Socket *tcpip;

extern struct timeval start_time;
extern int transfer_minutes;

extern void compress_maybe(std::string& input, int encode_with, bool try_compress = true);
extern void connect_to_fldigi(void *);
extern void decompress_maybe(std::string& input);
extern void transfer(std::string tosend);

extern int  rx_fldigi(char *buffer, int limit);
extern int  rx_fldigi(std::string &);
extern void rx_extract_reset();

extern bool binary(std::string &);
extern bool c_binary(int c);
extern bool isPlainText(char *_buffer, size_t count);
extern bool isPlainText(std::string &_buffer);

#endif
