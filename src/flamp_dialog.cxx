// =====================================================================
//
// flamp_dialog.cxx
//
// Author(s):
//	Dave Freese, W1HKJ
//  Robert Stiles, KK5VD
//
//	Copyright: 2010, 2013
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

#include "config.h"

#include "gettext.h"
#include "flamp_dialog.h"
#include "status.h"
#include "flamp.h"
#include "fileselect.h"
#include "debug.h"
#include "icons.h"

#include "xml_io.h"
#include "file_io.h"
#include "amp.h"

//======================================================================

Fl_Double_Window *main_window = 0;
Fl_Double_Window *wCmdLine = 0;

Fl_Tabs*   tabs = 0;
Fl_Output* txt_rx_filename = 0;
Fl_Output* txt_rx_datetime = 0;
Fl_Output* txt_rx_descrip = 0;
Fl_Output* txt_rx_callinfo = 0;
Fl_Output* txt_rx_filesize = 0;
Fl_Output* txt_rx_numblocks = 0;
Fl_Output* txt_rx_blocksize = 0;
Fl_Output* txt_rx_missing_blocks = 0;

Fl_Button* btn_parse_blocks = 0;

Fl_BlockMap* rx_progress = 0;

FTextView* txt_rx_output = 0;
Fl_Hold_Browser* rx_queue = 0;

Fl_Input2* txt_tx_mycall = 0;
Fl_Input2* txt_tx_myinfo = 0;
Fl_Input2* txt_tx_send_to = 0;
Fl_Simple_Counter* cnt_blocksize = 0;
Fl_Simple_Counter* cnt_repeat_nbr = 0;
Fl_Simple_Counter* cnt_repeat_header = 0;

Fl_Check_Button* btn_use_compression = 0;
Fl_ComboBox* encoders = 0;
Fl_ComboBox* cbo_modes = 0;
Fl_Output*   txt_transfer_size_time = 0;
Fl_Output*   txt_tx_numblocks = 0;
Fl_Output*   txt_tx_filename = 0;
Fl_Input2*   txt_tx_descrip = 0;
Fl_Input*    drop_file = 0;
Fl_Hold_Browser* tx_queue = 0;

Fl_Button* btn_save_file = 0;
Fl_Button* btn_rx_remove = 0;
Fl_Button* btn_open_file = 0;
Fl_Button* btn_copy_missing = 0;
Fl_Button* btn_rxq_to_txq = 0;

Fl_Button* btn_tx_remove_file = 0;
Fl_Button* btn_send_file = 0;
Fl_Button* btn_send_queue = 0;

// Configuraton panel

Fl_Check_Button* btn_sync_mode_flamp_fldigi = 0;
Fl_Check_Button* btn_sync_mode_fldigi_flamp = 0;
Fl_Check_Button* btn_fldigi_xmt_mode_change = 0;

Fl_Check_Button* btn_enable_tx_unproto = 0;
Fl_Check_Button* btn_enable_txrx_interval = 0;
Fl_Check_Button* btn_enable_header_modem = 0;
Fl_Check_Button* btn_disable_header_modem_on_block_fills = 0;
Fl_ComboBox* cbo_header_modes = 0;

Fl_Simple_Counter* cnt_tx_internval_mins = 0;
Fl_Simple_Counter* cnt_rx_internval_secs = 0;

Fl_Check_Button* btn_enable_tx_on_report = 0;

Fl_Check_Button* btn_clear_tosend_on_tx_blocks = 0;

// end Configuration panel.

// Event panel

Fl_Check_Button* btn_repeat_at_times = 0;
Fl_ComboBox*     cbo_repeat_every = 0;
Fl_Input2*       txt_repeat_times = 0;

Fl_Input2*       txt_auto_load_queue_path = 0;
Fl_Check_Button* btn_auto_load_queue = 0;
Fl_Check_Button* btn_load_from_tx_folder = 0;
Fl_Button*       btn_manual_load_queue = 0;

Fl_Check_Button* btn_repeat_forever = 0;
Fl_Light_Button* do_events = 0;

Fl_Output* outTimeValue = 0;

// Hamcast Event subpanel panel

Fl_Check_Button * btn_hamcast_mode_cycle = 0;

Fl_Check_Button * btn_hamcast_mode_enable_1 = 0;
Fl_ComboBox * cbo_hamcast_mode_selection_1 = 0;
Fl_Simple_Counter * cnt_hamcast_mode_selection_repeat_1 = 0;
Fl_Output * txt_hamcast_select_1_time = 0;

Fl_Check_Button * btn_hamcast_mode_enable_2 = 0;
Fl_ComboBox * cbo_hamcast_mode_selection_2 = 0;
Fl_Simple_Counter * cnt_hamcast_mode_selection_repeat_2 = 0;
Fl_Output * txt_hamcast_select_2_time = 0;

Fl_Check_Button * btn_hamcast_mode_enable_3 = 0;
Fl_ComboBox * cbo_hamcast_mode_selection_3 = 0;
Fl_Simple_Counter * cnt_hamcast_mode_selection_repeat_3 = 0;
Fl_Output * txt_hamcast_select_3_time = 0;

Fl_Check_Button * btn_hamcast_mode_enable_4 = 0;
Fl_ComboBox * cbo_hamcast_mode_selection_4 = 0;
Fl_Simple_Counter * cnt_hamcast_mode_selection_repeat_4 = 0;
Fl_Output * txt_hamcast_select_4_time = 0;

Fl_Output * txt_hamcast_select_total_time = 0;

// end hamcast Event sub panel.

// end Event panel

Fl_Input2*  txt_tx_selected_blocks = 0;

//----------------------------------------------------------------------
// Mode, CPS (avg), Latency (in seconds)
struct st_modes s_basic_modes[] = {
	{"DOMX22",          7.9,  0.0}, {"DOMX44",       16.3, 0.0},  {"DOMX88",       17.9, 0.0},

	{"MFSK16",          5.8,  0.0}, {"MFSK22",        8.0, 0.0},  {"MFSK31",        5.5, 0.0},
	{"MFSK32",         12.0,  0.0}, {"MFSK64",       24.0, 0.0},  {"MFSK128",      48.0, 0.0},
	{"MFSK64L",        24.0,  6.5}, {"MFSK128L",     48.0, 6.5},

	{"MT63-500S",       5.0,  6.4}, {"MT63-1KS",     10.0, 3.2},  {"MT63-2KS",     20.0, 3.2},
	{"MT63-500L",       5.0, 12.8}, {"MT63-1KL",     10.0, 6.4},  {"MT63-2KL",     20.0, 1.6},

	{"BPSK125",        12.0,  0.0}, {"BPSK250",      24.0, 0.0},
	{"BPSK500",        48.0,  0.0}, {"BPSK1000",     96.0, 0.0},

	{"PSK63RC4",       15.0,  0.0}, {"PSK63RC5",     19.0, 0.0},  {"PSK63RC10",    38.0, 0.0},
	{"PSK63RC20",      74.0,  0.0}, {"PSK63RC32",   120.0, 0.0},

	{"PSK125C12",     144.0,  0.0},

	{"PSK125R",         7.0,  0.0}, {"PSK125RC4",    28.0, 0.0},  {"PSK125RC5",    35.0, 0.0},
	{"PSK125RC10",     70.0,  0.0}, {"PSK125RC12",   85.0, 0.0},  {"PSK125RC16",  112.0, 0.0},

	{"PSK250C6",      144.0,  0.0},
	
	{"PSK250R",        15.0,  0.0}, {"PSK250RC2",    30.0, 0.0},  {"PSK250RC3",    45.0, 0.0},
	{"PSK250RC5",      75.0,  0.0}, {"PSK250RC6",    90.0, 0.0},  {"PSK250RC7",   105.0, 0.0},

	{"PSK500C2",       96.0,  0.0}, {"PSK500C4",     92.0, 0.0},
	{"PSK500R",        29.0,  0.0}, {"PSK500RC2",    58.0, 0.0},  {"PSK500RC3",    85.8, 0.0},
	{"PSK500RC4",     114.4,  0.0},

	{"PSK800C2",      153.6,  0.0}, {"PSK800RC2",    92.8, 0.0},

	{"PSK1000C2",     192.0,  0.0}, {"PSK1000R",     60.0, 0.0},  {"PSK1000RC2",  120.0,  0.0},

