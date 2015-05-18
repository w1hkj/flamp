/** ********************************************************
 *
 ***********************************************************/

#include "config.h"

static const char *copyright[] = {
	" =====================================================================",
	"",
	" FM_MSNGR " VERSION, // fm_msngr.cxx
	"",
	" Author(s):",
	"    Robert Stiles, KK5VD, Copyright (C) 2015",
	"",
	" This program is free software: you can redistribute it and/or modify",
	" it under the terms of the GNU General Public License as published by",
	" the Free Software Foundation, either version 3 of the License, or",
	" (at your option) any later version.",
	"",
	" This program is distributed in the hope that it will be useful,",
	" but WITHOUT ANY WARRANTY; without even the implied warranty of",
	" MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the",
	" GNU General Public License for more details.",
	"",
	" You should have received a copy of the GNU General Public License",
	" along with this program.  If not, see <http://www.gnu.org/licenses/>.",
	" =====================================================================",
	0
};

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <ctime>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>
#include <libgen.h>
#include <ctype.h>
#include <sys/time.h>
#include <string>

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

#include "fm_msngr.h"
#include "fm_msngr_src/fm_dialog.h"
#include "fm_msngr_src/fm_control.h"
#include "fm_msngr_src/fm_port_io.h"

#include "debug.h"
#include "util.h"
#include "gettext.h"

#include "icons.h"
#include "fileselect.h"

#include "status.h"
#include "pixmaps.h"
#include "threads.h"
#include "xml_io.h"
#include "file_io.h"


#ifdef WIN32
#  include "fm_msngr_rc.h"
#  include "compat.h"
#endif

#include <FL/filename.H>

#include <FL/x.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Image.H>

using namespace std;


//! @brief Command line help string initialization
const char *options[] = {
	"fm_msgnr Unique Options",
	"",
	"  --help",
	"  --version",
	"  --confg-dir folder-path-name (including drive letter on Windows)",
	"      Windows: C:/Documents and Settings/<username>/folder-name",
	"               C:/Users/<username/folder-name",
	"               H:/hamstuff/folder-name",
	"      Linux:   /home/<username>/folder-name",
	"      OS X:    /Users/<username>/folder-name",
	"",
	"  Note: Enclosing \"'s must be used when the path or file name",
	"        contains spaces.",
	"",
	"  Unless the empty file NBEMS.DIR is found in the same folder as the",
	"  fm_msngr executable.  The existence of an empty file NBEMS.DIR forces",
	"  the confg-dir to be a placed in the same folder as the executable",
	"  folder and named fm_msngr.files (.fm_msngr on Linux / OS X)",
	"",
	"  NBEMS.DIR may contain a single line specifying the absolute",
	"  path-name of the fm_msngr-dir.  If NBEMS.DIR is not empty then",
	"  the specified path-name takes precedence over the --confg-dir",
	"  command line parameter.  The specified path-name must be a valid",
	"  one for the operating system!  Enclosing \"'s are not required.",
	"",
	"=======================================================================",
	"",
	" Interprocess Network/Port Commands",
	"",
	"  --arq-server-address HOSTNAME",
	"      Set the ARQ TCP server address",
	"      The default is: 127.0.0.1",
	"",
	"  --arq-server-port PORT",
	"      Set the ARQ TCP server port",
	"      The default is: 7322",
	"",
	"  --xmlrpc-server-address HOSTNAME",
	"      Set the XML-RPC server address",
	"      The default is: 127.0.0.1",
	"",
	"  --xmlrpc-server-port PORT",
	"      Set the XML-RPC server port",
	"      The default is: 7362",
	"",
	"=======================================================================",
	"",
	"Fltk User Interface options",
	"",
	"  -bg\t-background [COLOR]",
	"  -bg2\t-background2 [COLOR]",
	"  -di\t-display [host:n.n]",
	"  -dn\t-dnd : enable drag and drop",
	"  -nodn\t-nodnd : disable drag and drop",
	"  -fg\t-foreground [COLOR]",
	"  -g\t-geometry [WxH+X+Y]",
	"  -i\t-iconic",
	"  -k\t-kbd : enable keyboard focus:",
	"  -nok\t-nokbd : en/disable keyboard focus",
	"  -na\t-name [CLASSNAME]",
	"  -s\t-scheme [none | gtk+ | plastic]",
	"     default = gtk+",
	"  -ti\t-title [WINDOWTITLE]",
	"  -to\t-tooltips : enable tooltips",
	"  -not\t-notooltips : disable tooltips\n",
	0
};

//! @brief Thread management global variables.


Fl_Window * main_window = (Fl_Double_Window *)0;

std::string NBEMS_dir = "";
std::string title = "";
std::string BaseDir = "";
std::string HomeDir = "";
std::string buffer = "";

