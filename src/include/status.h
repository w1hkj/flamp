// =====================================================================
//
// status.h
//
// Author(s):
// 	Dave Freese, W1HKJ Copyright (C) 2010
//	Robert Stiles, KK5VD Copyright (C) 2015
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

#ifndef _status_H
#define _status_H

#include <string>

using namespace std;

struct status {
	int fm_window_x;
	int fm_window_y;
	int fm_window_w;
	int fm_window_h;

	bool fm_enable_flnet;
	bool fm_utc_timestamp;

	short int  XmtColor_red;
	short int  XmtColor_green;
	short int  XmtColor_blue;

	short int  RxColor_red;
	short int  RxColor_green;
	short int  RxColor_blue;

	short int  RxFontcolor_red;
	short int  RxFontcolor_green;
	short int  RxFontcolor_blue;

	int RxFontsize;
	int RxFontnbr;
	std::string rx_font_name;

	short int  TxColor_red;
	short int  TxColor_green;
	short int  TxColor_blue;

	short int  TxFontcolor_red;
	short int  TxFontcolor_green;
	short int  TxFontcolor_blue;

	int TxFontsize;
	int TxFontnbr;
	std::string tx_font_name;

	std::string socket_addr;
	std::string socket_port;
	std::string xmlrpc_addr;
	std::string xmlrpc_port;

	std::string user_socket_addr;
	std::string user_socket_port;
	std::string user_xmlrpc_addr;
	std::string user_xmlrpc_port;

	std::string fm_frame_count;

	std::string fm_inp_callsign;

	bool festival_enabled;
	bool festival_voice_all_enabled;

	std::string festival_path;
	std::string input_festival_cl;
	std::string festival_path_opts;

	std::string audio_player_path;
	std::string audio_player_path_opts;

	bool display_time_utc_local;
	bool display_time;
	bool display_date;

	bool display_call_use;
	bool display_name_use;
	bool display_qth_use;
	bool display_state_use;
	bool display_provence_use;
	bool display_locator_use;
	bool display_check_in_time_use;
	bool display_check_out_time_use;
	bool display_user1_use;
	bool display_user2_use;

	int display_call_pos;
	int display_name_pos;
	int display_qth_pos;
	int display_state_pos;
	int display_provence_pos;
	int display_locator_pos;
	int display_check_in_time_pos;
	int display_check_out_time_pos;
	int display_user1_pos;
	int display_user2_pos;

	std::string display_call_tag;
	std::string display_name_tag;
	std::string display_qth_tag;
	std::string display_state_tag;
	std::string display_provence_tag;
	std::string display_locator_tag;
	std::string display_check_in_time_tag;
	std::string display_check_out_time_tag;
	std::string display_user1_tag;
	std::string display_user2_tag;

	bool enable_display_list_names;
	bool enable_display_list_header;
	std::string display_list_header;

	bool log_time_utc_local;

	bool log_call_use;
	bool log_name_use;
	bool log_qth_use;
	bool log_state_use;
	bool log_provence_use;
	bool log_locator_use;
	bool log_check_in_time_use;
	bool log_check_out_time_use;
	bool log_user1_use;
	bool log_user2_use;

	int log_call_pos;
	int log_name_pos;
	int log_qth_pos;
	int log_state_pos;
	int log_provence_pos;
	int log_locator_pos;
	int log_check_in_time_pos;
	int log_check_out_time_pos;
	int log_user1_pos;
	int log_user2_pos;

	std::string log_call_tag;
	std::string log_name_tag;
	std::string log_qth_tag;
	std::string log_state_tag;
	std::string log_provence_tag;
	std::string log_locator_tag;
	std::string log_check_in_time_tag;
	std::string log_check_out_time_tag;
	std::string log_user1_tag;
	std::string log_user2_tag;

	std::string log_csv_delimitor;

	bool ncs_link_to_fldigi;
	bool ncs_link_to_flnet;
	bool ncs_print_labels;
	bool ncs_call_print;
	bool ncs_name_print;
	bool ncs_qth_print;
	bool ncs_state_print;
	bool ncs_traffic_print;
	bool ncs_user1_print;
	bool ncs_user2_print;
	bool ncs_send_to_print;

	std::string ncs_load_stms_file_path;

	std::string user1_preset_file;
	std::string user2_preset_file;

	int ncs_window_pos_x;
    int ncs_window_pos_y;

	void saveLastState();
	void loadLastState();
};

extern status progStatus;

#endif
