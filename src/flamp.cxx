/** ********************************************************
 *
 ***********************************************************/

#include "config.h"

static const char *copyright[] = {
	" =====================================================================",
	"",
	" FLAMP "  VERSION, // flamp.cxx
	"",
	" Author(s):",
	"    Robert Stiles, KK5VD, Copyright (C) 2013, 2014, 2015",
	"    Dave Freese, W1HKJ, Copyright (C) 2012, 2013, 2014, 2015",
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
#include "ztimer.h"
#include "script_parsing.h"
#include "global_amp.h"
#include "transmit_camp.h"

#ifdef WIN32
#  include "flamprc.h"
#  include "compat.h"
#endif

#include <FL/filename.H>

#include <FL/x.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Image.H>

using namespace std;

std::string rx_buffer;
std::string tx_buffer;
std::string tmp_buffer;

const char *sz_flmsg = "<flmsg>";
const char *sz_cmd = "<cmd>";
const char *sz_flamp = "}FLAMP";

TagSearch *cQue;


std::string testfname = "Bulletin1.txt";

bool testing = true;
bool transmitting = false;

bool transmit_stop = false;
bool generate_time_table = false;
bool event_bail_flag = false;
bool do_events_flag = false;

void auto_load_tx_queue(void);

int blocksize = 64;
int repeatNN = 1;


unsigned int modem_rotation_index = 0;
vector<std::string> bc_modems;
std::string g_modem;
std::string g_header_modem;

int g_event_driven = 0;

//! @brief Command line help string initialization
const char *options[] = {
	"Flamp Unique Options",
	"",
	"  --help",
	"  --version",
	"  --time-table",
	"    Used to generate timing tables for the various modes. Disabling ",
	"    requires program restart.",
	"  --confg-dir folder-path-name (including drive letter on Windows)",
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


int xmlrpc_errno = 0;
int file_io_errno = 0;


std::string NBEMS_dir = "";
std::string title = "";
std::string BaseDir = "";
std::string flampHomeDir = "";
std::string flamp_rcv_dir = "";
std::string flamp_xmt_dir = "";
std::string flamp_script_dir = "";
std::string flamp_script_default_dir = "";
std::string buffer = "";

std::string cmd_fname = "";

std::string xmt_fname = "";
std::string rx_fname = "";

void * create_tx_table(void *);
double measure_tx_time(char character_to_send, unsigned int no_of_characters, double time_out_duration);

char *copyTo(char *src, char *dest, int *limit, int stop_character);
void * transmit_serial_relay(void *ptr);
void * transmit_relay_interval(void *ptr);
void check_io_mode(void *);
void check_call_and_id(void *v);

double time_f(void);

class cAmpGlobal tx_amp;
class cAmpGlobal rx_amp;

// utility functions

bool rx_complete = false;
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
Pixmap  flamp_icon_pixmap;

#define KNAME "flamp"

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


/** ********************************************************
 *
 ***********************************************************/
void update_cAmp_changes(cAmp *amp)
{
	if(!amp) {
		int n = tx_queue->value();
		if(n < 1) return;
		amp = tx_amp.index2amp(n);
		if(!amp) return;
	}

	bool compression  = progStatus.use_compression;
	bool unproto      = progStatus.enable_tx_unproto;
	int block_size    = progStatus.blocksize;
	int encoder_index = progStatus.encoder;
	int repeat        = progStatus.repeat_header;
	int repeat_header = progStatus.repeatNN;

	std::string callto;
	std::string blocks;
	std::string descrip;
	std::string encoder_string;
	std::string my_call;
	std::string my_info;
	std::string tosend;

	callto.assign(txt_tx_send_to->value());
	blocks.assign(txt_tx_selected_blocks->value());
	descrip.assign(txt_tx_descrip->value());
	encoder_string.assign(progStatus.encoder_string);
	my_call.assign(progStatus.my_call);
	my_info.assign(progStatus.my_info);

	amp->callto(callto);
	amp->compress(compression);
	amp->my_call(my_call);
	amp->my_info(my_info);
	amp->tx_base_conv_index(encoder_index);
	amp->tx_base_conv_str(encoder_string);
	amp->unproto(unproto);
	amp->unproto_markers(progStatus.enable_unproto_markers);
	amp->xmt_blocksize(block_size);
	amp->xmt_descrip(descrip);
	amp->xmt_tosend(blocks);
	amp->repeat(repeat);
	amp->header_repeat(repeat_header);
	amp->xmt_modem(g_modem);
	amp->update_required(true);
	amp->amp_update();
	amp->file_hash();
}

/** ********************************************************
 *
 ***********************************************************/
void amp_mark_all_for_update(void)
{
	int index = 0;
	int count = 0;
	cAmp *amp = (cAmp *)0;

	count = tx_amp.size();

	for(index = 0; index < count; index++) {
		amp = tx_amp.index2amp(index + 1);
		if(amp)
			amp->update_required(true);
	}
}

/** ********************************************************
 *
 ***********************************************************/
void amp_update_all(void)
{
	int index = 0;
	int count = 0;
	cAmp *amp = (cAmp *)0;

	int block_size    = progStatus.blocksize;
	int repeat_header = progStatus.repeat_header;
	int repeatNN      = progStatus.repeatNN;

	std::string my_call;
	std::string my_info;

	my_call.assign(progStatus.my_call);
	my_info.assign(progStatus.my_info);

	count = tx_amp.size();

	for(index = 0; index < count; index++) {
		amp = tx_amp.index2amp(index + 1);
		if(amp) {
			amp->my_call(my_call);
			amp->my_info(my_info);
			amp->xmt_blocksize(block_size);
			amp->repeat(repeatNN);
			amp->header_repeat(repeat_header);
			amp->xmt_modem(g_modem);
			amp->update_required(true);
			amp->amp_update();
		} else break;
	}
}

#if 0
/** ********************************************************
 *
 ***********************************************************/
void amp_update(cAmp *amp)
{
	std::string temp;

	if(amp->update_required()) {
		temp.clear();
		temp.assign(amp->xmt_buffer());

		if(!amp->unproto()) {
			compress_maybe(temp, amp->tx_base_conv_index(), (amp->compress() | amp->forced_compress()));
			amp->xmt_data(temp);
		}
		tx_buffer = amp->xmt_data();
		amp->file_hash();
		estimate(amp, true);
		amp->update_required(false);
	}
}
#endif // 0

/** ********************************************************
 *
 ***********************************************************/
void clear_tx_panel(void)
{
	txt_tx_send_to->value("QST");
	txt_tx_filename->value("");
	txt_tx_filename->value("");
	txt_tx_descrip->value("");
	txt_tx_selected_blocks->value("");
	txt_tx_numblocks->value("");
	txt_transfer_size_time->value("");
	btn_enable_tx_unproto->value(false);
	unproto_widgets(0);
}

#if 0
/** ********************************************************
 *
 ***********************************************************/
void update_tx_panel(cAmp *amp)
{
	if(!amp) {
		int n = tx_queue->value();
		if(n < 1) return;
		amp = tx_amp.index2amp(n);
		if(!amp) return;
	}

	amp->my_call(progStatus.my_call);
	amp->my_info(progStatus.my_info);
	amp->xmt_blocksize(progStatus.blocksize);

	string fn = amp->xmt_fname();
	string ds = amp->xmt_descrip();
	string ns = amp->xmt_numblocks();
	string ts = amp->xmt_tosend();
	string ct = amp->callto();

	txt_tx_filename->value(fn.c_str());
	txt_tx_descrip->value(ds.c_str());
	txt_tx_selected_blocks->value(ts.c_str());
	txt_tx_send_to->value(ct.c_str());

	progStatus.enable_tx_unproto = amp->unproto();
	btn_enable_tx_unproto->value(progStatus.enable_tx_unproto);

	progStatus.use_compression = amp->compress();
	btn_use_compression->value(progStatus.use_compression);

	progStatus.encoder = amp->tx_base_conv_index();
	encoders->index(progStatus.encoder - 1);

	std::string temp;

	if(amp->update_required()) {
		temp.clear();
		temp.assign(amp->xmt_buffer());

		if(progStatus.enable_tx_unproto == false) {
			compress_maybe(temp, amp->tx_base_conv_index(), (amp->compress() | amp->forced_compress()));
			amp->xmt_data(temp);
		} else {
			amp->unproto_markers(progStatus.enable_unproto_markers);
		}

		tx_buffer = amp->xmt_data();
		amp->file_hash();
	}

	estimate(amp, true);
	unproto_widgets(amp);

	if(progStatus.enable_tx_unproto == true) {
		txt_tx_numblocks->value("1");
	} else {
		txt_tx_numblocks->value(amp->xmt_numblocks().c_str());
	}
}
#endif // 0
void update_tx_panel(cAmp *amp)
{
	if(!amp) {
		int n = tx_queue->value();
		if(n < 1) return;
		amp = tx_amp.index2amp(n);
		if(!amp) return;
	}

	amp->my_call(progStatus.my_call);
	amp->my_info(progStatus.my_info);
	amp->xmt_blocksize(progStatus.blocksize);
	amp->repeat(progStatus.repeatNN);
	amp->header_repeat(progStatus.repeat_header);
	amp->xmt_modem(g_modem);

	progStatus.enable_tx_unproto = amp->unproto();
	btn_enable_tx_unproto->value(progStatus.enable_tx_unproto);

	progStatus.use_compression = amp->compress();
	btn_use_compression->value(progStatus.use_compression);

	progStatus.encoder = amp->tx_base_conv_index();
	encoders->index(progStatus.encoder - 1);

	amp->amp_update();

	estimate(amp, true);

	string fn = amp->xmt_fname();
	string ds = amp->xmt_descrip();
	string ns = amp->xmt_numblocks();
	string ts = amp->xmt_tosend();
	string ct = amp->callto();

	txt_tx_filename->value(fn.c_str());
	txt_tx_descrip->value(ds.c_str());
	txt_tx_selected_blocks->value(ts.c_str());
	txt_tx_send_to->value(ct.c_str());

	if(progStatus.enable_tx_unproto == true) {
		txt_tx_numblocks->value("1");
	} else {
		txt_tx_numblocks->value(amp->xmt_numblocks().c_str());
	}

	unproto_widgets(amp);
}

/** ********************************************************
 *
 ***********************************************************/
void show_selected_xmt(cAmp *amp)
{
	int index = 0;
	int count = tx_queue->size();
	cAmp *tmp = (cAmp *)0;

	for(index = 0; index < count; index++) {
		tmp = tx_amp.index2amp(index + 1);
		if(tmp == amp) {
			show_selected_xmt(index);
			break;
		}
	}
}

/** ********************************************************
 *
 ***********************************************************/
void show_selected_xmt(int n)
{
	cAmp * amp = (cAmp *)0;

	if (!n) {
		n = tx_queue->value();
		if(n == 0)
			n = tx_queue->size();
	}

	if(n > tx_queue->size())
		return;

	if(n < 1) {
		amp = (cAmp *)0;
		clear_tx_panel();
	} else {
		amp = tx_amp.index2amp(n);
		tx_queue->select(n);
		update_tx_panel(amp);
		tx_amp.set(amp);
	}
}

/** ********************************************************
 *
 ***********************************************************/
void show_current_selected_file(void *ptr)
{
	int index = 0;

	if(ptr) {
		index = *(int *)ptr;
		index++;
	}

	if(index < 1)
		return ;

	if(index > tx_queue->size())
		return ;

	show_selected_xmt(index);
}

/** ********************************************************
 *
 ***********************************************************/
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
		{ flampHomeDir,     "FLAMP",	0 },
		{ flamp_rcv_dir,    "FLAMP/rx",      0 },
		{ flamp_xmt_dir,    "FLAMP/tx",      0 },
		{ flamp_script_dir, "FLAMP/scripts",      0 },
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

	flamp_script_default_dir.assign(flamp_script_dir);
}

/** ********************************************************
 *
 ***********************************************************/
void addfile(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	std::string xmtfname;
	bool useCompression = sp->comp();
	bool proto = sp->proto();
	std::string desc;
	//int block_size = sp->blocks();

	xmtfname.assign(sp->file());
	xmt_fname = xmtfname;
	string xmt_fname2 = xmtfname;

	desc.assign(sp->desc());

	int use_comp_on_file = 0;
	int use_forced_comp_on_file = 0;

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

	if(proto) {
		if(useCompression)
			use_comp_on_file = 1;
		else
			use_comp_on_file = 0;

		if (isbinary(tx_buffer) && !useCompression) {
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
	}

	cAmp *nu = new cAmp(tx_buffer, fl_filename_name(xmt_fname.c_str()));
	nu->xmt_full_path_fname(xmt_fname2);
	nu->amp_type(TX_AMP);

	if(sp->call_to().empty())
		nu->callto("QST");
	else
		nu->callto(sp->call_to().c_str());

	if(desc.size())
		nu->xmt_descrip(desc);
	else
		nu->xmt_descrip("");

	if(proto)
		nu->unproto(false);
	else
		nu->unproto(true);

	if(use_comp_on_file) {
		nu->compress(true);
	}

	if(use_forced_comp_on_file) {
		nu->forced_compress(true);
	}

	switch(sp->base()) {
		default:
		case 64:
			nu->tx_base_conv_index(BASE64);
			nu->tx_base_conv_str("base64");
			break;

		case 128:
			nu->tx_base_conv_index(BASE128);
			nu->tx_base_conv_str("base128");
			break;

		case 256:
			nu->tx_base_conv_index(BASE256);
			nu->tx_base_conv_str("base256");
			break;
	}

	nu->unproto_markers(progStatus.enable_unproto_markers);
	nu->xmt_blocksize(progStatus.blocksize);
	nu->repeat(progStatus.repeatNN);
	nu->header_repeat(progStatus.repeat_header);
	nu->my_call(progStatus.my_call);
	nu->my_info(progStatus.my_info);
	nu->update_required(true);
	nu->xmt_modem(g_modem);
	nu->amp_update();

	estimate(nu, false);

	tx_amp.add(nu);
	tx_queue->add(xmt_fname.c_str());

	LOG_INFO("File added to transmit queue: %s", xmtfname.c_str());
}

/** ********************************************************
 *
 ***********************************************************/
void addfile(std::string xmtfname, void *rx, bool useCompression, \
             char *desc = (char *)0, char *callto = (char *)0)
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

	if(use_comp_on_file)
		fl_alert2("Suggest using compression on this file");


	cAmp *nu = new cAmp(tx_buffer, fl_filename_name(xmt_fname.c_str()));
	nu->amp_type(TX_AMP);
	nu->xmt_full_path_fname(xmt_fname2);

	if(callto) {
		nu->callto(callto);
	} else {
		nu->callto("QST");
	}

	if(desc) {
		nu->xmt_descrip(desc);
	} else {
		nu->xmt_descrip("");
	}

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

	nu->xmt_blocksize(progStatus.blocksize);

	nu->unproto_markers(progStatus.enable_unproto_markers);
	nu->my_call(progStatus.my_call);
	nu->my_info(progStatus.my_info);
	nu->repeat(progStatus.repeatNN);
	nu->header_repeat(progStatus.repeat_header);
	nu->xmt_modem(g_modem);
	nu->update_required(true);
	nu->amp_update();
	nu->file_hash();

	LOG_INFO("File added to transmit queue: %s", xmtfname.c_str());

	if(rx > 0) {
		cAmp *rAmp = (cAmp *) rx;
		int xfrBlockSize = rAmp->rx_blocksize_int();

		xfrBlockSize = valid_block_size(xfrBlockSize);

		cnt_blocksize->value(xfrBlockSize);
		txt_tx_descrip->value(txt_rx_descrip->value());

		nu->xmt_descrip(txt_rx_descrip->value());
		nu->tx_blocksize(xfrBlockSize);

		progStatus.blocksize = xfrBlockSize;
		txt_tx_selected_blocks->value("");
	}

	nu->update_required(true);
	estimate(nu, false);

	tx_amp.add(nu);
	tx_amp.set(nu);
	tx_queue->add(xmt_fname.c_str());
	show_selected_xmt(tx_amp.amp2index(nu));
}

/** ********************************************************
 *
 ***********************************************************/
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


/** ********************************************************
 *
 ***********************************************************/
void replace_add_queue_item(char *filename, bool compFlag, char *desc = (char *)0, char *callto = (char *)0)
{
	int count = 0;
	int i     = 0;
	cAmp *tx = (cAmp *)0;
	char *cPtr = (char *)0;
	bool compress = false;
	std::string fn;
	std::string _callto;
	std::string _desc;

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

	fn.clear();
	_callto.clear();
	_desc.clear();

	count = tx_amp.size();

	for(i = 1; i <= count; i++) {
		tx = tx_amp.index2amp(i);
		if(tx) {
			cPtr = (char *) tx->xmt_full_path_fname().c_str();
			if(strncmp(filename, cPtr, FILENAME_MAX) == 0) {
				if(!tx->xmt_file_modified()) return;
				compress = tx->compress();
				_desc.assign(tx->xmt_descrip());
				_callto.assign(tx->callto());
				LOG_INFO("File removed from transmit queue: %s", cPtr);
				tx_amp.set(tx);
				tx_removefile(false);
				break;
			}
		}
	}

	if(compFlag) compress = true;

	fn.assign(filename);

	if(!_desc.empty())
		desc = (char *) _desc.c_str();

	if(!_callto.empty())
		callto =  (char *) _callto.c_str();

	addfile(fn, 0, compress, desc, callto);
}

/** ********************************************************
 *
 ***********************************************************/
void auto_load_tx_queue_from_tx_directory(void)
{
	char *eMsg = (char *) "TX Queue access in progress, Auto load aborted";
	char *filepath = (char *)0;
	struct dirent **list = 0;
	int count = 0;
	int index = 0;

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

	memset(bname,    0, FILENAME_MAX);
	memset(path,     0, FILENAME_MAX);
	memset(filepath, 0, FILENAME_MAX);
	memset(comp,     0, FILENAME_MAX);
	memset(line,     0, FILENAME_MAX);
	memset(desc,     0, FILENAME_MAX);

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
		copyTo(cPtr, desc,  &size, '\n');

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

/** ********************************************************
 *
 ***********************************************************/
void auto_load_tx_queue(void)
{
	if(progStatus.auto_load_queue == false)
		return;

	if(progStatus.load_from_tx_folder)
		auto_load_tx_queue_from_tx_directory();
	else
		cb_load_tx_queue();
}

/** ********************************************************
 *
 ***********************************************************/
void readfile()
{
	string xmtfname;
	xmtfname.clear();
	xmtfname = flampHomeDir;
	const char *p = FSEL::select(_("Open file"), "*.*",
								 xmtfname.c_str());
	if (!p) return;
	if (strlen(p) == 0) return;
	xmtfname = p;

	addfile(xmtfname, 0, false, 0, 0);
}

/** ********************************************************
 *
 ***********************************************************/
void update_rx_missing_blocks(void)
{
	cAmp * amp = rx_amp.get_amp();
	if (!amp) return;
	std::string tmp;
	tmp.assign(txt_relay_selected_blocks->value());
	amp->rx_relay_blocks(tmp);
}

/** ********************************************************
 *
 ***********************************************************/
void show_rx_amp()
{
	cAmp * amp = rx_amp.get_amp();
	if (!amp) return;

	txt_rx_filename->value(amp->get_rx_fname().c_str());
	txt_rx_datetime->value(amp->rx_time_stamp().c_str());
	txt_rx_descrip->value(amp->rx_desc().c_str());
	txt_rx_callinfo->value(amp->rx_callinfo().c_str());
	txt_rx_filesize->value(amp->rx_fsize().c_str());
	txt_rx_numblocks->value(amp->rx_numblocks().c_str());
	txt_rx_blocksize->value(amp->rx_blocksize().c_str());
	txt_rx_missing_blocks->value(amp->rx_missing().c_str());
	txt_relay_selected_blocks->value(amp->rx_relay_blocks().c_str());
	rx_progress->set(amp->rx_blocks(), amp->rx_nblocks());
	if (amp->rx_completed() && txt_rx_output->buffer()->length() == 0) {
		string data = amp->rx_recvd_string();
		decompress_maybe(data);
		if (isbinary(data))
			txt_rx_output->addstr("Data appears to be binary\n\nSave and view with appropriate software");
		else
			txt_rx_output->addstr(data.c_str());
	}
}

/** ********************************************************
 *
 ***********************************************************/
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
	txt_tx_selected_blocks->value("");
	txt_rx_output->clear();
}

/** ********************************************************
 *
 ***********************************************************/
void clear_missing(void *ptr)
{
	cAmp *amp = 0;
	int index = 0;
	int count = 0;

	if (transmit_stop == true) return;

	if(progStatus.clear_tosend_on_tx_blocks) {
		if(transmit_queue) {
			count = tx_amp.count();
			for(index = 0; index < count; index++) {
				amp = tx_amp.index2amp(index + 1);
				if(!amp) continue;
				amp->reset_preamble_detection();
				amp->xmt_tosend_clear();
			}
		} else {
			amp = tx_amp.get_amp();
			if(!amp) return;
			amp->xmt_tosend_clear();
			amp->reset_preamble_detection();
		}

		txt_tx_selected_blocks->value("");
	}
}

/** ********************************************************
 *
 ***********************************************************/
void show_selected_rcv(int n)
{
	if (!n) return;
	rx_amp.set(n);
	txt_rx_output->clear();
	show_rx_amp();
}


/** ********************************************************
 *
 ***********************************************************/
static const char *cancel = "Cancel";
static const char *yes = "Yes";
static const char *cont = "You are about to transmit! Continue?";

void send_missing_report()
{
	string fname = txt_rx_filename->value();
	if (fname.empty()) return;

	cAmp * amp = rx_amp.get_amp();
	if(!amp) return;

	string report("\nDE ");
	report.append(txt_tx_mycall->value());
	report.append("\nFile : ").append(fname).append("\n");
	report.append(amp->rx_report());
	report.append("DE ").append(txt_tx_mycall->value()).append(" K \n");

	if (progStatus.fldigi_xmt_mode_change) {
		send_new_modem(cbo_modes->value());
	}

	if(progStatus.use_tx_on_report) {
		if (!bConnected) connect_to_fldigi(0);
		if (!bConnected) return;
		int results = fl_choice("%s", cancel, yes, (const char *)0, cont);
		if(results < 1) return;
		send_via_fldigi(report);
	} else {
		report.append("\n\n^r");
		send_report(report);
	}
}

/** ********************************************************
 *
 ***********************************************************/
void recv_missing_report()
{
	cAmp *amp = 0;
	size_t count = (size_t) tx_amp.size();
	for (size_t num = 0; num < count; num++) {
		amp = (cAmp *) tx_amp.index2amp(num + 1);
		if(!amp) continue;
		amp->tx_parse_report();
	}

	amp = tx_amp.get_amp();
	if (!amp) return;

	txt_tx_selected_blocks->value(amp->xmt_tosend().c_str());
}

/** ********************************************************
 *
 ***********************************************************/
void relay_missing_report()
{
	cAmp *camp = 0;
	size_t count = (size_t) rx_amp.size();
	for (size_t num = 0; num < count; num++) {
		camp = (cAmp *) rx_amp.index2amp(num + 1);
		if(!camp) continue;
		camp->tx_parse_report();
	}

	camp = rx_amp.get_amp();
	if (!camp) return;

	std::string tmp;
	tmp.clear();
	tmp.assign(camp->rx_relay_blocks()).append(" ");
	tmp.append(camp->xmt_tosend());
	camp->rx_relay_blocks(tmp);

	txt_relay_selected_blocks->value(camp->rx_relay_blocks().c_str());
}

/** ********************************************************
 *
 ***********************************************************/
void send_relay_data()
{
	if(transmitting)
		return;

	if (!bConnected) connect_to_fldigi(0);
	if (!bConnected) return;

	cAmp *amp = rx_amp.get_amp();

	if (!amp)
		return;

	if(rx_queue->value() == 0)
		return;

	int results = 0;
	std::string tx_string;
	std::string missing_blocks;

	static RELAY_DATA relay_data;

	relay_data.serial_data.clear();
	relay_data.header.clear();
	relay_data.data.clear();
	relay_data.amp = amp;

	missing_blocks.clear();
	missing_blocks.assign(txt_relay_selected_blocks->value());

	if(progStatus.use_txrx_interval || progStatus.use_header_modem) {
		results = amp->tx_relay_vector(progStatus.my_call, missing_blocks);
		if(results < 1) return;
		relay_data.header = amp->xmt_vector_header();
		relay_data.data = amp->xmt_vector_data();
	} else {
		relay_data.serial_data = amp->tx_relay_string(progStatus.my_call, missing_blocks);
	}

	results = fl_choice("%s", cancel, yes, (const char *)0, cont);

	if(results < 1) return;

	transmit_relay(&relay_data);

	if(progStatus.clear_tosend_on_tx_blocks) {
		amp->xmt_tosend_clear();
		amp->rx_relay_blocks("");
		amp->reset_preamble_detection();
		txt_relay_selected_blocks->value("");
	}
}

/** ********************************************************
 *
 ***********************************************************/
void tx_removefile(bool all)
{
	if(active_data_io) {
		LOG_INFO("Unable to remove TX queue item while being accessed.");
		return;
	}

	int flag = 0;

	if(progStatus.enable_delete_warning && !all) {
		flag = fl_choice("Remove file %s from queue?", (const char *)"No", (const char *)"Yes", (const char *)0, txt_tx_filename->value());
		if(flag < 1) return;
	}

	cAmp *amp = tx_amp.get_amp();

	if (!tx_amp.size() || !tx_queue->size() || !amp) {
		clear_tx_panel();
		return;
	}

	size_t n = 0;
	size_t count = tx_queue->size();

	if(count != tx_amp.size())
		return;

	if(all) {
		for(int i = count; i > 0; i--) {
			show_selected_xmt(i);
			tx_queue->remove(i);
			tx_amp.remove(i);
		}
	} else {
		n = tx_queue->value();
		show_selected_xmt(n);
		tx_queue->remove(n);
		tx_amp.remove(n);
	}

	count = tx_queue->size();
	if(count) {
		if(count >= n)
			show_selected_xmt(n);
		else
			show_selected_xmt(count);
	} else {
		clear_tx_panel();
	}
}

/** ********************************************************
 *
 ***********************************************************/
void auto_rx_save_file(cAmp *_amp)
{
	if (!_amp) return;

	size_t fsize = _amp->rx_size();

	if(!_amp->rx_completed()) {
		LOG_ERROR(_("Only completed files can be Saved"));
		return;
	}

	if (!fsize || _amp->get_rx_fname().empty()) return;

	if(_amp->file_saved()) {
		int sel = fl_choice("File Already Saved", "Overwrite", "Cancel", (char *)0);
		if(sel) return;
	}

	std::string rx_directory;
	std::string rx_fname;
	char date_directory[32];
	char test_char = 0;

	time_t rawtime;
	struct tm * ztime;
	time ( &rawtime );

	memset(date_directory, 0, sizeof(date_directory));

	if(progStatus.auto_rx_save_local_time) {
		ztime = localtime(&rawtime);
		snprintf(date_directory, sizeof(date_directory) - 1, "%04d_%02d_%02d", \
				 ztime->tm_year + 1900, ztime->tm_mon + 1, ztime->tm_mday);
	} else {
		ztime = gmtime(&rawtime);
		snprintf(date_directory, sizeof(date_directory) - 1, "%04d_%02d_%02d_UTC", \
				 ztime->tm_year + 1900, ztime->tm_mon + 1, ztime->tm_mday);
	}

	rx_directory.assign(flamp_rcv_dir);

	test_char = rx_directory[rx_directory.size() - 1];
	if(test_char != PATH_CHAR_SEP)
		rx_directory.append(PATH_SEP);

	rx_directory.append(date_directory);

	mkdir((const char *) rx_directory.c_str(), 0777);

	test_char = rx_directory[rx_directory.size() - 1];

	if(test_char != PATH_CHAR_SEP)
		rx_directory.append(PATH_SEP);

	rx_fname.assign(rx_directory);
	rx_fname.append(_amp->get_rx_fname());

	FILE *dfile = fopen(rx_fname.c_str(), "wb");

	if (!dfile) {
		LOG_ERROR("could not open write/binary %s", rx_fname.c_str());
		return;
	}

	string data = _amp->rx_recvd_string();
	decompress_maybe(data);

	size_t r = fwrite((void *)data.c_str(), 1, data.length(), dfile);

	if (r != data.length()) {
		LOG_ERROR("%s", "write error");
		return;
	}

	_amp->file_saved(true);

	fclose(dfile);
}

/** ********************************************************
 *
 ***********************************************************/
void writefile(int xfrFlag)
{
	cAmp * amp = rx_amp.get_amp();
	if (!amp) return;

	size_t fsize = amp->rx_size();

	if(xfrFlag && !amp->rx_completed()) {
		fl_alert2("Only completed files can be transfered");
		return;
	}

	if (!fsize || amp->get_rx_fname().empty()) return;

	static char rx_filename[FL_PATH_MAX];
	std::string rx_directory;
	std::string rx_fname;

	rx_directory.assign(flamp_rcv_dir);
	rx_fname.assign(amp->get_rx_fname());

	char test_char = rx_directory[rx_directory.size() - 1];

	rx_fname.assign(rx_directory);

	if(test_char != PATH_CHAR_SEP)
		rx_fname.append(PATH_SEP);

	rx_fname.append(amp->get_rx_fname());

	const char *p = FSEL::saveas(_("Save file"), "file\t*.*",
								 rx_fname.c_str());
	if (!p) return;
	if (strlen(p) == 0) return;

	memset(rx_filename, 0, FL_PATH_MAX);
	strncpy(rx_filename, p, FL_PATH_MAX - 1);

	FILE *dfile = fopen(rx_filename, "wb");

	if (!dfile) {
		LOG_ERROR("could not open write/binary %s", rx_fname.c_str());
		return;
	}

	string data = amp->rx_recvd_string();
	decompress_maybe(data);

	size_t r = fwrite((void *)data.c_str(), 1, data.length(), dfile);

	if (r != data.length()) {
		LOG_ERROR("%s", "write error");
		return;
	}

	fclose(dfile);

	if(xfrFlag && dfile) {
		addfile(rx_fname, amp, amp->compress(), (char *) amp->rx_desc().c_str(), 0);
	}
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
void abort_tx(void)
{
	send_abort();
	send_abort();

	for(int i = 0; i < 40; i++) { // 4 seconds
		MilliSleep(100);
		if(get_trx_state() == "RX") break;
	}
}

/** ********************************************************
 *
 ***********************************************************/
void abort_tx_from_main(void * ptr)
{
	abort_tx();

	if(!generate_time_table) {
		std::string idMessage;
		// A number of non printable characters are required to overcome long interleave modems.
		idMessage.assign("\n\n\n\n\n\n\n\n\n\n\nFILE TRANSFER ABORTED\n\nDE ").append(progStatus.my_call).append(" BK\n\n");
		send_via_fldigi(idMessage);
	}
}

/** ********************************************************
 *
 ***********************************************************/
void abort_request(void)
{
	int response = fl_choice("Terminate Current Transmission?", "No", "Yes", NULL);
	if (response == 1) {
		static int value = TX_BUTTON;
		deactivate_button((void *) &value);
		transmit_stop = true;
		ztime_end = 0;
		continuous_exception = false;
		//abort_tx();
	}
}

/** ********************************************************
 *
 ***********************************************************/
void send_fldigi_modem(void *ptr)
{
	if(!ptr) return;
	int *val = (int *) ptr;

	static char buffer[32];

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
void transmit_queue_main_thread(void *ptr)
{
	if(progStatus.auto_load_queue && active_data_io == false) {
		auto_load_tx_queue();
	}

	transmit_queued(true, false);
}

/** ********************************************************
 *
 ***********************************************************/
void deactivate_button(void *ptr)
{
	if (!ptr) return;
	int *value = (int *) ptr;

	if( *value == TX_BUTTON )
		btn_send_file->deactivate();
	else if( *value ==TX_ALL_BUTTON )
		btn_send_queue->deactivate();
}

/** ********************************************************
 *
 ***********************************************************/
void activate_button(void *ptr)
{
	if (!ptr) return;
	int * value = (int *) ptr;

	if( *value == TX_BUTTON )
		btn_send_file->activate();
	else if( *value == TX_ALL_BUTTON )
		btn_send_queue->activate();
}


/** ********************************************************
 *
 ***********************************************************/
void set_button_to_xmit(void *ptr)
{
	btn_send_file->label(XMT_LABEL);
}

/** ********************************************************
 *
 ***********************************************************/
void set_button_to_cancel(void *ptr)
{
	btn_send_file->label(CANX_LABEL);
}

/** ********************************************************
 *
 ***********************************************************/
void set_relay_button_label(void *data)
{
	char *str = (char *)0;

	if(!data)
		str = (char *) RELAY_LABEL;
	else
		str = (char *) data;

	btn_send_relay->label(str);
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
void set_xmit_label(void *data)
{
	if(data) {
		btn_send_file->label((const char *)data);
		free(data);
	}
}

/** ********************************************************
 *
 ***********************************************************/
int receive_data_stream(void *ptr)
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

/** ********************************************************
 *
 ***********************************************************/
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

/** ********************************************************
 *
 ***********************************************************/
void process_missing_stream(void)
{
	string retbuff;

	char tag[32];
	char crc[5];
	char hash[5];
	int len = 0;
	int conv = 0;
	size_t count = 0;
	string txrx_hash;
	size_t i = 0;
	cAmp *ctmp = (cAmp *)0;

	retbuff.assign(tmp_buffer);

	conv = sscanf(retbuff.c_str(), "<%s %d %4s>{%4s", tag, &len, crc, hash);

	if (conv == 4) {

		count = tx_amp.size();

		for (i = 0; i < count; i++) {
			ctmp = tx_amp.index2amp(i + 1);
			if(!ctmp) continue;
			txrx_hash = ctmp->xmt_hash();
			if (memcmp((const char *)hash, (const char *)txrx_hash.c_str(), sizeof(hash) - 1) == 0) {
				ctmp->append_report(retbuff);
				break;
			}
		}

		// For relay operations
		count = rx_amp.size();
		for (i = 0; i < count; i++) {
			ctmp = rx_amp.index2amp(i + 1);
			if(!ctmp) continue;
			txrx_hash = ctmp->rx_hash();
			if (memcmp((const char *)hash, (const char *)txrx_hash.c_str(), sizeof(hash) - 1) == 0) {
				ctmp->append_report(retbuff);
				break;
			}
		}

	}
}


/** ********************************************************
 *
 ***********************************************************/
void process_data_stream(void)
{
	string retbuff;

	char tag[32];
	char crc[5];
	char hash[5];
	int len;
	size_t conv;
	cAmp *tmp = 0;
	size_t i = 0;
	size_t count = rx_amp.size();

	retbuff.assign(tmp_buffer);

	conv = sscanf(retbuff.c_str(), "<%s %d %4s>{%4s", tag, &len, crc, hash);

	if (conv == 4) {

		cAmp *existing = 0;

		for (i = 0; i < count; i++) {
			tmp = rx_amp.index2amp(i + 1);
			if (tmp->hash(hash)) {
				existing = tmp;
				break;
			}
		}

		if (!existing) { // a new rx process

			std::string temp_fname;
			temp_fname.assign("Unassigned");

			cAmp *nu = new cAmp();
			nu->amp_type(RX_AMP);
			nu->rx_fname(temp_fname);
			nu->rx_hash(hash);
			nu->rx_to_tx_hash(); // For relay ops
			nu->rx_append(retbuff);
			nu->rx_parse_buffer();
			rx_amp.add(nu);
			string s;
			s.assign("@f").append(nu->rx_sz_percent()).append("\t").append(nu->rx_hash()).append("\t");
			s.append(nu->get_rx_fname());
			rx_queue->add(s.c_str());
			rx_queue->select(rx_queue->size());
			rx_amp.set(nu);
			clear_rx_amp();
			show_rx_amp();
			LOG_INFO("New Amp instance: %s", nu->get_rx_fname().c_str());

		} else {

			string bline;
			if (!existing->rx_completed()) {
				existing->rx_append(retbuff);
				existing->rx_parse_buffer();
				bline.assign("@f").append(existing->rx_sz_percent()).append("\t").append(existing->rx_hash()).append("\t").append(existing->get_rx_fname());
				rx_queue->text(i+1, bline.c_str());
			} else {
				if(progStatus.auto_rx_save && !existing->file_saved()) {
					auto_rx_save_file(existing);
				}
			}
		}
	}

	if (count != 0)
		show_rx_amp();
}

/** ********************************************************
 *
 ***********************************************************/
void receive_remove_from_queue(bool all)
{
	if(progStatus.enable_delete_warning && !all) {
		int flag = fl_choice("Remove file %s from queue?", (const char *)"No", (const char *)"Yes", (const char *)0, txt_tx_filename->value());
		if(flag < 1) return;
	}

	if (rx_queue->size()) {
		int n = rx_queue->value();

		while(n > 0) {
			cAmp * amp = rx_amp.index2amp(n);
			rx_amp.set((cAmp *)0);

			if (amp) {
				rx_amp.remove(n);
			}

			rx_queue->remove(n);

			if (rx_queue->size()) {
				n = 1;
				rx_amp.set(n);
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

			n = rx_queue->value();

			if(all == false) break;
		}
	}
}

#if 0
/** ********************************************************
 *
 ***********************************************************/
void estimate(cAmp *amp, bool visable) {

	static char sz_xfr_size[50];
	float xfr_time = 0;
	float oh = 0;
	int transfer_size;
	int n = 0;

	if(tx_amp.size() < 1)
		return;

	if(!amp) {
		n = tx_queue->value();
		if(n < 1) return;
		amp = tx_amp.index2amp(n);
		if(!amp) return;
	}

	amp->amp_update();
	string xmtstr = amp->tx_string(" K\n");

	if(visable) {

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
		txt_tx_numblocks->value(amp->xmt_numblocks().c_str());
	}
}
#else
/** ********************************************************
 *
 ***********************************************************/
void estimate(cAmp *amp, bool visable)
{
	int n = 0;

	if(tx_amp.size() < 1)
		return;

	if(!amp) {
		n = tx_queue->value();
		if(n < 1) return;
		amp = tx_amp.index2amp(n);
		if(!amp) return;
	}

	amp->amp_update();

	if(visable) {
		txt_transfer_size_time->value(amp->estimate().c_str());
		txt_tx_numblocks->value(amp->xmt_numblocks().c_str());
	}
}

#endif

/** ********************************************************
 *
 ***********************************************************/
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

	count = tx_amp.size();
	total_xfr_time = 0;
	modem_index_count = BROADCAST_MAX_MODEMS;

	xmtstr.clear();

	for(index = 0; index < count; index++) {
		tx = tx_amp.index2amp(index + 1);
		xmtstr.append(tx->tx_string(" K\n"));
	}

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
				if(progStatus.hamcast_mode_enable_1)
					flag = true;
				txt_hamcast_select_1_time->value(sz_xfr_size);
				break;
			case 1:
				if(progStatus.hamcast_mode_enable_2)
					flag = true;
				txt_hamcast_select_2_time->value(sz_xfr_size);
				break;
			case 2:
				if(progStatus.hamcast_mode_enable_3)
					flag = true;
				txt_hamcast_select_3_time->value(sz_xfr_size);
				break;
			case 3:
				if(progStatus.hamcast_mode_enable_4)
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

	tx_amp.free_all();
	rx_amp.free_all();

	if (tcpip) {
		tcpip->close();
		delete tcpip;
		delete localaddr;
	}

	debug::stop();

	if(cQue) delete cQue;

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

	if (strcasecmp(argv[idx], "--time-table") == 0) {
		generate_time_table = true;
		idx++;
		return 1;
	}

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
	NBEMS_dir.clear();
	{
		string appname = argv[0];
		string appdir;
		char dirbuf[FL_PATH_MAX + 1];
		fl_filename_expand(dirbuf, FL_PATH_MAX, appname.c_str());
		appdir.assign(dirbuf);

#ifdef __WOE32__
		size_t p = appdir.rfind("flamp.exe");
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
		size_t p = appdir.rfind("flamp");
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

	string debug_file = flampHomeDir;
	debug_file.append("debug_log.txt");
	debug::start(debug_file.c_str());

	LOG_INFO("Base dir: %s", NBEMS_dir.c_str());

	main_window = flamp_dialog();
	main_window->resize( progStatus.mainX, progStatus.mainY, main_window->w(), main_window->h());
	main_window->callback(exit_main);

	Fl::add_handler(default_handler);

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

	try {
		cQue = new TagSearch(receive_data_stream, process_que);
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

	watch_dog_seconds = time(0);
	watch_dog_thread = new pthread_t;
	if (pthread_create(watch_dog_thread, NULL, watch_dog_loop, NULL)) {
		perror("pthread_create: ztimer watch dog not started");
	}

	ztimer((void *)true);

	if(progStatus.auto_load_queue) {
		auto_load_tx_queue();
	}

	unproto_widgets(0);

	Fl::add_timeout(0.5, check_io_mode);

	return Fl::run();
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

	check_call_and_id(v);
}


/** ********************************************************
 *
 ***********************************************************/
void check_call_and_id(void *v)
{
	if(progStatus.my_call.empty() || progStatus.my_info.empty()) {
		if(tabs && Config_tab) {
			tabs->value(Config_tab);
			fl_choice2(_("Update Callsign and Info"), _("Okay"), NULL, NULL);
		}
	}
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
	open_url("http://www.w1hkj.com/flamp-help/index.html");
}

/** ********************************************************
 *
 ***********************************************************/
void cb_folders()
{
	open_url(flampHomeDir.c_str());
}

/** ********************************************************
 *
 ***********************************************************/
void url_to_file(char *path, size_t buffer_length)
{
	if(!path && buffer_length < 1) return;

	char *convert_buffer = (char *)0;
	char *cPtr = path;
	char *cEnd = &path[buffer_length];
	char *dPtr = (char *)0;
	int value = 0;
	int count = 0;

	convert_buffer = new char [buffer_length + 1];
	if(!convert_buffer) return;

	memset(convert_buffer, 0, buffer_length + 1);
	dPtr = convert_buffer;

	while(cPtr < cEnd) {
		if(*cPtr == '%') {
			sscanf(cPtr + 1, "%02x", &value);
			*dPtr++ = (char) value;
			cPtr += 3;
		} else {
			*dPtr++ = *cPtr++;
		}
		count++;
	}

	memset(path, 0, buffer_length);
	memcpy(path, convert_buffer, count);

	delete [] convert_buffer;
}

/** ********************************************************
 *
 ***********************************************************/
void drop_file_changed(void)
{
	string buffer = Fl::event_text();
	size_t length = Fl::event_length();
	char *fname = (char *)0;
	size_t n = 0;
	int valid = 0;

	char *cPtr = (char *)0;
	char *cFileName = (char *)0;
	const char *cBufferEnd = (char *)0;

	drop_file->value("  DnD");
	drop_file->redraw();

	fname = new char[length + 1];
	if(!fname) {
		LOG_INFO("File name allocation error\n");
		return;
	}

	memset(fname, 0, length + 1);
	memcpy(fname, buffer.c_str(), length);

	if ((n = buffer.find("file:///")) != string::npos) {
		buffer.erase(n, 7);
		memcpy(fname, buffer.c_str(), buffer.size());
		url_to_file(fname, length);
		valid = 1;
	} else	if ((n = buffer.find(":\\")) != string::npos) {
		valid = 1;
	} else 	if ((n = buffer.find("/")) != string::npos) {
		valid = 1;
	}

	if(valid == 0) return;

	cBufferEnd = &fname[length];
	cFileName = cPtr = (char *) fname;

	// Skip leading spaces
	while(*cPtr <= ' ' && cPtr < cBufferEnd)
		cPtr++;

	// Begining of file path
	fname = cPtr;

	// Null terminate control characters
	while(cPtr < cBufferEnd) {
	   if(*cPtr < ' ') *cPtr = 0;
	   cPtr++;
	}

	addfile(fname, 0, false, 0, 0);

	if(fname) {
		delete [] fname;
		fname = 0;
	}
}