void * create_tx_table(void *);
double measure_tx_time(char character_to_send, unsigned int no_of_characters, double time_out_duration);
char *copyTo(char *src, char *dest, int *limit, int stop_character);
double time_f(void);


/** ********************************************************
 *
 ***********************************************************/
bool isbinary(string s)
{
	for (size_t n = 0; n < s.length(); n++)
		if ((s[n] & 0x80) == 0x80) return true;
	return false;
}


/** ********************************************************
 *
 ***********************************************************/
void checkdirectories(void)
{
	struct DIRS {
		string& dir;
		const char* suffix;
		void (*new_dir_func)(void);
	};

	DIRS NBEMS_dirs[] = {
		{ NBEMS_dir,      0,        0 },
		{ HomeDir,     "FM_MSNGR",	0 },
	};

	int r;

	for (size_t i = 0; i < sizeof(NBEMS_dirs)/sizeof(*NBEMS_dirs); i++) {
		if (NBEMS_dirs[i].suffix)
			NBEMS_dirs[i].dir.assign(NBEMS_dir).append(NBEMS_dirs[i].suffix).append("/");

		if ((r = mkdir(NBEMS_dirs[i].dir.c_str(), 0777)) == -1 && errno != EEXIST) {
			cerr << _("Could not make directory") << ' ' << NBEMS_dirs[i].dir
			     << ": " << strerror(errno) << '\n';
			exit(EXIT_FAILURE);
		}
		else if (r == 0 && NBEMS_dirs[i].new_dir_func)
			NBEMS_dirs[i].new_dir_func();
	}
}

/** ********************************************************
 *
 ***********************************************************/
char *copyTo(char *src, char *dest, int *limit, int stop_character)
{
	int count = 0;
	int index = 0;
	int noOfCharsCopied = 0;
	char *cPtr = (char *)0;
	char *dPtr = (char *)0;

	if(!src || !dest || !limit) return (char *)0;

	cPtr = src;
	dPtr = dest;
	count = *limit;

	for(index = 0; index < count; index++) {
		if(*cPtr == '\0') break;
		if(*cPtr == '\r') break;
		if(*cPtr == '\n') break;
		if(*cPtr == stop_character) break;
		*dPtr++ = *cPtr++;
		noOfCharsCopied++;
	}

	if(noOfCharsCopied) {
		if(count >= noOfCharsCopied)
			count -= noOfCharsCopied;
		else
			count = 0;
	}

	if(count > 0)
		cPtr = &src[noOfCharsCopied + 1];
	else
		cPtr = (char *)0;

	dest[noOfCharsCopied] = 0;

	*limit = count;
	return cPtr;
}


/** ********************************************************
 *
 ***********************************************************/
double time_f(void)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return ((double) now.tv_sec) + (((double)now.tv_usec) * 0.000001);
}

/** ********************************************************
 *
 ***********************************************************/
void cb_exit(void)
{
	if(transmitting) {
		transmit_stop = true;
	}

	progStatus.saveLastState();

	FSEL::destroy();

	debug::stop();
}


/** ********************************************************
 *
 ***********************************************************/
int parse_args(int argc, char **argv, int& idx)
{
	char check_for[48];


	if (strstr(argv[idx], "--config-dir")) {
		std::string temp_dir = "";
		idx++;
		string tmp = argv[idx];
		if (!tmp.empty()) temp_dir = tmp;
		size_t p = string::npos;
		while ( (p = temp_dir.find("\\")) != string::npos)
			temp_dir[p] = '/';
		if (temp_dir[temp_dir.length()-1] != '/')
			temp_dir += '/';
		idx++;
		NBEMS_dir.assign(temp_dir);
		return 1;
	}

	memset(check_for, 0, sizeof(check_for));
	strncpy(check_for, "--arq-server-address", sizeof(check_for) - 1);
	if (strstr(argv[idx], check_for)) {
		idx++;
		if(argv[idx]) {
			if(numbers_and_dots_only(argv[idx], 4)) {
				progStatus.user_socket_addr.assign(argv[idx]);
				LOG_DEBUG("%s %s\n", check_for, argv[idx]);
				idx++;
			} else {
				progStatus.user_socket_addr.clear();
				LOG_INFO("%s Invalid parameter %s\n", check_for, argv[idx]);
			}
		}

		return 1;
	}

	memset(check_for, 0, sizeof(check_for));
	strncpy(check_for, "--arq-server-port", sizeof(check_for) - 1);
	if (strstr(argv[idx], check_for)) {
		idx++;
		if(argv[idx]) {
			if(numbers_and_dots_only(argv[idx], 1)) {
				progStatus.user_socket_port.assign(argv[idx]);
				LOG_DEBUG("%s %s\n", check_for, argv[idx]);
				idx++;
			} else {
				progStatus.user_socket_port.clear();
				LOG_INFO("%s Invalid parameter %s\n", check_for, argv[idx]);
			}
		}

		return 1;
	}


	memset(check_for, 0, sizeof(check_for));
	strncpy(check_for, "--xmlrpc-server-address", sizeof(check_for) - 1);
	if (strstr(argv[idx], check_for)) {
		idx++;
		if(argv[idx]) {
			if(numbers_and_dots_only(argv[idx], 4)) {
				progStatus.user_xmlrpc_addr.assign(argv[idx]);
				LOG_DEBUG("%s %s\n", check_for, argv[idx]);
				idx++;
			} else {
				progStatus.user_xmlrpc_addr.clear();
				LOG_INFO("%s Invalid parameter %s\n", check_for, argv[idx]);
			}
		}

		return 1;
	}

	memset(check_for, 0, sizeof(check_for));
	strncpy(check_for, "--xmlrpc-server-port", sizeof(check_for) - 1);
	if (strstr(argv[idx], check_for)) {
		idx++;
		if(argv[idx]) {
			if(numbers_and_dots_only(argv[idx], 1)) {
				progStatus.user_xmlrpc_port.assign(argv[idx]);
				LOG_DEBUG("%s %s\n", check_for, argv[idx]);
				idx++;
			} else {
				progStatus.user_xmlrpc_port.clear();
				LOG_INFO("%s Invalid parameter %s\n", check_for, argv[idx]);
			}
		}

		return 1;
	}

	if (strcasecmp(argv[idx], "--help") == 0) {
		int i = 0;
		while (copyright[i] != NULL) {
			printf("%s\n", copyright[i]);
			i++;
		}

		printf("\n\n");
		i = 0;
		while (options[i] != NULL) {
			printf("%s\n", options[i]);
			i++;
		}
		exit (0);
	}


	if (strcasecmp(argv[idx], "--version") == 0) {
		printf("Version: " VERSION "\n");
		exit (0);
	}
	return 0;
}

