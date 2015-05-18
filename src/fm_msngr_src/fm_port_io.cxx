// ----------------------------------------------------------------------------
//  fm_port_io.cxx
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>

#include "fm_msngr.h"
#include "fm_msngr_src/fm_dialog.h"
#include "fm_msngr_src/fm_control.h"
#include "fm_msngr_src/fm_port_io.h"
#include "fm_msngr_src/call_table.h"

#include "util.h"

#include "debug.h"
#include "util.h"
#include "gettext.h"

#include "icons.h"
#include "fileselect.h"

#include "status.h"
#include "pixmaps.h"
#include "threads.h"
#include "xml_io.h"
#include "file_io.h"

pthread_t arqio_thread_id;
pthread_mutex_t arqio_mutex_loop = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  arqio_cond  = PTHREAD_COND_INITIALIZER;

bool testing = true;
bool transmitting = false;
bool transmit_stop = false;

std::string tmp_buffer;

int xmlrpc_errno = 0;
bool arqio_thread_running = false;
bool arqio_exit_flag = true;
bool rx_complete = false;

void process_data_stream(void);
int process_que(void *ptr);
int receive_data_stream(void *ptr);
int parse_que(void *ptr);
int que_match(void *ptr);
void *parse_cque(void *ptr);

/** ********************************************************
 *
 ***********************************************************/
void deactivate_button(void *ptr)
{
	if (!ptr) return;
	int *value = (int *) ptr;

	//	if( *value == TX_BUTTON )
	//		btn_send_file->deactivate();
}

/** ********************************************************
 *
 ***********************************************************/
void activate_button(void *ptr)
{
	if (!ptr) return;
	int * value = (int *) ptr;

	//	if( *value == TX_BUTTON )
	//		btn_send_file->activate();
}

/** ********************************************************
 *
 ***********************************************************/
void abort_tx(void)
{
	send_abort();
	send_abort();

	for(int i = 0; i < 40; i++) { // 4 seconds
		MilliSleep(100);
		if(get_trx_state() == "RX") break;
	}
}

/** ********************************************************
 *
 ***********************************************************/
void abort_tx_from_main(void * ptr)
{
	abort_tx();

	std::string idMessage;
	// A number of non printable characters are required to overcome long interleave modems.
	idMessage.assign("\n\n\n\n\n\n\n\n\n\n\nFILE TRANSFER ABORTED\n\nDE ").append(progStatus.fm_inp_callsign.c_str()).append(" BK\n\n");
	send_via_fldigi(idMessage);
}

/** ********************************************************
 *
 ***********************************************************/
void abort_request(void)
{
	int response = fl_choice("Terminate Current Transmission?", "No", "Yes", NULL);
	if (response == 1) {
		static int value = TX_BUTTON;
		deactivate_button((void *) &value);
		transmit_stop = true;
	}
}

/** ********************************************************
 *
 ***********************************************************/
void get_trx_state_in_main_thread(void *ptr)
{
	if (!ptr) return;

	std::string *str = (std::string *)ptr;

	*str = get_trx_state();
}


// utility functions


/** ********************************************************
 *
 ***********************************************************/
bool numbers_and_dots_only(char *str, int expected_argc)
{
	int f = 0, s = 0, t = 0, fo = 0;
	int argc = 0;

	if(str == (char *)0) return false;
	if(*str == (char) 0) return false;

	argc = sscanf((const char *)str, "%d.%d.%d.%d", &f, &s, &t, &fo);

	if(argc == expected_argc) return true;

	return false;
}

/** ********************************************************
 *
 ***********************************************************/
void arqio_loop_shutdown(void)
{
	arqio_exit_flag = true;
	pthread_cond_signal(&arqio_cond);
	pthread_join(arqio_thread_id, 0);
	arqio_thread_running = false;
}

/** ********************************************************
 *
 ***********************************************************/
void *arqio_loop(void *ptr)
{
	arqio_thread_running = true;
	std::string temp = "";
	arqio_exit_flag = false;

	while(!arqio_exit_flag) {
		MilliSleep(100);
		rx_fldigi(temp);
		if(!temp.empty())
			process_input_buffer(temp);
		temp.clear();
	}

	arqio_thread_running = false;

	pthread_cancel(pthread_self());
	return (void *)0;
}

/** ********************************************************
 *
 ***********************************************************/
void start_io(void)
{
	string addr;
	string port;

	if(progStatus.user_socket_addr.size())
		addr.assign(progStatus.user_socket_addr);
	else
		addr.assign(progStatus.socket_addr);

	if(progStatus.user_socket_port.size())
		port.assign(progStatus.user_socket_port);
	else
		port.assign(progStatus.socket_port);

	localaddr = new Address(addr.c_str(), port.c_str());
	if (!localaddr) exit(EXIT_FAILURE);
	tcpip = new Socket (*localaddr);
	tcpip->set_timeout(0.01);
	connect_to_fldigi(0);

	if (pthread_create(&arqio_thread_id, NULL, arqio_loop, NULL)) {
		perror("pthread_create: arq read loop");
		exit(EXIT_FAILURE);
	}

	open_xmlrpc();
	xmlrpc_thread = new pthread_t;
	if (pthread_create(xmlrpc_thread, NULL, xmlrpc_loop, NULL)) {
		perror("pthread_create: xmlrpc");
		exit(EXIT_FAILURE);
	}

	if(window_frame->shown()) {
		start_fm_thread();
		fm_window_open = true;
	}
}

/** ********************************************************
 *
 ***********************************************************/
int que_match(void *ptr)
{
	return 0;
}

/** ********************************************************
 *
 ***********************************************************/
void *parse_cque(void *ptr)
{
	return (void *)0;
}

/** ********************************************************
 *
 ***********************************************************/
int parse_que(void *ptr)
{
	return 0;
}

/** ********************************************************
 *
 ***********************************************************/
void process_data_stream(void)
{

}

/** ********************************************************
 *
 ***********************************************************/
int receive_data_stream(void *ptr)
{
	return 0;
}