	{"Olivia-4-250",    3.0,  0.0}, {"Olivia-8-250",  1.5, 0.0},  {"Olivia-4-500",  6.0,  0.0 },
	{"Olivia-8-500",    3.0,  0.0}, {"Olivia-16-500", 1.5, 0.0},  {"Olivia-8-1K",   6.0,  0.0 },
	{"Olivia-16-1K",    4.0,  0.0}, {"Olivia-32-1K",  2.0, 0.0},  {"Olivia-64-2K",  2.0,  0.0 },

	{"THOR16",         3.25,  0.0}, {"THOR22",        4.46, 0.0},
	{"THOR25x4",       5.03,  0.0}, {"THOR50x1",     10.06, 0.0}, {"THOR50x2",     10.06, 0.0},
	{"THOR100",       20.13,  0.0}, {"",              0.0,  0.0}
};

// reported by get_modes
/*
 NULL|CW|
 CTSTIA|DOMEX4|DOMEX5|DOMEX8|DOMX11|DOMX16|DOMX22|DOMX44|DOMX88|
 FELDHELL|SLOWHELL|HELLX5|HELLX9|FSKHELL|FSKH105|HELL80|
 MFSK8|MFSK16|MFSK32|MFSK4|MFSK11|MFSK22|MFSK31|MFSK64|MFSK128||MFSK64L|MFSK128L|
 WEFAX576|WEFAX288|NAVTEX|SITORB|
 MT63-500S|MT63-1KS|MT63-2KS|
 MT63-500L|MT63-1KL|MT63-2KL|
 BPSK31|BPSK63|BPSK63F|BPSK125|BPSK250|BPSK500|
 QPSK31|QPSK63|QPSK125|QPSK250|QPSK500|
 PSK125R|PSK250R|PSK500R|BPSK1000|PSK1000R|
 OLIVIA|Olivia-4-250|Olivia-8-250|Olivia-4-500|Olivia-8-500|
 Olivia-16-500|Olivia-8-1K|Olivia-16-1K|Olivia-32-1K|Olivia-64-2K|
 RTTY|
 THOR4|THOR5|THOR8|THOR11|THOR16|THOR22|THOR25X4|THOR50X1|THOR50X2|THOR100|
 THROB1|THROB2|THROB4|THRBX1|THRBX2|THRBX4|
 PSK63RC4|PSK63RC5|PSK63RC10|PSK63RC20|PSK63RC32|
 PSK125RC4|PSK125RC5|PSK125RC10|PSK125C12|PSK125RC12|PSK125RC16|
 PSK250C6|PSK250RC2|PSK250RC3|PSK250RC5|PSK250RC6|PSK250RC7|
 PSK500C2|PSK500C4|PSK500RC2|PSK500RC3|PSK500RC4|
 PSK800C2|PSK800RC2|
 PSK1000C2|PSK1000RC2|
 SSB|WWV|ANALYSIS|
 */

st_modes s_modes[100];

std::string valid_modes;

bool valid_mode_check(std::string &md)
{
	return (valid_modes.find(md) != string::npos);
}

void update_cbo_modes(std::string &fldigi_modes)
{
	for (int n = 0; n < 100; n++) { s_modes[n].s_mode = ""; s_modes[n].f_cps = 0; }

	valid_modes.clear();
	cbo_modes->clear();
	cbo_header_modes->clear();
	cbo_hamcast_mode_selection_1->clear();

	int i = 0, j = 0;
	while (s_basic_modes[i].f_cps != 0) {
		if (fldigi_modes.find(s_basic_modes[i].s_mode) != string::npos) {
			s_modes[j] = s_basic_modes[i];
			cbo_modes->add(s_modes[j].s_mode.c_str());
			cbo_header_modes->add(s_modes[j].s_mode.c_str());
			cbo_hamcast_mode_selection_1->add(s_modes[j].s_mode.c_str());
			cbo_hamcast_mode_selection_2->add(s_modes[j].s_mode.c_str());
			cbo_hamcast_mode_selection_3->add(s_modes[j].s_mode.c_str());
			cbo_hamcast_mode_selection_4->add(s_modes[j].s_mode.c_str());
			valid_modes.append(s_modes[j].s_mode).append("|");
			j++;
		}
		i++;
	}

	cbo_modes->index(progStatus.selected_mode);
	cbo_header_modes->index(progStatus.header_selected_mode);
	cbo_hamcast_mode_selection_1->index(progStatus.hamcast_mode_selection_1);
	cbo_hamcast_mode_selection_2->index(progStatus.hamcast_mode_selection_2);
	cbo_hamcast_mode_selection_3->index(progStatus.hamcast_mode_selection_3);
	cbo_hamcast_mode_selection_4->index(progStatus.hamcast_mode_selection_4);
	assign_bc_modem_list();
	g_modem.assign(cbo_modes->value());
	g_header_modem.assign(cbo_header_modes->value());
}

void init_cbo_modes()
{
	string min_modes;
	min_modes.assign("DOMX22|MFSK16|MFSK22|MFSK31|");
	min_modes.append("MT63-500S|MT63-1KS|MT63-2KS|");
	min_modes.append("MT63-500L|MT63-1KL|MT63-2KL|");
	min_modes.append("BPSK125|BPSK250|BPSK500|");
	min_modes.append("PSK125R|PSK250R|PSK500R|");
	min_modes.append("OL 4-250|OL 8-250|OL 4-500|OL 8-500|OL 16-500|OL 8-1K|OL 16-1K|");
	min_modes.append("THOR16|THOR22");
	update_cbo_modes(min_modes);
}

void init_cbo_events()
{
	cbo_repeat_every->add("5 min");
	cbo_repeat_every->add("15 min");
	cbo_repeat_every->add("30 min");
	cbo_repeat_every->add("Hourly");
	cbo_repeat_every->add("Even hours");
	cbo_repeat_every->add("Odd hours");
	cbo_repeat_every->add("Repeated at");
	cbo_repeat_every->add("One time at");
	cbo_repeat_every->add("Continuous at");
	cbo_repeat_every->index(progStatus.repeat_every);
}

void cb_cbo_modes()
{
	progStatus.selected_mode = cbo_modes->index();
	g_modem.assign(cbo_modes->value());
	estimate();
	if (progStatus.sync_mode_flamp_fldigi)
		send_new_modem(cbo_modes->value());
}

//----------------------------------------------------------------------
void cb_mnuExit(void *, void *)
{
	cb_exit();
}

void cb_mnuEventLog(void *, void *)
{
	debug::show();
}

static void cb_mnuOnLineHelp(Fl_Menu_*, void*) {
	show_help();
}

static void cb_mnu_folders(Fl_Menu_*, void*) {
	cb_folders();
}

void cb_mnuAbout(void *, void *)
{
	fl_message2("\tFlamp: %s\n\n" \
				"\tAuthors:\n" \
				"\t\tDave Freese, W1HKJ\n" \
				"\t\tRobert Stiles, KK5VD",
				FLAMP_VERSION);
}

void cb_mnuCmdLineParams(void *, void *)
{
	if (!wCmdLine) {
		wCmdLine = new Fl_Double_Window(0,0,604,404,"Command Line Options");
		wCmdLine->begin();
		Fl_Browser *bwsCmds = new Fl_Browser(2,2,600,400,"");
		int i = 0;
		string cmdline;
		while (options[i] != NULL) {
			cmdline.assign("@f").append(options[i]);
			bwsCmds->add(cmdline.c_str());
			i++;
		}
		wCmdLine->end();
	}
	wCmdLine->show();
}

