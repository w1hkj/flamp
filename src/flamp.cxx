#include "config.h"

const char *copyright[] = {
	" =====================================================================",
	"",
	" FLAMP "  VERSION, // flamp.cxx
	"",
	"  Author(s):",
	"    Robert Stiles, KK5VD, Copyright (C) 2013",
	"    Dave Freese, W1HKJ, Copyright (C) 2012, 2013",
	"",
	" This software is distributed in the hope that it will be useful,",
	" but WITHOUT ANY WARRANTY; without even the implied warranty of",
	" MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  It is",
	" copyright under the GNU General Public License.",
	"",
	" You should have received a copy of the GNU General Public License",
	" along with the program; if not, write to:",
	"",
	" Free Software Foundation, Inc.",
	" 59 Temple Place, Suite 330",
	" Boston, MA  02111-1307 USA",
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

#include "flamp.h"
#include "amp.h"
#include "flamp_dialog.h"

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
#include "tagSearch.h"
#include "time_table.h"

#ifdef WIN32
#  include "flamprc.h"
#  include "compat.h"
#endif

#include <FL/filename.H>

#include <FL/x.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Image.H>

using namespace std;

string rx_buffer;
string tx_buffer;
string tmp_buffer;

const char *sz_flmsg = "<flmsg>";
const char *sz_cmd = "<cmd>";
const char *sz_flamp = "}FLAMP";

static TagSearch *cQue;


string testfname = "Bulletin1.txt";

bool testing = true;
bool transmitting = false;
bool transmit_queue = false;
bool transmit_stop = false;
bool generate_time_table = false;

static char szoutTimeValue[] = "12:30:00";
static char sztime[] = "123000";
static int  ztime;
static time_t ztime_current;
static time_t ztime_end;

static bool continuous_exception = false;
static bool event_timer_on = false;

bool loading_from_queue_file = false;

void auto_load_tx_queue(void);

int blocksize = 64;
int repeatNN = 1;

unsigned int modem_rotation_index = 0;
vector<std::string> bc_modems;
std::string g_modem;
std::string g_header_modem;


int g_event_driven = 0;

const char *options[] = {
	"Flamp Unique Options",
	"",
	"  --help",
	"  --version",
	"  --time-table",
	"    Used to generate timing tables for the various modes. Disabling "
	"    requires program restart."
	"  --flamp-dir folder-path-name (including drive letter on Windows)",
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

pthread_t *xmlrpc_thread = 0;
pthread_mutex_t mutex_xmlrpc  = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_file_io = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_tx_data = PTHREAD_MUTEX_INITIALIZER;

int xmlrpc_errno = 0;
int file_io_errno = 0;
bool active_data_io = false;
float tx_time_g = 0;

string title = "";
string BaseDir = "";
string flampHomeDir = "";
string flamp_rcv_dir = "";
string flamp_xmt_dir = "";
string buffer = "";

string cmd_fname = "";

string xmt_fname = "";
string rx_fname = "";

std::vector<cAmp *> tx_array;
std::vector<cAmp *> rx_array;

void * create_tx_table(void *);
double measure_tx_time(char character_to_send, unsigned int no_of_characters, double time_out_duration);
double time_f(void);
char *copyTo(char *src, char *dest, int *limit, int stop_character);

class cAmpGlobal amp;

cAmpGlobal::cAmpGlobal()
{
	rx_amp = 0;
	tx_amp = 0;

	pthread_mutex_init(&mutex_txAmp, NULL);
	pthread_mutex_init(&mutex_rxAmp, NULL);
}

cAmpGlobal::~cAmpGlobal()
{
	pthread_mutex_destroy(&mutex_txAmp);
	pthread_mutex_destroy(&mutex_rxAmp);
}

cAmp * cAmpGlobal::tx_cAmp(void)
{
	pthread_mutex_lock(&mutex_txAmp);
	cAmp *ret = tx_amp;
	pthread_mutex_unlock(&mutex_txAmp);
	return ret;
}

bool cAmpGlobal::tx_cAmp(cAmp *amp)
{
	pthread_mutex_lock(&mutex_txAmp);

	bool ret = true;

	tx_amp = amp;

	if(!tx_amp) {
		ret = false;
	}

	pthread_mutex_unlock(&mutex_txAmp);

	return ret;
}

cAmp * cAmpGlobal::rx_cAmp(void)
{

	pthread_mutex_lock(&mutex_rxAmp);
	cAmp *ret = rx_amp;
	pthread_mutex_unlock(&mutex_rxAmp);

	return ret;
}

bool cAmpGlobal::rx_cAmp(cAmp *amp)
{
	pthread_mutex_lock(&mutex_rxAmp);

	bool ret = true;

	rx_amp = amp;

	if(!tx_amp) {
		ret = false;
	}

	pthread_mutex_unlock(&mutex_rxAmp);

	return ret;
}

// utility functions

bool rx_complete = false;
bool isbinary(string s)
{
	for (size_t n = 0; n < s.length(); n++)
		if ((s[n] & 0x80) == 0x80) return true;
	return false;
}

#if !defined(__APPLE__) && !defined(__WOE32__) && USE_X
Pixmap  flamp_icon_pixmap;

#define KNAME "flamp"

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

bool parse_repeat_times(bool delete_flag)
{
	char ttime[10];
	char etime[10];
	bool local_flag = false;
	int time_end = 0;

	snprintf(ttime, sizeof(ttime), "%06d", ztime);

	ttime[4] = '-';
	ttime[5] = 0;

	size_t s = progStatus.repeat_times.length();
	size_t p = progStatus.repeat_times.find(ttime);

	if(p != std::string::npos) {
		if(progStatus.repeat_every != 8) return false;
		delete_flag = false;
		continuous_exception = true;
		local_flag = true;
	} else {
		ttime[4] = 0;
		p = progStatus.repeat_times.find(ttime);
		if(p == std::string::npos) return false;
		if(p > 0)
			if(progStatus.repeat_times[p - 1] == '-')
				return false;
		local_flag = false;
	}

	int len = 4;
	while (((p + len) < s) &&
		   !isdigit(progStatus.repeat_times[p + len])) len++;

	int count = 0;

	if(continuous_exception && local_flag) {

		while (((p + len) < s) &&
			   isdigit(progStatus.repeat_times[p + len])) {
			etime[count++] = progStatus.repeat_times[p + len];
			if(count > 4) break;
			if((p + len) < s) len++;
		}

		etime[4] = 0;

		if(count == 4) {
			time_end = atoi(etime);
		} else return false;

		if(time_end > 2400) time_end = 2400;
		if(time_end < 0) time_end = 0;

		{
			int current_time = ztime / 100;
			int current_total_minutes = ((current_time / 100) * 60) + (current_time % 100);
			int end_total_minutes = ((time_end / 100) * 60) + (time_end % 100);
			int diff_time_minutes = 0;

			if(current_time > time_end) {
				diff_time_minutes = ((24 * 60) - current_total_minutes);
				diff_time_minutes += end_total_minutes;
			} else {
				diff_time_minutes = abs(current_total_minutes - end_total_minutes);
			}

			ztime_end = time(0) + (time_t) (diff_time_minutes * 60);

		}
	}

	if(delete_flag) {
		if((p + len) > s) {
			len = s - p;
			if(len < 0) len = 0;
		}

		if(len)
			progStatus.repeat_times.erase(p, len);

		txt_repeat_times->value(progStatus.repeat_times.c_str());
	}

	if (progStatus.repeat_times.empty()) {
		do_events->value(0);
		cb_do_events((Fl_Light_Button *)0, (void*)0);
	}

	return true;
}

bool assign_bc_modem_list(void)
{
	int count = 0;

	bc_modems.clear();

	if(progStatus.hamcast_mode_enable_1) {
		bc_modems.push_back(cbo_hamcast_mode_selection_1->value());
		count++;
	}

	if(progStatus.hamcast_mode_enable_2) {
		bc_modems.push_back(cbo_hamcast_mode_selection_2->value());
		count++;
	}

	if(progStatus.hamcast_mode_enable_3) {
		bc_modems.push_back(cbo_hamcast_mode_selection_3->value());
		count++;
	}

	if(progStatus.hamcast_mode_enable_4) {
		bc_modems.push_back(cbo_hamcast_mode_selection_4->value());
		count++;
	}

	if(count) {
		return true;
	}

	return false;
}

void ztimer(void* first_call)
{
	struct timeval tv;
	static bool tx_toggle = false;
	static time_t tx_start = 0;
	static time_t tx_time_seconds = 0;
	static time_t tx_time_minutes = 0;
	static std::string tx_state;
	static std::string tx_duration;

	gettimeofday(&tv, NULL);

	if (first_call) {
		double st = 1.0 - tv.tv_usec / 1e6;
		Fl::repeat_timeout(st, ztimer);
	} else
		Fl::repeat_timeout(1.0, ztimer);

	if(generate_time_table) return; // No Events Allow in Time Table Generation Mode.

	struct tm tm;
	time_t t_temp;

	t_temp=(time_t)tv.tv_sec;
	gmtime_r(&t_temp, &tm);

	if (!strftime(sztime, sizeof(sztime), "%H%M%S", &tm))
		strcpy(sztime, "000000");

	ztime = atoi(sztime);

	if (!strftime(szoutTimeValue, sizeof(szoutTimeValue), "%H:%M:%S", &tm))
		memset(szoutTimeValue, 0, sizeof(szoutTimeValue));

	tx_state = get_trx_state();

	if((tx_state == "TX") && tx_toggle == false)  {
		tx_start = time(0);
		tx_toggle = true;
	}

	if((tx_state == "RX") && tx_toggle == true) {
		tx_time_seconds = time(0) - tx_start;
		tx_start = tx_time_seconds;
		tx_time_minutes = tx_time_seconds / 60.0;
		tx_time_seconds -= (tx_time_minutes * 60);
		tx_toggle = false;
		if(tx_time_g <= 0.0) tx_time_g = 1;

		LOG_DEBUG("TX time [mm:ss]: %02ld:%02ld [m:%ld c:%1.3f r:%1.6f] %s", \
		   tx_time_minutes, tx_time_seconds, tx_start, tx_time_g, \
		   tx_start/tx_time_g, g_modem.c_str());
	}

	outTimeValue->value(szoutTimeValue);

	event_timer_on = do_events->value();

	if (event_timer_on) {

		if (progStatus.repeat_at_times && (ztime % 100 == 0) && (progStatus.repeat_every != 8)) {
			switch (progStatus.repeat_every) {
				case 0 : // every 5 minutes
					if (ztime % 500 == 0) 	Fl::awake(transmit_queue_main_thread, (void *)0);
					break;
				case 1 : // every 15 minutes
					if (ztime % 1500 == 0) Fl::awake(transmit_queue_main_thread, (void *)0);
					break;
				case 2 : // every 30 minutes
					if (ztime % 3000 == 0) Fl::awake(transmit_queue_main_thread, (void *)0);
					break;
				case 3 : // hourly
					if (ztime % 10000 == 0) Fl::awake(transmit_queue_main_thread, (void *)0);
					break;
				case 4 : // even hours
					if (ztime == 0 || ztime == 20000 || ztime == 40000 ||
						ztime == 60000 || ztime == 80000 || ztime == 100000 ||
						ztime == 120000 || ztime == 140000 || ztime == 160000 ||
						ztime == 180000 || ztime == 200000 || ztime == 220000 )
						Fl::awake(transmit_queue_main_thread, (void *)0);
					break;
				case 5 : // odd hours
					if (ztime == 10000 || ztime == 30000 || ztime == 50000 ||
						ztime == 70000 || ztime == 90000 || ztime == 110000 ||
						ztime == 130000 || ztime == 150000 || ztime == 170000 ||
						ztime == 190000 || ztime == 210000 || ztime == 230000 )
						Fl::awake(transmit_queue_main_thread, (void *)0);
					break;
				case 6 : // at specified times
				{
					if (parse_repeat_times(false))
						Fl::awake(transmit_queue_main_thread, (void *)0);
				}
					break;
				case 7 : // One time scheduled
				{
					if(parse_repeat_times(true))
						Fl::awake(transmit_queue_main_thread, (void *)0);
				}
					break;

				default : // do nothing
					break;
			}
		} else if(progStatus.repeat_at_times && (progStatus.repeat_every == 8)) {

			ztime_current = time(0);

			parse_repeat_times(false);

			if(ztime_current >= ztime_end) {
				ztime_end = 0;
				continuous_exception = false;
			}

			if(continuous_exception) {
				try {
					if (tx_state == "RX")
						Fl::awake(transmit_queue_main_thread, (void *)0);
				}
				catch (...) {
				}
			}
		} else if (progStatus.repeat_forever) {
			try {
				if (tx_state == "RX")
					Fl::awake(transmit_queue_main_thread, (void *)0);
			}
			catch (...) {
			}
		}
	}
}

#if FLAMP_FLTK_API_MAJOR == 1 && FLAMP_FLTK_API_MINOR == 3
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

void checkdirectories(void)
{
	char dirbuf[FL_PATH_MAX + 1];
#ifdef __WOE32__
	if (BaseDir.empty()) {
		fl_filename_expand(dirbuf, sizeof(dirbuf) -1, "$USERPROFILE/");
		BaseDir = dirbuf;
	}
	if (flampHomeDir.empty()) flampHomeDir.assign(BaseDir).append("flamp.files/");
#else
	if (BaseDir.empty()) {
		fl_filename_expand(dirbuf, sizeof(dirbuf) -1, "$HOME/");
		BaseDir = dirbuf;
	}
	if (flampHomeDir.empty()) flampHomeDir.assign(BaseDir).append(".flamp/");
#endif

	struct DIRS {
		string& dir;
		const char* suffix;
		void (*new_dir_func)(void);
	};
	DIRS flamp_dirs[] = {
		{ flampHomeDir,  0,	0 },
		{ flamp_rcv_dir, "rx", 0 },
		{ flamp_xmt_dir, "tx", 0 },
	};

	int r;

	for (size_t i = 0; i < sizeof(flamp_dirs)/sizeof(*flamp_dirs); i++) {
		if (flamp_dirs[i].dir.empty() && flamp_dirs[i].suffix)
			flamp_dirs[i].dir.assign(flampHomeDir).append(flamp_dirs[i].suffix).append(PATH_SEP);

		if ((r = mkdir(flamp_dirs[i].dir.c_str(), 0777)) == -1 && errno != EEXIST) {
			cerr << _("Could not make directory") << ' ' << flamp_dirs[i].dir
			<< ": " << strerror(errno) << '\n';
			exit(EXIT_FAILURE);
		}
		else if (r == 0 && flamp_dirs[i].new_dir_func)
			flamp_dirs[i].new_dir_func();
	}
}

void addfile(string xmtfname, void *rx, bool useCompression, char *desc)
{
	xmt_fname = xmtfname;
	string xmt_fname2 = xmtfname;
	cAmp *rAmp = (cAmp *) rx;

	int use_comp_on_file = 0;
	int use_forced_comp_on_file = 0;

	if(rx > 0 && !rAmp->rx_completed()) {
		fl_alert2("Only completed files can be transfered");
		return;
	}

	FILE *dfile = fopen(xmt_fname.c_str(), "rb");
	if (!dfile) {
		LOG_ERROR("could not open read/binary %s", xmt_fname.c_str());
		exit (1);
	}
	fseek(dfile, 0, SEEK_END);
	size_t fsize = ftell(dfile);
	if (fsize <= 0) {
		LOG_ERROR("%s", "fsize error");
		return;
	}
	fseek(dfile, 0, SEEK_SET);
	tx_buffer.resize(fsize);
	size_t r = fread((void *)tx_buffer.c_str(), 1, fsize, dfile);
	if (r != fsize) {
		LOG_ERROR("%s", "read error");
		return;
	}

	fclose(dfile);

	if(useCompression)
		use_comp_on_file = 1;
	else
		use_comp_on_file = 0;

	txt_tx_filename->value(xmt_fname.c_str());
	if (isbinary(tx_buffer) && !progStatus.use_compression) {
		use_comp_on_file = 1;
	}

	// Looking for command/control strings. Force compression if found.
	if(tx_buffer.find(sz_flmsg) != std::string::npos) {
		use_forced_comp_on_file = 1;
	}

	if(tx_buffer.find(sz_cmd) != std::string::npos) {
		use_forced_comp_on_file = 1;
	}

	if(tx_buffer.find(sz_flamp) != std::string::npos) {
		use_forced_comp_on_file = 1;
	}

	if(use_comp_on_file && loading_from_queue_file == false)
		fl_alert2("Suggest using compression on this file");
	loading_from_queue_file = false;

	cAmp *nu = new cAmp(tx_buffer, fl_filename_name(xmt_fname.c_str()));
	nu->xmt_full_path_fname(xmt_fname2);

	if(desc)
		nu->xmt_descrip(desc);
	else
		nu->xmt_descrip("");

	tx_array.push_back(nu);
	tx_queue->add(xmt_fname.c_str());
	tx_queue->select(tx_queue->size());

	if(use_comp_on_file) {
		nu->compress(true);
		encoders->index(BASE64 - 1);
	}

	if(use_forced_comp_on_file) {
		nu->forced_compress(true);
		encoders->index(BASE64 - 1);
	}

	nu->tx_base_conv_index(encoders->index() + 1);
	nu->tx_base_conv_str(encoders->value());

	cAmp *tx_amp = nu;
	amp.tx_cAmp(nu);

	LOG_INFO("File added to transmit queue: %s", xmtfname.c_str());

	if(rx > 0) {
		cAmp *rAmp = (cAmp *) rx;
		int xfrBlockSize = rAmp->rx_blocksize_int();

		xfrBlockSize = valid_block_size(xfrBlockSize);

		cnt_blocksize->value(xfrBlockSize);
		txt_tx_descrip->value(txt_rx_descrip->value());

		tx_amp->xmt_descrip(txt_rx_descrip->value());
		tx_amp->tx_blocksize(xfrBlockSize);

		progStatus.blocksize = xfrBlockSize;
		txt_tx_selected_blocks->value("");
		update_selected_xmt();
	}

	estimate();
	estimate_bc();
}

int valid_block_size(int value)
{
	value = ((value / CNT_BLOCK_SIZE_STEP_RATE)
			 * CNT_BLOCK_SIZE_STEP_RATE);

	if(value < CNT_BLOCK_SIZE_MINIMUM)
		value = CNT_BLOCK_SIZE_MINIMUM;

	if(value > CNT_BLOCK_SIZE_MAXIMUM)
		value = CNT_BLOCK_SIZE_MAXIMUM;

	return value;
}

void replace_add_queue_item(char *filename, bool compFlag, char *desc)
{
	int count = 0;
	int i     = 0;
	cAmp *tx = (cAmp *)0;
	char *cPtr = (char *)0;
	bool compress = false;
	string fn;

	if(!filename) {
		LOG_DEBUG("filename parameter null");
		return;
	}

	count = strnlen(filename, FILENAME_MAX);
	if(count < 1) {
		LOG_DEBUG("No file name in variable.");
		return;
	}

	FILE *fd = fopen(filename, "r");

	if(!fd) {
		LOG_INFO("File %s not Found", filename);
		return;
	}

	fseek(fd, 0, SEEK_END);
	size_t fsize = ftell(fd);
	fclose(fd);

	if (fsize < 1) {
		LOG_INFO("File %s contains not data", filename);
		return;
	}

	count = tx_queue->size();

	for(i = 1; i <= count; i++) {
		tx = tx_array[i - 1];
		if(tx) {
			cPtr = (char *) tx->xmt_full_path_fname().c_str();
			if(strncmp(filename, cPtr, FILENAME_MAX) == 0) {
				if(!tx->xmt_file_modified()) return;
				compress = tx->compress();
				LOG_INFO("File removed from transmit queue: %s", cPtr);
				tx_queue->select(i);
				amp.tx_cAmp(tx);
				tx_removefile();
				break;
			}
		}
	}

	if(compFlag) compress = true;

	fn.assign(filename);

	loading_from_queue_file = true;
	addfile(fn, 0, compress, desc);
}

void auto_load_tx_queue_from_tx_directory(void)
{
	char *eMsg = (char *) "TX Queue access in progress, Auto load aborted";
	char *filepath = (char *)0;
	struct dirent **list = 0;
	int count = 0;
	int index = 0;
	//int tmp = 0;

	if(active_data_io == true) {
		LOG_INFO("%s", eMsg);
		return;
	}

	if(flamp_xmt_dir.size() < 1) return;

	count = fl_filename_list(flamp_xmt_dir.c_str(), &list);

	if (!count) {
		LOG_INFO("%s directoy not found or empty", flamp_xmt_dir.c_str());
		fl_filename_free_list(&list, count);
		return;
	}

	filepath = new char[FILENAME_MAX+1];

	if(!filepath) {
		LOG_ERROR("Internal memory allocation Error");
		fl_filename_free_list(&list, count);
		return;
	}

	struct stat stat_buf;

	for(index = 0; index < count; index++) {

		memset(filepath, 0, FILENAME_MAX + 1);
		strncpy(filepath, flamp_xmt_dir.c_str(), FILENAME_MAX);
		strncat(filepath, list[index]->d_name, FILENAME_MAX);

		if(stat((const char *) filepath, &stat_buf)) continue;

		if(stat_buf.st_mode & S_IFREG)
			replace_add_queue_item(filepath, 0, 0);
	}

	delete[] filepath;

	fl_filename_free_list(&list, count);
}

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

void auto_load_tx_queue_from_list(void)
{
	char *line     = (char *) 0;
	char *bname    = (char *) 0;
	char *path     = (char *) 0;
	char *srcpath  = (char *) 0;
	char *filepath = (char *) 0;
	char *desc     = (char *) 0;
	char *comp     = (char *) 0;

	char *cPtr     = (char *) 0;
	char *eMsg     = (char *) "Memory Allocation Error";
	char *eMsg2    = (char *) "First line in Queue List must be ";
	char *eMsg3    = (char *) "TX Queue access in progress, Auto load aborted";
	char *QueueTag = (char *) "FLAMPTXQUEUE";

	int size  = 0;
	int i     = 0;
	int qTagSize = 0;

	FILE *fd = (FILE *)0;

	if(active_data_io == true) {
		LOG_INFO("%s", eMsg3);
		return;
	}

	if(progStatus.auto_load_queue_path.size() < 1) {
		LOG_INFO("Auto Queue file name/path not set");
		return;
	}

	srcpath = new char[FILENAME_MAX];
	if(!srcpath) {
		LOG_ERROR("%s", eMsg);
		goto EXIT_AUTO_LOAD;
	}

	memset(srcpath, 0, FILENAME_MAX);
	strncpy(srcpath, progStatus.auto_load_queue_path.c_str(), FILENAME_MAX - 1);
	size = strnlen(srcpath, FILENAME_MAX);

	for(i = 0; i < size; i++)
		if(srcpath[i] == '\\') srcpath[i] = '/';

	fd = fopen(srcpath, "r");

	if(!fd) {
		LOG_INFO("Auto Queue file name/path not found (%s)", srcpath);
		goto EXIT_AUTO_LOAD;
	}

	bname = new char[FILENAME_MAX];
	if(!bname) {
		LOG_ERROR("%s", eMsg);
		goto EXIT_AUTO_LOAD;
	}

	path  = new char[FILENAME_MAX];
	if(!path) {
		LOG_ERROR("%s", eMsg);
		goto EXIT_AUTO_LOAD;
	}

	filepath  = new char[FILENAME_MAX];
	if(!filepath) {
		LOG_ERROR("%s", eMsg);
		goto EXIT_AUTO_LOAD;
	}

	line  = new char[FILENAME_MAX];
	if(!line) {
		LOG_ERROR("%s", eMsg);
		goto EXIT_AUTO_LOAD;
	}

	comp = new char[FILENAME_MAX];
	if(!comp) {
		LOG_ERROR("%s", eMsg);
		goto EXIT_AUTO_LOAD;
	}

	desc  = new char[FILENAME_MAX];
	if(!desc) {
		LOG_ERROR("%s", eMsg);
		goto EXIT_AUTO_LOAD;
	}

	memset(bname, 0, FILENAME_MAX);
	memset(path, 0, FILENAME_MAX);
	memset(filepath, 0, FILENAME_MAX);
	memset(comp, 0, FILENAME_MAX);
	memset(line, 0, FILENAME_MAX);
	memset(desc, 0, FILENAME_MAX);

	if (fgets(bname, FILENAME_MAX - 1, fd))
		size = strnlen(bname, FILENAME_MAX);
	else
		size = 0;
	qTagSize = strnlen(QueueTag, 16);

	if(size < qTagSize) {
		LOG_INFO("%s %s", eMsg2, QueueTag);
		goto EXIT_AUTO_LOAD;
	}

	i = memcmp(bname, QueueTag, qTagSize - 1);

	if(i != 0) {
		LOG_INFO("%s %s", eMsg2, QueueTag);
		goto EXIT_AUTO_LOAD;
	}

	cPtr = (char *) dirname((char *) srcpath);
	strncpy(path, cPtr,	FILENAME_MAX - 1);

	if(size < 1)
		strncpy(path, "./", 2);
	else if(path[size - 1] != '/')
		strncat(path, "/", 1);

	while(1) {

		if(ferror(fd)) {
			LOG_INFO("Error in reading file %s", bname);
		}

		if(feof(fd))   break;

		memset(bname, 0, FILENAME_MAX);
		memset(desc,  0, FILENAME_MAX);
		memset(comp,  0, FILENAME_MAX);
		memset(line,  0, FILENAME_MAX);

		if (fgets(line, FILENAME_MAX - 1, fd))
			size = FILENAME_MAX;
		else
			size = 0;

		size = FILENAME_MAX;
		cPtr = line;
		cPtr = copyTo(cPtr, bname, &size, ',');
		cPtr = copyTo(cPtr, comp,  &size, ',');
		cPtr = copyTo(cPtr, desc,  &size, '\n');

		size = strnlen(bname, FILENAME_MAX);

		if(size < 1) {
			LOG_INFO("Empty Line Found in Queue List (end of list reached)");
			break;
		}

		memset(filepath, 0, FILENAME_MAX);
		strncpy(filepath, path, FILENAME_MAX - 1);
		strncat(filepath, bname, FILENAME_MAX - 1);

		if(*comp == '1') i = 1;
		else i = 0;

		replace_add_queue_item(filepath, i, desc);

	}

EXIT_AUTO_LOAD:;

	if(line)     delete[] line;
	if(bname)    delete[] bname;
	if(path)     delete[] path;
	if(srcpath)  delete[] srcpath;
	if(filepath) delete[] filepath;
	if(desc)     delete[] desc;
	if(comp)     delete[] comp;

	if(fd) fclose(fd);
}

void auto_load_tx_queue(void)
{
	if(progStatus.auto_load_queue == false)
		return;

	if(progStatus.load_from_tx_folder)
		auto_load_tx_queue_from_tx_directory();
	else
		auto_load_tx_queue_from_list();
}

void readfile()
{
	string xmtfname;
	xmtfname.clear();
	const char *p = FSEL::select(_("Open file"), "any file\t*.*",
								 xmtfname.c_str());
	if (!p) return;
	if (strlen(p) == 0) return;
	xmtfname = p;

	addfile(xmtfname, 0, false, 0);
}

void show_selected_xmt(int n)
{
	if (!n) return;

	cAmp * tx_amp = tx_array[n-1];
	if(!tx_amp) return;

	amp.tx_cAmp(tx_amp);

	string fn = tx_amp->xmt_fname();
	string ds = tx_amp->xmt_descrip();
	string ns = tx_amp->xmt_numblocks();
	string ts = tx_amp->xmt_tosend();

	txt_tx_filename->value(fn.c_str());
	txt_tx_descrip->value(ds.c_str());
	txt_tx_selected_blocks->value(ts.c_str());

	if(progStatus.enable_tx_unproto == true) {
		txt_tx_numblocks->value("1");
	} else {
		txt_tx_numblocks->value(ns.c_str());
	}

	btn_use_compression->value(tx_amp->compress());
	progStatus.encoder = tx_amp->tx_base_conv_index();
	encoders->index(progStatus.encoder - 1);
	tx_buffer = tx_amp->xmt_data();
}

void show_rx_amp()
{
	cAmp * rx_amp = amp.rx_cAmp();
	if (!rx_amp) return;

	txt_rx_filename->value(rx_amp->get_rx_fname().c_str());
	txt_rx_datetime->value(rx_amp->rx_time_stamp().c_str());
	txt_rx_descrip->value(rx_amp->rx_desc().c_str());
	txt_rx_callinfo->value(rx_amp->rx_callinfo().c_str());
	txt_rx_filesize->value(rx_amp->rx_fsize().c_str());
	txt_rx_numblocks->value(rx_amp->rx_numblocks().c_str());
	txt_rx_blocksize->value(rx_amp->rx_blocksize().c_str());
	txt_rx_missing_blocks->value(rx_amp->rx_missing().c_str());
	rx_progress->set(rx_amp->rx_blocks(), rx_amp->rx_nblocks());
	if (rx_amp->rx_completed() && txt_rx_output->buffer()->length() == 0) {
		string data = rx_amp->rx_recvd_string();
		decompress_maybe(data);
		if (isbinary(data))
			txt_rx_output->addstr("Data appears to be binary\n\nSave and view with appropriate software");
		else
			txt_rx_output->addstr(data.c_str());
	}
}

void clear_rx_amp()
{
	txt_rx_filename->value("");
	txt_rx_datetime->value("");
	txt_rx_descrip->value("");
	txt_rx_callinfo->value("");
	txt_rx_filesize->value("");
	txt_rx_numblocks->value("");
	txt_rx_blocksize->value("");
	txt_rx_missing_blocks->value("");
	txt_rx_output->clear();
}

void clear_missing(void *ptr)
{
	cAmp *tx_amp = 0;
	int index = 0;
	int count = 0;

	if (transmit_stop == true) return;

	if(progStatus.clear_tosend_on_tx_blocks) {
		if(transmit_queue) {
			count = tx_array.size();
			for(index = 0; index < count; index++) {
				tx_amp = tx_array[index];
				if(!tx_amp) continue;
				tx_amp->reset_preamble_detection();
				tx_amp->xmt_tosend_clear();
			}
		} else {
			tx_amp = amp.tx_cAmp();
			if(!tx_amp) return;
			tx_amp->xmt_tosend_clear();
			tx_amp->reset_preamble_detection();
		}

		txt_tx_selected_blocks->value("");
	}
}

void show_selected_rcv(int n)
{
	if (!n) return;

	cAmp * rx_amp = rx_array[n-1];
	amp.rx_cAmp(rx_amp);

	txt_rx_output->clear();
	show_rx_amp();
}

void send_missing_report()
{
	string fname = txt_rx_filename->value();
	if (fname.empty()) return;

	cAmp * rx_amp = amp.rx_cAmp();
	if(!rx_amp) return;

	string report("\nDE ");
	report.append(txt_tx_mycall->value());
	report.append("\nFile : ").append(fname).append("\n");
	report.append(rx_amp->rx_report());
	report.append("DE ").append(txt_tx_mycall->value()).append(" K \n");

	if (progStatus.fldigi_xmt_mode_change) {
		send_new_modem(cbo_modes->value());
	}

	if(progStatus.use_tx_on_report) {
		if (!bConnected) connect_to_fldigi(0);
		if (!bConnected) return;
		send_via_fldigi(report);
	} else {
		report.append("\n\n^r");
		send_report(report);
	}
}

void update_selected_xmt()
{
	cAmp *tx_amp = amp.tx_cAmp();
	if (!tx_amp) return;

	tx_amp->xmt_descrip(txt_tx_descrip->value());
	tx_amp->xmt_tosend(txt_tx_selected_blocks->value());
	tx_amp->compress(btn_use_compression->value());
	tx_amp->tx_base_conv_index(progStatus.encoder);
	tx_amp->tx_base_conv_str(encoders->value());
}

void recv_missing_report()
{
	cAmp *camp = 0;
	for (size_t num = 0; num < tx_array.size(); num++) {
		camp = (cAmp *) tx_array[num];
		if(!camp) continue;
		camp->tx_parse_report();
	}

	cAmp *tx_amp = amp.tx_cAmp();
	if (!tx_amp) return;

	txt_tx_selected_blocks->value(tx_amp->xmt_tosend().c_str());
}

void tx_removefile()
{
	if(active_data_io) {
		LOG_INFO("Unable to remove TX queue item while being accessed.");
		return;
	}

	tx_buffer.clear();
	txt_tx_numblocks->value("");
	txt_tx_filename->value("");
	txt_tx_descrip->value("");
	txt_transfer_size_time->value("");
	txt_tx_selected_blocks->value("");

	cAmp *tx_amp = amp.tx_cAmp();
	if (!tx_amp) return;

	if (!tx_array.size()) return;
	if (!tx_queue->size()) return;
	int n = tx_queue->value();
	if (!n) return;
	tx_queue->remove(n);
	n--;

	amp.tx_cAmp((cAmp *)0);
	tx_amp = tx_array[n];
	amp.free_cAmp(tx_amp);

	tx_array.erase(tx_array.begin() + n);

	if (tx_queue->size()) {
		n = 1;
		tx_amp = tx_array[n - 1];
		amp.tx_cAmp(tx_amp);
		tx_queue->select(n);
		estimate();
		estimate_bc();
	}
}

void writefile(int xfrFlag)
{
	cAmp * rx_amp = amp.rx_cAmp();
	if (!rx_amp) return;

	size_t fsize = rx_amp->rx_size();

	if(xfrFlag && !rx_amp->rx_completed()) {
		fl_alert2("Only completed files can be transfered");
		return;
	}

	if (!fsize || rx_amp->get_rx_fname().empty()) return;

	rx_fname.assign(flampHomeDir).append("rx").append(PATH_SEP).append(rx_amp->get_rx_fname());
	const char *p = FSEL::saveas(_("Save file"), "file\t*.*",
								 rx_fname.c_str());
	if (!p) return;
	if (strlen(p) == 0) return;
	rx_fname = p;

	FILE *dfile = fopen(rx_fname.c_str(), "wb");
	if (!dfile) {
		LOG_ERROR("could not open write/binary %s", rx_fname.c_str());
		exit (1);
	}
	string data = rx_amp->rx_recvd_string();
	decompress_maybe(data);

	size_t r = fwrite((void *)data.c_str(), 1, data.length(), dfile);
	if (r != data.length()) {
		LOG_ERROR("%s", "write error");
		return;
	}
	fclose(dfile);

	if(xfrFlag && dfile) {
		addfile(rx_fname, rx_amp, rx_amp->compress(), 0);
	}
}

double time_f(void)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return ((double) now.tv_sec) + (((double)now.tv_usec) * 0.000001);
}

std::string tx_string(cAmp *tx, int callsign_flag, std::string t_string)
{
	std::string auto_send;

	pthread_mutex_lock(&mutex_tx_data);

	active_data_io = true;

	std::string send_to = txt_tx_send_to->value();
	std::string temp;

	if (send_to.empty()) send_to.assign("QST");
	send_to.append(" DE ").append(progStatus.my_call);
	for (size_t n = 0; n < send_to.length(); n++)
		send_to[n] = toupper(send_to[n]);

	if(callsign_flag & CALLSIGN_PREAMBLE)
		auto_send.assign("\n\n").append(send_to).append("\n\n");

	tx->my_call(progStatus.my_call);
	tx->my_info(progStatus.my_info);
	tx->xmt_blocksize(progStatus.blocksize);

	temp.clear();
	temp.assign(tx->xmt_buffer());

	if(progStatus.enable_tx_unproto == false) {
		compress_maybe(temp, tx->tx_base_conv_index(), (tx->compress() | tx->forced_compress()));//true);
		tx->xmt_data(temp);
		tx->repeat(progStatus.repeatNN);
		tx->header_repeat(progStatus.repeat_header);
		auto_send.append(tx->xmt_string());
	} else {
		tx->xmt_unproto();
		auto_send.append(tx->xmt_unproto_string());
	}

	if(callsign_flag & CALLSIGN_POSTAMBLE)
		auto_send.append("\n\n").append(send_to).append(t_string);

	auto_send.append("\n\n\n");

	active_data_io = false;

	pthread_mutex_unlock(&mutex_tx_data);

	return auto_send;
}

// Create time table for the various modes
// Starts at current selected mode.
#define MIN_TX_TIME 10.0
#ifdef OLD_WAY
void * create_tx_table(void *ptr)
{
	TX_FLDIGI_THREAD *thread_ptr = (TX_FLDIGI_THREAD *)ptr;

	if (!bConnected) connect_to_fldigi(0);
	if (!bConnected) return run_in_thread_destroy(thread_ptr, 3);

	int index = 0;
	int mode = 0;
	int start_mode = 0;
	int end_mode = 0;
	int buffer_send_size = 0;
	int buffer_start_size = 50;
	int buffer_size = 50;
	int char_count = 10;
	int adjust_count = 5;
	int first_char_count = 0;
	int second_char_count = 0;
	int modulus = 64;

	double time_used = 0;
	double temp = 0.0;
	double overhead = 0.0;
	double time_seconds = 0;
	double denominator = 1;
	double min_tx_time = 10;
	double first_measurement  = 0;
	double second_measurement = 0;

	char filename[256];
	std::string response;
	std::string txBuffer;
	std::string mode_name;

	FILE *fd = (FILE *)0;

	transmitting = true;

	{
		Fl::awake(set_button_to_cancel, (void *)0);
		static int value = TX_ALL_BUTTON;
		Fl::awake(deactivate_button, (void *) &value);
	}
	turn_rsid_off();

	// Not currently used, left for future use.
	pthread_mutex_lock(&thread_ptr->mutex);
	thread_ptr->thread_running = 1;
	thread_ptr->exit_thread = 0;
	pthread_mutex_unlock(&thread_ptr->mutex);

	memset(filename, 0, sizeof(filename));

	for(index = 0; index < 100; index++) {
		snprintf(filename, sizeof(filename)-1, "time_table-%02d.txt", index);
		fd = fopen(filename, "r");
		if(!fd) break;
		fclose(fd);
	}

	if(index > 99) {
		LOG_DEBUG("time_table-xx.txt files reached count limit, delete some.");
		return run_in_thread_destroy(thread_ptr, 3);
	}

	if(fd) fclose(fd);
	fd = fopen(filename, "w");

	if(fd) {
		LOG_DEBUG("Table Gen filename %s", filename);
		fprintf(fd, "typedef struct {\n"
				"\tchar *mode_name;\n"
				"\tfloat scale;\n"
				"\tfloat overhead;\n"
				"\tfloat table[256];\n"
				"} MODE_TIME_TABLE;\n\n");

		fprintf(fd, "%s", "MODE_TIME_TABLE mode_time_table[] = {\n");
		fflush(fd);

		if(thread_ptr->que) {
			start_mode = 0;
		} else {
			start_mode = cbo_modes->index();
		}

		end_mode = cbo_modes->lsize();

		mode = start_mode;

		do {
			buffer_send_size = buffer_start_size;

			LOG_DEBUG("Processing Modem %s", cbo_modes->value());

			send_clear_tx();
			send_new_modem("CW");

			cbo_modes->index(mode);
			mode_name = cbo_modes->value();

			send_new_modem(mode_name.c_str());
			wait_seconds(1);

			adjust_count = modulus;
			char_count = adjust_count;

			first_measurement = measure_tx_time('A', char_count, 120);

			if(first_measurement == 0.0) {
				transmit_stop = true;
				LOG_ERROR("FLDIGI does not support xmlrpc command main.get_tx_duration");
			}

			wait_seconds(1);
			double wait_time = MIN_TX_TIME;
			do {
				if(transmit_stop || thread_ptr->exit_thread) {
					if(fd) fclose(fd);
					return run_in_thread_destroy(thread_ptr, 3);
				}
				wait_seconds(1);
				first_measurement = measure_tx_time('A', char_count, 120);
				first_char_count = char_count;
				LOG_DEBUG("first_measurement=%f first_char_count=%d", first_measurement, first_char_count);
				char_count = (int) ((char_count / first_measurement) * wait_time);
				char_count = (int) (((double) char_count) * 2.0) + adjust_count;
				char_count += (modulus - (char_count % modulus));
			} while (first_measurement < wait_time);

			wait_time = first_measurement + MIN_TX_TIME;

			do {
				if(transmit_stop || thread_ptr->exit_thread) {
					if(fd) fclose(fd);
					return run_in_thread_destroy(thread_ptr, 3);
				}
				wait_seconds(1);
				second_measurement = measure_tx_time('A', char_count, 120);
				second_char_count = char_count;
				LOG_DEBUG("second_measurement=%f second_char_count=%d", second_measurement, second_char_count);
				char_count = (int) ((char_count / second_measurement) * wait_time);
				char_count = (int) (((double) char_count) * 2.0) + adjust_count;
				char_count += (modulus - (char_count % modulus));
			} while (second_measurement < wait_time);

			temp =  second_measurement - first_measurement;
			temp /=  (second_char_count - first_char_count);
			temp *=  second_char_count;
			overhead = second_measurement - temp;

			LOG_DEBUG("Mode %s overhead=%f", mode_name.c_str(), overhead);

			fprintf(fd, "\t{\n\t\t(char *) \"%s\", 1.0, %1.6f, \n\t\t{\n\t\t\t", mode_name.c_str(), overhead);
			fflush(fd);

			min_tx_time = overhead + MIN_TX_TIME;
			buffer_send_size = first_char_count;

			for(index = 0; index < 256; index++) {
				if(transmit_stop || thread_ptr->exit_thread) {
					char msg[] = "\nNOTICE: User requested Termination\n";
					fprintf(fd, "%s", msg);
					LOG_DEBUG("%s", msg);
					fclose(fd);
					return run_in_thread_destroy(thread_ptr, 3);
				}

				LOG_DEBUG("Proc Char = %d, %X : ", index, index);

				if((index % 8) == 0 && index != 0) {
					fprintf(fd, "\n\t\t\t");
				}

				/* if(c_binary(index)) {
					fprintf(fd, "%3.6f", 0.0);
					if(index < 255)
						fprintf(fd, ", ");
					continue;
				} */

				do {
					if(transmit_stop || thread_ptr->exit_thread) {
						fclose(fd);
						return run_in_thread_destroy(thread_ptr, 3);
					}

					send_clear_tx();
					send_new_modem("CW");

					// Leave here.  Bored user my effect tx panal mode
					cbo_modes->index(mode);
					send_new_modem(cbo_modes->value());
					//wait_seconds(1);

					buffer_size = buffer_send_size;
					time_used = measure_tx_time(index, buffer_send_size, 120.0);

					LOG_DEBUG("char_count=%d time_used=%f min_tx_time=%f overhead=%f", buffer_send_size, time_used, min_tx_time, overhead);

					if(time_used < min_tx_time || time_used > (min_tx_time * 1.3)) {
						buffer_send_size = ((buffer_send_size / time_used) * min_tx_time);
						buffer_send_size = (int) (((double) buffer_send_size) * 1.1);
						buffer_send_size += (modulus - (buffer_send_size % modulus));
						LOG_DEBUG("Buffer Adjusted to %d", buffer_send_size);
					}
				} while (time_used < min_tx_time);


				if(buffer_size)
				    time_seconds = 1.0 * (time_used - overhead) / buffer_size;
				else
					time_seconds = 0.0;

				fprintf(fd, "%3.6f", time_seconds);

				if(index < 255)
					fprintf(fd, ", ");

				if(index < ' ')
					LOG_DEBUG("Character \'%d (%X)\' Transmit time in seconds=%f", (index & 0xff), (index & 0xff), time_seconds);
				else
					LOG_DEBUG("Character \'%c\' Transmit time in seconds=%f", (index & 0xff), time_seconds);

				fflush(fd);

			}

			fprintf(fd, "\n\t\t}\n\t}");

			mode++;

			if(mode < end_mode)// && thread_ptr->que)
				fprintf(fd, ",");

			fprintf(fd, "\n");
			fflush(fd);

			//if(!thread_ptr->que) break;

		} while(mode < end_mode);

		fprintf(fd, "%s", "};\n");
		fclose(fd);

	} else {
		LOG_ERROR("%s = %d", "File open Error", errno);
	}

	return run_in_thread_destroy(thread_ptr, 3);
}
#else

double measure_tx_time(int character_to_send, double *over_head);

void * create_tx_table(void *ptr)
{
	TX_FLDIGI_THREAD *thread_ptr = (TX_FLDIGI_THREAD *)ptr;

	if (!bConnected) connect_to_fldigi(0);
	if (!bConnected) return run_in_thread_destroy(thread_ptr, 3);

	int index = 0;
	int mode = 0;
	int start_mode = 0;
	int end_mode = 0;

	double overhead = 0.0;
	double time_seconds = 0;

// these variables not used in current implementation
//	int buffer_send_size = 0;
//	int buffer_start_size = 50;
//	int buffer_size = 50;
//	int char_count = 10;
//	int adjust_count = 5;
//	int first_char_count = 0;
//	int second_char_count = 0;
//	int modulus = 64;
//	double time_used = 0;
//	double temp = 0.0;
//	double denominator = 1;
//	double min_tx_time = 10;
//	double first_measurement  = 0;
//	double second_measurement = 0;

	char filename[256];
	std::string response;
	std::string txBuffer;
	std::string mode_name;

	FILE *fd = (FILE *)0;

	transmitting = true;

	{
		Fl::awake(set_button_to_cancel, (void *)0);
		static int value = TX_ALL_BUTTON;
		Fl::awake(deactivate_button, (void *) &value);
	}
	turn_rsid_off();

	// Not currently used, left for future use.
	pthread_mutex_lock(&thread_ptr->mutex);
	thread_ptr->thread_running = 1;
	thread_ptr->exit_thread = 0;
	pthread_mutex_unlock(&thread_ptr->mutex);

	memset(filename, 0, sizeof(filename));

	for(index = 0; index < 100; index++) {
		snprintf(filename, sizeof(filename)-1, "time_table-%02d.txt", index);
		fd = fopen(filename, "r");
		if(!fd) break;
		fclose(fd);
	}

	if(index > 99) {
		LOG_DEBUG("time_table-xx.txt files reached count limit, delete some.");
		return run_in_thread_destroy(thread_ptr, 3);
	}

	if(fd) fclose(fd);
	fd = fopen(filename, "w");

	if(fd) {
		LOG_DEBUG("Table Gen filename %s", filename);
		fprintf(fd, "typedef struct {\n"
				"\tchar *mode_name;\n"
				"\tfloat scale;\n"
				"\tfloat overhead;\n"
				"\tfloat table[256];\n"
				"} MODE_TIME_TABLE;\n\n");

		fprintf(fd, "%s", "MODE_TIME_TABLE mode_time_table[] = {\n");
		fflush(fd);

		if(thread_ptr->que) {
			start_mode = 0;
		} else {
			start_mode = cbo_modes->index();
		}

		end_mode = cbo_modes->lsize();

		mode = start_mode;

		do {
			LOG_DEBUG("Processing Modem %s", cbo_modes->value());

			send_clear_tx();
			send_new_modem("CW");

			cbo_modes->index(mode);
			mode_name = cbo_modes->value();

			send_new_modem(mode_name.c_str());
			wait_seconds(1);

			time_seconds = measure_tx_time((int) 'A', &overhead);

			LOG_DEBUG("Mode %s overhead=%f", mode_name.c_str(), overhead);

			fprintf(fd, "\t{\n\t\t(char *) \"%s\", 1.0, %1.6f, \n\t\t{\n\t\t\t", mode_name.c_str(), overhead);
			fflush(fd);

// unused variables
//			min_tx_time = overhead + MIN_TX_TIME;
//			buffer_send_size = first_char_count;

			if(mode_name.find("Olivia") != string::npos || mode_name.find("MT63") != string::npos) {
				for(index = 0; index < 128; index++) {
					if((index % 8) == 0 && index != 0) {
						fprintf(fd, "\n\t\t\t");
					}
					fprintf(fd, "%3.6f, ", time_seconds);
				}
				fflush(fd);

				time_seconds = measure_tx_time((int) 130, &overhead);

				for(index = 128; index < 256; index++) {
					if((index % 8) == 0 && index != 0) {
						fprintf(fd, "\n\t\t\t");
					}

					fprintf(fd, "%3.6f", time_seconds);

					if(index < 255)
						fprintf(fd, ", ");
				}
				fflush(fd);

			} else {
				for(index = 0; index < 256; index++) {
					if(transmit_stop || thread_ptr->exit_thread) {
						char msg[] = "\nNOTICE: User requested Termination\n";
						fprintf(fd, "%s", msg);
						LOG_DEBUG("%s", msg);
						fclose(fd);
						return run_in_thread_destroy(thread_ptr, 3);
					}

					LOG_DEBUG("Proc Char = %d, %X : ", index, index);

					if((index % 8) == 0 && index != 0) {
						fprintf(fd, "\n\t\t\t");
					}

					time_seconds = measure_tx_time(index, 0);

					fprintf(fd, "%3.6f", time_seconds);

					if(index < 255)
						fprintf(fd, ", ");

					if(index < ' ')
						LOG_DEBUG("Charater \'%d (%X)\' Transmit time in seconds=%f", (index & 0xff), (index & 0xff), time_seconds);
					else
						LOG_DEBUG("Charater \'%c\' Transmit time in seconds=%f", (index & 0xff), time_seconds);

					fflush(fd);

				}
			}

			fprintf(fd, "\n\t\t}\n\t}");

			mode++;

			if(mode < end_mode)
				fprintf(fd, ",");

			fprintf(fd, "\n");
			fflush(fd);

		} while(mode < end_mode);

		fprintf(fd, "};\n");
		fflush(fd);
		fclose(fd);

	} else {
		LOG_ERROR("%s = %d", "File open Error", errno);
	}

	return run_in_thread_destroy(thread_ptr, 3);
}

#endif //OLD_WAY

#ifdef OLD_WAY
double measure_tx_time(char character_to_send, unsigned int no_of_characters, double time_out_duration)
{
	double duration = 0;
	unsigned int samples = 0;
	unsigned int sample_rate = 0;
	unsigned int over_head = 0;
	double retun_val = 0.0;
	int millisecond_delay = 1000;

	std::string tx_duration;

	if(no_of_characters < 1) {
		LOG_ERROR("No of Characters = %u", no_of_characters);
		transmit_stop = true;
		return retun_val;
	}

	for(int i = 0; i < 256; i++)
		tx_duration = get_tx_char_n_timing(i, no_of_characters);
		//tx_duration = get_tx_char_n_timing(character_to_send, no_of_characters);

	if(tx_duration.size() > 0) {
		sscanf(tx_duration.c_str(), "%u:%u", &samples, &sample_rate, &over_head);
		//sscanf(tx_duration.c_str(), "%u:%u:%u", &samples, &sample_rate);
	}

	if(sample_rate != 0)
		duration = 1.0 * samples / sample_rate;
	else
		duration = 1.0;

	// MilliSleep(millisecond_delay);

	return duration;
}
#else
double measure_tx_time(int character_to_send, double *over_head)
{
	double duration = 0;
	unsigned int s = 0;
	unsigned int sr = 0;
	unsigned int oh = 0;
	double retun_val = 0.0;
//	int millisecond_delay = 1000;

	std::string tx_duration;

	set_xmlrpc_timeout(8.0);
	tx_duration = get_char_timing(character_to_send);
	// tx_duration = get_tx_char_n_timing(character_to_send, 0);
	set_xmlrpc_timeout_default();

	if(tx_duration.size() < 1) {
		LOG_ERROR("No Data returned.");
		transmit_stop = true;
		return retun_val;
	}

	int count = sscanf(tx_duration.c_str(), "%u:%u:%u", &s, &sr, &oh);

	if(count != 3) {
		LOG_ERROR("Returned parameter error. Count = %d should be 3", count);
		transmit_stop = true;
		return retun_val;
	}

	if(sr != 0) {
		duration = 1.0 * s / sr;

		if(over_head) {
			*over_head = 1.0 * oh / sr;
		}
	}
	else
		duration = 1.0;


	//MilliSleep(millisecond_delay);

	return duration;
}

#endif

void transmit_current()
{
	if (!bConnected) connect_to_fldigi(0);
	if (!bConnected) return;

	if(transmitting) return;

	transmit_stop  = false;
	transmit_queue = false;

	if(generate_time_table) {
		run_in_thread(create_tx_table, 0, 0);
	} else {
		if(progStatus.use_txrx_interval) {
			run_in_thread(transmit_interval, TX_SEGMENTED, false);
		} else if(progStatus.use_header_modem) {
			turn_rsid_on();
			run_in_thread(transmit_interval, TX_CONTINIOUS, false);
		} else {
			run_in_thread(transmit_serial_current, 0, false);
		}
	}
}

void transmit_queued(bool event_driven)
{
	if (!bConnected) connect_to_fldigi(0);
	if (!bConnected) return;

	if(transmitting) return;

	transmit_stop  = false;
	transmit_queue = true;
	g_event_driven = event_driven;

	if(generate_time_table) {
		run_in_thread(create_tx_table, 1, 1);
	} else {
		if(progStatus.auto_load_queue && active_data_io == false && event_driven) {
			auto_load_tx_queue();
		}
		if(progStatus.use_txrx_interval) {
			run_in_thread(transmit_interval, TX_SEGMENTED, true);
		} else if(progStatus.use_header_modem) {
			turn_rsid_on();
			run_in_thread(transmit_interval, TX_CONTINIOUS, true);
		} else {
			run_in_thread(transmit_serial_queued, 0, true);
		}
	}
}

void * transmit_serial_current(void *ptr)
{
	void * ret = 0;
	transmitting = true;

	{
		Fl::awake(set_button_to_cancel, (void *)0);
		static int value = TX_ALL_BUTTON;
		Fl::awake(deactivate_button, (void *) &value);
	}

	int n = tx_queue->value();
	if (!n)
		return run_in_thread_destroy((TX_FLDIGI_THREAD *) ptr, 3);

	if (progStatus.fldigi_xmt_mode_change)
		send_new_modem(cbo_modes->value());

	float tx_time = 0;

	cAmp *tx = 0;
	string temp = "";
	string temp2 = "";
	string autosend = "";
	string send_to = "";

	temp.clear();
	autosend.clear();
	send_to.clear();

	active_data_io = true;

	tx = tx_array[n-1];

	if(!tx) {
		active_data_io = false;
		return ptr;
	}

	autosend = tx_string(tx, CALLSIGN_PREAMBLE | CALLSIGN_POSTAMBLE, " K\n");

	int value = tx->xmt_buffer().size();
	if (!value) {
		active_data_io = false;
		return run_in_thread_destroy((TX_FLDIGI_THREAD *) ptr, 3);
	}

	float oh = 0;
	tx_time = seconds_from_string(cbo_modes->value(), autosend, &oh);
	tx_time += oh;
	tx_time_g = tx_time;
	tx_time *= 2.0;

	send_via_fldigi(autosend);

	wait_for_rx((int) tx_time);

	//if (transmit_stop == true) send_abort();

	show_selected_xmt(n);

	ret = run_in_thread_destroy((TX_FLDIGI_THREAD *) ptr, 3);

	return ret;
}

void * transmit_serial_queued(void *ptr)
{
	transmitting = true;

	Fl::awake(set_button_to_cancel, (void *)0);
	static int val = TX_ALL_BUTTON;
	Fl::awake(deactivate_button, (void *) &val);

	float tx_time = 0;
	TX_FLDIGI_THREAD *thread = (TX_FLDIGI_THREAD *)ptr;

	if (tx_array.size() == 0)
		return run_in_thread_destroy(thread, 3);

	if (progStatus.fldigi_xmt_mode_change)
		send_new_modem(thread->modem.c_str());

	cAmp *tx;
	string temp = "";
	string autosend = "";
	string terminator = "";
	string null_string = "";
	temp.clear();
	autosend.clear();

	unsigned int count = tx_array.size();

	if(continuous_exception)
		terminator.assign(" BK\n");
	else
		terminator.assign(" K\n");

	for (size_t num = 0; num < count; num++) {
		tx = tx_array[num];
		if (tx->xmt_buffer().empty())
			return run_in_thread_destroy(thread, 3);

		if(num == 0)
			temp = tx_string(tx, CALLSIGN_PREAMBLE | CALLSIGN_POSTAMBLE, null_string);
		else if(num == (count - 1))
			temp = tx_string(tx, CALLSIGN_POSTAMBLE, terminator);
		else
			temp = tx_string(tx, CALLSIGN_POSTAMBLE, null_string);

		autosend.append(temp);
	}

	send_via_fldigi(autosend);

	float oh = 0;
	tx_time = seconds_from_string(thread->modem.c_str(), autosend, &oh);
	tx_time += oh;
	tx_time_g = tx_time;
	tx_time *= 1.10;

	wait_for_rx((int) tx_time);

	//if (transmit_stop == true) send_abort();

	return run_in_thread_destroy(thread, 3);
}

void turn_rsid_on()
{
	// Turn on RX/TX RSIDs
	send_via_fldigi("<cmd><txrsid>ON</txrsid></cmd><cmd><rsid>ON</rsid></cmd>");
	wait_seconds(1);
}

void turn_rsid_off()
{
	// Turn on RX/TX RSIDs
	send_via_fldigi("<cmd><txrsid>OFF</txrsid></cmd><cmd><rsid>OFF</rsid></cmd>");
	wait_seconds(1);
}

void * transmit_interval(void * ptr)
{
	transmitting = true;

	Fl::awake(set_button_to_cancel, (void *)0);
	static int val = TX_ALL_BUTTON;
	Fl::awake(deactivate_button, (void *) &val);

	cAmp *tx = (cAmp *)0;
	TX_FLDIGI_THREAD *thread_ptr = (TX_FLDIGI_THREAD *)ptr;
	int count = 0;
	int limit = 0;
	int index = 0;
	bool fills = 0;

	std::vector<std::string> vector_data;
	std::string temp;
	std::string tail;
	std::string autosend;
	std::string send_to = txt_tx_send_to->value();

	int n = 0;
	int value = 0;

	pthread_mutex_lock(&thread_ptr->mutex);
	thread_ptr->thread_running = 1;
	thread_ptr->exit_thread = 0;
	pthread_mutex_unlock(&thread_ptr->mutex);

	if(thread_ptr->que) {
		index = 0;
		count = 0;
		limit = tx_array.size();
	} else {
		n = tx_queue->value();
		if (!n)	return run_in_thread_destroy(thread_ptr, 3);
		index = n - 1;
		count = 0;
		limit = 1;
	}

	if(progStatus.use_header_modem) {
		turn_rsid_on();
	}

	if (send_to.empty()) send_to.assign("QST");
	send_to.append(" DE ").append(progStatus.my_call);
	for (size_t n = 0; n < send_to.length(); n++)
		send_to[n] = toupper(send_to[n]);

	do {
		if(transmit_stop)
			return run_in_thread_destroy(thread_ptr, 3);

		tx = tx_array[index];

		if(!tx)
			return run_in_thread_destroy(thread_ptr, 3);

		autosend.assign("\n\n").append(send_to);//.append("\n");

		value = tx->xmt_buffer().size();
		if (!value) {
			LOG_DEBUG("Empty xmt_buffer");
			return run_in_thread_destroy(thread_ptr, 3);
		}

		tx->my_call(progStatus.my_call);
		tx->my_info(progStatus.my_info);
		tx->xmt_blocksize(progStatus.blocksize);

		temp.assign(tx->xmt_buffer());
		compress_maybe(temp, tx->tx_base_conv_index(), (tx->compress() | tx->forced_compress()));//true);
		tx->xmt_data(temp);
		tx->repeat(progStatus.repeatNN);
		tx->header_repeat(progStatus.repeat_header);

		if(!tx->xmt_vector_string()) {
			LOG_DEBUG("Empty xmt_vector_string");
			return run_in_thread_destroy(thread_ptr, 3);
		}

		if(check_block_tx_time(tx, thread_ptr) == false)
			return run_in_thread_destroy(thread_ptr, 3);

		// Header section

		vector_data = tx->xmt_vector_header();

		tail.clear();
		tail.assign("\n").append(send_to).append(" BK\n\n");

		fills = false;
		if(progStatus.disable_header_modem_on_block_fills && tx->preamble_detected() == false) {
			char *cPtr = (char *) txt_tx_selected_blocks->value();
			if(cPtr) {
				int _limit = 0;
				while(*cPtr) {
					if(isdigit(*cPtr)) {
						fills = true;
						break;
					}
					cPtr++;
					if(_limit++ > 16) break;
				}
			}
		}

		if(progStatus.use_header_modem && fills == false) {
			LOG_DEBUG("Header Modem Sent to FLDGI. (%s)", cbo_header_modes->value());

			send_new_modem(thread_ptr->header_modem);

			if(!send_vector_to_fldigi(thread_ptr->header_modem, autosend, tail, vector_data, thread_ptr->mode)) {
				return run_in_thread_destroy(thread_ptr, 3);
			}
			send_new_modem(thread_ptr->modem);
		} else {
			if (progStatus.fldigi_xmt_mode_change)
				send_new_modem(thread_ptr->modem);

			if(!send_vector_to_fldigi(thread_ptr->modem, autosend, tail, vector_data, thread_ptr->mode)) {
				return run_in_thread_destroy(thread_ptr, 3);
			}
		}

		if(thread_ptr->mode != TX_MODEM_SAME) {
			if(thread_ptr->mode == TX_SEGMENTED)
				wait_seconds(cnt_rx_internval_secs->value());
			else
				wait_seconds(1);
		}

		// Data section

		if(transmit_stop)
			return run_in_thread_destroy(thread_ptr, 3);

		vector_data = tx->xmt_vector_data();

		tail.clear();

		if(count >= (limit - 1) && !continuous_exception)
			tail.assign("\n").append(send_to).append(" K\n");
		else
			tail.assign("\n").append(send_to).append(" BK\n");

		autosend.clear();

		send_vector_to_fldigi(thread_ptr->modem, autosend, tail, vector_data, thread_ptr->mode);

		show_selected_xmt(index);

		index++;
		count++;

		if(count < limit) {
			wait_seconds(cnt_rx_internval_secs->value());
		}

	} while(count < limit);

	return run_in_thread_destroy(thread_ptr, 3);
}

bool send_vector_to_fldigi(std::string modem, std::string &autosend, std::string &tail, std::vector<std::string> vector_data, int mode)
{
	std::string temp;
	int count = 0;
	int index = 0;
	float data_time_seconds = 0;
	float transfer_segment_time = 0;
	float transfer_limit_time = 0;
	float overhead = 0;
	float length = 0;
	float next_length = 0;

	std::string send_to = txt_tx_send_to->value();

	if (send_to.empty()) send_to.assign("QST");
	send_to.append(" DE ").append(progStatus.my_call);
	for (size_t n = 0; n < send_to.length(); n++)
		send_to[n] = toupper(send_to[n]);

	temp.clear();
	count = vector_data.size();
	for(index = 0; index < count; index++) {
		temp.append(vector_data[index]);
	}

	data_time_seconds = seconds_from_string(modem, temp, &overhead);
	data_time_seconds += overhead;
	tx_time_g = data_time_seconds;

	if(data_time_seconds <= 0.0) return false;

	if(mode == TX_SEGMENTED) {
		transfer_limit_time = (float) cnt_tx_internval_mins->value() * 60;
		if(transfer_limit_time < 1) return false;
	} else
		transfer_limit_time = (float) ID_TIME_SECONDS;

	length = overhead;

	if(data_time_seconds > transfer_limit_time) {

		temp.assign("\n");

		for(index = 0; index < count; index++) {

			if(transmit_stop) return false;

			length += seconds_from_string(modem, vector_data[index], &overhead);

			temp.append(vector_data[index]);

			if(index < (count - 1)) {
				next_length = seconds_from_string(modem, vector_data[index + 1], &overhead);
			} else
				next_length = 0;

			transfer_segment_time = length + next_length + INTERVAL_TIME_BUFFER;

			if (transfer_segment_time > transfer_limit_time) {

				tx_time_g = length;

				autosend.append(temp).append("\n").append(send_to);

				if(mode == TX_SEGMENTED) {
					autosend.append(" BK");
				} else {
					autosend.append(" ID");
				}

				autosend.append("\n");

				send_via_fldigi(autosend);

				if(mode == TX_SEGMENTED) {
					if (!wait_for_rx(transfer_segment_time * 10)) return false;
					wait_seconds(cnt_rx_internval_secs->value());
				}

				autosend.assign("\n");
				temp.clear();
				length = overhead;
			}
		}

		if(temp.size())
			autosend.append(temp);

		if(tail.size())
			autosend.append(tail);

		if(autosend.size()) {
			tx_time_g = seconds_from_string(modem, autosend, &overhead);
			tx_time_g += overhead;
			send_via_fldigi(autosend);
			if (!wait_for_rx(transfer_segment_time * 10)) return false;
		}

	} else {
		autosend.append("\n").append(temp);
		if(tail.size())
			autosend.append(tail);

		send_via_fldigi(autosend);

		if (!wait_for_rx(data_time_seconds * 10)) return false;
	}

	if(progStatus.use_header_modem) {
		if(progStatus.hamcast_mode_cycle && transmit_queue && g_event_driven) {
			std::string m;
			switch(modem_rotation_index) {
				case 0:
					send_new_modem(cbo_hamcast_mode_selection_1->value());
					break;
				case 1:
					send_new_modem(cbo_hamcast_mode_selection_1->value());
					break;
				case 2:
					send_new_modem(cbo_hamcast_mode_selection_1->value());
					break;
				case 3:
					send_new_modem(cbo_hamcast_mode_selection_1->value());
					break;
				default:
					send_new_modem(cbo_modes->value());
			}
		}
		else {
			send_new_modem(cbo_modes->value()); // Change to primary modem
		}
	}

	return true;
}

bool check_block_tx_time(cAmp *tx, TX_FLDIGI_THREAD *thread_ptr)
{
	if(!tx) return false;

	int index = 0;
	int count = 0;
	float max_tx_time = 0;
	float interval_time = (float) cnt_tx_internval_mins->value();
	float tx_time = 0;
	bool return_value = true;
	char str_buffer[256];
	string modem;
	std::vector<std::string> vector_data;

	vector_data = tx->xmt_vector_header();
	count = vector_data.size();

	modem.clear();
	if(progStatus.use_header_modem)
		modem.assign(cbo_header_modes->value());
	else
		modem.assign(cbo_modes->value());

	for(index = 0; index < count; index++) {
		tx_time = minutes_from_string(modem, vector_data[index], (float *)0);
		if(tx_time > max_tx_time) {
			max_tx_time = tx_time;
		}
	}

	if(max_tx_time > interval_time) {
		memset(str_buffer, 0, sizeof(str_buffer));
		snprintf(str_buffer, sizeof(str_buffer) - 1,
				 "Header Section Modem: %s\nRequired Block Tx time: %3.1f Mins.\nUse Faster Mode, Increase Interval Time,\nor decrease block size.", \
				 modem.c_str(), max_tx_time);
		LOG_INFO("%s", str_buffer);
		return_value = false;
		if(thread_ptr) {
			memset(&thread_ptr->err_msg[0], 0, THREAD_ERR_MSG_SIZE);
			strncpy(&thread_ptr->err_msg[0], str_buffer, THREAD_ERR_MSG_SIZE - 1);
			thread_ptr->err_flag = true;
		}
	}

	vector_data.clear();
	vector_data = tx->xmt_vector_data();

	modem.clear();
	modem.assign(cbo_modes->value());

	max_tx_time = 0;

	for(index = 0; index < count; index++) {
		tx_time = minutes_from_string(modem, vector_data[index], (float *)0);
		if(tx_time > max_tx_time) {
			max_tx_time = tx_time;
		}
	}

	if(max_tx_time > interval_time) {
		memset(str_buffer, 0, sizeof(str_buffer));
		snprintf(str_buffer, sizeof(str_buffer) - 1,
				 "Data Section Modem: %s\nRequired Block Tx time: %3.1f Mins.\nUse Faster Mode, Increase Interval Time,\nor decrease block size.", \
				 modem.c_str(), max_tx_time);
		LOG_INFO("%s", str_buffer);
		return_value = false;

		if(thread_ptr) {
			memset(&thread_ptr->err_msg[0], 0, THREAD_ERR_MSG_SIZE);
			strncpy(&thread_ptr->err_msg[0], str_buffer, THREAD_ERR_MSG_SIZE - 1);
			thread_ptr->err_flag = true;
		}
	}

	return return_value;
}

bool wait_for_rx(int max_wait_seconds)
{
	static std::string response;
	time_t eTime = time((time_t *)0) + 3;
	time_t sTime = 0;

	if(max_wait_seconds < 1) return false;

	do {
		MilliSleep(500);

		response = get_trx_state();

		if(response == "TX") break;

		if(transmit_stop) return false;

	} while (time((time_t *)0) < eTime);

	MilliSleep(500);

	sTime = time((time_t *)0);
	eTime = sTime + max_wait_seconds;

	do {
		MilliSleep(500);

		response = get_trx_state();

		if(response == "RX") {
			return true;
		}

		if(transmit_stop) break;

	} while (time((time_t *)0) < eTime);

	return false;
}

void wait_seconds(int seconds)
{
	time_t eTime = time((time_t *)0) + seconds + 1;

	if(seconds < 1) return;

	while (time((time_t *)0) < eTime) {
		MilliSleep(500);
	}
}

TX_FLDIGI_THREAD * run_in_thread(void *(*func)(void *), int mode, bool queued)
{
	TX_FLDIGI_THREAD *tx_thread = (TX_FLDIGI_THREAD *)0;
	int count = 0;

	if(transmitting) return tx_thread;

	tx_thread = new TX_FLDIGI_THREAD;

	if(tx_thread) {

		tx_thread->err_flag = false;
		tx_thread->mode = mode;
		tx_thread->que  = queued;

		if(progStatus.hamcast_mode_cycle && g_event_driven && queued) {
			if(modem_rotation_index >= bc_modems.size())
				modem_rotation_index = 0;

			tx_thread->modem.assign(bc_modems[modem_rotation_index]);
			modem_rotation_index++;

		} else {
			tx_thread->modem.assign(g_modem);
		}

		tx_thread->header_modem.assign(g_header_modem);

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

void * run_in_thread_destroy(TX_FLDIGI_THREAD *tx_thread, int level)
{
	if(!tx_thread) return 0;

	{
		static int value = TX_BUTTON;
		Fl::awake(deactivate_button, (void *) &value);
	}

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


	Fl::awake(set_button_to_xmit, (void *)0);
	Fl::awake(clear_missing, (void *)0);

	wait_for_rx(5);

	{
		static int value = TX_BUTTON;
		Fl::awake(activate_button, (void *)&value);
		static int value2 = TX_ALL_BUTTON;
		Fl::awake(activate_button, (void *)&value2);
	}

	//if (transmit_stop == true) send_abort();

	transmitting = false;
	transmit_queue = false;

	return 0;
}

void abort_and_id(void)
{
	std::string idMessage;
	send_abort();
	send_abort();

	if(!generate_time_table) {
		// A number of non printable characters are required to overcome long interleave modems.
		idMessage.assign("\n\n\n\n\n\n\n\n\n\n\nFILE TRANSFER ABORTED\n\nDE ").append(progStatus.my_call).append(" BK\n\n\n");
		send_via_fldigi(idMessage);
	}
}

void abort_request(void)
{
	int response = fl_choice("Terminate Current Transmission?", "No", "Yes", NULL);
	if (response == 1) {
		static int value = TX_BUTTON;
		deactivate_button((void *) &value);
		transmit_stop = true;
		ztime_end = 0;
		continuous_exception = false;
		abort_and_id();
	}
}

void send_fldigi_modem(void *ptr)
{
	if(!ptr) return;
	int *val = (int *) ptr;

	char buffer[32];

	memset(buffer, 0, sizeof(buffer));

	switch( *val ) {
		case HEADER_MODEM:
			strncpy(buffer, cbo_header_modes->value(), sizeof(buffer) - 1);
			break;

		case DATA_MODEM:
			strncpy(buffer, cbo_modes->value(), sizeof(buffer) - 1);
			break;

		default: return;
	}

	send_new_modem(buffer);
}

void get_trx_state_in_main_thread(void *ptr)
{
	if (!ptr) return;

	std::string *str = (std::string *)ptr;

	*str = get_trx_state();
}

void transmit_queue_main_thread(void *ptr)
{
	transmit_queued(true);
}

void send_via_fldigi_in_main_thread(void *ptr)
{
	if (!ptr) return;

	string *data = (string *)ptr;

	if (!bConnected) connect_to_fldigi(0);
	if (!bConnected) return;

	send_via_fldigi(*data);
}

void deactivate_button(void *ptr)
{
	if (!ptr) return;
	int *value = (int *) ptr;

	if( *value == TX_BUTTON )
		btn_send_file->deactivate();
	else if( *value ==TX_ALL_BUTTON )
		btn_send_queue->deactivate();
}

void activate_button(void *ptr)
{
	if (!ptr) return;
	int * value = (int *) ptr;

	if( *value == TX_BUTTON )
		btn_send_file->activate();
	else if( *value == TX_ALL_BUTTON )
		btn_send_queue->activate();
}

void set_button_to_xmit(void *ptr)
{
	btn_send_file->label(XMT_LABEL);
}

void set_button_to_cancel(void *ptr)
{
	btn_send_file->label(CANX_LABEL);
}

void thread_error_msg(void *data)
{
	if(data) {
		fl_alert2("%s", (const char *)data);
		free(data);
	}
}

void set_xmit_label(void *data)
{
	if(data) {
		btn_send_file->label((const char *)data);
		free(data);
	}
}

void receive_data_stream()
{
	//if(bConnected) // Prevents cQue->sleep() from reaching the desired delay
	//	cQue->signal();
}

int alt_receive_data_stream(void *ptr)
{
	static char buffer[CNT_BLOCK_SIZE_MAXIMUM];
	int n = 0;
	int size = sizeof(buffer) - 1;

	if (!bConnected) {
		connect_to_fldigi(0);
		if (!bConnected) {
			cQue->sleep(2, 0);
			return 0;
		}
	}
	else {
		n = rx_fldigi(buffer, size);
		if(n < 1) {
			cQue->milliSleep(50);
			return 0;
		}

		cQue->addToQueueNullFiltered(buffer, n);
	}

	return n;
}

int process_que(void *ptr)
{
	size_t readCount = 0, count = 0;
	size_t oldCount = -1;
	static char buffer[CNT_BLOCK_SIZE_MAXIMUM + 128];
	static char tagBuffer[32];
	static char aTag[32];
	static char sz_missing[] = "MISSING";
	unsigned int chksum = 0;
	int argCount = 0;
	int reset = 0;
	time_t tm_time = 0;

	unsigned int crcVal = 0;

	Circular_queue *que = (Circular_queue *)ptr;

	memset(buffer, 0, sizeof(buffer));

	count = 20;
	oldCount = -1;

	while(!que->thread_exit()) {
		readCount = que->lookAheadToTerminator(tagBuffer, '>', count);

		if(readCount >=  count) break;
		if(readCount > 0 && tagBuffer[readCount - 1] == '>') break;

		if(oldCount != readCount) // Reset timer
			que->timeOut(tm_time, 0, TIME_SET);
		else
			que->milliSleep(100);

		if(que->timeOut(tm_time, 10, TIME_COUNT)) { // In seconds
			return 0;
		}

		oldCount = readCount;
	}

	if(tagBuffer[readCount - 1] != '>')
		return que->adjustReadQueIndex(1);

	int temp;
	argCount = sscanf(tagBuffer, "<%s %d %x>", aTag, &temp, &chksum);
	count = temp;

	if(argCount < 3 || count < 1)
		return que->adjustReadQueIndex(1);

	que->adjustReadQueIndex(readCount);

	if(count > (sizeof(buffer) - 1))
		count = sizeof(buffer) - 1;

	crcVal = 0;
	oldCount = -1;
	tm_time = 0;

	while(!que->thread_exit()) {
		reset = 1;
		readCount = que->lookAheadCRC(buffer, count, &crcVal, &reset);

		if(readCount >= count) break;

		if(oldCount != readCount) // Reset timer
			que->timeOut(tm_time, 0, TIME_SET);
		else
			que->milliSleep(100);

		if(que->timeOut(tm_time, 10, TIME_COUNT)) { // In seconds
			return 0;
		}

		oldCount = readCount;
	}

	if(chksum != crcVal)
		return que->adjustReadQueIndex(1);

	tmp_buffer.assign(tagBuffer);
	tmp_buffer.append(buffer, count);

	if(memcmp(aTag, sz_missing, sizeof(sz_missing)) == 0)
		process_missing_stream();
	else
		process_data_stream();

	return que->adjustReadQueIndex(readCount);

}

void process_missing_stream(void)
{
	string retbuff;

	char tag[32];
	char crc[5];
	char hash[5];
	int len = 0;
	int conv = 0;
	string tx_hash;
	size_t i = 0;

	retbuff.assign(tmp_buffer);

	conv = sscanf(retbuff.c_str(), "<%s %d %4s>{%4s", tag, &len, crc, hash);

	if (conv == 4) {
		for (i = 0; i < tx_array.size(); i++) {
			if(!tx_array[i]) continue;
			tx_hash = tx_array[i]->xmt_hash();
			if (memcmp((const char *)hash, (const char *)tx_hash.c_str(), sizeof(hash) - 1) == 0) {
				tx_array[i]->append_report(retbuff);
			}
		}
	}
}


void process_data_stream(void)
{
	string retbuff;

	char tag[32];
	char crc[5];
	char hash[5];
	int len;
	int conv;

	size_t i = 0;

	retbuff.assign(tmp_buffer);

	conv = sscanf(retbuff.c_str(), "<%s %d %4s>{%4s", tag, &len, crc, hash);

	if (conv == 4) {

		cAmp *existing = 0;

		for (i = 0; i < rx_array.size(); i++) {
			if (rx_array[i]->hash(hash)) {
				existing = rx_array[i];
				break;
			}
		}

		if (!existing) { // a new rx process

			cAmp *nu = new cAmp();
			nu->rx_hash(hash);
			nu->rx_append(retbuff);
			nu->rx_parse_buffer();
			rx_array.push_back(nu);
			string s;
			s.assign("@f").append(nu->rx_sz_percent()).append("\t");
			s.append(nu->get_rx_fname());
			rx_queue->add(s.c_str());
			rx_queue->select(rx_queue->size());

			amp.rx_cAmp(nu);

			clear_rx_amp();
			show_rx_amp();
			LOG_INFO("New Amp instance: %s", nu->get_rx_fname().c_str());

		} else {

			string bline;
			if (!existing->rx_completed()) {
				existing->rx_append(retbuff);
				existing->rx_parse_buffer();
				bline.assign("@f").append(existing->rx_sz_percent()).append("\t").append(existing->get_rx_fname());
				rx_queue->text(i+1, bline.c_str());
			}
		}
	}

	if (rx_array.size() != 0)
		show_rx_amp();
}

void receive_remove_from_queue()
{
	if (rx_queue->size()) {
		int n = rx_queue->value();
		cAmp * rx_amp = rx_array[n - 1];

		amp.rx_cAmp((cAmp *)0);

		if (rx_amp) {
			rx_array.erase(rx_array.begin() + n - 1);
		}

		amp.free_cAmp(rx_amp);

		rx_queue->remove(n);

		if (rx_queue->size()) {
			n = 1;
			amp.rx_cAmp(rx_array[n - 1]);
			rx_queue->select(n);
			show_selected_rcv(n);
		} else {
			txt_rx_filename->value("");
			txt_rx_datetime->value("");
			txt_rx_descrip->value("");
			txt_rx_callinfo->value("");
			txt_rx_filesize->value("");
			txt_rx_numblocks->value("");
			txt_rx_missing_blocks->value("");
			rx_progress->clear();
			txt_rx_output->clear();
		}
		rx_queue->redraw();
	}
}

void transfer_time(std::string modem, float &cps, int &transfer_size, std::string buffer)
{
	transfer_size = buffer.length();

	if (transfer_size == 0) {
		return;
	}

	st_modes *stm = s_modes;
	while (stm->f_cps && (stm->s_mode != modem)) stm++;
	if (!stm->f_cps) return;

	if (stm->s_mode.find("MT63") != string::npos) {
		for (size_t j = 0; j < buffer.length(); j++)
			if ((buffer[j] & 0x80) == 0x80) transfer_size += 3;
	}

	cps = stm->f_cps;

}

void estimate() {
	int n = tx_queue->value();
	if (!n) return;

	static char sz_xfr_size[50];
	float xfr_time = 0;
	float oh = 0;
	int transfer_size;

	cAmp *tx = tx_array[n-1];

	string xmtstr = tx_string(tx, CALLSIGN_PREAMBLE | CALLSIGN_POSTAMBLE, " K\n");

	transfer_size = xmtstr.length();

	if (transfer_size == 0) {
		txt_transfer_size_time->value("");
		return;
	}

	xfr_time = seconds_from_string(cbo_modes->value(), xmtstr, &oh);
	xfr_time += oh;

	if (xfr_time < 60)
		snprintf(sz_xfr_size, sizeof(sz_xfr_size), "%d bytes / %d secs",
				 transfer_size, (int)(xfr_time + 0.5));
	else
		snprintf(sz_xfr_size, sizeof(sz_xfr_size), "%d bytes / %d m %d s",
				 transfer_size,
				 (int)(xfr_time / 60), ((int)xfr_time) % 60);
	txt_transfer_size_time->value(sz_xfr_size);

	show_selected_xmt(n);
}

void estimate_bc(void) {
	static char sz_xfr_size[50];
	float xfr_time = 0;
	float oh = 0;
	float total_xfr_time = 0;
	int index = 0;
	int modem_index = 0;
	int modem_index_count = 0;
	int count = 0;
	bool flag = 0;

	cAmp *tx = 0;
	std::string md;
	std::string temp;
	std::string xmtstr;

	count = tx_array.size();
	total_xfr_time = 0;
	modem_index_count = BROADCAST_MAX_MODEMS;

	xmtstr.clear();

	for(index = 0; index < count; index++) {
		tx = tx_array[index];
		xmtstr.append(tx_string(tx, CALLSIGN_POSTAMBLE | CALLSIGN_POSTAMBLE, " K\n"));
	}

	xfr_time = 0;

	for(modem_index = 0; modem_index < modem_index_count; modem_index++) {
		switch(modem_index) {
			case 0:
				md.assign(cbo_hamcast_mode_selection_1->value());
				break;
			case 1:
				md.assign(cbo_hamcast_mode_selection_2->value());
				break;
			case 2:
				md.assign(cbo_hamcast_mode_selection_3->value());
				break;
			case 3:
				md.assign(cbo_hamcast_mode_selection_4->value());
				break;
		}

		xfr_time = seconds_from_string(md, xmtstr, &oh);
		xfr_time += oh;

		if (xfr_time < 60)
			snprintf(sz_xfr_size, sizeof(sz_xfr_size), "%d secs",
					 (int)(xfr_time + 0.5));
		else
			snprintf(sz_xfr_size, sizeof(sz_xfr_size), "%d m %d s",
					 (int)(xfr_time / 60), ((int)xfr_time) % 60);

		flag = false;
		switch(modem_index) {
			case 0:
				if(btn_hamcast_mode_enable_1->value())
					flag = true;
				txt_hamcast_select_1_time->value(sz_xfr_size);
				break;
			case 1:
				if(btn_hamcast_mode_enable_2->value())
					flag = true;
				txt_hamcast_select_2_time->value(sz_xfr_size);
				break;
			case 2:
				if(btn_hamcast_mode_enable_3->value())
					flag = true;
				txt_hamcast_select_3_time->value(sz_xfr_size);
				break;
			case 3:
				if(btn_hamcast_mode_enable_4->value())
					flag = true;
				txt_hamcast_select_4_time->value(sz_xfr_size);
				break;
		}

		if(flag)
			total_xfr_time += xfr_time;
	}

	if (total_xfr_time < 60)
		snprintf(sz_xfr_size, sizeof(sz_xfr_size), "%d secs",
				 (int)(total_xfr_time + 0.5));
	else
		snprintf(sz_xfr_size, sizeof(sz_xfr_size), "%d m %d s",
				 (int)(total_xfr_time / 60), ((int)total_xfr_time) % 60);

	txt_hamcast_select_total_time->value(sz_xfr_size);
}

void cb_exit()
{
	if(transmitting) {
		transmit_stop = true;
	}

	progStatus.saveLastState();
	FSEL::destroy();
	cAmp *amp;
	int size = tx_array.size();
	for (int i = 0; i < size; i++) {
		amp = tx_array[i];
		delete amp;
		tx_array.pop_back();
	}
	size = rx_array.size();
	for (int i = 0; i < size; i++) {
		amp = rx_array[i];
		delete amp;
		rx_array.pop_back();
	}
	if (tcpip) {
		tcpip->close();
		delete tcpip;
		delete localaddr;
	}
	debug::stop();

	if(cQue) delete cQue;

	exit(0);
}

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

int parse_args(int argc, char **argv, int& idx)
{
	char check_for[48];

	if (strcasecmp(argv[idx], "--time-table") == 0) {
		generate_time_table = true;
		idx++;
		return 1;
	}

	if (strstr(argv[idx], "--flamp-dir")) {
		idx++;
		// ignore if already set via NBEMS.DIR file contents
		if (!flampHomeDir.empty()) return 1;
		string tmp = argv[idx];
		if (!tmp.empty()) flampHomeDir = tmp;
		size_t p = string::npos;
		while ( (p = flampHomeDir.find("\\")) != string::npos)
			flampHomeDir[p] = '/';
		if (flampHomeDir[flampHomeDir.length()-1] != '/')
			flampHomeDir += '/';
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

void exit_main(Fl_Widget *w)
{
	if (Fl::event_key() == FL_Escape)
		return;
	cb_exit();
}

int main(int argc, char *argv[])
{
	string appname = argv[0];
	{
		string appdir;
		char apptemp[FL_PATH_MAX];
		fl_filename_expand(apptemp, sizeof(apptemp), appname.c_str());
		appdir.assign(apptemp);

#ifdef __WOE32__
		size_t p = appdir.rfind("flamp.exe");
		appdir.erase(p);
#else
		size_t p = appdir.rfind("flamp");
		if (appdir.find("./flamp") != std::string::npos) {
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
					flampHomeDir = dirline;
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

	string debug_file = flampHomeDir;
	debug_file.append("debug_log.txt");
	debug::start(debug_file.c_str());

	LOG_INFO("Application: %s", appname.c_str());
	LOG_INFO("Base dir: %s", BaseDir.c_str());

	main_window = flamp_dialog();
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
		perror("pthread_create");
		exit(EXIT_FAILURE);
	}

	try {
		cQue = new TagSearch(alt_receive_data_stream, process_que);
	} catch (const TagSearchException& e) {
		LOG_ERROR("%d, %s", e.error(), e.what());
		exit (EXIT_FAILURE);
	}

	cQue->resumeQueue();

	txt_tx_mycall->value(progStatus.my_call.c_str());
	txt_tx_myinfo->value(progStatus.my_info.c_str());

	cnt_blocksize->value(progStatus.blocksize);
	cnt_repeat_nbr->value(progStatus.repeatNN);
	cnt_repeat_header->value(progStatus.repeat_header);

	tx_buffer.clear();
	rx_buffer.clear();

	ztimer((void *)true);

	if(progStatus.auto_load_queue) {
		auto_load_tx_queue();
	}

	unproto_widgets();

	return Fl::run();
}

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

void show_help()
{
	open_url("http://www.w1hkj.com/flamp2.1-help/index.html");
}

void cb_folders()
{
	open_url(flampHomeDir.c_str());
}

void drop_file_changed()
{
	string buffer = Fl::event_text();
	size_t n = 0;
	size_t limit = 0;
	int valid = 0;

	char *cPtr = (char *)0;
	char *cFileName = (char *)0;
	const char *cBufferEnd = (char *)0;

	drop_file->value("  DnD");
	drop_file->redraw();

	if ((n = buffer.find("file:///")) != string::npos)
		buffer.erase(0, n + 7);

	n = buffer.find(":\\");
	if(n == string::npos)
		n = buffer.find("/");

	if(n != string::npos) {
		valid = 1;
		limit = buffer.size();
		cBufferEnd = &((buffer.c_str())[limit]);
		cFileName = cPtr = (char *) buffer.c_str();

		while(cPtr < cBufferEnd) {

			// Skip though the current filename path to find delimters
			while(*cPtr >= ' ' && cPtr < cBufferEnd)
				cPtr++;

			// Null terminate CR/LF delimiters
			while(*cPtr == '\n' || *cPtr == '\r') {
				*cPtr = 0;
				cPtr++;
				if(cPtr >= cBufferEnd) break;
			}

			if(valid)
				addfile(cFileName, 0, false, 0);

			// Remove any leading spaces and control characters
			while(*cPtr <= ' ' && cPtr < cBufferEnd)
				cPtr++;

			// Reassign a possible new filename path.
			cFileName = cPtr;
			valid = 0;

			// Minimal validation check on system specific file name path sturcture
			while(cPtr < cBufferEnd) {
				if(*cPtr == '/') { // Unix/BSD type file systems
					valid = 1;
					break;
				}

				if(cPtr[0] == ':' && cPtr[1] == '\\') { // Windows file system
					valid = 1;
					break;
				}
				cPtr++;
			}
		}
	}
}

