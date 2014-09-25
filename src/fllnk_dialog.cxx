// =====================================================================
//
// fllnk_dialog.cxx
//
// Author(s):
//	Dave Freese, W1HKJ, Copyright (C) 2010, 2011, 2012, 2013
//  Robert Stiles, KK5VD, Copyright (C) 2013, 2014
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

#include "config.h"

#include "gettext.h"
#include "fllnk_dialog.h"
#include "status.h"
#include "fllnk.h"
#include "fileselect.h"
#include "debug.h"
#include "icons.h"

#include "xml_io.h"
#include "file_io.h"
#include "amp.h"
#include "ztimer.h"

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

Fl_Button *btn_send_relay = 0;
Fl_Button *btn_parse_relay_blocks = 0;
Fl_Input2 *txt_relay_selected_blocks = 0;


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

Fl_Check_Button* btn_enable_delete_warning = 0;

Fl_Check_Button* btn_enable_unproto_markers = 0;

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
std::string valid_modes;

bool valid_mode_check(std::string &md)
{
	return (valid_modes.find(md) != string::npos);
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
	fl_message2("\tFllnk: %s\n\n" \
				"\tAuthors:\n" \
				"\t\tRobert Stiles, KK5VD",
				"\t\tDave Freese, W1HKJ\n" \
				FLLNK_VERSION);
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

void cb_sync_mode_flamp_fldigi(Fl_Check_Button *b, void *)
{
	progStatus.sync_mode_flamp_fldigi = btn_sync_mode_flamp_fldigi->value();
}

void cb_sync_mode_fldigi_flamp(Fl_Check_Button *b, void *)
{
	progStatus.sync_mode_fldigi_flamp = btn_sync_mode_fldigi_flamp->value();
}

void cb_fldigi_xmt_mode_change(Fl_Check_Button *b, void *)
{
	if(progStatus.use_txrx_interval == true) {
		progStatus.fldigi_xmt_mode_change = true;
		btn_fldigi_xmt_mode_change->value(progStatus.fldigi_xmt_mode_change);
	} else {
		progStatus.fldigi_xmt_mode_change = btn_fldigi_xmt_mode_change->value();
	}
}

Fl_Double_Window* fllnk_dialog() {

	int W = 500, H = 496;
	int X = 2, Y = 26;

	Fl_Double_Window* w = new Fl_Double_Window(W, H, "");;
	w->begin();

	Fl_Menu_Bar* mb = new Fl_Menu_Bar(0, 0, W, 22);
	mb->menu(menu_);
	int y = Y;

	tabs = new Fl_Tabs(X, y = Y, W-4, H-Y-2, "");
	tabs->labelcolor(FL_BLACK);
	tabs->selection_color(fl_rgb_color(245, 255, 250)); // mint cream

	// Connect Tab

	y=Y+26;
	Fl_Group *connect_tab = new Fl_Group(4, y, W-8, H-y-2, _("Connect"));

	y += 10;
	txt_rx_filename = new Fl_Output(100, y, W-108, 20, _("File:"));
	txt_rx_filename->box(FL_DOWN_BOX);
	txt_rx_filename->tooltip("");

	txt_rx_datetime = new Fl_Output(100, y+=26, W-194, 20, _("Date time:"));
	txt_rx_datetime->box(FL_DOWN_BOX);
	txt_rx_datetime->tooltip("");


	txt_rx_descrip = new Fl_Output(100, y+=26, W-194, 20, _("Description:"));
	txt_rx_descrip->box(FL_DOWN_BOX);
	txt_rx_descrip->tooltip("");

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

	txt_rx_missing_blocks = new Fl_Output(100, y+=26, W-194, 20, _("Missing"));
	txt_rx_missing_blocks->box(FL_DOWN_BOX);
	txt_rx_missing_blocks->tooltip(_("Blocks not yet received"));


	rx_progress = new Fl_BlockMap(100, y+=26, W-108, 20, _("Blocks"));
	rx_progress->box(FL_DOWN_BOX);

	txt_rx_output = new FTextView(8, y+=32, W-16, 80, "Data");
	txt_rx_output->box(FL_DOWN_BOX);
	txt_rx_output->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);
	txt_rx_output->tooltip(_("Ascii Text\nData type message"));

	connect_tab->resizable(txt_rx_output);
	connect_tab->end();

	// Configuration Tab
	y = Y + 26;
	Fl_Group *config_tab = new Fl_Group(X+2, y, W-2*(X+2), H-y-2, _("Configure"));

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


	config_tab->end();
	
	// Configuration Tab End
	
	tabs->add(connect_tab);
	tabs->add(config_tab);
	
	tabs->end();
	w->end();
	

	return w;
}

