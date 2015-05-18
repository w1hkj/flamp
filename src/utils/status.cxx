// =====================================================================
//
// status.cxx
//
// Author(s):
//  Robert Stiles, KK5VD Copyright (C) 2015
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
// =====================================================================


#include <iostream>
#include <fstream>
#include <string>

#include <FL/Fl.H>
#include <FL/Fl_Preferences.H>

#include "status.h"
#include "config.h"
#include "fm_msngr.h"
#include "fm_msngr_src/fm_control.h"
#include "fm_msngr_src/fm_dialog.h"
#include "fm_msngr_src/fm_control_tts.h"
#include "fm_msngr_src/call_table.h"

status progStatus = {

	50,                 // int fm_window_x;
	50,                 // int fm_window_y;
	640,                // int fm_window_w;
	480,                // int fm_window_h;

	false,              // bool fm_enable_flnet;
	true,               // bool fm_enable_empty_frames;

	255,                // short int  XmtColor_red;
	0,                  // short int  XmtColor_green;
	0,                  // short int  XmtColor_blue;

	255,                // short int  RxColor_red;
	240,                // short int  RxColor_green;
	177,                // short int  RxColor_blue;

	0,                  // short int  RxFontcolor_red;
	0,                  // short int  RxFontcolor_green;
	0,                  // short int  RxFontcolor_blue;
	13,                 // int RxFontsize;
	0,                  // int RxFontnbr;
	"",                 // std::string rx_font_name;

	189,                // short int  TxColor_red;
	230,                // short int  TxColor_green;
	255,                // short int  TxColor_blue;

	0,                  // short int  TxFontcolor_red;
	0,                  // short int  TxFontcolor_green;
	0,                  // short int  TxFontcolor_blue;
	13,                 // int TxFontsize;
	0,                  // int TxFontnbr;
	"",                 // std::string tx_font_name;

	"127.0.0.1",	    // fldigi socket address
	"7322",			    // fldigi socket port
	"127.0.0.1",	    // fldigi xmlrpc socket address
	"7362",			    // fldigi xmlrpc socket port

	"",                 // fldigi socket address command line parameter
	"",                 // fldigi socket port    command line parameter
	"",                 // fldigi xmlrpc address command line parameter
	"",                 // fldigi xmlrpc port    command line parameter

	"96",               // std::string fm_frame_count

	"",	                // std::string fm_inp_callsign

	false,              // bool festival_enabled
	false,              // bool festival_voice_all_enabled

	"",	                // std::string festival_path
	"",                 // std::string input_festival_cl
	"",	                // std::string festival_path_opts

	"",	                // std::string audio_player_path
	"",	                // std::string audio_player_path_opts

	false,              // bool display_time_utc_local;
	true,               // bool display_time;
	false,              // bool display_date;

	true,               // bool display_call_use;
	true,               // bool display_name_use;
	true,               // bool display_qth_use;
	true,               // bool display_state_use;
	true,               // bool display_provence_use;
	true,               // bool display_locator_use;
	true,               // bool display_check_in_time_use;
	true,               // bool display_check_out_time_use;
	true,               // bool display_check_user1_use;
	true,               // bool display_check_user2_use;

	1,                  // int display_call_pos;
	2,                  // int display_name_pos;
	3,                  // int display_qth_pos;
	4,                  // int display_state_pos;
	5,                  // int display_provence_pos;
	6,                  // int display_locator_pos;
	7,                  // int display_check_in_time_pos;
	8,                  // int display_check_out_time_pos;
	9,                  // int display_check_user1_pos;
	10,                 // int display_check_user2_pos;

	CALL_TAG,           // std::string display_call_tag;
	NAME_TAG,           // std::string display_name_tag;
	QTH_TAG,            // std::string display_qth_tag;
	STATE_TAG,          // std::string display_state_tag;
	PROV_TAG,           // std::string display_provence_tag;
	LOC_TAG,            // std::string display_locator_tag;
	CHECK_IN_TAG,       // std::string display_check_in_time_tag;
	CHECK_OUT_TAG,      // std::string display_check_out_time_tag;
	USER1_TAG,          // std::string display_check_user1_tag;
	USER2_TAG,          // std::string display_check_user2_tag;

	false,              // bool enable_display_list_names;
	true,               // bool enable_display_list_header;
	"Logged Operators", // std::string display_list_header;

	false,              // bool log_time_utc_local;

	true,               // bool log_call_use;
	true,               // bool log_name_use;
	true,               // bool log_qth_use;
	true,               // bool log_state_use;
	true,               // bool log_provence_use;
	true,               // bool log_locator_use;
	true,               // bool log_check_in_time_use;
	true,               // bool log_check_out_time_use;
	true,               // bool log_check_user1_use;
	true,               // bool log_check_user2_use;

	1,                  // int log_call_pos;
	2,                  // int log_name_pos;
	3,                  // int log_qth_pos;
	4,                  // int log_state_pos;
	5,                  // int log_provence_pos;
	6,                  // int log_locator_pos;
	7,                  // int log_check_in_time_pos;
	8,                  // int log_check_out_time_pos;
	9,                  // int log_check_user1_pos;
	10,                 // int log_check_user2_pos;

	CALL_TAG,           // std::string log_call_tag;
	NAME_TAG,           // std::string log_name_tag;
	QTH_TAG,            // std::string log_qth_tag;
	STATE_TAG,          // std::string log_state_tag;
	PROV_TAG,           // std::string log_provence_tag;
	LOC_TAG,            // std::string log_locator_tag;
	CHECK_IN_TAG,       // std::string log_check_in_time_tag;
	CHECK_OUT_TAG,      // std::string log_check_out_time_tag;
	USER1_TAG,          // std::string log_check_user1_tag;
	USER2_TAG,          // std::string log_check_user2_tag;

	",",                // std::string log_csv_delimitor;

	false,              // bool ncs_link_to_fldigi;
	false,              // bool ncs_link_to_flnet;
	false,              // bool ncs_print_labels;
	false,              // bool ncs_call_print;
	false,              // bool ncs_name_print;
	false,              // bool ncs_qth_print;
	false,              // bool ncs_state_print;
	false,              // bool ncs_traffic_print;
	false,              // bool ncs_user1_print;
	false,              // bool ncs_user2_print;
	false,              // bool ncs_send_to_print;

	"",                 // std::string ncs_load_stms_file_path;

	"",                 // std::string user1_preset_file;
	"",                 // std::string user2_preset_file;
	 0,                 // int ncs_window_pos_x;
	 0                  // int ncs_window_pos_y;


};

