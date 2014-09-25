/** ********************************************************
 *
 ***********************************************************/

#include "config.h"

const char *copyright[] = {
	" =====================================================================",
	"",
	" FLLNK "  VERSION, // fllnk.cxx
	"",
	" Author(s):",
	"    Robert Stiles, KK5VD, Copyright (C) 2014",
	"    Dave Freese, W1HKJ, Copyright (C) 2012, 2013",
	"",
	" This is free software; you can redistribute it and/or modify",
	" it under the terms of the GNU General Public License as published by",
	" the Free Software Foundation; either version 3 of the License, or",
	" (at your option) any later version.",
	"",
	" This software is distributed in the hope that it will be useful,",
	" but WITHOUT ANY WARRANTY; without even the implied warranty of",
	" MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the",
	" GNU General Public License for more details.",
	"",
	" You should have received a copy of the GNU General Public License",
	" along with this program.  If not, see <http://www.gnu.org/licenses/>.",
	"",
	" =====================================================================",
	0
};

#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <ctime>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <libgen.h>
#include <ctype.h>
#include <sys/time.h>

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

#include "fllnk.h"
#include "fllnk_dialog.h"

#include "debug.h"
#include "util.h"
#include "gettext.h"
#include "flinput2.h"
#include "date.h"
#include "calendar.h"
#include "icons.h"
#include "fileselect.h"
#include "file_io.h"
#include "status.h"
#include "pixmaps.h"
#include "threads.h"
#include "xml_io.h"
#include "ztimer.h"

#ifdef WIN32
#  include "fllnkrc.h"
#  include "compat.h"
#endif

#include <FL/filename.H>

#include <FL/x.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Image.H>

using namespace std;



