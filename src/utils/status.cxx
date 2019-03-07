// =====================================================================
//
// status.cxx
//
// Author(s):
//	Dave Freese, W1HKJ Copyright (C) 2010
//  Robert Stiles, KK5VD Copyright (C) 2013
//
// This file is part of FLAMP.
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
#include "flamp.h"
#include "flamp_dialog.h"
#include "file_io.h"

status progStatus = {
	50,				// int mainX;
	50,				// int mainY;

	"",				// my_call
	"",				// my_info
	"127.0.0.1",	// fldigi socket address
	"7322",			// fldigi socket port
	"127.0.0.1",	// fldigi xmlrpc socket address
	"7362",			// fldigi xmlrpc socket port

	// User Assigned addr/ports not saved.
	"",				// User assigned fldigi socket address
	"",				// User assigned fldigi socket port
	"",				// User assigned fldigi xmlrpc socket address
	"",				// User assigned fldigi xmlrpc socket port

	true,			// use_compression
	BASE256,		// encoder
	"base256",      // encoder string name
	1,				// selected_mode
	64,				// blocksize
	1,				// repeatNN
	1,				// repeat_header

	false,			// bool sync_mode_flamp_fldigi;
	false,			// bool sync_mode_fldigi_flamp;
	false,			// bool fldigi_xmt_mode_change;

	1,				// int repeat_every;
	false,			// bool repeat_at_times;
	"",				// string repeat_times;
	false,			// bool repeat_at_times;

	false, 			// bool use_txrx_interval;
	3.0, 			// int  tx_interval_minutes;
	10, 			// int  rx_interval_seconds;

	false, 			// bool use_header_modem;
	1,				// int  header_selected_mode;
	false,          // bool disable_header_modem_on_block_fills;

	0,				// int  use_tx_on_report;

	false,			// bool clear_tosend_on_tx_blocks;

	false,          // bool enable_delete_warning;

	false,          // bool enable_tx_unproto
	false,          // bool enable_unproto_markers

	false,          // bool queue_fills_only

	false,          // bool auto_load_queue

	false,          // bool load_from_tx_folder

	"",				// string auto_load_queue_path

	false,          // bool hamcast_mode_cycle

	false,          // bool hamcast_mode_enable_1
	1,              // int hamcast_mode_selection_1

	false,          // bool hamcast_mode_enable_2
	1,              // int hamcast_mode_selection_2

	false,          // bool hamcast_mode_enable_3
	1,              // int hamcast_mode_selection_3

	false,          // bool hamcast_mode_enable_4
	1,              // int hamcast_mode_selection_4

	true,          // bool auto_rx_save
	true,          // bool auto_rx_save_local_time
};

extern std::string selected_encoder_string;

/** ********************************************************
 *
 ***********************************************************/
