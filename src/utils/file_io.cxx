// file_io.cxx
//
// Author: Dave Freese, W1HKJ (2012)
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
// along with the program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

// $Id: main.c 141 2008-07-19 15:59:57Z jessekornblum $

#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>

#include <FL/Fl.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/x.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_File_Icon.H>
#include <math.h>

#include "config.h"
#include "flamp_config.h"
#include "flamp.h"

#include "debug.h"
#include "util.h"
#include "gettext.h"
#include "flinput2.h"
#include "date.h"
#include "calendar.h"
#include "icons.h"
#include "fileselect.h"
#include "file_io.h"
#include "xml_io.h"
#include "status.h"
#include "pixmaps.h"

#include "base64.h"
#include "base128.h"
#include "base256.h"
#include "lzma/LzmaLib.h"
#include "status.h"

#ifdef WIN32
#  include "flamprc.h"
#  include "compat.h"
#  define dirent fl_dirent_no_thanks
#endif

#include <FL/filename.H>
#include "dirent-check.h"

#include <FL/x.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Image.H>

using namespace std;

Socket *tcpip = (Socket *)0;
Address *localaddr = (Address *)0;

const char *b64_start = "[b64:start]";
const char *b64_end = "\n[b64:end]";
const char *b128_start = "[b128:start]";
const char *b128_end = "\n[b128:end]";
const char *b256_start = "[b256:start]";
const char *b256_end = "\n[b256:end]";

string errtext;

base64 b64; // use b65(1) to insert lf for ease of viewing required
base128 b128;
base256 b256;

string inptext = "";
string wtext = "";
string check = "";
string wrap_outfilename = "";
string wrap_inpfilename = "";
string wrap_inpshortname = "";
string wrap_outshortname = "";
string wrap_foldername = "";

void base64encode(string &inptext)
{
	string outtext;
	outtext = b64.encode(inptext);
	inptext = b64_start;
	inptext.append(outtext);
	inptext.append(b64_end);
}

void base128encode(string &inptext)
{
	string outtext;
	outtext = b128.encode(inptext);
	inptext = b128_start;
	inptext.append(outtext);
	inptext.append(b128_end);
}

void base256encode(string &inptext)
{
	string outtext = b256.encode(inptext);
	inptext = b256_start;
	inptext.append(outtext);
	inptext.append(b256_end);
}

void convert2crlf(string &s)
{
	size_t p = s.find('\n', 0);

	while (p != string::npos) {
		s.replace(p, 1, "\r\n");
		p = s.find('\n', p + 2);
	}
}

bool convert2lf(string &s)
{
	bool converted = false;
	size_t p = s.find("\r\n", 0);

	while (p != string::npos) {
		s.replace(p, 2, "\n");
		p = s.find("\r\n", p + 1);
		converted = true;
	}
	return converted;
}

#define LZMA_STR "\1LZMA"

void compress_maybe(string& input, bool try_compress)//bool file_transfer)
{
//	if (!progStatus.use_compression && !file_transfer) return;

	// allocate 110% of the original size for the output buffer
	size_t outlen = (size_t)ceil(input.length() * 1.1);
	unsigned char* buf = new unsigned char[outlen];

	size_t plen = LZMA_PROPS_SIZE;
	unsigned char outprops[LZMA_PROPS_SIZE];
	uint32_t origlen = htonl(input.length());

	string bufstr;

//	if (progStatus.use_compression) { // || file_transfer) {
	if (try_compress) {
// replace input with: LZMA_STR + original size (in network byte order) + props + data
		int r;
		bufstr.assign(LZMA_STR);
		if ((r = LzmaCompress(
					buf, &outlen, 
					(const unsigned char*)input.data(), input.length(),
					outprops, &plen, 9, 0, -1, -1, -1, -1, -1)) == SZ_OK) {
			bufstr.append((const char*)&origlen, sizeof(origlen));
			bufstr.append((const char*)&outprops, sizeof(outprops));
			bufstr.append((const char*)buf, outlen);
//			if (file_transfer && input.length() < bufstr.length()) {
			if (input.length() < bufstr.length()) {
				LOG_DEBUG("%s", "Lzma could not compress data");
				bufstr.assign(input);
			}
		} else {
			LOG_ERROR("Lzma Compress failed: %s", LZMA_ERRORS[r]);
			bufstr.assign(input);
		}
		if (progStatus.encoder == BASE256)
			base256encode(bufstr);
		else if (progStatus.encoder == BASE128)
			base128encode(bufstr);
		else
			base64encode(bufstr);
	} else {
		bufstr = input;
		if (binary(bufstr)) {
			if (progStatus.encoder == BASE256)
				base256encode(bufstr);
			else if (progStatus.encoder == BASE128)
				base128encode(bufstr);
			else
				base64encode(bufstr);
		}
	}

	delete [] buf;

	input = bufstr;

	return;
}