//! @brief Command line help string initialization
const char *options[] = {
	"Fllnk Unique Options",
	"",
	"  --help",
	"  --version",
	"    Used to generate timing tables for the various modes. Disabling ",
	"    requires program restart.",
	"  --fllnk-dir folder-path-name (including drive letter on Windows)",
	"      Windows: C:/Documents and Settings/<username>/folder-name",
	"               C:/Users/<username/folder-name",
	"               H:/hamstuff/folder-name",
	"      Linux:   /home/<username>/folder-name",
	"      OS X:    /home/<username>/folder-name",
	"",
	"  Note: Enclosing \"'s must be used when the path or file name",
	"        contains spaces.",
	"",
	"  Unless the empty file NBEMS.DIR is found in the same folder as the",
	"  flamp executable.  The existence of an empty file NBEMS.DIR forces",
	"  the flamp-dir to be a placed in the same folder as the executable",
	"  folder and named flamp.files (.flamp on Linux / OS X)",
	"",
	"  NBEMS.DIR may contain a single line specifying the absolute",
	"  path-name of the flamp-dir.  If NBEMS.DIR is not empty then",
	"  the specified path-name takes precedence over the --flamp-dir",
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
pthread_t *xmlrpc_thread = 0;
pthread_mutex_t mutex_xmlrpc  = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_file_io = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_tx_data = PTHREAD_MUTEX_INITIALIZER;


int xmlrpc_errno = 0;
int file_io_errno = 0;
bool active_data_io = false;

string title = "";
string BaseDir = "";
string fllnkHomeDir = "";
string fllnk_config_dir = "";
string fllnk_aux_dir = "";
string buffer = "";

string cmd_fname = "";

string xmt_fname = "";
string rx_fname = "";

double time_f(void);
char *copyTo(char *src, char *dest, int *limit, int stop_character);



bool rx_complete = false;
bool transmitting = false;
bool transmit_stop = false;

std::string g_modem;
// utility functions
/** ********************************************************
 *
 ***********************************************************/
bool isbinary(string s)
{
	for (size_t n = 0; n < s.length(); n++)
		if ((s[n] & 0x80) == 0x80) return true;
	return false;
}

#if !defined(__APPLE__) && !defined(__WOE32__) && USE_X
Pixmap  fllnk_icon_pixmap;

#define KNAME "fllnk"

/** ********************************************************
 *
 ***********************************************************/
void make_pixmap(Pixmap *xpm, const char **data)
{
	Fl_Window w(0,0, KNAME);
	w.xclass(KNAME);
	w.show();
	w.make_current();
	Fl_Pixmap icon(data);
	int maxd = (icon.w() > icon.h()) ? icon.w() : icon.h();
	*xpm = fl_create_offscreen(maxd, maxd);
	fl_begin_offscreen(*xpm);
	fl_color(FL_BACKGROUND_COLOR);
	fl_rectf(0, 0, maxd, maxd);
	icon.draw(maxd - icon.w(), maxd - icon.h());
	fl_end_offscreen();
}

#endif

#if FLAMP_FLTK_API_MAJOR == 1 && FLAMP_FLTK_API_MINOR == 3
/** ********************************************************
 *
 ***********************************************************/
int default_handler(int event)
{
	if (event != FL_SHORTCUT)
		return 0;

	else if (Fl::event_ctrl())  {
		Fl_Widget* w = Fl::focus();
		return w->handle(FL_KEYBOARD);
	}

	return 0;
}

#endif

/** ********************************************************
 *
 ***********************************************************/
void checkdirectories(void)
{
	char dirbuf[FL_PATH_MAX + 1];
#ifdef __WOE32__
	if (BaseDir.empty()) {
		fl_filename_expand(dirbuf, sizeof(dirbuf) -1, "$USERPROFILE/");
		BaseDir = dirbuf;
	}
	if (fllnkHomeDir.empty()) fllnkHomeDir.assign(BaseDir).append("fllnk.files/");
#else
	if (BaseDir.empty()) {
		fl_filename_expand(dirbuf, sizeof(dirbuf) -1, "$HOME/");
		BaseDir = dirbuf;
	}
	if (fllnkHomeDir.empty()) fllnkHomeDir.assign(BaseDir).append(".fllnk/");
#endif

	struct DIRS {
		string& dir;
		const char* suffix;
		void (*new_dir_func)(void);
	};
	DIRS fllnk_dirs[] = {
		{ fllnkHomeDir,  0,	0 },
		{ fllnk_config_dir, "config", 0 },
		{ fllnk_aux_dir, "aux", 0 },
	};

	int r;

	for (size_t i = 0; i < sizeof(fllnk_dirs)/sizeof(*fllnk_dirs); i++) {
		if (fllnk_dirs[i].dir.empty() && fllnk_dirs[i].suffix)
			fllnk_dirs[i].dir.assign(fllnkHomeDir).append(fllnk_dirs[i].suffix).append(PATH_SEP);

		if ((r = mkdir(fllnk_dirs[i].dir.c_str(), 0777)) == -1 && errno != EEXIST) {
			cerr << _("Could not make directory") << ' ' << fllnk_dirs[i].dir
			<< ": " << strerror(errno) << '\n';
			exit(EXIT_FAILURE);
		}
		else if (r == 0 && fllnk_dirs[i].new_dir_func)
			fllnk_dirs[i].new_dir_func();
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
void addfile(std::string, void *, bool, char *)
{

}

/** ********************************************************
 *
 ***********************************************************/
void select_file(std::string &selected_file_name,
	std::string msg = _("Open file"),
	std::string filetype = "any file\t*.*")
{
	char *fn_buffer = (char *)0;

	fn_buffer = new char[FILENAME_MAX + 1];
	if(!fn_buffer) {
		selected_file_name.clear();
		return;
	}

	memset(fn_buffer, 0, FILENAME_MAX + 1);

	const char *p = FSEL::select(msg.c_str(), filetype.c_str(),
								 fn_buffer);
	if (!p) return;
	if (strlen(p) == 0) return;

	selected_file_name.assign(p);

	delete [] fn_buffer;
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
bool wait_for_rx(int max_wait_seconds)
{
	static std::string response;
	time_t eTime = time((time_t *)0) + 2;
	time_t sTime = 0;
	int sleep_time = 200;

	if(max_wait_seconds < 1) return false;

	do {
		response = get_trx_state();

		if(response == "TX") break;

		if(transmit_stop) return false;

		MilliSleep(sleep_time);

	} while (time((time_t *)0) < eTime);

	sTime = time((time_t *)0);
	eTime = sTime + max_wait_seconds;

	do {
		response = get_trx_state();

		if(response == "RX") {
			return true;
		}

		if(transmit_stop) break;

		MilliSleep(sleep_time);

	} while (time((time_t *)0) < eTime);

	return false;
}

/** ********************************************************
 *
 ***********************************************************/
void wait_seconds(int seconds)
{
	time_t eTime = time((time_t *)0) + seconds + 1;

	if(seconds < 1) return;

	while (time((time_t *)0) < eTime) {
		MilliSleep(500);
	}
}

/** ********************************************************
 *
 ***********************************************************/
TX_FLDIGI_THREAD * run_in_thread(void *(*func)(void *), int mode)
{
	TX_FLDIGI_THREAD *tx_thread = (TX_FLDIGI_THREAD *)0;
	int count = 0;

	if(transmitting) return tx_thread;

	tx_thread = new TX_FLDIGI_THREAD;

	if(tx_thread) {

		tx_thread->err_flag = false;

		if(!pthread_mutex_init(&tx_thread->mutex, NULL)) {
			count++;
			if(!pthread_cond_init(&tx_thread->condition, NULL)) {
				count++;
				if(!pthread_attr_init(&tx_thread->attr)) {
					count++;
					if(!pthread_attr_setdetachstate(&tx_thread->attr, PTHREAD_CREATE_DETACHED)) {
						count++;
						if(!pthread_create(&tx_thread->thread, &tx_thread->attr, func, (void *) tx_thread)) {
							return tx_thread;
						}
					}
				}
			}
		}

		run_in_thread_destroy(tx_thread, count);
		LOG_ERROR("%s", "Thread creation error: run_in_thread()");
	}

	return (TX_FLDIGI_THREAD *) 0;
}

/** ********************************************************
 *
 ***********************************************************/
void * run_in_thread_destroy(TX_FLDIGI_THREAD *tx_thread, int level)
{
	if(!tx_thread) return 0;

	pthread_mutex_lock(&tx_thread->mutex);
	tx_thread->thread_running = 0;
	pthread_mutex_unlock(&tx_thread->mutex);

	if(level > 3 || level < 1) level = 3;

	switch (level) {
		case 3: pthread_attr_destroy(&tx_thread->attr);
		case 2:	pthread_cond_destroy(&tx_thread->condition);
		case 1: pthread_mutex_destroy(&tx_thread->mutex);
	}

	if(tx_thread->err_flag) {
		char *msg = (char *) malloc(THREAD_ERR_MSG_SIZE);

		if(msg) {
			memset(msg, 0, THREAD_ERR_MSG_SIZE);
			strncpy(msg, tx_thread->err_msg, THREAD_ERR_MSG_SIZE - 1);
			Fl::awake(thread_error_msg, (void *)msg);
		}
	}

	delete tx_thread;

	transmitting = false;

	return 0;
}

/** ********************************************************
 *
 ***********************************************************/
void thread_error_msg(void *data)
{
	if(data) {
		fl_alert2("%s", (const char *)data);
		free(data);
	}
}

/** ********************************************************
 *
 ***********************************************************/
void abort_and_id(void)
{
	std::string idMessage;
	send_abort();
	send_abort();

	// A number of non printable characters are required to overcome long interleave modems.
	idMessage.assign("\n\n\n\n\n\n\n\n\n\n\nFILE TRANSFER ABORTED\n\nDE ").append(progStatus.my_call).append(" BK\n\n\n");
	send_via_fldigi(idMessage);
}

/** ********************************************************
 *
 ***********************************************************/
void abort_request(void)
{
	int response = fl_choice("Terminate Current Transmission?", "No", "Yes", NULL);
	if (response == 1) {
		transmit_stop = true;
		ztime_end = 0;
		abort_and_id();
	}
}

/** ********************************************************
 *
 ***********************************************************/
void get_trx_state_in_main_thread(void *ptr)
{
	if (!ptr) return;

	std::string *str = (std::string *)ptr;

	*str = get_trx_state();
}

/** ********************************************************
 *
 ***********************************************************/
void send_via_fldigi_in_main_thread(void *ptr)
{
	if (!ptr) return;

	string *data = (string *)ptr;

	if (!bConnected) connect_to_fldigi(0);
	if (!bConnected) return;

	send_via_fldigi(*data);
}

/** ********************************************************
 *
 ***********************************************************/
void set_button_label(void *data)
{
	BUTTON_LABEL *bl = (BUTTON_LABEL *)data;
	Fl_Button *button;
	std::string label;

	if(!data) return;

	bl = (BUTTON_LABEL *) data;
	button = bl->button;
	label.assign(bl->label);

	if(button && label.size()) {
		button->label(label.c_str());
	}
}

/** ********************************************************
 *
 ***********************************************************/
void cb_exit()
{
	if(transmitting) {
		transmit_stop = true;
	}

	exit_watch_dog = true;

	progStatus.saveLastState();
	FSEL::destroy();

	if (tcpip) {
		tcpip->close();
		delete tcpip;
		delete localaddr;
	}
	debug::stop();

	exit(0);
}

/** ********************************************************
 *
 ***********************************************************/
bool numbers_and_dots_only(char *str, int expected_argc)
{
	int f = 0, s = 0, t = 0, fo = 0;
	int argc = 0;

	if(str == (char *)0) return false;
	if(*str == (char) 0) return false;

	argc = sscanf((const char *)str, "%d.%d.%d.%d", &f, &s, &t, &fo);

	if(argc == expected_argc) return true;

	return false;
}

/** ********************************************************
 *
 ***********************************************************/
int parse_args(int argc, char **argv, int& idx)
{
	char check_for[48];


	if (strstr(argv[idx], "--flamp-dir")) {
		idx++;
		// ignore if already set via NBEMS.DIR file contents
		if (!fllnkHomeDir.empty()) return 1;
		string tmp = argv[idx];
		if (!tmp.empty()) fllnkHomeDir = tmp;
		size_t p = string::npos;
		while ( (p = fllnkHomeDir.find("\\")) != string::npos)
			fllnkHomeDir[p] = '/';
		if (fllnkHomeDir[fllnkHomeDir.length()-1] != '/')
			fllnkHomeDir += '/';
		idx++;
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
		printf("Version: "VERSION"\n");
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
int main(int argc, char *argv[])
{
	string appname = argv[0];
	{
		string appdir;
		char apptemp[FL_PATH_MAX];
		fl_filename_expand(apptemp, sizeof(apptemp), appname.c_str());
		appdir.assign(apptemp);

#ifdef __WOE32__
		size_t p = appdir.rfind("fllnk.exe");
		appdir.erase(p);
#else
		size_t p = appdir.rfind("fllnk");
		if (appdir.find("./fllnk") != std::string::npos) {
			if (getcwd(apptemp, sizeof(apptemp)))
				appdir.assign(apptemp).append("/");
		} else
			appdir.erase(p);
#endif

		if (p != std::string::npos) {
			string testfile;
			testfile.assign(appdir).append("NBEMS.DIR");
			FILE *testdir = fopen(testfile.c_str(),"r");
			if (testdir) {
				string dirline = "";
				char ch = fgetc(testdir);
				while (!feof(testdir)) {
					dirline += ch;
					ch = fgetc(testdir);
				}
				fclose(testdir);
				// strip leading & trailing white space
				while (dirline.length() && dirline[0] <= ' ')
					dirline.erase(0,1);
				while (dirline.length() && dirline[dirline.length()-1] <= ' ')
					dirline.erase(dirline.length()-1, 1);
				if (dirline.empty())
					BaseDir = appdir;
				else {
					size_t p = 0;
					while ( (p = dirline.find("\\")) != string::npos)
						dirline[p] = '/';
					if (dirline[dirline.length()-1] != '/')
						dirline += '/';
					fllnkHomeDir = dirline;
				}
			}
		}
	}

	int arg_idx;
	if (Fl::args(argc, argv, arg_idx, parse_args) != argc) {
		return 0;
	}

	Fl::lock();
	Fl::scheme("gtk+");

	checkdirectories();
	progStatus.loadLastState();

	string debug_file = fllnkHomeDir;
	debug_file.append("debug_log.txt");
	debug::start(debug_file.c_str());

	LOG_INFO("Application: %s", appname.c_str());
	LOG_INFO("Base dir: %s", BaseDir.c_str());

	main_window = fllnk_dialog();
	main_window->resize( progStatus.mainX, progStatus.mainY, main_window->w(), main_window->h());
	main_window->callback(exit_main);
	
#if FLAMP_FLTK_API_MAJOR == 1 && FLAMP_FLTK_API_MINOR == 3
	Fl::add_handler(default_handler);
#endif
	
	Fl_File_Icon::load_system_icons();
	FSEL::create();
	
#if defined(__WOE32__)
#  ifndef IDI_ICON
#	define IDI_ICON 101
#  endif
	main_window->icon((char*)LoadIcon(fl_display, MAKEINTRESOURCE(IDI_ICON)));
	main_window->show (argc, argv);
#elif !defined(__APPLE__)
	make_pixmap(&flamp_icon_pixmap, flamp_icon);
	main_window->icon((char *)flamp_icon_pixmap);
	main_window->show(argc, argv);
#else
	main_window->show(argc, argv);
#endif
	
	if (string(main_window->label()) == "") {
		string main_label = PACKAGE_NAME;
		main_label.append(": ").append(PACKAGE_VERSION);
		main_window->label(main_label.c_str());
	}
	
	string addr;
	string port;
	
	if(progStatus.user_socket_addr.size())
		addr.assign(progStatus.user_socket_addr);
	else
		addr.assign(progStatus.socket_addr);
	
	if(progStatus.user_socket_port.size())
		port.assign(progStatus.user_socket_port);
	else
		port.assign(progStatus.socket_port);
	
	localaddr = new Address(addr.c_str(), port.c_str());
	if (!localaddr) exit(EXIT_FAILURE);
	tcpip = new Socket (*localaddr);
	tcpip->set_timeout(0.01);
	connect_to_fldigi(0);
	
	open_xmlrpc();
	xmlrpc_thread = new pthread_t;
	if (pthread_create(xmlrpc_thread, NULL, xmlrpc_loop, NULL)) {
		perror("pthread_create: xmlrpc");
		exit(EXIT_FAILURE);
	}
	

	watch_dog_seconds = time(0);
	watch_dog_thread = new pthread_t;
	if (pthread_create(watch_dog_thread, NULL, watch_dog_loop, NULL)) {
		perror("pthread_create: ztimer watch dog not started");
	}
	
	ztimer((void *)true);
	
	return Fl::run();
}

/** ********************************************************
 *
 ***********************************************************/
void open_url(const char* url)
{
	LOG_INFO("%s", url);
#ifndef __WOE32__
	const char* browsers[] = {
#  ifdef __APPLE__
		getenv("FLDIGI_BROWSER"),   // valid for any OS - set by user
		"open"                      // OS X
#  else
		"fl-xdg-open",			    // Puppy Linux
		"xdg-open",			        // other Unix-Linux distros
		getenv("FLDIGI_BROWSER"),   // force use of spec'd browser
		getenv("BROWSER"),		    // most Linux distributions
		"sensible-browser",
		"firefox",
		"mozilla"				    // must be something out there!
#  endif
	};
	switch (fork()) {
		case 0:
#  ifndef NDEBUG
			unsetenv("MALLOC_CHECK_");
			unsetenv("MALLOC_PERTURB_");
#  endif
			for (size_t i = 0; i < sizeof(browsers)/sizeof(browsers[0]); i++)
				if (browsers[i])
					execlp(browsers[i], browsers[i], url, (char*)0);
			exit(EXIT_FAILURE);
		case -1:
			fl_alert2(_("Could not run a web browser:\n%s\n\n"
						"Open this URL manually:\n%s"),
					  strerror(errno), url);
	}
#else
	if ((int)ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL) <= 32)
		fl_alert2(_("Could not open url:\n%s\n"), url);
#endif
}

/** ********************************************************
 *
 ***********************************************************/
void show_help()
{
	open_url("http://www.w1hkj.com/flamp2.1-help/index.html");
}

/** ********************************************************
 *
 ***********************************************************/
void cb_folders()
{
	open_url(fllnkHomeDir.c_str());
}
