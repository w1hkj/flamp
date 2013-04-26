// =====================================================================
//
// flamp.cxx
//
// Author: Dave Freese, W1HKJ
// Copyright: 2010, 2011
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  It is
// copyright under the GNU General Public License.
//
// You should have received a copy of the GNU General Public License
// along with the program; if not, write to the Free Software
// Foundation, Inc.
// 59 Temple Place, Suite 330
// Boston, MA  02111-1307 USA
//
// =====================================================================

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

#include "config.h"


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
#include "circular_queue.h"

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

string rx_buffer;
string rx_rotary;
string tx_buffer;
string tmp_buffer;

static Circular_queue *cQue;

const char *searchTags[] = {
	"<FILE ",
	"<ID ",
	"<DESC ",
	"<DATA ",
	"<PROG ",
	"<CNTL ",
	"<SIZE ",
	(char *)0
};

int search_tag_count = (sizeof(searchTags) / sizeof(char *)) - 1;

string testfname = "Bulletin1.txt";

bool testing = true;
bool transmit_selected = false;
bool transmit_queue = false;

int blocksize = 64;
int repeatNN = 1;

const char *options[] = {\
"flamp unique options",
"  --help",
"  --version",
"  --flamp-dir folder-path-name (including drive letter on Windows)",
"      Windows: C:/Documents and Settings/<username>/folder-name",
"               C:/Users/<username/folder-name",
"               H:/hamstuff/folder-name",
"      Linux:   /home/<username>/folder-name",
"      OS X:    /home/<username>/folder-name",
"",
"      note that enclosing \"'s must be used when the path or file name",
"      contains spaces.",
"",
"    Unless the empty file NBEMS.DIR is found in the same folder as the",
"    flamp executable.  The existence of an empty file NBEMS.DIR forces",
"    the flamp-dir to be a placed in the same folder as the executable",
"    folder and named flamp.files (.flamp on Linux / OS X)",
"",
"    NBEMS.DIR may contain a single line specifying the absolute",
"    path-name of the flamp-dir.  If NBEMS.DIR is not empty then",
"    the specified path-name takes precedence over the --flamp-dir",
"    command line parameter.  The specified path-name must be a valid",
"    one for the operating system!  Enclosing \"'s are not required.",
"=======================================================================",
"Fltk User Interface options",
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
pthread_mutex_t mutex_xmlrpc = PTHREAD_MUTEX_INITIALIZER;

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

cAmp *tx_amp = 0;
cAmp *rx_amp = 0;


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

static char szoutTimeValue[] = "12:30:00";
static char sztime[] = "123000";
static int  ztime;

void ztimer(void* first_call)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	if (first_call) {
		double st = 1.0 - tv.tv_usec / 1e6;
		Fl::repeat_timeout(st, ztimer);
	} else
		Fl::repeat_timeout(1.0, ztimer);

	struct tm tm;
	time_t t_temp;

	t_temp=(time_t)tv.tv_sec;
	gmtime_r(&t_temp, &tm);

	if (!strftime(sztime, sizeof(sztime), "%H%M%S", &tm))
		strcpy(sztime, "000000");
	ztime = atoi(sztime);

	if (!strftime(szoutTimeValue, sizeof(szoutTimeValue), "%H:%M:%S", &tm))
		memset(szoutTimeValue, 0, sizeof(szoutTimeValue));

	outTimeValue->value(szoutTimeValue);

	if (do_events->value()) {
		if (progStatus.repeat_at_times && (ztime % 100 == 0)) {
			switch (progStatus.repeat_every) {
				case 0 : // every 5 minutes
					if (ztime % 500 == 0) transmit_queued();
					break;
				case 1 : // every 15 minutes
					if (ztime % 1500 == 0) transmit_queued();
					break;
				case 2 : // every 30 minutes
					if (ztime % 3000 == 0) transmit_queued();
					break;
				case 3 : // hourly
					if (ztime % 10000 == 0) transmit_queued();
					break;
				case 4 : // even hours
					if (ztime == 0 || ztime == 20000 || ztime == 40000 ||
						ztime == 60000 || ztime == 80000 || ztime == 100000 ||
						ztime == 120000 || ztime == 140000 || ztime == 160000 ||
						ztime == 180000 || ztime == 200000 || ztime == 220000 )
						transmit_queued();
					break;
				case 5 : // odd hours
					if (ztime == 10000 || ztime == 30000 || ztime == 50000 ||
						ztime == 70000 || ztime == 90000 || ztime == 110000 ||
						ztime == 130000 || ztime == 150000 || ztime == 170000 ||
						ztime == 190000 || ztime == 210000 || ztime == 230000 )
						transmit_queued();
					break;
				case 6 : // at specified times
					{
						char ttime[] = "000000";
						snprintf(ttime, sizeof(ttime), "%06d", ztime);
						ttime[4] = 0;
						if (progStatus.repeat_times.find(ttime) != std::string::npos)
							transmit_queued();
					}
					break;
				case 7 : // One time scheduled
					{
						char ttime[] = "000000";
						snprintf(ttime, sizeof(ttime), "%06d", ztime);
						ttime[4] = 0;
						if (progStatus.repeat_times.find(ttime) != std::string::npos) {
							transmit_queued();
							size_t p = progStatus.repeat_times.find(ttime);
							int len = 4;
							while ((p + len < progStatus.repeat_times.length()) &&
									!isdigit(progStatus.repeat_times[p + len])) len++;
							progStatus.repeat_times.erase(p, len);
							txt_repeat_times->value(progStatus.repeat_times.c_str());
							if (progStatus.repeat_times.empty()) {
								do_events->value(0);
								cb_do_events((Fl_Light_Button *)0, (void*)0);
							}
						}
					}
					break;
				default : // do nothing
					break;
			}
		} else if (progStatus.repeat_forever) {
			try {
				if (get_trx_state() == "RX")
					transmit_queued();
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

void addfile(string xmtfname, void *rx)
{
	xmt_fname = xmtfname;
	cAmp *rAmp = (cAmp *) rx;

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
	txt_tx_filename->value(xmt_fname.c_str());
	if (isbinary(tx_buffer) && !progStatus.use_compression) {
		fl_alert2("Suggest using compression on this file");
	}
	cAmp *nu = new cAmp(tx_buffer, fl_filename_name(xmt_fname.c_str()));
	struct stat statbuf;
	stat(xmt_fname.c_str(), &statbuf);
	nu->time_stamp(&statbuf.st_mtime);
	nu->xmt_descrip("");
	tx_array.push_back(nu);
	tx_queue->add(xmt_fname.c_str());
	tx_queue->select(tx_queue->size());
	nu->tx_base_conv_index(encoders->index() + 1);
	nu->tx_base_conv_str(encoders->value());

	tx_amp = nu;

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

void readfile()
{
	string xmtfname;
	xmtfname.clear();
	const char *p = FSEL::select(_("Open file"), "any file\t*.*",
					xmtfname.c_str());
	if (!p) return;
	if (strlen(p) == 0) return;
	xmtfname = p;

	addfile(xmtfname, 0);
}

void show_selected_xmt(int n)
{
	if (!n) return;
	tx_amp = tx_array[n-1];
	string fn = tx_amp->xmt_fname();
	string ds = tx_amp->xmt_descrip();
	string ns = tx_amp->xmt_numblocks();
	string ts = tx_amp->xmt_tosend();
	txt_tx_filename->value(fn.c_str());
	txt_tx_descrip->value(ds.c_str());
	txt_tx_selected_blocks->value(ts.c_str());
	txt_tx_numblocks->value(ns.c_str());
	btn_use_compression->value(tx_amp->compress());
	progStatus.encoder = tx_amp->tx_base_conv_index();
	encoders->index(progStatus.encoder - 1);
	tx_buffer = tx_amp->xmt_data();
}

void show_rx_amp()
{
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

void show_selected_rcv(int n)
{
	if (!n) return;
	rx_amp = rx_array[n-1];
	txt_rx_output->clear();
	show_rx_amp();
}

void send_missing_report()
{
	string fname = txt_rx_filename->value();
	if (fname.empty()) return;
	string report("\nDE ");
	report.append(txt_tx_mycall->value());
	report.append("\nFile : ").append(fname).append("\n");
	report.append(rx_amp->rx_report());
	report.append("DE ").append(txt_tx_mycall->value()).append(" K \n");
	send_report(report);
}

void update_selected_xmt()
{
	if (!tx_amp) return;
	tx_amp->xmt_descrip(txt_tx_descrip->value());
	tx_amp->xmt_tosend(txt_tx_selected_blocks->value());
	tx_amp->compress(btn_use_compression->value());
	tx_amp->tx_base_conv_index(progStatus.encoder);
	tx_amp->tx_base_conv_str(encoders->value());
}

void recv_missing_report()
{
	string rx_data;
	try {
		rx_data = get_rx_data();
		LOG_DEBUG("%s", rx_data.c_str());
	} catch (...) {
		return;
	}

	for (size_t num = 0; num < tx_array.size(); num++)
		tx_array[num]->tx_parse_report(rx_data);

	if (!tx_amp) return;
	txt_tx_selected_blocks->value(tx_amp->xmt_tosend().c_str());
}

void tx_removefile()
{
	tx_buffer.clear();
	txt_tx_numblocks->value("");
	txt_tx_filename->value("");
	txt_tx_descrip->value("");
	txt_transfer_size_time->value("");
	txt_tx_selected_blocks->value("");

	if (!tx_amp) return;
	if (!tx_array.size()) return;
	if (!tx_queue->size()) return;
	int n = tx_queue->value();
	if (!n) return;
	tx_queue->remove(n);
	n--;
	tx_amp = tx_array[n];
	delete tx_amp;
	tx_amp = 0;
	tx_array.erase(tx_array.begin() + n);
	if (tx_queue->size()) {
		tx_queue->select(1);
		estimate();
	}
}

void writefile(int xfrFlag)
{
	if (!rx_amp) return;
	size_t fsize = rx_amp->rx_size();

	if(xfrFlag && !rx_amp->rx_completed()) {
   		fl_alert2("Only completed files can be transfered");
   		return;
	}

	if (!fsize || rx_amp->get_rx_fname().empty()) return;

	rx_fname.assign(flampHomeDir).append(rx_amp->get_rx_fname());
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
		addfile(rx_fname, rx_amp);
	}
}


int parse_args(int argc, char **argv, int& idx)
{
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
	if (strcasecmp(argv[idx], "--help") == 0) {
		int i = 0;
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

void transmit_current()
{
	if (!bConnected) connect_to_fldigi(0);
	if (!bConnected) return;

	int n = tx_queue->value();
	if (!n) return;

	if (progStatus.fldigi_xmt_mode_change)
		send_new_modem();

	cAmp *tx = tx_array[n-1];
	string temp;
	string autosend;
	string send_to = txt_tx_send_to->value();
	if (send_to.empty()) send_to.assign("QST");
	send_to.append(" DE ").append(progStatus.my_call);
	for (size_t n = 0; n < send_to.length(); n++)
		send_to[n] = toupper(send_to[n]);

	autosend.assign("\n\n").append(send_to).append("\n");

	if (tx->xmt_buffer().empty()) return;

	tx->my_call(progStatus.my_call);
	tx->my_info(progStatus.my_info);
	tx->xmt_blocksize(progStatus.blocksize);

	temp.assign(tx->xmt_buffer());
	compress_maybe(temp, tx->tx_base_conv_index(), tx->compress());//true);
	tx->xmt_data(temp);
	tx->repeat(progStatus.repeatNN);
	tx->header_repeat(progStatus.repeat_header);

	autosend.append(tx->xmt_string());

	autosend.append(send_to).append(" K\n");

	send_via_fldigi(autosend);

	transmit_selected = false;
}

void transmit_queued()
{
	if (!bConnected) connect_to_fldigi(0);
	if (!bConnected) return;

	if (tx_array.size() == 0) return;

	if (progStatus.fldigi_xmt_mode_change)
		send_new_modem();

	cAmp *tx;
	string temp;
	string autosend;
	string send_to = txt_tx_send_to->value();
	if (send_to.empty()) send_to.assign("QST");
	send_to.append(" de ").append(progStatus.my_call);
	for (size_t n = 0; n < send_to.length(); n++)
		send_to[n] = toupper(send_to[n]);

	autosend.assign("\n\n").append(send_to).append("\n");

	for (size_t num = 0; num < tx_array.size(); num++) {
		tx = tx_array[num];
		if (tx->xmt_buffer().empty()) return;

		tx->my_call(progStatus.my_call);
		tx->my_info(progStatus.my_info);
		tx->xmt_blocksize(progStatus.blocksize);

		temp.assign(tx->xmt_buffer());
		compress_maybe(temp, tx->tx_base_conv_index(), tx->compress());//true);
		tx->xmt_data(temp);
		tx->repeat(progStatus.repeatNN);
		tx->header_repeat(progStatus.repeat_header);
		autosend.append(tx->xmt_string()).append("\n");
	}

	autosend.append(send_to).append(" K\n");

	send_via_fldigi(autosend);

	transmit_queue = false;
}

void receive_data_stream()
{
	//if(bConnected) // Prevent cQue->sleep() from reaching the desired deplay
    //    cQue->signal();
}

int alt_receive_data_stream(void *ptr)
{
	static char buffer[CNT_BLOCK_SIZE_MAXIMUM];
	int n = 0;
	int size = sizeof(buffer) - 1;
    static time_t tm_time = 0;
    
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
            
            if(n > 0) {
                cQue->timeOut(tm_time, 0, TIME_SET);
            } else {
                if(cQue->timeOut(tm_time, 10, TIME_COUNT))
                    connect_to_fldigi(0);
            }
			return 0;
		}

		cQue->addToQueue(buffer, n);
	}

	return n;
}

int process_que(void *ptr, char *tag)
{
	size_t readCount = 0, count = 0;
	size_t oldCount = -1;
	static char buffer[CNT_BLOCK_SIZE_MAXIMUM + 128];
	static char tagBuffer[32];
	static char aTag[32];
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

	process_data_stream();

	return que->adjustReadQueIndex(readCount);

}

void process_data_stream(void)
{
	string retbuff;

	char tag[20];
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
			rx_amp = nu;
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
		rx_amp = rx_array[n-1];
		if (rx_amp) {
			rx_array.erase(rx_array.begin() + n - 1);
			delete rx_amp;
		}
		rx_queue->remove(n);
		if (rx_queue->size()) {
			rx_queue->select(1);
			show_selected_rcv(1);
		}
		rx_queue->redraw();
	}

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

void estimate() {
	int n = tx_queue->value();
	if (!n) return;

	static char sz_xfr_size[50];
	float cps = 0, xfr_time = 0;
	int transfer_size;

	cAmp *tx = tx_array[n-1];
	tx->xmt_blocksize(progStatus.blocksize);
	tx->my_call(progStatus.my_call);
	tx->my_info(progStatus.my_info);

	string temp;
	temp.assign(tx->xmt_buffer());
	compress_maybe(temp, tx->tx_base_conv_index(), tx->compress());//true);
	tx->xmt_data(temp);
	tx->repeat(progStatus.repeatNN);
	tx->header_repeat(progStatus.repeat_header);

	string xmtstr = tx->xmt_string();
	transfer_size = xmtstr.length();

	if (transfer_size == 0) {
		txt_transfer_size_time->value("");
		return;
	}

	st_modes *stm = s_modes;
	while (stm->f_cps && (stm->s_mode != cbo_modes->value())) stm++;
	if (!stm->f_cps) return;

	if (stm->s_mode.find("MT63") != string::npos) {
		for (size_t j = 0; j < xmtstr.length(); j++)
			if ((xmtstr[j] & 0x80) == 0x80) transfer_size += 3;
	}

	cps = stm->f_cps;

	if (transfer_size <= 0) return;

	xfr_time = transfer_size / cps;
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

void cb_exit()
{
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

	localaddr = new Address(progStatus.socket_addr.c_str(), progStatus.socket_port.c_str());
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
		cQue = new Circular_queue(18, searchTags, search_tag_count, process_que, alt_receive_data_stream);
	} catch (const CircQueException& e) {
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

	return Fl::run();
}

void open_url(const char* url)
{
LOG_INFO("%s", url);
#ifndef __WOE32__
	const char* browsers[] = {
#  ifdef __APPLE__
		getenv("FLDIGI_BROWSER"), // valid for any OS - set by user
		"open"					// OS X
#  else
		"fl-xdg-open",			// Puppy Linux
		"xdg-open",			   // other Unix-Linux distros
		getenv("FLDIGI_BROWSER"), // force use of spec'd browser
		getenv("BROWSER"),		// most Linux distributions
		"sensible-browser",
		"firefox",
		"mozilla"				 // must be something out there!
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
	open_url("http://www.w1hkj.com/flamp2.0-help/index.html");
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
				addfile(cFileName, 0);

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
