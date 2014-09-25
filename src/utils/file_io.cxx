// file_io.cxx
//
// Author(s): Dave Freese, W1HKJ (2012)
//            Robert Stiles, KK5VD (2013)
//
//
// This file is part of FLLNK.
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
#include "fllnk_config.h"
#include "fllnk.h"

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
#include "status.h"

#ifdef WIN32
#  include "fllnkrc.h"
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
bool bConnected = false;

string errtext;

string wrap_foldername = "";

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


void connect_to_fldigi(void *)
{
	pthread_mutex_lock(&mutex_file_io);
	try {
		tcpip->connect();
		file_io_errno = errno;
		bConnected = true;
		LOG_INFO("Connected to %d", tcpip->fd());
	}
	catch (const SocketException& e) {
		if(e.error() != 0) {
			bConnected = false;
			LOG_ERROR("%s %d", e.what(), file_io_errno);
		}
	}
	pthread_mutex_unlock(&mutex_file_io);
}

void send_via_fldigi(string tosend)
{
	pthread_mutex_lock(&mutex_file_io);

	if (!bConnected) {
		LOG_ERROR("%s", "Not connected to fldigi");
		pthread_mutex_unlock(&mutex_file_io);
		return;
	}
	try {
		tcpip->send(tosend.c_str());
		file_io_errno = errno;
	}
	catch (const SocketException& e) {
		if(e.error() != 0) {
			bConnected = false;
			LOG_ERROR("%s %d", e.what(), file_io_errno);
		}
	}

	pthread_mutex_unlock(&mutex_file_io);
	return;
}

string rx_buff;

int rx_fldigi(std::string &retbuff)
{
	int buff_length = 0;

	pthread_mutex_lock(&mutex_file_io);
	if (!bConnected) {
		pthread_mutex_unlock(&mutex_file_io);
		return 0;
	}
	try {
		rx_buff.clear();
		tcpip->set_nonblocking();
		tcpip->recv(rx_buff);
		file_io_errno = errno;
		retbuff = rx_buff;
		buff_length = rx_buff.length();
	}
	catch (const SocketException& e) {
		if(e.error() != 0) {
			bConnected = false;
			LOG_ERROR("%s %d", e.what(), file_io_errno);
		}
	}

	pthread_mutex_unlock(&mutex_file_io);

	return buff_length;
}

int rx_fldigi(char *buffer, int limit)
{
	int buff_length = 0;

	pthread_mutex_lock(&mutex_file_io);
	if (!bConnected) {
		pthread_mutex_unlock(&mutex_file_io);
		return 0;
	}
	try {
		tcpip->set_nonblocking();
		buff_length = tcpip->recv(buffer, (size_t) limit);
		file_io_errno = errno;
	}
	catch (const SocketException& e) {
		if(e.error() != 0) {
			bConnected = false;
			LOG_ERROR("%s %d", e.what(), file_io_errno);
		}
	}

	pthread_mutex_unlock(&mutex_file_io);
	return buff_length;
}

// allowable characters for uncompress / unencoded transmissions
// are in 0 locations.

int not_allowed[256] = {
	//	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, //  16
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //  32
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //  48
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //  64
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //  80
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //  96
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 112
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, // 128
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 144
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 160
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 176
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 192
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 208
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 224
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 240
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1  // 256
};

bool binary(std::string &s)
{
	for (size_t n = 0; n < s.length(); n++) {
		if (not_allowed[(s[n] & 0xFF)])
			return true;
	}
	return false;
}

bool c_binary(int c)
{
	if (not_allowed[c & 0xFF])
		return true;
	else
		return false;
}

bool isPlainText(std::string &_buffer)
{
	int count = 0;
	int index = 0;
	int data = 0;

	count = _buffer.size();
	for(index = 0; index < count; index++) {
		data = _buffer[index];
		if(c_binary(data) || (data & 0x80)) {
			return false;
		}
	}
	return true;
}

bool isPlainText(char *_buffer, size_t count)
{
	size_t index = 0;
	int data = 0;

	if(!_buffer || count) return false;

	for(index = 0; index < count; index++) {
		data = _buffer[index];
		if(c_binary(data) || (data & 0x80)) {
			return false;
		}
	}
	return true;
}

int convert_to_plain_text(std::string &_buffer)
{
	char *buffer = (char *)0;
	char *dest_buffer = (char *)0;
	size_t result_count = 0;
	size_t count = 0;

	buffer = (char *) _buffer.c_str();
	count = (size_t) _buffer.size();

	if(!buffer || count < 1) return 0;

	dest_buffer = (char *) malloc(count + 2);

	if(!dest_buffer) return 0;

	memset(dest_buffer, 0, count + 2);

	result_count = convert_to_plain_text(buffer, dest_buffer, count);

	if(result_count > 0) {
		_buffer.assign(dest_buffer, result_count);
	} else {
		_buffer.clear();
	}

	free(dest_buffer);

	return (int) result_count;
}

int convert_to_plain_text(char *_src, char *_dst, size_t count)
{
	size_t index = 0;
	size_t data = 0;
	//size_t change_count = 0;
	size_t dest_count = 0;
	
	char *buffer = _src;
	char *dest = (char *)0;
	char *cPtr = (char *)0;
	
	if(_src == (char *)0 || _dst == (char *)0) return 0;
	
	if(count < 1) return 0;
	
	dest = (char *) malloc(count + 2);
	
	if(dest == (char *)0) return 0;
	
	memset(dest, 0, count + 2);
	
	cPtr = dest;
	for(index = 0; index < count; index++) {
		data = buffer[index];
		if(c_binary(data) || (data & 0x80)) {
			continue;
		}
		*cPtr++ = data;
		dest_count++;
	}
	
	if(dest_count > 0) {
		memcpy(_dst, dest, dest_count);
	}
	
	free(dest);
	
	return (int) dest_count;
}

