// =====================================================================
//
// fm_msngr.h
//
//  Author(s):
//    Robert Stiles, KK5VD, Copyright (C) 2015
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

#ifndef fm_msngr_H
#define fm_msngr_H

#include <string>
#include <vector>
#include <FL/Fl.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Double_Window.H>

#include "threads.h"


//#define DEBUG 1
#undef DEBUG

#define TX_STATE 1
#define RX_STATE 0

#define TX_BUTTON       2     //!< @brief Flag indicating transmit a single file.
#define CNT_BLOCK_SIZE_STEP_RATE	16
#define CNT_BLOCK_SIZE_MINIMUM		16
#define CNT_BLOCK_SIZE_MAXIMUM		2048
using namespace std;


typedef struct {
	void *widget;
	string strValue;
} STRING_VALUE;

typedef struct {
	void *widget;
	int intValue;
} INT_VALUE;


extern bool transmit_stop;
extern bool transmitting;

extern const char *options[];

extern int file_io_errno;
extern int xmlrpc_errno;

extern pthread_mutex_t mutex_file_io;
extern pthread_mutex_t mutex_xmlrpc;
extern pthread_t *xmlrpc_thread;

extern string buffer;
extern string HomeDir;
extern string title;

extern bool do_events_flag;
extern bool tx_ztimer_flag;
extern void abort_and_id(void);
extern void abort_request(void);
extern void activate_button(void *ptr);
extern void get_c_string_value(void *ptr);
extern void get_int_value(void *ptr);
extern void get_string_value(void *ptr);
extern void send_fldigi_modem(void *ptr);
extern void send_missing_report(void);
extern void send_relay_data(void);
extern void send_via_fldigi_in_main_thread(void *ptr);
extern void set_button_to_cancel(void *);
extern void set_button_to_xmit(void *);
extern void set_relay_button_label(void *);
extern void set_xmit_label(void *data);
extern void show_help(void);
extern void thread_error_msg(void *data);
extern void turn_rsid_off(void);
extern void turn_rsid_on(void);
extern void tx_removefile(bool flag);
#endif
