// ----------------------------------------------------------------------------
// fm_control.h
//
// Copyright (C) 2015  Robert Stiles, KK5VD
//
// This file is a part of Frame Messenger.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------

#ifndef __fm_control_h
#define __fm_control_h

#include <string>

#define FM_MIN_WINDOW_WIDTH  640
#define FM_MIN_WINDOW_HEIGHT 480

#define FM_LABEL_FONT_SIZE   13
#define FM_LABLE_DND         " DnD"

#define SET_NCS_LOGIN _("NCS Check In")
#define SET_LOGIN _("RX Check In")
#define SET_RX _("RX")
#define SET_TX _("TX")

extern bool fm_thread_running;
extern bool fm_data_available;
extern bool	fm_window_open;

extern void open_csv_log_configure(void);
extern void open_disp_log_configure(void);

extern void open_frame_messgeger(void);
extern void close_frame_messgeger(void);
extern void open_calltable(void);

extern void fm_transmit(void);
extern void fm_putchar(int ch);
extern int  fm_getchar(void);
extern void fm_putstring(std::string &string);
//extern void fm_tx_button(bool flag);

extern void fm_insert_tx_data();
extern void fm_save_rx_data();
extern void set_fm_window_defaults(void);
extern void save_fm_window_defaults(void);
extern void fm_dnd_file(void);

extern void paste_last_tx(void);
extern void move_to_callsign(std::string _call);

extern void fm_copy_selected_tx(void);
extern void fm_delete_selected_tx(void);
extern void fm_copy_all_tx(void);
extern void fm_paste_to_tx(void);
extern void fm_clear_tx(void);
extern void fm_copy_selected_rx(void);
extern void fm_copy_all_rx(void);
extern void fm_clear_rx(void);
extern void start_fm_thread(void);
extern void process_input_buffer(std::string _data);
extern void clear_callsign_pick_list(void);

extern void fm_append_tx(std::string data);
extern void fm_append_tx(char *data);

extern void clear_tx_buffer(void);
extern void cancel_tx(void);

extern void fm_log_short_list_to_tx(void);
extern void fm_log_long_list_to_tx(void);
extern void set_marker(std::string _trx, std::string _call, std::string _last);
extern std::string last_call_sign_heard(void);

#endif /* defined(__fm_control_h) */