/** ********************************************************
 *
 ***********************************************************/
void exit_main(Fl_Widget *w)
{
	if (Fl::event_key() == FL_Escape)
		return;
	cb_exit();
}


/** ********************************************************
 *
 ***********************************************************/
void check_io_mode(void *v)
{
	// Check to see what io mode FLDIGI is in.
	std::string io_mode = get_io_mode();
	int flag = 0;
	if(!io_mode.empty()) {
		flag = strncmp(io_mode.c_str(), "ARQ", 3);
		if(flag != 0) {
			flag = fl_choice2(_("KISS interface active! Switch FLDIGI to ARQ?"),
							  _("No"), _("Yes"), NULL);
			if(flag == 1)
				enable_arq();
		}
	}
}

/** ********************************************************
 *
 ***********************************************************/
int main(int argc, char *argv[])
{
	NBEMS_dir.clear();
	{
		string appname = argv[0];
		string appdir;
		char dirbuf[FL_PATH_MAX + 1];
		fl_filename_expand(dirbuf, FL_PATH_MAX, appname.c_str());
		appdir.assign(dirbuf);

#ifdef __WOE32__
		size_t p = appdir.rfind("fm_msngr.exe");
		appdir.erase(p);
		p = appdir.find("FL_APPS/");
		if (p != string::npos) {
			NBEMS_dir.assign(appdir.substr(0, p + 8));
		} else {
			fl_filename_expand(dirbuf, sizeof(dirbuf) -1, "$USERPROFILE/");
			NBEMS_dir.assign(dirbuf);
		}
		NBEMS_dir.append("NBEMS.files/");

#else

		fl_filename_absolute(dirbuf, sizeof(dirbuf), argv[0]);
		appdir.assign(dirbuf);
		size_t p = appdir.rfind("fm_msngr");
		if (p != string::npos)
			appdir.erase(p);
		p = appdir.find("FL_APPS/");
		if (p != string::npos)
			NBEMS_dir.assign(appdir.substr(0, p + 8));
		else {
			fl_filename_expand(dirbuf, FL_PATH_MAX, "$HOME/");
			NBEMS_dir = dirbuf;
		}

		DIR *isdir = 0;
		string test_dir;
		test_dir.assign(NBEMS_dir).append("NBEMS.files/");
		isdir = opendir(test_dir.c_str());
		if (isdir) {
			NBEMS_dir = test_dir;
			closedir(isdir);
		} else {
			NBEMS_dir.append(".nbems/");
		}

#endif
	}

	int arg_idx;
	if (Fl::args(argc, argv, arg_idx, parse_args) != argc) {
		return 0;
	}

	Fl::lock();
	Fl::scheme("gtk+");

	checkdirectories();
	progStatus.loadLastState();

	string debug_file = HomeDir;
	debug_file.append("debug_log.txt");
	debug::start(debug_file.c_str());

	LOG_INFO("Base dir: %s", NBEMS_dir.c_str());

	main_window = open_frame_window(argc, argv);

	start_io();

	Fl::add_timeout(0.5, check_io_mode);

	return Fl::run();
}