Fl_Menu_Item menu_[] = {
	{_("&File"), 0,  0, 0, 64, FL_NORMAL_LABEL, 0, 14, 0},
	{_("&Folders"), 0, (Fl_Callback*)cb_mnu_folders, 0, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, 14, 0},
	{_("E&xit"), 0x40078,  (Fl_Callback*)cb_mnuExit, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
	{0,0,0,0,0,0,0,0,0},
	{_("&Help"), 0,  0, 0, 64, FL_NORMAL_LABEL, 0, 14, 0},
	{_("&Debug log"), 0,  (Fl_Callback*)cb_mnuEventLog, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
	{_("On Line help"), 0,  (Fl_Callback*)cb_mnuOnLineHelp, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
	{_("Command line parameters"), 0, (Fl_Callback*)cb_mnuCmdLineParams, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
	{_("About"), 0, (Fl_Callback*)cb_mnuAbout, 0, 128, FL_NORMAL_LABEL, 0, 14, 0},
	{0,0,0,0,0,0,0,0,0},

	{0,0,0,0,0,0,0,0,0}
};

static void cb_tx_mycall(Fl_Input2*, void*)
{
	progStatus.my_call = txt_tx_mycall->value();
}

static void cb_tx_myinfo(Fl_Input2*, void*)
{
	progStatus.my_info = txt_tx_myinfo->value();
}

static void cb_tx_descrip(Fl_Input2*, void*)
{
	update_selected_xmt();
}

static void cb_selected_blocks(Fl_Input2*, void*)
{
	update_selected_xmt();
}

static void cb_btn_save_file(Fl_Button*, void*) {
	writefile(0);
}

static void cb_btn_transfer_file_txQ(Fl_Button*, void*) {
	writefile(1);
}

static void cb_btn_rx_remove(Fl_Button*, void*) {
	receive_remove_from_queue();
}

static void cb_btn_open_file(Fl_Button*, void*) {
	readfile();
}

static void cb_btn_tx_remove_file(Fl_Button*, void*) {
	tx_removefile();
}

static void cb_btn_copy_missing(Fl_Button*, void*) {
	send_missing_report();
}

static void cb_btn_parse_blocks(Fl_Button*, void*)
{
	recv_missing_report();
}

static void cb_tx_queue(Fl_Hold_Browser *hb, void*)
{
	if (tx_queue->value() == 0) {
		txt_tx_filename->value("");
		txt_tx_descrip->value("");
		txt_tx_selected_blocks->value("");
		txt_tx_numblocks->value("");
		txt_transfer_size_time->value("");
		return;
	}
	estimate();
}

static void cb_rx_queue(Fl_Hold_Browser *hb, void*)
{
	int n = hb->value();
	show_selected_rcv(n);
}

static void cb_btn_send_file(Fl_Button*, void*) {
	if(transmitting) {
		abort_request();
		return;
	}

	if ((tx_queue->value() == 0) || (tx_queue->size() == 0)) return;

	btn_send_queue->deactivate();

	transmit_current();
}

static void cb_btn_send_queue(Fl_Button*, void*) {
	if(transmitting) {
		abort_request();
		return;
	}

	if (tx_queue->size() == 0) return;

	btn_send_queue->deactivate();

	transmit_queued(false);
}

static void cb_cnt_blocksize(Fl_Button*, void*) {
	progStatus.blocksize = (int)cnt_blocksize->value();
	txt_tx_selected_blocks->value("");
	update_selected_xmt();
	estimate();
	estimate_bc();
}

static void cb_cnt_repeat_nbr(Fl_Button*, void*) {
	progStatus.repeatNN = (int)cnt_repeat_nbr->value();
	estimate();
	estimate_bc();
}

static void cb_repeat_header(Fl_Button*, void*) {
	progStatus.repeat_header = (int)cnt_repeat_header->value();
	estimate();
	estimate_bc();
}

void cb_use_compression()
{
	progStatus.use_compression = btn_use_compression->value();
	txt_tx_selected_blocks->value("");
	update_selected_xmt();
	estimate();
	estimate_bc();
}

void cb_use_encoder()
{
	progStatus.encoder = encoders->index()+1;
	update_selected_xmt();
	estimate();
}

void init_encoders()
{
	encoders->clear();
	encoders->add("base64");
	encoders->add("base128");
	encoders->add("base256");
	encoders->index(progStatus.encoder-1);
}

void cb_sync_mode_flamp_fldigi(Fl_Check_Button *b, void *)
{
	progStatus.sync_mode_flamp_fldigi = b->value();
}

void cb_sync_mode_fldigi_flamp(Fl_Check_Button *b, void *)
{
	progStatus.sync_mode_fldigi_flamp = b->value();
}

void cb_fldigi_xmt_mode_change(Fl_Check_Button *b, void *)
{
	if(progStatus.use_txrx_interval == true) {
		progStatus.fldigi_xmt_mode_change = true;
		btn_fldigi_xmt_mode_change->value(progStatus.fldigi_xmt_mode_change);
	} else {
		progStatus.fldigi_xmt_mode_change = b->value();
	}
}

void cb_repeat_at_times(Fl_Check_Button *b, void *)
{
	progStatus.repeat_at_times = btn_repeat_at_times->value();
	if (progStatus.repeat_at_times) {
		btn_repeat_forever->value(0);
		progStatus.repeat_forever = false;
	}
}

void cb_auto_load_que(Fl_Check_Button *b, void *)
{
	int val = false;

	val = btn_auto_load_queue->value();
	progStatus.auto_load_queue_path.assign(txt_auto_load_queue_path->value());

	if(progStatus.auto_load_queue_path.size() < 1 && val == true) {
		progStatus.auto_load_queue = false;
		btn_auto_load_queue->value(false);
		return;
	}
	progStatus.auto_load_queue = val;
	btn_auto_load_queue->value(val);
}

void cb_load_from_tx_folder(Fl_Check_Button *b, void *)
{
	progStatus.load_from_tx_folder = btn_load_from_tx_folder->value();
}

void cb_manual_load_que(Fl_Button *b, void *)
{
	int old = progStatus.auto_load_queue;

	progStatus.auto_load_queue = true;
	auto_load_tx_queue();
	progStatus.auto_load_queue = old;
}

void cb_auto_load_queue_path(Fl_Input2 *b, void *)
{
	progStatus.auto_load_queue_path.assign(txt_auto_load_queue_path->value());
}

void cb_repeat_every(Fl_ComboBox *cb, void *)
{
	progStatus.repeat_every = cbo_repeat_every->index();
}

void cb_repeat_times(Fl_Input2 *txt, void *)
{
	progStatus.repeat_times = txt_repeat_times->value();
}

void cb_repeat_forever(Fl_Check_Button *b, void *)
{
	progStatus.repeat_forever = btn_repeat_forever->value();
	if (progStatus.repeat_forever) {
		btn_repeat_at_times->value(0);
		progStatus.repeat_at_times = false;
	}
}

static void cb_drop_file(Fl_Input*, void*) {
	drop_file_changed();
}

void cb_do_events(Fl_Light_Button *b, void*)
{
	if(generate_time_table) do_events->value(0);

	if (do_events->value() == 1) {
		do_events->label("Stop Events");
	} else {
		do_events->label("Start Events");
	}
	do_events->redraw_label();
}

void cb_enable_txrx_interval(Fl_Check_Button *a, void *b)
{
	progStatus.use_txrx_interval = (bool) btn_enable_txrx_interval->value();

	if(progStatus.use_txrx_interval == true) {
		progStatus.fldigi_xmt_mode_change = true;
		btn_fldigi_xmt_mode_change->value(progStatus.fldigi_xmt_mode_change);
		progStatus.enable_tx_unproto = false;
		btn_enable_tx_unproto->value(false);
		estimate_bc();
	}
}

void cb_tx_interval_mins(Fl_Simple_Counter *a, void *b)
{
	progStatus.tx_interval_minutes = cnt_tx_internval_mins->value();
}

void cb_rx_interval_secs(Fl_Simple_Counter *a, void *b)
{
	progStatus.rx_interval_seconds = cnt_rx_internval_secs->value();
}

void cb_enable_header_modem(Fl_Check_Button *a, void *b)
{
	progStatus.use_header_modem = btn_enable_header_modem->value();

	if(progStatus.use_header_modem) {
		progStatus.enable_tx_unproto = false;
		btn_enable_tx_unproto->value(false);
		estimate_bc();
	}
}

void cb_header_modes(Fl_ComboBox *a, void *b)
{
	progStatus.header_selected_mode = cbo_header_modes->index();
	g_header_modem.assign(cbo_header_modes->value());
}

void cb_disable_header_modem_on_block_fills(Fl_Check_Button *a, void *b)
{
	progStatus.disable_header_modem_on_block_fills = (bool) btn_disable_header_modem_on_block_fills->value();
}

void cb_enable_tx_on_report(Fl_Check_Button *a, void *b)
{
	progStatus.use_tx_on_report = btn_enable_tx_on_report->value();
}

void cb_clear_tosend_on_tx_blocks(Fl_Check_Button *a, void *b)
{
	progStatus.clear_tosend_on_tx_blocks = btn_clear_tosend_on_tx_blocks->value();
}

void cb_enable_tx_unproto(Fl_Check_Button *a, void *b)
{
	progStatus.enable_tx_unproto = btn_enable_tx_unproto->value();

	if(progStatus.enable_tx_unproto) {
		btn_enable_txrx_interval->value(false);
		btn_enable_header_modem->value(false);
		progStatus.use_txrx_interval = false;
		progStatus.use_header_modem = false;
	}

	unproto_widgets();
	estimate();
	estimate_bc();
}

void unproto_widgets(void)
{
	if(progStatus.enable_tx_unproto) {
		cnt_blocksize->deactivate();
		cnt_repeat_nbr->deactivate();
		cnt_repeat_header->deactivate();
		btn_use_compression->deactivate();
		encoders->deactivate();
		txt_tx_numblocks->deactivate();
	} else {
		cnt_blocksize->activate();
		cnt_repeat_nbr->activate();
		cnt_repeat_header->activate();
		btn_use_compression->activate();
		encoders->activate();
		txt_tx_numblocks->activate();
	}
}

void cb_hamcast_mode_cycle(Fl_Check_Button *a, void *b)
{
	progStatus.hamcast_mode_cycle = btn_hamcast_mode_cycle->value();
	if(progStatus.hamcast_mode_cycle) {
		progStatus.hamcast_mode_cycle = assign_bc_modem_list();
		btn_hamcast_mode_cycle->value(progStatus.hamcast_mode_cycle);
	}
	estimate_bc();
}

void cb_hamcast_mode_enable_1(Fl_Check_Button *a, void *b)
{
	progStatus.hamcast_mode_enable_1 = btn_hamcast_mode_enable_1->value();
	if(!assign_bc_modem_list()) {
		progStatus.hamcast_mode_cycle = false;
		btn_hamcast_mode_cycle->value(progStatus.hamcast_mode_cycle);
	}
	estimate_bc();
}

void cb_hamcast_mode_selection_1(Fl_Check_Button *a, void *b)
{
	progStatus.hamcast_mode_selection_1 = cbo_hamcast_mode_selection_1->index();
	if(!assign_bc_modem_list()) {
		progStatus.hamcast_mode_cycle = false;
		btn_hamcast_mode_cycle->value(progStatus.hamcast_mode_cycle);
	}
	estimate_bc();
}

void cb_hamcast_mode_enable_2(Fl_Check_Button *a, void *b)
{
	progStatus.hamcast_mode_enable_2 = btn_hamcast_mode_enable_2->value();
	if(!assign_bc_modem_list()) {
		progStatus.hamcast_mode_cycle = false;
		btn_hamcast_mode_cycle->value(progStatus.hamcast_mode_cycle);
	}
	estimate_bc();
}

void cb_hamcast_mode_selection_2(Fl_Check_Button *a, void *b)
{
	progStatus.hamcast_mode_selection_2 = cbo_hamcast_mode_selection_2->index();
	if(!assign_bc_modem_list()) {
		progStatus.hamcast_mode_cycle = false;
		btn_hamcast_mode_cycle->value(progStatus.hamcast_mode_cycle);
	}
	estimate_bc();
}

void cb_hamcast_mode_enable_3(Fl_Check_Button *a, void *b)
{
	progStatus.hamcast_mode_enable_3 = btn_hamcast_mode_enable_3->value();
	if(!assign_bc_modem_list()) {
		progStatus.hamcast_mode_cycle = false;
		btn_hamcast_mode_cycle->value(progStatus.hamcast_mode_cycle);
	}
	estimate_bc();
}

void cb_hamcast_mode_selection_3(Fl_Check_Button *a, void *b)
{
	progStatus.hamcast_mode_selection_3 = cbo_hamcast_mode_selection_3->index();
	if(!assign_bc_modem_list()) {
		progStatus.hamcast_mode_cycle = false;
		btn_hamcast_mode_cycle->value(progStatus.hamcast_mode_cycle);
	}
	estimate_bc();
}

void cb_hamcast_mode_enable_4(Fl_Check_Button *a, void *b)
{
	progStatus.hamcast_mode_enable_4 = btn_hamcast_mode_enable_4->value();
	if(!assign_bc_modem_list()) {
		progStatus.hamcast_mode_cycle = false;
		btn_hamcast_mode_cycle->value(progStatus.hamcast_mode_cycle);
	}
	estimate_bc();
}

void cb_hamcast_mode_selection_4(Fl_Check_Button *a, void *b)
{
	progStatus.hamcast_mode_selection_4 = cbo_hamcast_mode_selection_4->index();
	if(!assign_bc_modem_list()) {
		progStatus.hamcast_mode_cycle = false;
		btn_hamcast_mode_cycle->value(progStatus.hamcast_mode_cycle);
	}
	estimate_bc();
}

Fl_Double_Window* flamp_dialog() {
	int W = 500, H = 444;
	int X = 2, Y = 26;

	Fl_Double_Window* w = new Fl_Double_Window(W, H, "");;
	w->begin();

	Fl_Menu_Bar* mb = new Fl_Menu_Bar(0, 0, W, 22);
	mb->menu(menu_);
	int y = Y;

	tabs = new Fl_Tabs(X, y = Y, W-4, H-Y-2, "");
	tabs->labelcolor(FL_BLACK);
	tabs->selection_color(fl_rgb_color(245, 255, 250)); // mint cream

	// Receive Tab
	y=Y+26;
	Fl_Group *Rx_tab = new Fl_Group(4, y, W-8, H-y-2, _("Receive"));

	y += 10;
	txt_rx_filename = new Fl_Output(100, y, W-108, 20, _("File:"));
	txt_rx_filename->box(FL_DOWN_BOX);
	txt_rx_filename->tooltip("");

	txt_rx_datetime = new Fl_Output(100, y+=26, W-194, 20, _("Date time:"));
	txt_rx_datetime->box(FL_DOWN_BOX);
	txt_rx_datetime->tooltip("");

	btn_save_file = new Fl_Button(W - 88, y, 80, 20, _("Save"));
	btn_save_file->callback((Fl_Callback*)cb_btn_save_file);
	btn_save_file->tooltip("");

	txt_rx_descrip = new Fl_Output(100, y+=26, W-194, 20, _("Description:"));
	txt_rx_descrip->box(FL_DOWN_BOX);
	txt_rx_descrip->tooltip("");

	btn_rx_remove = new Fl_Button(W - 88, y, 80, 20, _("Remove"));
	btn_rx_remove->callback((Fl_Callback*)cb_btn_rx_remove);
	btn_rx_remove->tooltip("");

	txt_rx_callinfo = new Fl_Output(100, y+=26, W - 108, 20, _("Call/info"));
	txt_rx_callinfo->box(FL_DOWN_BOX);
	txt_rx_callinfo->tooltip("");

	int sp = (W-108 - 4 * 50) * 4 / 10;
	txt_rx_filesize = new Fl_Output(100, y+=26, 50, 20, _("# bytes"));
	txt_rx_filesize->box(FL_DOWN_BOX);
	txt_rx_filesize->tooltip(_("# of data bytes"));

	txt_rx_numblocks = new Fl_Output(100 + (50+sp), y, 50, 20, _("Nbr blks"));
	txt_rx_numblocks->box(FL_DOWN_BOX);
	txt_rx_numblocks->tooltip("");

	txt_rx_blocksize = new Fl_Output(100 + 2*(50+sp), y, 50, 20, _("Blk size"));
	txt_rx_blocksize->box(FL_DOWN_BOX);
	txt_rx_blocksize->tooltip("");

	btn_rxq_to_txq = new Fl_Button(W - 88, y, 80, 20, _("To TxQ"));
	btn_rxq_to_txq->callback((Fl_Callback*)cb_btn_transfer_file_txQ);
	btn_rxq_to_txq->tooltip("");

	txt_rx_missing_blocks = new Fl_Output(100, y+=26, W-194, 20, _("Missing"));
	txt_rx_missing_blocks->box(FL_DOWN_BOX);
	txt_rx_missing_blocks->tooltip(_("Blocks not yet received"));

	btn_copy_missing = new Fl_Button(W - 88, y, 80, 20, _("Report"));
	btn_copy_missing->callback((Fl_Callback*)cb_btn_copy_missing);
	btn_copy_missing->tooltip(_("Confirmation report-->fldigi"));

	rx_progress = new Fl_BlockMap(100, y+=26, W-108, 20, _("Blocks"));
	rx_progress->box(FL_DOWN_BOX);

	txt_rx_output = new FTextView(8, y+=32, W-16, 80, "Data");
	txt_rx_output->box(FL_DOWN_BOX);
	txt_rx_output->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);
	txt_rx_output->tooltip(_("Ascii Text\nData type message"));

	static const int cols[] = {60, 0};
	y += 102;
	rx_queue = new Fl_Hold_Browser(8, y, W-16, H-y-6, _("Receive Queue"));
	rx_queue->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);
	rx_queue->column_widths(cols);
	rx_queue->has_scrollbar(Fl_Browser_::VERTICAL_ALWAYS);
	rx_queue->callback((Fl_Callback*)cb_rx_queue);
	rx_queue->tooltip("");

	Rx_tab->resizable(txt_rx_output);
	Rx_tab->end();

	// Receive Tab End

	// Transmit Tab
	y = Y + 26;
	Fl_Group *Tx_tab = new Fl_Group(X+2, y, W-2*(X+2), H-y-2, _("Transmit"));

	y += 10;

	txt_tx_send_to = new Fl_Input2(X+70, y, W - 78, 20, _("Send to"));
	txt_tx_send_to->box(FL_DOWN_BOX);
	txt_tx_send_to->tooltip(_("QST (or blank) / Enumerated callsigns"));
	txt_tx_send_to->value("QST");

	txt_tx_filename = new Fl_Output(70, y+=26, W - 78, 20, _("File"));
	txt_tx_filename->box(FL_DOWN_BOX);
	txt_tx_filename->tooltip("");

	txt_tx_descrip = new Fl_Input2(70, y+=26, W - 78, 20, _("Descrip"));
	txt_tx_descrip->box(FL_DOWN_BOX);
	txt_tx_descrip->tooltip(_("Short description of file contents"));
	txt_tx_descrip->callback((Fl_Callback*)cb_tx_descrip);

	cnt_blocksize = new Fl_Simple_Counter(X+70, y+=26, 60, 20, _("Blk size"));
	cnt_blocksize->step(CNT_BLOCK_SIZE_STEP_RATE);
	cnt_blocksize->value(64);
	cnt_blocksize->minimum(CNT_BLOCK_SIZE_MINIMUM);
	cnt_blocksize->maximum(CNT_BLOCK_SIZE_MAXIMUM);
	cnt_blocksize->align(FL_ALIGN_LEFT);
	cnt_blocksize->callback((Fl_Callback*)cb_cnt_blocksize);
	cnt_blocksize->tooltip(_("Maximum size of each data block"));

	cnt_repeat_nbr = new Fl_Simple_Counter(X+70+60+65, y, 60, 20, _("Xmt Rpt"));
	cnt_repeat_nbr->step(1);
	cnt_repeat_nbr->value(1);
	cnt_repeat_nbr->minimum(1);
	cnt_repeat_nbr->maximum(999);
	cnt_repeat_nbr->align(FL_ALIGN_LEFT);
	cnt_repeat_nbr->callback((Fl_Callback*)cb_cnt_repeat_nbr);
	cnt_repeat_nbr->tooltip(_("Repeat transmission specified # times"));

	cnt_repeat_header = new Fl_Simple_Counter(X+70+60+65+60+65, y, 60, 20, _("Hdr Rpt"));
	cnt_repeat_header->step(1);
	cnt_repeat_header->value(1);
	cnt_repeat_header->minimum(1);
	cnt_repeat_header->maximum(10);
	cnt_repeat_header->align(FL_ALIGN_LEFT);
	cnt_repeat_header->callback((Fl_Callback*)cb_repeat_header);
	cnt_repeat_header->tooltip(_("Repeat header #-times/transmission"));

	txt_tx_numblocks = new Fl_Output(W-10-50, y, 50, 20, "# Bks");
	txt_tx_numblocks->align(FL_ALIGN_LEFT);
	txt_tx_numblocks->tooltip(_("Transfer size in blocks"));
	txt_tx_numblocks->value("");

	txt_tx_selected_blocks = new Fl_Input2(X+70, y+=26, W - X - 162, 20, _("blocks"));
	txt_tx_selected_blocks->box(FL_DOWN_BOX);
	txt_tx_selected_blocks->tooltip(_("Clear for all\nComma separated block #s"));
	txt_tx_selected_blocks->callback((Fl_Callback*)cb_selected_blocks);

	btn_parse_blocks = new Fl_Button(W - 88, y, 80, 20, _("Fetch"));
	btn_parse_blocks->callback((Fl_Callback*)cb_btn_parse_blocks);
	btn_parse_blocks->tooltip(_("Fetch & parse fldigi block reports"));

	btn_use_compression = new Fl_Check_Button(55, y+=26, 30, 20, _("Comp"));
	btn_use_compression->tooltip(_("Data will be sent compressed"));
	btn_use_compression->align(FL_ALIGN_LEFT);
	btn_use_compression->down_box(FL_DOWN_BOX);
	btn_use_compression->callback((Fl_Callback*)cb_use_compression);
	btn_use_compression->value(progStatus.use_compression);

	encoders = new Fl_ComboBox(80, y, 100, 20, "");
	encoders->begin();
	encoders->when(FL_WHEN_RELEASE);
	encoders->tooltip(_("Encode after compression"));
	encoders->callback((Fl_Callback*)cb_use_encoder);
	encoders->end();

	cbo_modes = new Fl_ComboBox(185, y, 118, 20, "");
	cbo_modes->begin();
	cbo_modes->align(FL_ALIGN_RIGHT);
	cbo_modes->when(FL_WHEN_RELEASE);
	cbo_modes->tooltip(_("fldigi modem type"));
	cbo_modes->box(FL_DOWN_BOX);
	cbo_modes->color(FL_BACKGROUND2_COLOR);
	cbo_modes->selection_color(FL_BACKGROUND_COLOR);
	cbo_modes->labeltype(FL_NORMAL_LABEL);
	cbo_modes->labelfont(0);
	cbo_modes->labelsize(14);
	cbo_modes->labelcolor(FL_FOREGROUND_COLOR);
	cbo_modes->callback((Fl_Callback*)cb_cbo_modes);
	cbo_modes->end();

	txt_transfer_size_time = new Fl_Output(306, y, 186, 20, "");
	txt_transfer_size_time->tooltip(_("Transfer size / time"));
	txt_transfer_size_time->value("");

	btn_send_file = new Fl_Button(W - 370, y+=25, 70, 22, _(XMT_LABEL));
	btn_send_file->callback((Fl_Callback*)cb_btn_send_file);
	btn_send_file->tooltip(_("Transmit this file"));

	btn_send_queue = new Fl_Button(W - 370 + 76, y, 70, 22, _("Xmt All"));
	btn_send_queue->callback((Fl_Callback*)cb_btn_send_queue);
	btn_send_queue->tooltip(_("Transmit entire queue"));

	btn_tx_remove_file = new Fl_Button(W -370 + 2*76, y, 70, 22, _("Remove"));
	btn_tx_remove_file->callback((Fl_Callback*)cb_btn_tx_remove_file);
	btn_tx_remove_file->tooltip(_("Remove highlighted file from transmit queue"));

	btn_open_file = new Fl_Button(W - 370 + 3*76, y, 70, 22, _("Add"));
	btn_open_file->callback((Fl_Callback*)cb_btn_open_file);
	btn_open_file->tooltip(_("Select file to add to queue"));

	drop_file = new Fl_Input(W - 370 + 4*76, y - 2, 50, 24);
	drop_file->box(FL_OVAL_BOX);
	drop_file->textcolor(fl_rgb_color( 200, 0, 0) );
	drop_file->value("  DnD");
	drop_file->color(fl_rgb_color(244, 255, 255));
	drop_file->cursor_color(fl_rgb_color(244, 255, 255));
	drop_file->label("");
	drop_file->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	drop_file->labelcolor(fl_rgb_color( 200, 0, 0) );
	drop_file->tooltip(_("drag and drop tx queue files here ..."));
	drop_file->callback((Fl_Callback*)cb_drop_file);
	drop_file->when(FL_WHEN_CHANGED);

	y += 26;
	tx_queue = new Fl_Hold_Browser(8, y, W-16, H-y-6, _("Transmit Queue"));
	tx_queue->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);
	tx_queue->has_scrollbar(Fl_Browser_::VERTICAL_ALWAYS);
	tx_queue->callback((Fl_Callback*)cb_tx_queue);
	tx_queue->tooltip("");

	Tx_tab->resizable(tx_queue);
	Tx_tab->end();

	// Transmit Tab End

	// Timed Even Tab

	//Fl_Group *Timed_Events_tab = new Fl_Group(X+2, y=Y+26, W-2*(X+2), H-y-2, _("Events"));
	y = Y+26;

	Fl_Group *Events_tab = new Fl_Group(X+2, y, W-2*(X+2), H-y-2, _("Events"));

	Fl_Tabs *event_tabs = new Fl_Tabs(X+2, y, W-2*(X+2), H-y-2, "");
	event_tabs->labelcolor(FL_BLACK);
	event_tabs->selection_color(fl_rgb_color(245, 255, 250)); // mint cream


	//y += 16;
	y = Y + 52;
	Fl_Group *Timed_Events_tab = new Fl_Group(X+2, y, W-2*(X+2), H-y-2, _("Timed"));
	y += 8;

	Fl_Multiline_Output* explain_events = new Fl_Multiline_Output(
																  X+4, y, W - X - 8, 84, "");
	explain_events->tooltip("");
	explain_events->color(fl_rgb_color(255, 250, 205));

	explain_events->value("\tTimed / Continuous events :\n" \
						  "\tEach transmission is identical to a 'Xmt All', that is the\n" \
						  "\tentire queue is transmitted.  The unproto 'QST (calls) de URCALL'\n" \
						  "\tand the program identifier '<PROG 11 8E48>FLAMP 2.x.x' are\n" \
						  "\tincluded." \
						  );

	//Fl_Group *Timed_Repeat_grp = new Fl_Group(X+4, y+=126, W-2*(X+4), 146, _("Timed Events")); //120
	Fl_Group *Timed_Repeat_grp = new Fl_Group(X+4, y+=100, W-2*(X+4), 146, _("Timed Events")); //120
	Timed_Repeat_grp->box(FL_ENGRAVED_BOX);
	Timed_Repeat_grp->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);

	btn_repeat_at_times = new Fl_Check_Button(X+10, y+=8, 20, 20,
											  _("Scheduled times of transmission"));
	btn_repeat_at_times->tooltip("");
	btn_repeat_at_times->align(FL_ALIGN_RIGHT);
	btn_repeat_at_times->down_box(FL_DOWN_BOX);
	btn_repeat_at_times->callback((Fl_Callback*)cb_repeat_at_times);
	btn_repeat_at_times->value(progStatus.repeat_at_times);

	cbo_repeat_every = new Fl_ComboBox(X + 250, y, 140, 20, "TX interval"); //20
	cbo_repeat_every->begin();
	cbo_repeat_every->align(FL_ALIGN_RIGHT);
	cbo_repeat_every->when(FL_WHEN_RELEASE);
	cbo_repeat_every->tooltip("");
	cbo_repeat_every->callback((Fl_Callback*)cb_repeat_every);
	cbo_repeat_every->end();

	txt_repeat_times = new Fl_Input2(X+10, y+=42, W -X -20, 20, "Xmt times (HHMM)");
	txt_repeat_times->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);
	txt_repeat_times->tooltip(_("Space/comma delimited times"));
	txt_repeat_times->callback((Fl_Callback*)cb_repeat_times);
	txt_repeat_times->when(FL_WHEN_CHANGED);
	txt_repeat_times->value(progStatus.repeat_times.c_str());

	btn_auto_load_queue = new Fl_Check_Button(X+10, y+=24, 20, 20,
											  _("Auto Load TX Queue"));
	btn_auto_load_queue->tooltip("");
	btn_auto_load_queue->align(FL_ALIGN_RIGHT);
	btn_auto_load_queue->down_box(FL_DOWN_BOX);
	btn_auto_load_queue->callback((Fl_Callback*)cb_auto_load_que);
	btn_auto_load_queue->value(progStatus.auto_load_queue);

	btn_load_from_tx_folder = new Fl_Check_Button(X+200, y, 20, 20,
												  _("Load from TX directory"));
	btn_load_from_tx_folder->tooltip("");
	btn_load_from_tx_folder->align(FL_ALIGN_RIGHT);
	btn_load_from_tx_folder->down_box(FL_DOWN_BOX);
	btn_load_from_tx_folder->callback((Fl_Callback*)cb_load_from_tx_folder);
	btn_load_from_tx_folder->value(progStatus.load_from_tx_folder);

	btn_manual_load_queue = new Fl_Button(W - 116, y+12, 90, 22, _("Load Queue"));
	btn_manual_load_queue->callback((Fl_Callback*)cb_manual_load_que);
	btn_manual_load_queue->tooltip(_("Transmit this file"));

	txt_auto_load_queue_path = new Fl_Input2(X+10, y+=42, W -X -20, 20, "Path to Load Queue File List");
	txt_auto_load_queue_path->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);
	txt_auto_load_queue_path->tooltip(_("Path to the file containing a list of files"));
	txt_auto_load_queue_path->callback((Fl_Callback*)cb_auto_load_queue_path);
	txt_auto_load_queue_path->when(FL_WHEN_CHANGED);
	txt_auto_load_queue_path->value(progStatus.auto_load_queue_path.c_str());

	Timed_Repeat_grp->end();

	Fl_Group* Continuous_Events_grp = new Fl_Group(X+4, y+=52, W-2*(X+4), 36, _("Continuous repeat"));
	Continuous_Events_grp->box(FL_ENGRAVED_BOX);
	Continuous_Events_grp->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);

	btn_repeat_forever = new Fl_Check_Button(X+10, y+8, 20, 20,
											 _("Continuous repeat of transmission"));
	btn_repeat_forever->tooltip("");
	btn_repeat_forever->align(FL_ALIGN_RIGHT);
	btn_repeat_forever->down_box(FL_DOWN_BOX);
	btn_repeat_forever->callback((Fl_Callback*)cb_repeat_forever);
	btn_repeat_forever->value(progStatus.repeat_forever);

	Continuous_Events_grp->end();

	outTimeValue = new Fl_Output(X+20, y+=44, 70, 24, ""); //52
	outTimeValue->box(FL_DOWN_BOX);
	outTimeValue->color(fl_rgb_color(255, 250, 205));
	outTimeValue->value("");

	do_events = new Fl_Light_Button(X+100, y, 120, 24, _("Start Events"));
	do_events->callback((Fl_Callback*)cb_do_events);

	Timed_Events_tab->end();

	// Timed Even Tab End
	// Timed Events Tab End
	y = Y + 52;
	Fl_Group *Hamcast_Events_tab = new Fl_Group(X+2, y, W-2*(X+2), H-y-2, _("Hamcast"));
	y += 8;

	y += 16;
	int lx = X+12;

	Fl_Group *Hamcast_mode_group = new Fl_Group(X+4, y+=8, W-2*(X+4), 146, _("Hamcast modem rotation / Queue Transmit Time (on Timed Events)")); //120
	Hamcast_mode_group->box(FL_ENGRAVED_BOX);
	Hamcast_mode_group->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);

	btn_hamcast_mode_cycle = new Fl_Check_Button(lx, y+=8, 20, 20,
												 _("Enable modem rotation"));
	btn_hamcast_mode_cycle->tooltip("");
	btn_hamcast_mode_cycle->align(FL_ALIGN_RIGHT);
	btn_hamcast_mode_cycle->down_box(FL_DOWN_BOX);
	btn_hamcast_mode_cycle->callback((Fl_Callback*)cb_hamcast_mode_cycle);
	btn_hamcast_mode_cycle->value(progStatus.hamcast_mode_cycle);

	txt_hamcast_select_total_time = new Fl_Output(lx+370, y, 100, 20, "Total Time: ");
	txt_hamcast_select_total_time->tooltip(_("Transfer time"));
	txt_hamcast_select_total_time->value("");

	btn_hamcast_mode_enable_1 = new Fl_Check_Button(lx, y+=26, 20, 20, _("Enable Modem 1"));
	btn_hamcast_mode_enable_1->tooltip("");
	btn_hamcast_mode_enable_1->align(FL_ALIGN_RIGHT);
	btn_hamcast_mode_enable_1->down_box(FL_DOWN_BOX);
	btn_hamcast_mode_enable_1->callback((Fl_Callback*)cb_hamcast_mode_enable_1);
	btn_hamcast_mode_enable_1->value(progStatus.hamcast_mode_enable_1);

	cbo_hamcast_mode_selection_1 = new Fl_ComboBox(lx+140, y, 118, 20, "");
	cbo_hamcast_mode_selection_1->begin();
	cbo_hamcast_mode_selection_1->align(FL_ALIGN_RIGHT);
	cbo_hamcast_mode_selection_1->when(FL_WHEN_RELEASE);
	cbo_hamcast_mode_selection_1->tooltip(_("fldigi modem type"));
	cbo_hamcast_mode_selection_1->box(FL_DOWN_BOX);
	cbo_hamcast_mode_selection_1->color(FL_BACKGROUND2_COLOR);
	cbo_hamcast_mode_selection_1->selection_color(FL_BACKGROUND_COLOR);
	cbo_hamcast_mode_selection_1->labeltype(FL_NORMAL_LABEL);
	cbo_hamcast_mode_selection_1->labelfont(0);
	cbo_hamcast_mode_selection_1->labelsize(14);
	cbo_hamcast_mode_selection_1->labelcolor(FL_FOREGROUND_COLOR);
	cbo_hamcast_mode_selection_1->callback((Fl_Callback*)cb_hamcast_mode_selection_1);
	cbo_hamcast_mode_selection_1->end();

	txt_hamcast_select_1_time = new Fl_Output(lx+370, y, 100, 20, "Time: ");
	txt_hamcast_select_1_time->tooltip(_("Transfer time"));
	txt_hamcast_select_1_time->value("");

	btn_hamcast_mode_enable_2 = new Fl_Check_Button(lx, y+=26, 20, 20, _("Enable Modem 2"));
	btn_hamcast_mode_enable_2->tooltip("");
	btn_hamcast_mode_enable_2->align(FL_ALIGN_RIGHT);
	btn_hamcast_mode_enable_2->down_box(FL_DOWN_BOX);
	btn_hamcast_mode_enable_2->callback((Fl_Callback*)cb_hamcast_mode_enable_2);
	btn_hamcast_mode_enable_2->value(progStatus.hamcast_mode_enable_2);

	cbo_hamcast_mode_selection_2 = new Fl_ComboBox(lx+140, y, 118, 20, "");
	cbo_hamcast_mode_selection_2->begin();
	cbo_hamcast_mode_selection_2->align(FL_ALIGN_RIGHT);
	cbo_hamcast_mode_selection_2->when(FL_WHEN_RELEASE);
	cbo_hamcast_mode_selection_2->tooltip(_("fldigi modem type"));
	cbo_hamcast_mode_selection_2->box(FL_DOWN_BOX);
	cbo_hamcast_mode_selection_2->color(FL_BACKGROUND2_COLOR);
	cbo_hamcast_mode_selection_2->selection_color(FL_BACKGROUND_COLOR);
	cbo_hamcast_mode_selection_2->labeltype(FL_NORMAL_LABEL);
	cbo_hamcast_mode_selection_2->labelfont(0);
	cbo_hamcast_mode_selection_2->labelsize(14);
	cbo_hamcast_mode_selection_2->labelcolor(FL_FOREGROUND_COLOR);
	cbo_hamcast_mode_selection_2->callback((Fl_Callback*)cb_hamcast_mode_selection_2);
	cbo_hamcast_mode_selection_2->end();

	txt_hamcast_select_2_time = new Fl_Output(lx+370, y, 100, 20, "Time: ");
	txt_hamcast_select_2_time->tooltip(_("Transfer time"));
	txt_hamcast_select_2_time->value("");

	btn_hamcast_mode_enable_3 = new Fl_Check_Button(lx, y+=26, 20, 20, _("Enable Modem 3"));
	btn_hamcast_mode_enable_3->tooltip("");
	btn_hamcast_mode_enable_3->align(FL_ALIGN_RIGHT);
	btn_hamcast_mode_enable_3->down_box(FL_DOWN_BOX);
	btn_hamcast_mode_enable_3->callback((Fl_Callback*)cb_hamcast_mode_enable_3);
	btn_hamcast_mode_enable_3->value(progStatus.hamcast_mode_enable_3);

	cbo_hamcast_mode_selection_3 = new Fl_ComboBox(lx+140, y, 118, 20, "");
	cbo_hamcast_mode_selection_3->begin();
	cbo_hamcast_mode_selection_3->align(FL_ALIGN_RIGHT);
	cbo_hamcast_mode_selection_3->when(FL_WHEN_RELEASE);
	cbo_hamcast_mode_selection_3->tooltip(_("fldigi modem type"));
	cbo_hamcast_mode_selection_3->box(FL_DOWN_BOX);
	cbo_hamcast_mode_selection_3->color(FL_BACKGROUND2_COLOR);
	cbo_hamcast_mode_selection_3->selection_color(FL_BACKGROUND_COLOR);
	cbo_hamcast_mode_selection_3->labeltype(FL_NORMAL_LABEL);
	cbo_hamcast_mode_selection_3->labelfont(0);
	cbo_hamcast_mode_selection_3->labelsize(14);
	cbo_hamcast_mode_selection_3->labelcolor(FL_FOREGROUND_COLOR);
	cbo_hamcast_mode_selection_3->callback((Fl_Callback*)cb_hamcast_mode_selection_3);
	cbo_hamcast_mode_selection_3->end();

	txt_hamcast_select_3_time = new Fl_Output(lx+370, y, 100, 20, "Time: ");
	txt_hamcast_select_3_time->tooltip(_("Transfer time"));
	txt_hamcast_select_3_time->value("");

	btn_hamcast_mode_enable_4 = new Fl_Check_Button(lx, y+=26, 20, 20, _("Enable Modem 4"));
	btn_hamcast_mode_enable_4->tooltip("");
	btn_hamcast_mode_enable_4->align(FL_ALIGN_RIGHT);
	btn_hamcast_mode_enable_4->down_box(FL_DOWN_BOX);
	btn_hamcast_mode_enable_4->callback((Fl_Callback*)cb_hamcast_mode_enable_4);
	btn_hamcast_mode_enable_4->value(progStatus.hamcast_mode_enable_4);

	cbo_hamcast_mode_selection_4 = new Fl_ComboBox(lx+140, y, 118, 20, "");
	cbo_hamcast_mode_selection_4->begin();
	cbo_hamcast_mode_selection_4->align(FL_ALIGN_RIGHT);
	cbo_hamcast_mode_selection_4->when(FL_WHEN_RELEASE);
	cbo_hamcast_mode_selection_4->tooltip(_("fldigi modem type"));
	cbo_hamcast_mode_selection_4->box(FL_DOWN_BOX);
	cbo_hamcast_mode_selection_4->color(FL_BACKGROUND2_COLOR);
	cbo_hamcast_mode_selection_4->selection_color(FL_BACKGROUND_COLOR);
	cbo_hamcast_mode_selection_4->labeltype(FL_NORMAL_LABEL);
	cbo_hamcast_mode_selection_4->labelfont(0);
	cbo_hamcast_mode_selection_4->labelsize(14);
	cbo_hamcast_mode_selection_4->labelcolor(FL_FOREGROUND_COLOR);
	cbo_hamcast_mode_selection_4->callback((Fl_Callback*)cb_hamcast_mode_selection_4);
	cbo_hamcast_mode_selection_4->end();

	txt_hamcast_select_4_time = new Fl_Output(lx+370, y, 100, 20, "Time: ");
	txt_hamcast_select_4_time->tooltip(_("Transfer time"));
	txt_hamcast_select_4_time->value("");
	Hamcast_Events_tab->end();
	// Hamcast Events Tab End

	event_tabs->add(Timed_Events_tab);
	event_tabs->add(Hamcast_Events_tab);

	event_tabs->end();
	Events_tab->end();
	// Events Tab End

	// Configuration Tab
	y = Y + 26;
	Fl_Group *Config_tab = new Fl_Group(X+2, y, W-2*(X+2), H-y-2, _("Configure"));

	y += 10;

	txt_tx_mycall = new Fl_Input2(X+70, y, 150, 20, _("Callsign"));
	txt_tx_mycall->box(FL_DOWN_BOX);
	txt_tx_mycall->tooltip(_("Callsign of transmitting station"));
	txt_tx_mycall->callback((Fl_Callback*)cb_tx_mycall);

	txt_tx_myinfo = new Fl_Input2(X+70, y+=26, W-78, 20, _("Info"));
	txt_tx_myinfo->box(FL_DOWN_BOX);
	txt_tx_myinfo->tooltip(_("QTH etc. of transmitting station"));
	txt_tx_myinfo->callback((Fl_Callback*)cb_tx_myinfo);


	btn_sync_mode_flamp_fldigi = new Fl_Check_Button(X+70, y+=32, 20, 20,
													 _("Auto sync fldigi to flamp mode selector"));
	btn_sync_mode_flamp_fldigi->tooltip("");
	btn_sync_mode_flamp_fldigi->align(FL_ALIGN_RIGHT);
	btn_sync_mode_flamp_fldigi->down_box(FL_DOWN_BOX);
	btn_sync_mode_flamp_fldigi->callback((Fl_Callback*)cb_sync_mode_flamp_fldigi);
	btn_sync_mode_flamp_fldigi->value(progStatus.sync_mode_flamp_fldigi);

	btn_sync_mode_fldigi_flamp = new Fl_Check_Button(X+70, y+=26, 20, 20,
													 _("Auto sync flamp to fldigi mode selector"));
	btn_sync_mode_fldigi_flamp->tooltip("");
	btn_sync_mode_fldigi_flamp->align(FL_ALIGN_RIGHT);
	btn_sync_mode_fldigi_flamp->down_box(FL_DOWN_BOX);
	btn_sync_mode_fldigi_flamp->callback((Fl_Callback*)cb_sync_mode_fldigi_flamp);
	btn_sync_mode_fldigi_flamp->value(progStatus.sync_mode_fldigi_flamp);

	btn_fldigi_xmt_mode_change = new Fl_Check_Button(X+70, y+=26, 20, 20,
													 _("Change fldigi mode just prior to transmit"));
	btn_fldigi_xmt_mode_change->tooltip("");
	btn_fldigi_xmt_mode_change->align(FL_ALIGN_RIGHT);
	btn_fldigi_xmt_mode_change->down_box(FL_DOWN_BOX);
	btn_fldigi_xmt_mode_change->callback((Fl_Callback*)cb_fldigi_xmt_mode_change);
	btn_fldigi_xmt_mode_change->value(progStatus.fldigi_xmt_mode_change);

	btn_enable_tx_on_report = new Fl_Check_Button(X+70, y+=26, 20, 20,
												  _("Enable Tx on Report"));
	btn_enable_tx_on_report->tooltip("");
	btn_enable_tx_on_report->align(FL_ALIGN_RIGHT);
	btn_enable_tx_on_report->down_box(FL_DOWN_BOX);
	btn_enable_tx_on_report->callback((Fl_Callback*)cb_enable_tx_on_report);
	btn_enable_tx_on_report->value(progStatus.use_tx_on_report);

	btn_enable_tx_unproto = new Fl_Check_Button(X+70, y+=26, 20, 20,
												_("Transmit unproto (plain text, 7bit ASCII)"));
	btn_enable_tx_unproto->tooltip("");
	btn_enable_tx_unproto->align(FL_ALIGN_RIGHT);
	btn_enable_tx_unproto->down_box(FL_DOWN_BOX);
	btn_enable_tx_unproto->callback((Fl_Callback*)cb_enable_tx_unproto);
	btn_enable_tx_unproto->value(progStatus.enable_tx_unproto);

	btn_clear_tosend_on_tx_blocks = new Fl_Check_Button(X+70, y+=26, 20, 20,
														_("Clear Missing Blocks on Non-Canceled Transmit(s)"));
	btn_clear_tosend_on_tx_blocks->tooltip("");
	btn_clear_tosend_on_tx_blocks->align(FL_ALIGN_RIGHT);
	btn_clear_tosend_on_tx_blocks->down_box(FL_DOWN_BOX);
	btn_clear_tosend_on_tx_blocks->callback((Fl_Callback*)cb_clear_tosend_on_tx_blocks);
	btn_clear_tosend_on_tx_blocks->value(progStatus.clear_tosend_on_tx_blocks);


	btn_disable_header_modem_on_block_fills= new Fl_Check_Button(X+70, y+=26, 20, 20,
																 _("Inhibit header modem on block fills"));
	btn_disable_header_modem_on_block_fills->tooltip("");
	btn_disable_header_modem_on_block_fills->align(FL_ALIGN_RIGHT);
	btn_disable_header_modem_on_block_fills->down_box(FL_DOWN_BOX);
	btn_disable_header_modem_on_block_fills->callback((Fl_Callback*)cb_disable_header_modem_on_block_fills);
	btn_disable_header_modem_on_block_fills->value(progStatus.disable_header_modem_on_block_fills);

	btn_enable_header_modem = new Fl_Check_Button(X+70, y+=26, 20, 20,
												  _("Enable header modem (Sync flamp to fldigi ignored)"));
	btn_enable_header_modem->tooltip("");
	btn_enable_header_modem->align(FL_ALIGN_RIGHT);
	btn_enable_header_modem->down_box(FL_DOWN_BOX);
	btn_enable_header_modem->callback((Fl_Callback*)cb_enable_header_modem);
	btn_enable_header_modem->value(progStatus.use_header_modem);

	cbo_header_modes = new Fl_ComboBox(X+94, y+=26, 118, 20, "");
	cbo_header_modes->begin();
	cbo_header_modes->align(FL_ALIGN_RIGHT);
	cbo_header_modes->when(FL_WHEN_RELEASE);
	cbo_header_modes->tooltip(_("fldigi modem type"));
	cbo_header_modes->box(FL_DOWN_BOX);
	cbo_header_modes->color(FL_BACKGROUND2_COLOR);
	cbo_header_modes->selection_color(FL_BACKGROUND_COLOR);
	cbo_header_modes->labeltype(FL_NORMAL_LABEL);
	cbo_header_modes->labelfont(0);
	cbo_header_modes->labelsize(14);
	cbo_header_modes->labelcolor(FL_FOREGROUND_COLOR);
	cbo_header_modes->callback((Fl_Callback*)cb_header_modes);
	cbo_header_modes->end();

	btn_enable_txrx_interval = new Fl_Check_Button(X+70, y+=26, 20, 20,
												   _("Enable TX/RX Interval"));
	btn_enable_txrx_interval->tooltip("");
	btn_enable_txrx_interval->align(FL_ALIGN_RIGHT);
	btn_enable_txrx_interval->down_box(FL_DOWN_BOX);
	btn_enable_txrx_interval->callback((Fl_Callback*)cb_enable_txrx_interval);
	btn_enable_txrx_interval->value(progStatus.use_txrx_interval);
	
	cnt_tx_internval_mins = new Fl_Simple_Counter(X+94, y+=26, 60, 20, _("Tx Duration Mins"));
	cnt_tx_internval_mins->step(1);
	cnt_tx_internval_mins->value(progStatus.tx_interval_minutes);
	cnt_tx_internval_mins->minimum(1);
	cnt_tx_internval_mins->maximum(ID_TIME_MINUTES);
	cnt_tx_internval_mins->align(FL_ALIGN_RIGHT);
	cnt_tx_internval_mins->callback((Fl_Callback*)cb_tx_interval_mins);
	cnt_tx_internval_mins->tooltip(_("Transmit Duration in Minutes"));
	
	cnt_rx_internval_secs = new Fl_Simple_Counter(X+94, y+=26, 60, 20, _("Rx Duration Secs"));
	cnt_rx_internval_secs->step(1);
	cnt_rx_internval_secs->value(progStatus.rx_interval_seconds);
	cnt_rx_internval_secs->minimum(1);
	cnt_rx_internval_secs->maximum(120);
	cnt_rx_internval_secs->align(FL_ALIGN_RIGHT);
	cnt_rx_internval_secs->callback((Fl_Callback*)cb_rx_interval_secs);
	cnt_rx_internval_secs->tooltip(_("Receive Duration in Seconds"));
	
	
	Config_tab->end();
	
	// Configuration Tab End
	
	tabs->add(Rx_tab);
	tabs->add(Tx_tab);
	tabs->add(Events_tab);
	tabs->add(Config_tab);
	
	tabs->end();
	w->end();
	
	init_encoders();
	init_cbo_modes();
	init_cbo_events();
	
	return w;
}

