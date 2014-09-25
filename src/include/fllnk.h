// =====================================================================
//
// fllnk.h
//
//  Author(s):
//    Robert Stiles, KK5VD, Copyright (C) 2013
//    Dave Freese, W1HKJ, Copyright (C) 2013
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

#ifndef fllnk_H
#define fllnk_H

#include <string>
#include <vector>
#include <FL/Fl.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>

#include "crc16.h"
#include "threads.h"
#include "timeops.h"

//#define DEBUG 1
#undef DEBUG

using namespace std;

#define THREAD_ERR_MSG_SIZE 256

//! @struct _tx_fldigi_thread
//! Structure information used to transmit cAmp data (threaded) and the creation of a character time table.

//! @typedef TX_FLDIGI_THREAD
//! @see _tx_fldigi_thread

typedef struct _tx_fldigi_thread {
	pthread_t thread;           //!< Thread
	pthread_mutex_t mutex;      //!< Mutex for transit thread.
	pthread_cond_t  condition;  //!< Condition used to signal exit when process is asleep.
	pthread_attr_t  attr;       //!< Flag for indicating thread is to be detached. pthread_execute()
	void *data;                 //!< For future use
	char err_msg[THREAD_ERR_MSG_SIZE]; //!< @brief Error message storage.
	bool err_flag;              //!< @brief Indicating an error occured
    bool thread_running;

	_tx_fldigi_thread() {       //!< @brief Clear struct _tx_fldigi_thread memory on allocation.
		memset(&thread,    0, sizeof(thread));
		memset(&mutex,     0, sizeof(mutex));
		memset(&condition, 0, sizeof(condition));
		memset(&attr,      0, sizeof(attr));
		memset(&err_msg,   0, sizeof(err_msg));
		data = (void *)0;
		err_flag = false;
		thread_running = false;
	}

} TX_FLDIGI_THREAD;

typedef struct {
	void *widget;
	string strValue;
} STRING_VALUE;

typedef struct {
	void *widget;
	int intValue;
} INT_VALUE;

typedef struct _button_label {
	Fl_Button *button;
	std::string label;
} BUTTON_LABEL;

extern Fl_Double_Window *mainwindow;
extern Fl_Double_Window *optionswindow;
extern Fl_Double_Window *config_files_window;
extern Fl_Double_Window *socket_window;

extern const char *options[];
extern string title;
extern string fllnkHomeDir;
extern string fllnk_dir;
extern string buffer;

extern bool transmitting;
extern bool transmit_stop;

extern void cb_exit(void);
extern void cb_folders(void);
extern void show_help(void);

extern bool wait_for_rx(int max_wait_seconds);
extern void wait_seconds(int seconds);

extern TX_FLDIGI_THREAD * run_in_thread(void *(*func)(void *), int mode);
extern void * run_in_thread_destroy(TX_FLDIGI_THREAD *tx_thread, int level);
extern void abort_request(void);

extern void set_button_label(void *);
extern void get_int_value(void *ptr);
extern void get_string_value(void *ptr);
extern void get_c_string_value(void *ptr);


extern void send_via_fldigi_in_main_thread(void *ptr);
extern void get_trx_state_in_main_thread(void *ptr);
extern void abort_and_id(void);
extern void thread_error_msg(void *data);

extern pthread_t *xmlrpc_thread;
extern pthread_mutex_t mutex_xmlrpc;
extern pthread_mutex_t mutex_file_io;
extern int xmlrpc_errno;
extern int file_io_errno;

#endif