extern std::string selected_encoder_string;

/** ********************************************************
 *
 ***********************************************************/
void status::saveLastState()
{
	Fl_Preferences spref(HomeDir.c_str(), "w1hkj.com",  PACKAGE_NAME);

	spref.set("version", PACKAGE_VERSION);

	spref.set("fm_window_x", fm_window_x);
	spref.set("fm_window_y", fm_window_y);
	spref.set("fm_window_w", fm_window_w);
	spref.set("fm_window_h", fm_window_h);

	spref.set("fm_enable_flnet",        fm_enable_flnet);
	spref.set("fm_utc_timestamp",       fm_utc_timestamp);

	spref.set("XmtColor_red",   XmtColor_red);
	spref.set("XmtColor_green", XmtColor_green);
	spref.set("XmtColor_blue",  XmtColor_blue);

	spref.set("RxColor_red",   RxColor_red);
	spref.set("RxColor_green", RxColor_green);
	spref.set("RxColor_blue",  RxColor_blue);

	spref.set("RxFontcolor_red",   RxFontcolor_red);
	spref.set("RxFontcolor_green", RxFontcolor_green);
	spref.set("RxFontcolor_blue",  RxFontcolor_blue);
	spref.set("RxFontsize",        RxFontsize);
	spref.set("RxFontnbr",         RxFontnbr);
	spref.set("rx_font_name",      rx_font_name.c_str());

	spref.set("TxColor_red",   TxColor_red);
	spref.set("TxColor_green", TxColor_green);
	spref.set("TxColor_blue",  TxColor_blue);

	spref.set("TxFontcolor_red",   TxFontcolor_red);
	spref.set("TxFontcolor_green", TxFontcolor_green);
	spref.set("TxFontcolor_blue",  TxFontcolor_blue);
	spref.set("TxFontsize",        TxFontsize);
	spref.set("TxFontnbr",         TxFontnbr);
	spref.set("tx_font_name",      tx_font_name.c_str());

	spref.set("socket_address", socket_addr.c_str());
	spref.set("socket_port",    socket_port.c_str());
	spref.set("xmlrpc_address", xmlrpc_addr.c_str());
	spref.set("xmlrpc_port",    xmlrpc_port.c_str());

	spref.set("fm_frame_count", fm_frame_count.c_str());

	spref.set("fm_inp_callsign", fm_inp_callsign.c_str());

	spref.set("festival_enabled",           festival_enabled);
	spref.set("festival_voice_all_enabled", festival_voice_all_enabled);
	spref.set("festival_path",              festival_path.c_str());
	spref.set("input_festival_cl",     	    input_festival_cl.c_str());
	spref.set("festival_path_opts",         festival_path_opts.c_str());

	spref.set("audio_player_path",      audio_player_path.c_str());
	spref.set("audio_player_path_opts", audio_player_path_opts.c_str());

	spref.set("display_time_utc_local", display_time_utc_local);
	spref.set("display_time", display_time);
	spref.set("display_date", display_date);

	spref.set("display_call_use", display_call_use);
	spref.set("display_name_use", display_name_use);
	spref.set("display_qth_use", display_qth_use);
	spref.set("display_state_use", display_state_use);
	spref.set("display_provence_use", display_provence_use);
	spref.set("display_locator_use", display_locator_use);
	spref.set("display_check_in_time_use", display_check_in_time_use);
	spref.set("display_check_out_time_use", display_check_out_time_use);
	spref.set("display_user1_use", display_user1_use);
	spref.set("display_user2_use", display_user2_use);

	spref.set("display_call_pos", display_call_pos);
	spref.set("display_name_pos", display_name_pos);
	spref.set("display_qth_pos", display_qth_pos);
	spref.set("display_state_pos", display_state_pos);
	spref.set("display_provence_pos", display_provence_pos);
	spref.set("display_locator_pos", display_locator_pos);
	spref.set("display_check_in_time_pos", display_check_in_time_pos);
	spref.set("display_check_out_time_pos", display_check_out_time_pos);
	spref.set("display_user1_pos", display_user1_pos);
	spref.set("display_user2_pos", display_user2_pos);

	spref.set("display_call_tag", display_call_tag.c_str());
	spref.set("display_name_tag", display_name_tag.c_str());
	spref.set("display_qth_tag", display_qth_tag.c_str());
	spref.set("display_state_tag", display_state_tag.c_str());
	spref.set("display_provence_tag", display_provence_tag.c_str());
	spref.set("display_locator_tag", display_locator_tag.c_str());
	spref.set("display_check_in_time_tag", display_check_in_time_tag.c_str());
	spref.set("display_check_out_time_tag", display_check_out_time_tag.c_str());
	spref.set("display_user1_tag", display_user1_tag.c_str());
	spref.set("display_user2_tag", display_user2_tag.c_str());

	spref.set("enable_display_list_names", enable_display_list_names);
	spref.set("enable_display_list_header", enable_display_list_header);
	spref.set("display_list_header", display_list_header.c_str());

	spref.set("log_time_utc_local", log_time_utc_local);

	spref.set("log_call_use", log_call_use);
	spref.set("log_name_use", log_name_use);
	spref.set("log_qth_use", log_qth_use);
	spref.set("log_state_use", log_state_use);
	spref.set("log_provence_use", log_provence_use);
	spref.set("log_locator_use", log_locator_use);
	spref.set("log_check_in_time_use", log_check_in_time_use);
	spref.set("log_check_out_time_use", log_check_out_time_use);
	spref.set("log_user1_use", log_user1_use);
	spref.set("log_user2_use", log_user2_use);

	spref.set("log_call_pos", log_call_pos);
	spref.set("log_name_pos", log_name_pos);
	spref.set("log_qth_pos", log_qth_pos);
	spref.set("log_state_pos", log_state_pos);
	spref.set("log_provence_pos", log_provence_pos);
	spref.set("log_locator_pos", log_locator_pos);
	spref.set("log_check_in_time_pos", log_check_in_time_pos);
	spref.set("log_check_out_time_pos", log_check_out_time_pos);
	spref.set("log_user1_pos", log_user1_pos);
	spref.set("log_user2_pos", log_user2_pos);

	spref.set("log_call_tag", log_call_tag.c_str());
	spref.set("log_name_tag", log_name_tag.c_str());
	spref.set("log_qth_tag", log_qth_tag.c_str());
	spref.set("log_state_tag", log_state_tag.c_str());
	spref.set("log_provence_tag", log_provence_tag.c_str());
	spref.set("log_locator_tag", log_locator_tag.c_str());
	spref.set("log_check_in_time_tag",  log_check_in_time_tag.c_str());
	spref.set("log_check_out_time_tag", log_check_out_time_tag.c_str());
	spref.set("log_user1_tag",  log_user1_tag.c_str());
	spref.set("log_user2_tag", log_user2_tag.c_str());

	spref.set("log_csv_delimitor", log_csv_delimitor.c_str());

	spref.set("ncs_link_to_fldigi", ncs_link_to_fldigi);
	spref.set("ncs_link_to_flnet", ncs_link_to_flnet);
	spref.set("ncs_print_labels", ncs_print_labels);
	spref.set("ncs_call_print", ncs_call_print);
	spref.set("ncs_name_print", ncs_name_print);
	spref.set("ncs_qth_print", ncs_qth_print);
	spref.set("ncs_state_print", ncs_state_print);
	spref.set("ncs_traffic_print", ncs_traffic_print);
	spref.set("ncs_user1_print", ncs_user1_print);
	spref.set("ncs_user2_print", ncs_user2_print);
	spref.set("ncs_send_to_print", ncs_send_to_print);

	spref.set("ncs_load_stms_file_path", ncs_load_stms_file_path.c_str());

	spref.set("user1_preset_file", user1_preset_file.c_str());
	spref.set("user2_preset_file", user2_preset_file.c_str());

	spref.set("ncs_window_pos_x", ncs_window_pos_x);
	spref.set("ncs_window_pos_y", ncs_window_pos_y);

}