void decompress_maybe(string& input)
{
// input is LZMA_STR + original size (in network byte order) + props + data
//	if (input.find(LZMA_STR) == string::npos)
//		return;

	int decode = NONE;//BASE64;
	size_t	p0 = string::npos, 
			p1 = string::npos,
			p2 = string::npos,
			p3 = string::npos;
	if ((p0 = p1 = input.find(b64_start)) != string::npos) {
		p1 += strlen(b64_start);
		p2 = input.find(b64_end, p1);
	} else if ((p0 = p1 = input.find(b128_start)) != string::npos) {
		p1 += strlen(b128_start);
		p2 = input.find(b128_end, p1);
		decode = BASE128;
	} else if ((p0 = p1 = input.find(b256_start)) != string::npos) {
		p1 += strlen(b256_start);
		p2 = input.find(b256_end, p1);
		decode = BASE256;
	}

	if (p2 == string::npos) {
		switch (decode) {
			case BASE64 :
//				LOG_ERROR("%s", "Base 64 decode failed");
				fprintf(stderr, "Base 64 decode failed\n");
				break;
			case BASE128 :
//				LOG_ERROR("%s", "Base 128 decode failed");
				fprintf(stderr, "Base 128 decode failed\n");
				break;
			case BASE256 :
//				LOG_ERROR("%s", "Base 256 decode failed");
				fprintf(stderr, "Base 256 decode failed\n");
				break;
			case NONE :
			default : ;
		}
		return;
	}
	switch (decode) {
		case BASE128 :
			p3 = p2 + strlen(b128_end); break;
		case BASE256 :
			p3 = p2 + strlen(b256_end); break;
		case BASE64 :
		default :
			p3 = p2 + strlen(b64_end);
	}

	string cmpstr = input.substr(p1, p2-p1);

	switch (decode) {
		case BASE128 :
			cmpstr = b128.decode(cmpstr); break;
		case BASE256 :
			cmpstr = b256.decode(cmpstr); break;
		case BASE64 :
		default:
			cmpstr = b64.decode(cmpstr);
	}

	if (cmpstr.find("ERROR") != string::npos) {
//		LOG_ERROR("%s", cmpstr.c_str());
		fprintf(stderr,"%s\n", cmpstr.c_str());
		return;
	}

	if (cmpstr.find(LZMA_STR) == string::npos) {
		input.replace(p0, p3 - p0, cmpstr);
		return;
	}

	const char* in = cmpstr.data();
	size_t outlen = ntohl(*reinterpret_cast<const uint32_t*>(in + strlen(LZMA_STR)));
	if (outlen > 1 << 25) {
//		LOG_ERROR("%s", "Refusing to decompress data (> 32 MiB)");
		fprintf(stderr, "Refusing to decompress data (> 32 MiB)\n");
		return;
	}
	unsigned char* buf = new unsigned char[outlen];
	unsigned char inprops[LZMA_PROPS_SIZE];
	memcpy(inprops, in + strlen(LZMA_STR) + sizeof(uint32_t), LZMA_PROPS_SIZE);
	size_t inlen = cmpstr.length() - strlen(LZMA_STR) - sizeof(uint32_t) - LZMA_PROPS_SIZE;

	int r;
	if ((r = LzmaUncompress(buf, &outlen, (const unsigned char*)in + cmpstr.length() - inlen, &inlen,
			inprops, LZMA_PROPS_SIZE)) != SZ_OK)
//		LOG_ERROR("Lzma Uncompress failed: %s", LZMA_ERRORS[r]);
		fprintf(stderr, "Lzma Uncompress failed: %s\n", LZMA_ERRORS[r]);
	else {
//		LOG_INFO("Decompress: in = %ld, out = %ld", (long int)inlen, (long int)outlen);
		cmpstr.assign((const char*)buf, outlen);
		input.replace(p0, p3 - p0, cmpstr);
	}
	delete [] buf;
}

void connect_to_fldigi(void *)
{
	try {
		localaddr = new Address(progStatus.socket_addr.c_str(), progStatus.socket_port.c_str());
		tcpip = new Socket (*localaddr);
		tcpip->set_timeout(0.01);
		tcpip->connect();
		LOG_INFO("Connected to %d", tcpip->fd());
	}
	catch (const SocketException& e) {
		if (tcpip) { 
			tcpip->close();
			delete tcpip;
			tcpip = (Socket *)0;
		}
		if (localaddr) { delete localaddr; localaddr = (Address *)0; }
//		LOG_ERROR("%s", "Could not connect to fldigi");
	}
}

void send_via_fldigi(string tosend)
{
	if (!tcpip) return;
	try {
		tcpip->send(tosend.c_str());
	}
	catch (const SocketException& e) {
		LOG_ERROR("Socket error %d, %s", e.error(), e.what());
		if (tcpip) {
			tcpip->close();
			delete tcpip;
			tcpip = (Socket *)0;
		}
		if (localaddr) { delete localaddr; localaddr = (Address *)0; }
		Fl::add_timeout(1.0, connect_to_fldigi);
	}
	return;
}

string rx_buff;

int rx_fldigi(std::string &retbuff)
{
	try {
		rx_buff.clear();
		tcpip->set_nonblocking();
		tcpip->recv(rx_buff);
		retbuff = rx_buff;
		return rx_buff.length();
	}
	catch (const SocketException& e) {
		LOG_ERROR("%s", e.what());
	}
	return 0;
}

// allowable characters for uncompress / unencoded transmissions
// are in 0 locations.

int not_allowed[256] = {
//	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1
};

bool binary(std::string &s)
{
	for (size_t n = 0; n < s.length(); n++) {
		if (not_allowed[(s[n] & 0xFF)])
			return true;
	}
	return false;
}