void status::saveLastState()
{
	Fl_Preferences FLAMPpref(flampHomeDir.c_str(), "w1hkj.com",  PACKAGE_NAME);

	int mX = main_window->x();
	int mY = main_window->y();
	if (mX >= 0 && mX >= 0) {
		mainX = mX;
		mainY = mY;
	}

	FLAMPpref.set("version", PACKAGE_VERSION);
	FLAMPpref.set("mainx", mX);
	FLAMPpref.set("mainy", mY);

	my_call = txt_tx_mycall->value();
	my_info = txt_tx_myinfo->value();

	FLAMPpref.set("mycall", my_call.c_str());
	FLAMPpref.set("myinfo", my_info.c_str());
	FLAMPpref.set("socket_address", socket_addr.c_str());
	FLAMPpref.set("socket_port", socket_port.c_str());
	FLAMPpref.set("xmlrpc_address", xmlrpc_addr.c_str());
	FLAMPpref.set("xmlrpc_port", xmlrpc_port.c_str());
	FLAMPpref.set("blocksize", blocksize);
	FLAMPpref.set("repeatNN", repeatNN);
	FLAMPpref.set("repeat_header", repeat_header);
	FLAMPpref.set("selected_mode", selected_mode);
	FLAMPpref.set("compression", use_compression);
	FLAMPpref.set("encoder", encoder);
	FLAMPpref.set("encoder_string", encoder_string.c_str());
	FLAMPpref.set("sync_mode_flamp_fldigi", sync_mode_flamp_fldigi);
	FLAMPpref.set("sync_mode_fldigi_flamp", sync_mode_fldigi_flamp);
	FLAMPpref.set("fldigi_xmt_mode_change", fldigi_xmt_mode_change);

	FLAMPpref.set("repeat_every", repeat_every);
	FLAMPpref.set("repeat_at_times", repeat_at_times);
	FLAMPpref.set("repeat_times", repeat_times.c_str());

	FLAMPpref.set("repeat_forever", repeat_forever);

	FLAMPpref.set("use_repeater_interval", use_txrx_interval);
	FLAMPpref.set("repeater_tx_minutes", tx_interval_minutes);
	FLAMPpref.set("repeater_rx_seconds", rx_interval_seconds);

	FLAMPpref.set("disable_header_modem_on_block_fills", disable_header_modem_on_block_fills);
	FLAMPpref.set("use_header_modem", use_header_modem);
	FLAMPpref.set("header_selected_mode", header_selected_mode);

	FLAMPpref.set("use_tx_on_report", use_tx_on_report);

	FLAMPpref.set("clear_tosend_on_tx_blocks", clear_tosend_on_tx_blocks);

	FLAMPpref.set("enable_delete_warning", enable_delete_warning);

	FLAMPpref.set("enable_tx_unproto", enable_tx_unproto);
	FLAMPpref.set("enable_unproto_markers", enable_unproto_markers);

	FLAMPpref.set("queue_fills_only", queue_fills_only);

	FLAMPpref.set("auto_load_queue", auto_load_queue);
	FLAMPpref.set("load_from_tx_folder", load_from_tx_folder);

	FLAMPpref.set("auto_load_queue_path", auto_load_queue_path.c_str());

	FLAMPpref.set("hamcast_mode_cycle", hamcast_mode_cycle);

	FLAMPpref.set("hamcast_mode_enable_1", hamcast_mode_enable_1);
	FLAMPpref.set("hamcast_mode_selection_1", hamcast_mode_selection_1);

	FLAMPpref.set("hamcast_mode_enable_2", hamcast_mode_enable_2);
	FLAMPpref.set("hamcast_mode_selection_2", hamcast_mode_selection_2);

	FLAMPpref.set("hamcast_mode_enable_3", hamcast_mode_enable_3);
	FLAMPpref.set("hamcast_mode_selection_3", hamcast_mode_selection_3);

	FLAMPpref.set("hamcast_mode_enable_4", hamcast_mode_enable_4);
	FLAMPpref.set("hamcast_mode_selection_4", hamcast_mode_selection_4);

	FLAMPpref.set("auto_rx_save", auto_rx_save);
	FLAMPpref.set("auto_rx_save_local_time", auto_rx_save_local_time);

}

/** ********************************************************
 *
 ***********************************************************/