/** ********************************************************
 *
 ***********************************************************/
void status::loadLastState()
{
	Fl_Preferences spref(HomeDir.c_str(), "w1hkj.com", PACKAGE_NAME);
	int i = 0;

	if (spref.entryExists("version")) {
		char *defbuffer;

		spref.get("fm_enable_flnet", i, fm_enable_flnet);
		fm_enable_flnet = (bool) i;

		spref.get("fm_utc_timestamp", i, fm_utc_timestamp);
		fm_utc_timestamp = (bool) i;

		spref.get("fm_window_x", i, fm_window_x);
		fm_window_x = i;

		spref.get("fm_window_y", i, fm_window_y);
		fm_window_y = i;

		spref.get("fm_window_w", i, fm_window_w);
		fm_window_w = i;

		spref.get("fm_window_h", i, fm_window_h);
		fm_window_h = i;

		spref.get("XmtColor_red",      i,   XmtColor_red);
		XmtColor_red = (short int)     i;

		spref.get("XmtColor_green",    i,  XmtColor_green);
		XmtColor_green = (short int)   i;

		spref.get("XmtColor_blue",     i,   XmtColor_blue);
		XmtColor_blue = (short int)    i;

		spref.get("RxColor_red",       i, RxColor_red);
		RxColor_red = (short int)      i;

		spref.get("RxColor_green",     i, RxColor_green);
		RxColor_green = (short int)    i;

		spref.get("RxColor_blue",      i, RxColor_blue);
		RxColor_blue = (short int)     i;

		spref.get("RxFontcolor_red",    i,  RxFontcolor_red);
		RxFontcolor_red = (int)         i;

		spref.get("RxFontcolor_green",  i, RxFontcolor_green);
		RxFontcolor_green = (short int) i;

		spref.get("RxFontcolor_blue",   i, RxFontcolor_blue);
		RxFontcolor_blue = (short int)  i;

		spref.get("RxFontsize",         i, RxFontsize);
		RxFontsize = (int)              i;

		spref.get("RxFontnbr",          i, RxFontnbr);
		RxFontnbr = (int)               i;

		spref.get("rx_font_name", defbuffer, rx_font_name.c_str());
		rx_font_name = defbuffer; free(defbuffer);

		spref.get("TxColor_red",        i, TxColor_red);
		TxColor_red = (short int)       i;

		spref.get("TxColor_green",      i, TxColor_green);
		TxColor_green = (short int)     i;

		spref.get("TxColor_blue",       i, TxColor_blue);
		TxColor_blue = (short int)      i;

		spref.get("TxFontcolor_red",    i, TxFontcolor_red);
		TxFontcolor_red = (short int)   i;

		spref.get("TxFontcolor_green",  i, TxFontcolor_green);
		TxFontcolor_green = (short int) i;

		spref.get("TxFontcolor_blue",   i, TxFontcolor_blue);
		TxFontcolor_blue = (short int)  i;

		spref.get("TxFontsize",         i, TxFontsize);
		TxFontsize = (int)              i;

		spref.get("TxFontnbr",          i, TxFontnbr);
		TxFontnbr = (int)               i;

		spref.get("tx_font_name", defbuffer, tx_font_name.c_str());
		tx_font_name = defbuffer; free(defbuffer);

		spref.get("socket_address", defbuffer, socket_addr.c_str());
		socket_addr = defbuffer; free(defbuffer);

		spref.get("socket_port", defbuffer, socket_port.c_str());
		socket_port = defbuffer; free(defbuffer);

		spref.get("xmlrpc_address", defbuffer, xmlrpc_addr.c_str());
		xmlrpc_addr = defbuffer; free(defbuffer);

		spref.get("xmlrpc_port", defbuffer, xmlrpc_port.c_str());
		xmlrpc_port = defbuffer; free(defbuffer);

		spref.get("fm_frame_count", defbuffer, fm_frame_count.c_str());
		fm_frame_count = defbuffer; free(defbuffer);

		spref.get("fm_inp_callsign", defbuffer, fm_inp_callsign.c_str());
		fm_inp_callsign = defbuffer; free(defbuffer);


		spref.get("festival_enabled", i, festival_enabled);
		festival_enabled = (bool) i;

		spref.get("festival_voice_all_enabled", i, festival_voice_all_enabled);
		festival_voice_all_enabled = (bool) i;

		spref.get("festival_path", defbuffer, festival_path.c_str());
		festival_path = defbuffer; free(defbuffer);

		spref.get("input_festival_cl", defbuffer, input_festival_cl.c_str());
		input_festival_cl = defbuffer; free(defbuffer);

		spref.get("festival_path_opts", defbuffer, festival_path_opts.c_str());
		festival_path_opts = defbuffer; free(defbuffer);


		spref.get("audio_player_path", defbuffer, audio_player_path.c_str());
		audio_player_path = defbuffer; free(defbuffer);

		spref.get("audio_player_path_opts", defbuffer, audio_player_path_opts.c_str());
		audio_player_path_opts = defbuffer; free(defbuffer);


		spref.get("display_time_utc_local", i, display_time_utc_local);
		display_time_utc_local = (bool) i;

		spref.get("display_time", i, display_time);
		display_time = (bool) i;

		spref.get("display_date", i, display_date);
		display_date = (bool) i;


		spref.get("display_call_use", i, display_call_use);
		display_call_use = (bool) i;

		spref.get("display_name_use", i, display_name_use);
		display_name_use = (bool) i;

		spref.get("display_qth_use", i, display_qth_use);
		display_qth_use = (bool) i;

		spref.get("display_state_use", i, display_state_use);
		display_state_use = (bool) i;

		spref.get("display_provence_use", i, display_provence_use);
		display_provence_use = (bool) i;

		spref.get("display_locator_use", i, display_locator_use);
		display_locator_use = (bool) i;

		spref.get("display_check_in_time_use", i, display_check_in_time_use);
		display_check_in_time_use = (bool) i;

		spref.get("display_check_out_time_use", i, display_check_out_time_use);
		display_check_out_time_use = (bool) i;

		spref.get("display_user1_use", i, display_user1_use);
		display_user1_use = (bool) i;

		spref.get("display_user2_use", i, display_user2_use);
		display_user2_use = (bool) i;


		spref.get("display_call_pos", i, display_call_pos);
		display_call_pos = (int) i;

		spref.get("display_name_pos", i, display_name_pos);
		display_name_pos = (int) i;

		spref.get("display_qth_pos", i, display_qth_pos);
		display_qth_pos = (int) i;

		spref.get("display_state_pos", i, display_state_pos);
		display_state_pos = (int) i;

		spref.get("display_provence_pos", i, display_provence_pos);
		display_provence_pos = (int) i;

		spref.get("display_locator_pos", i, display_locator_pos);
		display_locator_pos = (int) i;

		spref.get("display_check_in_time_pos", i, display_check_in_time_pos);
		display_check_in_time_pos = (int) i;

		spref.get("display_check_out_time_pos", i, display_check_out_time_pos);
		display_check_out_time_pos = (int) i;

		spref.get("display_user1_pos", i, display_user1_pos);
		display_user1_pos = (int) i;

		spref.get("display_user2_pos", i, display_user2_pos);
		display_user2_pos = (int) i;



		spref.get("display_call_tag", defbuffer, display_call_tag.c_str());
		display_call_tag = defbuffer; free(defbuffer);
		
		spref.get("display_name_tag", defbuffer, display_name_tag.c_str());
		display_name_tag = defbuffer; free(defbuffer);

		spref.get("display_qth_tag", defbuffer, display_qth_tag.c_str());
		display_qth_tag = defbuffer; free(defbuffer);

		spref.get("display_state_tag", defbuffer, display_state_tag.c_str());
		display_state_tag = defbuffer; free(defbuffer);

		spref.get("display_provence_tag", defbuffer, display_provence_tag.c_str());
		display_provence_tag = defbuffer; free(defbuffer);

		spref.get("display_locator_tag", defbuffer, display_locator_tag.c_str());
		display_locator_tag = defbuffer; free(defbuffer);

		spref.get("display_check_in_time_tag", defbuffer, display_check_in_time_tag.c_str());
		display_check_in_time_tag = defbuffer; free(defbuffer);

		spref.get("display_check_out_time_tag", defbuffer, display_check_out_time_tag.c_str());
		display_check_out_time_tag = defbuffer; free(defbuffer);

		spref.get("display_user1_tag", defbuffer, display_user1_tag.c_str());
		display_user1_tag = defbuffer; free(defbuffer);

		spref.get("display_user2_tag", defbuffer, display_user2_tag.c_str());
		display_user2_tag = defbuffer; free(defbuffer);


		spref.get("enable_display_list_header", i, enable_display_list_header);
		enable_display_list_header = (bool) i;

		spref.get("enable_display_list_names", i, enable_display_list_names);
		enable_display_list_names = (bool) i;

		spref.get("display_list_header", defbuffer, display_list_header.c_str());
		display_list_header = defbuffer; free(defbuffer);


		spref.get("log_time_utc_local", i, log_time_utc_local);
		log_time_utc_local = (bool) i;

		spref.get("log_call_use", i, log_call_use);
		log_call_use = (bool) i;

		spref.get("log_name_use", i, log_name_use);
		log_name_use = (bool) i;

		spref.get("log_qth_use", i, log_qth_use);
		log_qth_use = (bool) i;

		spref.get("log_state_use", i, log_state_use);
		log_state_use = (bool) i;

		spref.get("log_provence_use", i, log_provence_use);
		log_provence_use = (bool) i;

		spref.get("log_locator_use", i, log_locator_use);
		log_locator_use = (bool) i;

		spref.get("log_check_in_time_use", i, log_check_in_time_use);
		log_check_in_time_use = (bool) i;

		spref.get("log_check_out_time_use", i, log_check_out_time_use);
		log_check_out_time_use = (bool) i;

		spref.get("log_user1_use", i, log_user1_use);
		log_user1_use = (bool) i;

		spref.get("log_user2_use", i, log_user2_use);
		log_user2_use = (bool) i;


		spref.get("log_call_pos", i, log_call_pos);
		log_call_pos = (int) i;

		spref.get("log_name_pos", i, log_name_pos);
		log_name_pos = (int) i;

		spref.get("log_qth_pos", i, log_qth_pos);
		log_qth_pos = (int) i;

		spref.get("log_state_pos", i, log_state_pos);
		log_state_pos = (int) i;

		spref.get("log_provence_pos", i, log_provence_pos);
		log_provence_pos = (int) i;

		spref.get("log_locator_pos", i, log_locator_pos);
		log_locator_pos = (int) i;

		spref.get("log_check_in_time_pos", i, log_check_in_time_pos);
		log_check_in_time_pos = (int) i;

		spref.get("log_check_out_time_pos", i, log_check_out_time_pos);
		log_check_out_time_pos = (int) i;

		spref.get("log_user1_pos", i, log_user1_pos);
		log_user1_pos = (int) i;

		spref.get("log_user2_pos", i, log_user2_pos);
		log_user2_pos = (int) i;


		spref.get("log_call_tag", defbuffer, log_call_tag.c_str());
		log_call_tag = defbuffer; free(defbuffer);

		spref.get("log_name_tag", defbuffer, log_name_tag.c_str());
		log_name_tag = defbuffer; free(defbuffer);

		spref.get("log_qth_tag", defbuffer, log_qth_tag.c_str());
		log_qth_tag = defbuffer; free(defbuffer);

		spref.get("log_state_tag", defbuffer, log_state_tag.c_str());
		log_state_tag = defbuffer; free(defbuffer);
		
		spref.get("log_provence_tag", defbuffer, log_provence_tag.c_str());
		log_provence_tag = defbuffer; free(defbuffer);
		
		spref.get("log_locator_tag", defbuffer, log_locator_tag.c_str());
		log_locator_tag = defbuffer; free(defbuffer);
		
		spref.get("log_check_in_time_tag", defbuffer,  log_check_in_time_tag.c_str());
		log_check_in_time_tag = defbuffer; free(defbuffer);
		
		spref.get("log_check_out_time_tag", defbuffer, log_check_out_time_tag.c_str());
		log_check_out_time_tag = defbuffer; free(defbuffer);
		
		spref.get("log_user1_tag", defbuffer, log_user1_tag.c_str());
		log_user1_tag = defbuffer; free(defbuffer);

		spref.get("log_user2_tag", defbuffer, log_user2_tag.c_str());
		log_user2_tag = defbuffer; free(defbuffer);


		spref.get("log_csv_delimitor", defbuffer, log_csv_delimitor.c_str());
		log_csv_delimitor = defbuffer; free(defbuffer);


		spref.get("ncs_link_to_fldigi", i, ncs_link_to_fldigi);
		ncs_link_to_fldigi = (bool) i;

		spref.get("ncs_link_to_flnet", i, ncs_link_to_flnet);
		ncs_link_to_flnet = (bool) i;

		spref.get("ncs_print_labels", i, ncs_print_labels);
		ncs_print_labels = (bool) i;

		spref.get("ncs_call_print", i, ncs_call_print);
		ncs_call_print = (bool) i;

		spref.get("ncs_name_print", i, ncs_name_print);
		ncs_name_print = (bool) i;

		spref.get("ncs_qth_print", i, ncs_qth_print);
		ncs_qth_print = (bool) i;

		spref.get("ncs_state_print", i, ncs_state_print);
		ncs_state_print = (bool) i;

		spref.get("ncs_traffic_print", i, ncs_traffic_print);
		ncs_traffic_print = (bool) i;

		spref.get("ncs_user1_print", i, ncs_user1_print);
		ncs_user1_print = (bool) i;

		spref.get("ncs_user2_print", i, ncs_user2_print);
		ncs_user2_print = (bool) i;

		spref.get("ncs_send_to_print", i, ncs_send_to_print);
		ncs_send_to_print = (bool) i;

		spref.get("ncs_load_stms_file_path", defbuffer, ncs_load_stms_file_path.c_str());
		ncs_load_stms_file_path = defbuffer; free(defbuffer);

		spref.get("user1_preset_file", defbuffer, user1_preset_file.c_str());
		user1_preset_file = defbuffer; free(defbuffer);

		spref.get("user2_preset_file", defbuffer, user2_preset_file.c_str());
		user2_preset_file = defbuffer; free(defbuffer);

		spref.get("ncs_window_pos_x", i, ncs_window_pos_x);
		ncs_window_pos_x = (int)      i;

		spref.get("ncs_window_pos_y", i, ncs_window_pos_y);
		ncs_window_pos_y = (int)      i;

		if(festival_enabled) {
			open_festival();
		}
	}
}
