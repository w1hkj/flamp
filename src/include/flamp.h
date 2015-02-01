// =====================================================================
//
// flamp.h
//
//  Author(s):
//    Robert Stiles, KK5VD, Copyright (C) 2013
//    Dave Freese, W1HKJ, Copyright (C) 2013
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

#ifndef flamp_H
#define flamp_H

#include <string>
#include <vector>
#include <FL/Fl.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Double_Window.H>

#include "util.h"
#include "nls.h"
#include "gettext.h"
#include "crc16.h"
#include "threads.h"
#include "timeops.h"
#include "amp.h"

//#define DEBUG 1
#undef DEBUG

using namespace std;


typedef struct {
	void *widget;
	string strValue;
} STRING_VALUE;

typedef struct {
	void *widget;
	int intValue;
} INT_VALUE;


#define ID_TIME_MINUTES (8)     //!< @brief Maximum time (in minutes) for IDing.
#define ID_TIME_SECONDS (ID_TIME_MINUTES * 60) //!< @brief Maximum time (in seconds) for IDing.
#define MIN_INTERAL_TIME 1

void alt_receive_data_stream(void);

extern const char *flamp_beg;
extern const char *flamp_end;

extern Fl_Double_Window *mainwindow;
extern Fl_Double_Window *optionswindow;
extern Fl_Double_Window *config_files_window;
extern Fl_Double_Window *socket_window;

extern bool generate_time_table;
extern bool transmit_stop;
extern bool transmitting;

extern const char *options[];

extern int file_io_errno;
extern int xmlrpc_errno;

extern pthread_mutex_t mutex_file_io;
extern pthread_mutex_t mutex_xmlrpc;
extern pthread_t *xmlrpc_thread;

extern string buffer;
extern string flamp_dir;
extern string flampHomeDir;
extern string title;

extern bool do_events_flag;
extern bool tx_ztimer_flag;

extern int  process_que(void *que);
extern int  receive_data_stream(void *);
extern int  valid_block_size(int value);
extern void	update_rx_missing_blocks(void);
extern void abort_and_id(void);
extern void abort_request(void);
extern void activate_button(void *ptr);
extern void addfile(std::string, void *, bool, char *, char *);
extern void amp_mark_all_for_update(void);
extern void amp_update_all(void);
extern void amp_update(cAmp *amp);
extern void auto_load_tx_queue(void);
extern void cb_exit(void);
extern void cb_folders(void);
extern void cb_load_tx_queue(void);
extern void cb_scripts_default_location(void);
extern void cb_scripts(bool flag);
extern void clear_tx_panel(void);
extern void deactivate_button(void *ptr);
extern void drop_file_changed(void);
extern void estimate(cAmp *amp, bool visable);
extern void get_c_string_value(void *ptr);
extern void get_int_value(void *ptr);
extern void get_string_value(void *ptr);
extern void get_trx_state_in_main_thread(void *ptr);
extern void get_trx_state_in_main_thread(void *ptr);
extern void preamble_detected(void);
extern void process_data_stream(void);
extern void process_missing_stream(void);
extern void readfile(void);
extern void receive_remove_from_queue(bool flag);
extern void recv_missing_report(void);
extern void relay_missing_report(void);
extern void send_fldigi_modem(void *ptr);
extern void send_missing_report(void);
extern void send_relay_data(void);
extern void send_via_fldigi_in_main_thread(void *ptr);
extern void set_button_to_cancel(void *);
extern void set_button_to_xmit(void *);
extern void set_relay_button_label(void *);
extern void set_xmit_label(void *data);
extern void show_current_selected_file(void *);
extern void show_help(void);
extern void show_selected_rcv(int);
extern void show_selected_xmt(cAmp *amp);
extern void show_selected_xmt(int);
extern void thread_error_msg(void *data);
extern void transfer_time(std::string modem_name, float &cps, int &transfer_size, std::string buffer);
extern void transmit_current(void);
extern void transmit_queue_main_thread(void *ptr);
extern void transmit_queued(bool event_driven, bool shift_key_pressed);
extern void transmit_relay(RELAY_DATA *rd);
extern void turn_rsid_off(void);
extern void turn_rsid_on(void);
extern void tx_removefile(bool flag);
extern void update_cAmp_changes(cAmp *amp);
extern void writefile(int);
#endif
