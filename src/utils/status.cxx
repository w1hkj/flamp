// =====================================================================
//
// status.cxx
//
// Author(s):
//	Dave Freese, W1HKJ Copyright (C) 2010
//  Robert Stiles, KK5VD Copyright (C) 2013
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
// =====================================================================


#include <iostream>
#include <fstream>
#include <string>

#include <FL/Fl_Preferences.H>

#include "status.h"
#include "config.h"
#include "fllnk.h"
#include "fllnk_dialog.h"
#include "file_io.h"

status progStatus = {
	50,				// int mainX;
	50,				// int mainY;

	"127.0.0.1",	// fldigi socket address
	"7322",			// fldigi socket port
	"127.0.0.1",	// fldigi xmlrpc socket address
	"7362",			// fldigi xmlrpc socket port

	// User Assigned addr/ports not saved.
	"",				// User assigned fldigi socket address
	"",				// User assigned fldigi socket port
	"",				// User assigned fldigi xmlrpc socket address
	"",				// User assigned fldigi xmlrpc socket port

};

void status::saveLastState()
{
	Fl_Preferences FLLNKpref(fllnkHomeDir.c_str(), "w1hkj.com",  PACKAGE_NAME);

	int mX = main_window->x();
	int mY = main_window->y();
	if (mX >= 0 && mX >= 0) {
		mainX = mX;
		mainY = mY;
	}

	FLLNKpref.set("version", PACKAGE_VERSION);
	FLLNKpref.set("mainx", mX);
	FLLNKpref.set("mainy", mY);

	FLLNKpref.set("socket_address", socket_addr.c_str());
	FLLNKpref.set("socket_port", socket_port.c_str());
	FLLNKpref.set("xmlrpc_address", xmlrpc_addr.c_str());
	FLLNKpref.set("xmlrpc_port", xmlrpc_port.c_str());
}

void status::loadLastState()
{
	Fl_Preferences FLLNKpref(fllnkHomeDir.c_str(), "w1hkj.com", PACKAGE_NAME);

	if (FLLNKpref.entryExists("version")) {
		char *defbuffer;

		FLLNKpref.get("mainx", mainX, mainX);
		FLLNKpref.get("mainy", mainY, mainY);

		FLLNKpref.get("socket_address", defbuffer, socket_addr.c_str());
		socket_addr = defbuffer; free(defbuffer);
		FLLNKpref.get("socket_port", defbuffer, socket_port.c_str());
		socket_port = defbuffer; free(defbuffer);

		FLLNKpref.get("xmlrpc_address", defbuffer, xmlrpc_addr.c_str());
		xmlrpc_addr = defbuffer; free(defbuffer);
		FLLNKpref.get("xmlrpc_port", defbuffer, xmlrpc_port.c_str());
		xmlrpc_port = defbuffer; free(defbuffer);
	}
}
