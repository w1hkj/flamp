// file_io.cxx
//
// Author(s): Dave Freese, W1HKJ (2012)
//            Robert Stiles, KK5VD (2013, 2014, 2015)
//
//
// This file is part of FM_MSNGR.
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
#include "fm_msngr.h"
#include "fm_msngr_src/fm_control.h"
#include "fm_msngr_src/fm_dialog.h"

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
#include "fm_msngr_src/fm_dialog.h"
#include "fm_msngr_src/fm_control.h"

#include "status.h"

#ifdef WIN32
#  include "fm_msngrrc.h"
#  include "compat.h"
#  define dirent fl_dirent_no_thanks
#endif

#include <FL/filename.H>
#include "dirent-check.h"

#include <FL/x.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Image.H>

using namespace std;

Socket *tcpip      = (Socket *)0;
Address *localaddr = (Address *)0;
bool bConnected    = false;
int file_io_errno  = 0;

pthread_mutex_t mutex_file_io = PTHREAD_MUTEX_INITIALIZER;

string errtext;

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


#if 0  // Unused function(s)
/** ********************************************************
 *
 ***********************************************************/
static void convert2crlf(string &s)
{
	size_t p = s.find('\n', 0);

	while (p != string::npos) {
		s.replace(p, 1, "\r\n");
		p = s.find('\n', p + 2);
	}
}

/** ********************************************************
 *
 ***********************************************************/
static bool convert2lf(string &s)
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
#endif


/** ********************************************************
 *
 ***********************************************************/
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

/** ********************************************************
 *
 ***********************************************************/
void send_via_fldigi(std::string tosend)
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

std::string rx_buff;

/** ********************************************************
 *
 ***********************************************************/
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

/** ********************************************************
 *
 ***********************************************************/
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

/** ********************************************************
 *
 ***********************************************************/
bool binary(std::string &s)
{
	for (size_t n = 0; n < s.length(); n++) {
		if (not_allowed[(s[n] & 0xFF)])
			return true;
	}
	return false;
}

/** ********************************************************
 *
 ***********************************************************/
bool c_binary(int c)
{
	if (not_allowed[c & 0xFF])
		return true;
	else
		return false;
}

/** ********************************************************
 *
 ***********************************************************/
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

/** ********************************************************
 *
 ***********************************************************/
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


