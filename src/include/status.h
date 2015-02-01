// =====================================================================
//
// status.h
//
// Author(s):
// 	Dave Freese, W1HKJ Copyright (C) 2010
//	Robert Stiles, KK5VD Copyright (C) 2013
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

#ifndef _status_H
#define _status_H

#include <string>

using namespace std;

struct status {
	int		mainX;
	int		mainY;
	std::string my_call;
	std::string my_info;

	std::string socket_addr;
	std::string socket_port;
	std::string xmlrpc_addr;
	std::string xmlrpc_port;

	std::string user_socket_addr;
	std::string user_socket_port;
	std::string user_xmlrpc_addr;
	std::string user_xmlrpc_port;

	bool use_compression;
	int  encoder;
	std::string encoder_string;

	int  selected_mode;

	int blocksize;
	int repeatNN;
	int repeat_header;

	bool sync_mode_flamp_fldigi;
	bool sync_mode_fldigi_flamp;
	bool fldigi_xmt_mode_change;

	int repeat_every;
	bool repeat_at_times;
	string repeat_times;
	bool repeat_forever;

	bool use_txrx_interval;
	int  tx_interval_minutes;
	int  rx_interval_seconds;

	bool use_header_modem;
	int  header_selected_mode;
	bool disable_header_modem_on_block_fills;

	int  use_tx_on_report;

	bool clear_tosend_on_tx_blocks;

	bool enable_delete_warning;

	bool enable_tx_unproto;
	bool enable_unproto_markers;

	bool queue_fills_only;
	
	bool auto_load_queue;
	bool load_from_tx_folder;
	string auto_load_queue_path;

	bool hamcast_mode_cycle;

	bool hamcast_mode_enable_1;
	int  hamcast_mode_selection_1;

	bool hamcast_mode_enable_2;
	int  hamcast_mode_selection_2;

	bool hamcast_mode_enable_3;
	int  hamcast_mode_selection_3;

	bool hamcast_mode_enable_4;
	int  hamcast_mode_selection_4;

	void saveLastState();
	void loadLastState();
};

extern status progStatus;

#endif