void status::loadLastState()
{
	Fl_Preferences FLAMPpref(flampHomeDir.c_str(), "w1hkj.com", PACKAGE_NAME);

	if (FLAMPpref.entryExists("version")) {
		char *defbuffer;

		FLAMPpref.get("mainx", mainX, mainX);
		FLAMPpref.get("mainy", mainY, mainY);

		FLAMPpref.get("mycall", defbuffer, "");
		my_call = defbuffer; free(defbuffer);

		FLAMPpref.get("myinfo", defbuffer, "");
		my_info = defbuffer; free(defbuffer);

		FLAMPpref.get("socket_address", defbuffer, socket_addr.c_str());
		socket_addr = defbuffer; free(defbuffer);
		FLAMPpref.get("socket_port", defbuffer, socket_port.c_str());
		socket_port = defbuffer; free(defbuffer);

		FLAMPpref.get("xmlrpc_address", defbuffer, xmlrpc_addr.c_str());
		xmlrpc_addr = defbuffer; free(defbuffer);
		FLAMPpref.get("xmlrpc_port", defbuffer, xmlrpc_port.c_str());
		xmlrpc_port = defbuffer; free(defbuffer);

		FLAMPpref.get("blocksize", blocksize, blocksize);
		FLAMPpref.get("repeatNN", repeatNN, repeatNN);
		FLAMPpref.get("repeat_header", repeat_header, repeat_header);

		FLAMPpref.get("selected_mode", selected_mode, selected_mode);

		int i = 0;
		FLAMPpref.get("compression", i, use_compression);
		use_compression = i;

		FLAMPpref.get("sync_mode_flamp_fldigi", i, sync_mode_flamp_fldigi);
		sync_mode_flamp_fldigi = i;

		FLAMPpref.get("sync_mode_fldigi_flamp", i, sync_mode_fldigi_flamp);
		sync_mode_fldigi_flamp = i;


		FLAMPpref.get("fldigi_xmt_mode_change", i, fldigi_xmt_mode_change);
		fldigi_xmt_mode_change = i;

		FLAMPpref.get("encoder", encoder, encoder);
		FLAMPpref.get("encoder_string", defbuffer, encoder_string.c_str());
		encoder_string = defbuffer; free(defbuffer);

		FLAMPpref.get("repeat_every", repeat_every, repeat_every);
		FLAMPpref.get("repeat_at_times", i, repeat_at_times);
		repeat_at_times = i;

		FLAMPpref.get("repeat_times", defbuffer, repeat_times.c_str());
		repeat_times = defbuffer; free(defbuffer);

		FLAMPpref.get("repeat_forever", i, repeat_forever);
		repeat_forever = i;

		FLAMPpref.get("use_repeater_interval", i, use_txrx_interval);
		use_txrx_interval = (bool) i;

		FLAMPpref.get("repeater_tx_minutes", tx_interval_minutes, tx_interval_minutes);

		FLAMPpref.get("repeater_rx_seconds", i, rx_interval_seconds);
		rx_interval_seconds = i;

		FLAMPpref.get("disable_header_modem_on_block_fills", i, disable_header_modem_on_block_fills);
		disable_header_modem_on_block_fills = (bool) i;

		FLAMPpref.get("use_header_modem", i, use_header_modem);
		use_header_modem = (bool) i;

		FLAMPpref.get("header_selected_mode", i, header_selected_mode);
		header_selected_mode = i;

		FLAMPpref.get("use_tx_on_report", i, use_tx_on_report);
		use_tx_on_report = i;

		FLAMPpref.get("clear_tosend_on_tx_blocks", i, clear_tosend_on_tx_blocks);
		clear_tosend_on_tx_blocks = (bool) i;

		FLAMPpref.get("enable_delete_warning", i, enable_delete_warning);
		enable_delete_warning = (bool) i;

		FLAMPpref.get("enable_tx_unproto", i, enable_tx_unproto);
		enable_tx_unproto = (bool) i;

		FLAMPpref.get("enable_unproto_markers", i, enable_unproto_markers);
		enable_unproto_markers = (bool) i;

		FLAMPpref.get("queue_fills_only", i, queue_fills_only);
		queue_fills_only = (bool) i;

		FLAMPpref.get("auto_load_queue", i, auto_load_queue);
		auto_load_queue = (bool) i;

		FLAMPpref.get("load_from_tx_folder", i, load_from_tx_folder);
		load_from_tx_folder = (bool) i;

		FLAMPpref.get("auto_load_queue_path", defbuffer, "");
		auto_load_queue_path.assign(defbuffer);
		free(defbuffer);

		FLAMPpref.get("hamcast_mode_cycle", i, hamcast_mode_cycle);
		hamcast_mode_cycle = (bool) i;

		FLAMPpref.get("hamcast_mode_enable_1", i, hamcast_mode_enable_1);
		hamcast_mode_enable_1 = (bool) i;

		FLAMPpref.get("hamcast_mode_selection_1", i, hamcast_mode_selection_1);
		hamcast_mode_selection_1 = i;

		FLAMPpref.get("hamcast_mode_enable_2", i, hamcast_mode_enable_2);
		hamcast_mode_enable_2 = (bool) i;

		FLAMPpref.get("hamcast_mode_selection_2", i, hamcast_mode_selection_2);
		hamcast_mode_selection_2 = i;

		FLAMPpref.get("hamcast_mode_enable_3", i, hamcast_mode_enable_3);
		hamcast_mode_enable_3 = (bool) i;

		FLAMPpref.get("hamcast_mode_selection_3", i, hamcast_mode_selection_3);
		hamcast_mode_selection_3 = i;

		FLAMPpref.get("hamcast_mode_enable_4", i, hamcast_mode_enable_4);
		hamcast_mode_enable_4 = (bool) i;

		FLAMPpref.get("hamcast_mode_selection_4", i, hamcast_mode_selection_4);
		hamcast_mode_selection_4 = i;

		FLAMPpref.get("auto_rx_save", i, auto_rx_save);
		auto_rx_save = (bool) i;

		FLAMPpref.get("auto_rx_save_local_time", i, auto_rx_save_local_time);
		auto_rx_save_local_time = (bool) i;

		if(auto_load_queue_path.size() < 1)
			auto_load_queue = false;

		if(enable_tx_unproto) {
			use_header_modem = false;
		}
	}
}
