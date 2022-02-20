//======================================================================
//	transmit_camp.cxx
//
//  Author(s):
//	Dave Freese, W1HKJ, Copyright (C) 2010, 2011, 2012, 2013
//	Robert Stiles, KK5VD, Copyright (C) 2014
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <ctime>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>
#include <libgen.h>
#include <ctype.h>
#include <sys/time.h>
#include <math.h>

#include <FL/Fl.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/x.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_File_Icon.H>

#include "flamp.h"
#include "amp.h"
#include "flamp_dialog.h"

#include "debug.h"
#include "util.h"
#include "gettext.h"
#include "flinput2.h"
#include "date.h"
#include "calendar.h"
#include "icons.h"
#include "fileselect.h"
#include "file_io.h"
#include "status.h"
#include "pixmaps.h"
#include "threads.h"
#include "xml_io.h"
#include "tagSearch.h"
#include "time_table.h"
#include "ztimer.h"
#include "script_parsing.h"
#include "global_amp.h"
#include "transmit_camp.h"

pthread_mutex_t mutex_tx_data = PTHREAD_MUTEX_INITIALIZER;

bool active_data_io         = false;
bool transmit_queue         = false;
int last_selected_tx_file   = 0;
int tx_thread_running_count = 0;

static bool shift_key_flag = false;

static bool using_rsid = false;

static void wait_milliseconds(int milliseconds, int millisecond_increments);
static void wait_seconds(int seconds, int millisecond_increments);

/** ********************************************************
 *
 ***********************************************************/
void transfer_in_main_thread(void *ptr)
{
	if (!ptr) return;

	std::string *data = (std::string *)ptr;

	if (!bConnected) connect_to_fldigi(0);
	if (!bConnected) return;

	transfer(*data);
}

#if 0
/** ********************************************************
 *
 ***********************************************************/
std::string tx_string(cAmp *tx, std::string t_string)
{
	std::string auto_send;

	pthread_mutex_lock(&mutex_tx_data);

	active_data_io = true;

	std::string send_to = tx->callto();
	std::string temp;

	if (send_to.empty()) send_to.assign("QST");
	send_to.append(" DE ").append(progStatus.my_call);
	for (size_t n = 0; n < send_to.length(); n++)
		send_to[n] = toupper(send_to[n]);

	tx->my_call(progStatus.my_call);
	tx->my_info(progStatus.my_info);
	tx->xmt_blocksize(progStatus.blocksize);
	tx->repeat(progStatus.repeatNN);
	tx->header_repeat(progStatus.repeat_header);

	temp.clear();
	temp.assign(tx->xmt_buffer());

	if(tx->unproto() == false) {
		compress_maybe(temp, tx->tx_base_conv_index(), (tx->compress() | tx->forced_compress()));
		tx->xmt_data(temp);
		auto_send.append(tx->xmt_string(true));
	} else {
		tx->xmt_unproto(progStatus.enable_unproto_markers);
		auto_send.append(tx->xmt_unproto_string());
	}

	auto_send.append("\n").append(send_to).append(t_string);

	auto_send.append("\n\n\n");

	active_data_io = false;

	pthread_mutex_unlock(&mutex_tx_data);

	return auto_send;
}
#else
/** ********************************************************
 *
 ***********************************************************/
std::string tx_string(cAmp *tx, std::string t_string)
{
	std::string auto_send;

	pthread_mutex_lock(&mutex_tx_data);

	active_data_io = true;

	auto_send.assign(tx->tx_string(t_string));

	active_data_io = false;

	pthread_mutex_unlock(&mutex_tx_data);

	return auto_send;
}
#endif

// Create time table for the various modes
// Starts at current selected mode.
#define MIN_TX_TIME 10.0
double measure_tx_time(int character_to_send, double *over_head);

/** ********************************************************
 *
 ***********************************************************/
void * create_tx_table(void *ptr)
{
	TX_FLDIGI_THREAD *thread_ptr = (TX_FLDIGI_THREAD *)ptr;
	static bool in_use = false;

	if (!bConnected) connect_to_fldigi(0);
	if (!bConnected || in_use) {
		return run_in_thread_destroy(thread_ptr, 3, 0);
	}

	in_use = true;

	int end_mode   = 0;
	int index      = 0;
	int mode       = 0;
	int start_mode = 0;

	double overhead     = 0.0;
	double time_seconds = 0;

	char filename[256];

#define P_RELSOL "%3.6f"
#define P_RELSOL2 "%1.6f"

	std::string response;
	std::string txBuffer;
	std::string mode_name;

	FILE *fd = (FILE *)0;

	transmitting = true;

	{
		Fl::awake(set_button_to_cancel, (void *)0);
		static int value = TX_ALL_BUTTON;
		Fl::awake(deactivate_button, (void *) &value);
	}

	turn_rsid_off();

	pthread_mutex_lock(&thread_ptr->mutex);
	thread_ptr->thread_running = 1;
	thread_ptr->exit_thread = 0;
	pthread_mutex_unlock(&thread_ptr->mutex);

	memset(filename, 0, sizeof(filename));

	for(index = 0; index < 100; index++) {
		snprintf(filename, sizeof(filename)-1, "time_table-%02d.txt", index);
		fd = fopen(filename, "r");
		if(!fd) break;
		fclose(fd);
	}

	if(index > 99) {
		LOG_DEBUG("time_table-xx.txt files reached count limit, delete some.");
		in_use = 0;
		return run_in_thread_destroy(thread_ptr, 3, &in_use);
	}

	if(fd) fclose(fd);
	fd = fopen(filename, "w");

	if(fd) {
		LOG_DEBUG("Table Gen filename %s", filename);
		fprintf(fd, "typedef struct {\n"
				"\tchar *mode_name;\n"
				"\tfloat scale;\n"
				"\tfloat overhead;\n"
				"\tfloat table[256];\n"
				"} MODE_TIME_TABLE;\n\n");

		fprintf(fd, "%s", "static MODE_TIME_TABLE mode_time_table[] = {\n");
		fflush(fd);

		if(thread_ptr->que) {
			start_mode = 0;
		} else {
			start_mode = cbo_modes->index();
		}

		end_mode = cbo_modes->lsize();

		mode = start_mode;

		do {
			LOG_DEBUG("Processing Modem %s", cbo_modes->value());

			send_clear_tx();
			send_new_modem("CW");

			cbo_modes->index(mode);
			mode_name = cbo_modes->value();

			send_new_modem(mode_name.c_str());
			wait_seconds(1, 200);

			time_seconds = measure_tx_time((int) 'A', &overhead);

			LOG_DEBUG("Mode %s overhead=%f", mode_name.c_str(), overhead);

			fprintf(fd, "\t{\n\t\t(char *) \"%s\", 1.0, " P_RELSOL2 ", \n\t\t{\n\t\t\t", mode_name.c_str(), overhead);
			fflush(fd);

			if(mode_name.find("Olivia") != std::string::npos || mode_name.find("MT63") != std::string::npos) {
				for(index = 0; index < 128; index++) {
					if((index % 8) == 0 && index != 0) {
						fprintf(fd, "\n\t\t\t");
					}
					fprintf(fd, P_RELSOL", ", time_seconds);
				}
				fflush(fd);

				time_seconds = measure_tx_time((int) 130, &overhead);

				for(index = 128; index < 256; index++) {
					if((index % 8) == 0 && index != 0) {
						fprintf(fd, "\n\t\t\t");
					}

					fprintf(fd, P_RELSOL, time_seconds);

					if(index < 255)
						fprintf(fd, ", ");
				}
				fflush(fd);

			} else {
				for(index = 0; index < 256; index++) {
					if(transmit_stop || thread_ptr->exit_thread) {
						char msg[] = "\nNOTICE: User requested Termination\n";
						fprintf(fd, "%s", msg);
						LOG_DEBUG("%s", msg);
						fclose(fd);
						return run_in_thread_destroy(thread_ptr, 3, &in_use);
					}

					LOG_DEBUG("Proc Char = %d, %X : ", index, index);

					if((index % 8) == 0 && index != 0) {
						fprintf(fd, "\n\t\t\t");
					}

					time_seconds = measure_tx_time(index, 0);

					fprintf(fd, P_RELSOL, time_seconds);

					if(index < 255)
						fprintf(fd, ", ");

					if(index < ' ')
						LOG_DEBUG("Charater \'%d (%X)\' Transmit time in seconds=%f", (index & 0xff), (index & 0xff), time_seconds);
					else
						LOG_DEBUG("Charater \'%c\' Transmit time in seconds=%f", (index & 0xff), time_seconds);

					fflush(fd);

				}
			}

			fprintf(fd, "\n\t\t}\n\t}");

			mode++;

			if(mode < end_mode)
				fprintf(fd, ",");

			fprintf(fd, "\n");
			fflush(fd);

		} while(mode < end_mode);

		fprintf(fd, "};\n");
		fflush(fd);
		fclose(fd);

	} else {
		LOG_ERROR("%s = %d", "File open Error", errno);
	}

	in_use = 0;
	return run_in_thread_destroy(thread_ptr, 3, &in_use);
}

/** ********************************************************
 *
 ***********************************************************/
double measure_tx_time(int character_to_send, double *over_head)
{
	double duration  = 0;
	double retun_val = 0.0;
	unsigned int oh  = 0;
	unsigned int s   = 0;
	unsigned int sr  = 0;

	std::string tx_duration;

//	set_xmlrpc_timeout(8.0);
	tx_duration = get_char_timing(character_to_send);

//	set_xmlrpc_timeout_default();

	if(tx_duration.size() < 1) {
		LOG_ERROR("No Data returned.");
		transmit_stop = true;
		return retun_val;
	}

	int count = sscanf(tx_duration.c_str(), "%u:%u:%u", &s, &sr, &oh);

	if(count != 3) {
		LOG_ERROR("Returned parameter error. Count = %d should be 3", count);
		transmit_stop = true;
		return retun_val;
	}

	if(sr != 0) {
		duration = 1.0 * s / sr;

		if(over_head) {
			*over_head = 1.0 * oh / sr;
		}
	}
	else
		duration = 1.0;

	return duration;
}

/** ********************************************************
 *
 ***********************************************************/
void transmit_relay(RELAY_DATA *rd)
{
	if (!bConnected) connect_to_fldigi(0);
	if (!bConnected) return;

	if(transmitting) return;

	transmit_stop  = false;
	transmit_queue = false;

	if(progStatus.use_txrx_interval || progStatus.use_header_modem)
		run_in_thread(transmit_relay_interval, TX_SEGMENTED, false, false, rd);
	else
		run_in_thread(transmit_serial_relay, 0, false, false, rd);
}

/** ********************************************************
 *
 ***********************************************************/
void transmit_current()
{
	if (!bConnected) connect_to_fldigi(0);
	if (!bConnected) return;

	if(transmitting) return;

	amp_update_all();

	transmit_stop  = false;
	transmit_queue = false;

	last_selected_tx_file = tx_queue->value() - 1;

	if(generate_time_table) {
		run_in_thread(create_tx_table, 0, 0, 0, 0);
	} else {
		if(progStatus.use_txrx_interval) {
			run_in_thread(transmit_interval, TX_SEGMENTED, false, false, 0);
		} else if(progStatus.use_header_modem) {
			turn_rsid_on();
			run_in_thread(transmit_interval, TX_CONTINIOUS, false, false, 0);
		} else {
			run_in_thread(transmit_serial_current, 0, false, false, 0);
		}
	}
}

/** ********************************************************
 *
 ***********************************************************/
void transmit_queued(bool event_driven, bool shift_key_pressed)
{
	if (!bConnected) connect_to_fldigi(0);
	if (!bConnected) return;

	if(transmitting) return;

	transmit_stop  = false;
	transmit_queue = true;
	g_event_driven = event_driven;

	amp_update_all();

	if(event_driven)
		event_bail_flag = false;

	last_selected_tx_file = tx_queue->value() - 1;

	shift_key_flag = shift_key_pressed;

	if(generate_time_table) {
		run_in_thread(create_tx_table, 1, 1, event_driven, 0);
	} else {
		if(progStatus.use_txrx_interval) {
			run_in_thread(transmit_interval, TX_SEGMENTED, true, event_driven, 0);
		} else if(progStatus.use_header_modem) {
			turn_rsid_on();
			run_in_thread(transmit_interval, TX_CONTINIOUS, true, event_driven, 0);
		} else {
			run_in_thread(transmit_serial_queued, 0, true, event_driven, 0);
		}
	}
}

/** ********************************************************
 *
 ***********************************************************/
void * transmit_serial_current(void *ptr)
{
	void * ret = 0;
	transmitting = true;
	static bool in_use = false;

	if(in_use) {
		return run_in_thread_destroy((TX_FLDIGI_THREAD *) ptr, 3, 0);
	}

	in_use = true;

	{
		Fl::awake(set_button_to_cancel, (void *)0);
		static int value = TX_ALL_BUTTON;
		Fl::awake(deactivate_button, (void *) &value);
	}

	int n = tx_queue->value();

	if (!n) {
		return run_in_thread_destroy((TX_FLDIGI_THREAD *) ptr, 3, &in_use);
	}

	if (progStatus.fldigi_xmt_mode_change)
		send_new_modem(cbo_modes->value());

	float tx_time = 0;

	cAmp *tx = 0;
	std::string autosend = "";
	std::string send_to  = "";
	std::string temp     = "";
	std::string temp2    = "";

	autosend.clear();
	send_to.clear();
	temp.clear();

	active_data_io = true;

	tx = tx_amp.index2amp(n);

	if(!tx) {
		active_data_io = false;
		return run_in_thread_destroy((TX_FLDIGI_THREAD *) ptr, 3, &in_use);
	}

	tx = new cAmp(tx); // Operate on a copy of
	if(!tx) {
		active_data_io = false;
		return run_in_thread_destroy((TX_FLDIGI_THREAD *) ptr, 3, &in_use);
	}

	autosend = tx_string(tx, " K\n");

	printf("autosend %ld\n", (long) autosend.size());

	int value = tx->xmt_buffer().size();
	if (!value) {
		active_data_io = false;
		in_use = false;
		return run_in_thread_destroy((TX_FLDIGI_THREAD *) ptr, 3, &in_use);
	}

	float oh = 0;
	tx_time = seconds_from_string(cbo_modes->value(), autosend, &oh);
	tx_time += oh;
	tx_time_g = tx_time;
	tx_time *= 2.0;

	transfer(autosend);

	wait_for_rx((int) tx_time);

	delete tx;

	in_use = false;

	ret = run_in_thread_destroy((TX_FLDIGI_THREAD *) ptr, 3, &in_use);

	return ret;
}

/** ********************************************************
 *
 ***********************************************************/
void * transmit_serial_queued(void *ptr)
{
	transmitting = true;

	static bool in_use = false;
	TX_FLDIGI_THREAD *thread = (TX_FLDIGI_THREAD *) ptr;

	if(in_use) {
		return run_in_thread_destroy(thread, 3, &in_use);
	}

	in_use = true;

	Fl::awake(set_button_to_cancel, (void *)0);
	static int val = TX_ALL_BUTTON;
	Fl::awake(deactivate_button, (void *) &val);

	float tx_time = 0;

	if (tx_amp.size() == 0) {
		in_use = false;
		return run_in_thread_destroy(thread, 3, &in_use);
	}

	if (progStatus.fldigi_xmt_mode_change)
		send_new_modem(thread->modem.c_str());

	cAmp *tx;
	std::string autosend    = "";
	std::string null_string = "";
	std::string temp        = "";
	std::string terminator  = "";
	bool fills = false;

	autosend.clear();
	temp.clear();

	unsigned int count = (unsigned int) tx_amp.size();

	if(continuous_exception)
		terminator.assign(" BK\n");
	else
		terminator.assign(" K\n");

	for (size_t num = 0; num < count; num++) {

		tx = tx_amp.index2amp(num + 1);

		if(!tx) {
			in_use = false;
			return run_in_thread_destroy(thread, 3, &in_use);
		}

		tx = new cAmp(tx); // Operate on a copy of.

		if(!tx) {
			in_use = false;
			return run_in_thread_destroy(thread, 3, &in_use);
		}

		fills = false;

		if(shift_key_flag) {
			if(tx->xmt_tosend().size() && !tx->unproto())
				fills = true;
		}

		if((fills && shift_key_flag) || !shift_key_flag) {

			tx->xmt_modem(thread->modem.c_str());

			if (tx->xmt_buffer().empty())
				return run_in_thread_destroy(thread, 3, &in_use);

			if(num == (count - 1))
				temp = tx->tx_string(terminator);
			else
				temp = tx->tx_string(null_string);

			autosend.append(temp);
		}

		delete tx;
	}

	transfer(autosend);

	float oh = 0;
	tx_time = seconds_from_string(thread->modem.c_str(), autosend, &oh);
	tx_time += oh;
	tx_time_g = tx_time;
	tx_time *= 1.10;

	wait_for_rx((int) tx_time);

	in_use = false;

	return run_in_thread_destroy(thread, 3, &in_use);
}

/** ********************************************************
 *
 ***********************************************************/
void turn_rsid_on()
{
	// Turn on RX/TX RSIDs
	transfer("<cmd><txrsid>ON</txrsid></cmd><cmd><rsid>ON</rsid></cmd>");
	using_rsid = true;
	MilliSleep(1000);
}

/** ********************************************************
 *
 ***********************************************************/
void turn_rsid_off()
{
	// Turn on RX/TX RSIDs
	transfer("<cmd><txrsid>OFF</txrsid></cmd><cmd><rsid>OFF</rsid></cmd>");
	using_rsid = false;
	MilliSleep(1000);
}

/** ********************************************************
 *
 ***********************************************************/
void * transmit_serial_relay(void *ptr)
{
	float tx_time = 0;
	RELAY_DATA *relay_data = (RELAY_DATA *) 0;
	static bool in_use = false;
	TX_FLDIGI_THREAD *t_ptr = (TX_FLDIGI_THREAD *) ptr;
	void * ret = 0;

	cAmp *tx = 0;
	std::string autosend = "";
	std::string send_to  = "";
	std::string temp     = "";
	std::string temp2    = "";

	if(in_use) {
		return run_in_thread_destroy(t_ptr, 3, 0);
	}

	in_use = true;

	transmitting = true;

	{
		static char *msg = (char *) CANX_LABEL;
		Fl::awake(set_relay_button_label, (void *) msg);
	}

	if(!ptr) return ptr; // This should not happen.
	relay_data = (RELAY_DATA *) t_ptr->data;

	if(!relay_data)
		return run_in_thread_destroy(t_ptr, 3, &in_use);

	{
		static char *msg = (char *) CANX_LABEL;
		Fl::awake(set_relay_button_label, (void *) msg);
	}

	if (progStatus.fldigi_xmt_mode_change)
		send_new_modem(cbo_modes->value());

	temp.clear();
	autosend.clear();
	send_to.clear();

	active_data_io = true;

	tx = relay_data->amp;

	if(!tx) {
		active_data_io = false;
		return ptr;
	}

	autosend = relay_data->serial_data;

	int value = autosend.size();

	if (!value) {
		active_data_io = false;
		return run_in_thread_destroy(t_ptr, 3, &in_use);
	}

	float oh = 0;
	tx_time = seconds_from_string(cbo_modes->value(), autosend, &oh);
	tx_time += oh;
	tx_time_g = tx_time;
	tx_time *= 2.0;

	transfer(autosend);

	wait_for_rx((int) tx_time);

	ret = run_in_thread_destroy(t_ptr, 3, &in_use);

	return ret;
}

/** ********************************************************
 *
 ***********************************************************/
void * transmit_relay_interval(void * ptr)
{
	transmitting = true;
	static bool in_use = false;

	if(in_use) {
		return run_in_thread_destroy((TX_FLDIGI_THREAD *) ptr, 3, 0);
	}

	in_use = true;

	{
		static char *msg = (char *) CANX_LABEL;
		Fl::awake(set_relay_button_label, (void *) msg);
	}

	cAmp *tx = (cAmp *)0;
	int vector_data_count = 0;
	int vector_header_count = 0;
	RELAY_DATA *relay_data = (RELAY_DATA *) 0;
	std::string send_to = "";
	std::string tail;
	std::string temp;
	std::vector<std::string> vector_data;
	std::vector<std::string> vector_header_data;
	TX_FLDIGI_THREAD *thread_ptr = (TX_FLDIGI_THREAD *)ptr;

	pthread_mutex_lock(&thread_ptr->mutex);
	thread_ptr->thread_running = 1;
	thread_ptr->exit_thread = 0;
	pthread_mutex_unlock(&thread_ptr->mutex);

	if(progStatus.use_header_modem) {
		turn_rsid_on();
	}

	relay_data = (RELAY_DATA *) thread_ptr->data;

	if(!relay_data)
		return run_in_thread_destroy(thread_ptr, 3, &in_use);

	tx = relay_data->amp;
	if(!tx)
		return run_in_thread_destroy(thread_ptr, 3, &in_use);

	vector_header_data = relay_data->header;
	vector_header_count = vector_header_data.size();

	vector_data = relay_data->data;
	vector_data_count = vector_data.size();

	if((vector_header_count < 1) && (vector_data_count < 1)) {
		LOG_DEBUG("Empty xmt_vector_string");
		return run_in_thread_destroy(thread_ptr, 3, &in_use);
	}

	if(check_block_tx_time(vector_header_data, vector_data, thread_ptr) == false) {
		return run_in_thread_destroy(thread_ptr, 3, &in_use);
	}

	send_to.clear();
	send_to.append(" DE ").append(progStatus.my_call);
	for (size_t n = 0; n < send_to.length(); n++)
		send_to[n] = toupper(send_to[n]);

	tail.assign("\n").append(send_to).append(" K\n\n");

	thread_ptr->mode = TX_SEGMENTED;

	LOG_DEBUG("vector_header_count = %d", vector_header_count);

	if(progStatus.use_header_modem) {
		if(vector_header_count > 0) {
			LOG_DEBUG("Header Modem Sent to FLDGI. (%s)", g_header_modem.c_str());
			send_new_modem(thread_ptr->header_modem);

			if(!send_vector_to_fldigi(thread_ptr, thread_ptr->header_modem, tail, vector_header_data, thread_ptr->mode, tx)) {
				delete tx;
				return run_in_thread_destroy(thread_ptr, 3, &in_use);
			}
		}
	}

	wait_seconds(thread_ptr->rx_interval_time, 100);

	// Data section
	if(progStatus.use_header_modem || progStatus.fldigi_xmt_mode_change) {
		send_new_modem(thread_ptr->modem);
	}

	vector_data = relay_data->data;
	vector_data_count = vector_data.size();

	LOG_DEBUG("vector_data_count = %d", vector_data_count);

	if(vector_data_count)
		send_vector_to_fldigi(thread_ptr, thread_ptr->modem, tail, vector_data, thread_ptr->mode, tx);

	event_bail_flag = false;
	return run_in_thread_destroy(thread_ptr, 3, &in_use);
}

/** ********************************************************
 *
 ***********************************************************/
void * transmit_interval(void * ptr)
{
	static bool in_use = false;

	if(in_use) {
		return run_in_thread_destroy((TX_FLDIGI_THREAD *)ptr, 3, 0);
	}

	in_use = true;

	transmitting = true;

	Fl::awake(set_button_to_cancel, (void *)0);
	static int val = TX_ALL_BUTTON;
	Fl::awake(deactivate_button, (void *) &val);

	cAmp *tx = (cAmp *)0;
	TX_FLDIGI_THREAD *thread_ptr = (TX_FLDIGI_THREAD *)ptr;
	int count = 0;
	int vector_data_count = 0;
	int vector_header_count = 0;
	int limit = 0;
	int index = 0;
	bool fills = 0;

	std::vector<std::string> vector_data;
	std::vector<std::string> vector_header_data;
	std::string temp;
	std::string tail_k;
	std::string tail_bk;
	std::string tail;
	std::string send_to = "";

	int n = 0;
	int value = 0;

	vector_data.clear();
	vector_header_data.clear();
	temp.clear();
	tail.clear();
	send_to.clear();

	pthread_mutex_lock(&thread_ptr->mutex);
	thread_ptr->thread_running = 1;
	thread_ptr->exit_thread = 0;
	pthread_mutex_unlock(&thread_ptr->mutex);

	count = 0;

	if(thread_ptr->que) {
		index = 0;
		limit = tx_amp.size();
	} else {
		n = tx_queue->value();
		if (!n)	return run_in_thread_destroy(thread_ptr, 3, &in_use);
		index = n - 1;
		limit = 1;
	}

	if(progStatus.use_header_modem) {
		turn_rsid_on();
	}

	event_bail_flag = false;
	tx = 0;

	do {

		if(transmit_stop) {
			if(tx) delete tx;
			return run_in_thread_destroy(thread_ptr, 3, &in_use);
		}

		tx = tx_amp.index2amp(index + 1);
		if(tx)
			tx = new cAmp(tx); // Operate on a copy of

		if(!tx)
			return run_in_thread_destroy(thread_ptr, 3, &in_use);

		Fl::awake(show_current_selected_file, (void *)&index);

		send_to = tx->callto();
		if (send_to.empty()) send_to.assign("QST");
		send_to.append(" DE ").append(progStatus.my_call);
		for (size_t n = 0; n < send_to.length(); n++)
			send_to[n] = toupper(send_to[n]);

		tail_bk.clear();
		tail_bk.assign("\n").append(send_to).append(" BK\n\n");

		tail_k.clear();
		tail_k.assign("\n").append(send_to).append(" K\n\n");

		value = tx->xmt_buffer().size();
		if (!value) {
			LOG_DEBUG("Empty xmt_buffer");
			delete tx;
			return run_in_thread_destroy(thread_ptr, 3, &in_use);
		}

		temp.assign(tx->xmt_buffer());
		compress_maybe(temp, tx->tx_base_conv_index(), (tx->compress() | tx->forced_compress()));//true);

		bool data_repeat_inhibit = true;

		if(!tx->xmt_vector_string(progStatus.use_header_modem, progStatus.enable_unproto_markers, data_repeat_inhibit)) {
			LOG_DEBUG("Empty xmt_vector_string");
			delete tx;
			return run_in_thread_destroy(thread_ptr, 3, &in_use);
		}

		if(check_block_tx_time(tx->xmt_vector_header(), tx->xmt_vector_data(), thread_ptr) == false) {
			delete tx;
			return run_in_thread_destroy(thread_ptr, 3, &in_use);
		}


		fills = false;
		if((progStatus.disable_header_modem_on_block_fills && tx->preamble_detected() == false) || shift_key_flag) {
			if(thread_ptr->que) {
				if(tx->xmt_tosend().size() && !tx->unproto())
					fills = true;
			} else {
				char *cPtr = (char *) txt_tx_selected_blocks->value();
				if(cPtr) {
					int _limit = 0;
					while(*cPtr) {
						if(isdigit(*cPtr)) {
							fills = true;
							break;
						}
						cPtr++;
						if(_limit++ > 16) break;
					}
				}
			}
		}

		if((fills && shift_key_flag) || !shift_key_flag) {

			if(count > 0 && !transmit_stop) {
				wait_seconds(thread_ptr->rx_interval_time, 100);
			}

			if(transmit_stop) {
				delete tx;
				return run_in_thread_destroy(thread_ptr, 3, &in_use);
			}

			vector_data = tx->xmt_vector_data();
			vector_data_count = vector_data.size();

			// Header section
			vector_header_data = tx->xmt_vector_header();
			vector_header_count = vector_header_data.size();

			LOG_DEBUG("vector_header_count = %d", vector_header_count);

			int repeat_count = 0;
			int no_of_repeats = progStatus.repeatNN;

			for(repeat_count = 0; repeat_count < no_of_repeats; repeat_count++) {

				if(transmit_stop) {
					delete tx;
					return run_in_thread_destroy(thread_ptr, 3, &in_use);
				}

				if(repeat_count > 0) {
					wait_seconds(thread_ptr->rx_interval_time, 100);
				}

				int headerm = progStatus.use_header_modem;

				if(headerm && (tx->unproto() == false)) {
					if(vector_header_count > 0) {
						LOG_DEBUG("Header Modem Sent to FLDGI. (%s)", g_header_modem.c_str());
						send_new_modem(thread_ptr->header_modem);

						if(!send_vector_to_fldigi(thread_ptr, thread_ptr->header_modem, tail_bk, vector_header_data, thread_ptr->mode, tx)) {
							delete tx;
							return run_in_thread_destroy(thread_ptr, 3, &in_use);
						}
					}
				}

				if(vector_header_count > 0) {
					if(thread_ptr->mode == TX_SEGMENTED)
						wait_seconds(thread_ptr->rx_interval_time, 100);
					else
						wait_seconds(1, 100);
				}

				// Data section
				if(progStatus.use_header_modem || progStatus.fldigi_xmt_mode_change) {
					send_new_modem(thread_ptr->modem);
				}

				if(transmit_stop) {
					delete tx;
					return run_in_thread_destroy(thread_ptr, 3, &in_use);
				}


				LOG_DEBUG("vector_data_count = %d", vector_data_count);

				tail.assign(tail_bk);

				if((count >= (limit - 1)) && !continuous_exception)
					if(repeat_count >= (no_of_repeats - 1))
						tail.assign(tail_k);

				if(vector_data_count)
					send_vector_to_fldigi(thread_ptr, thread_ptr->modem, tail, vector_data, thread_ptr->mode, tx);
			}
		}

		if(event_bail_flag && thread_ptr->event_driven) break;

		if(tx) {
			delete tx;
			tx = 0;
		}

		index++;
		count++;

	} while(count < limit);

	event_bail_flag = false;

	Fl::awake(show_current_selected_file, (void *)&last_selected_tx_file);

	if(tx) {
		delete tx;
		tx = 0;
	}

	return run_in_thread_destroy(thread_ptr, 3, &in_use);
}

/** ********************************************************
 *
 ***********************************************************/
bool send_vector_to_fldigi(TX_FLDIGI_THREAD *thread, std::string modem, std::string &tail,
						   std::vector<std::string> vector_data, int mode, cAmp *tx)
{
	std::string temp;
	std::string send;
	std::string extras;
	int count = 0;
	int index = 0;
	float data_time_seconds = 0;
	float transfer_segment_time = 0;
	float transfer_limit_time = 0;
	float overhead = 0;
	float length = 0;
	float next_length = 0;
	float wait_factor = 3.0;
	float extras_time = 0.0;
	float rsid_time = 0.0;
	std::string send_to = tx->callto();

	if(transmit_stop) return false;

	send.clear();
	temp.clear();

	extras.assign("\n\n\n\n ");
	if(mode == TX_SEGMENTED) {
		extras.append("BK");
	} else {
		extras.append("ID");
	}
	extras_time = seconds_from_string(modem, extras, NULL);

	rsid_time = (using_rsid ? rsid_duration(modem) : 0);

	if(tx->amp_type() == RX_AMP)
		send_to.clear();
	else
		if (send_to.empty()) send_to.assign("QST");

	send_to.append(" DE ").append(progStatus.my_call);
	for (size_t n = 0; n < send_to.length(); n++)
		send_to[n] = toupper(send_to[n]);

	temp.clear();
	count = vector_data.size();
	for(index = 0; index < count; index++) {
		temp.append(vector_data[index]);
	}

	data_time_seconds = seconds_from_string(modem, temp, &overhead);

	data_time_seconds += overhead;
	tx_time_g = data_time_seconds;

	if(data_time_seconds <= 0.0) return false;

	if(mode == TX_SEGMENTED) {
		transfer_limit_time = thread->tx_interval_time;
		if (transfer_limit_time < 1) return false;
	} else
		transfer_limit_time = (float) ID_TIME_SECONDS;

	length = seconds_from_string(modem, send, &overhead);
	length = overhead;
	length += rsid_time;
	length += extras_time;
	length += 0.5;

	if(data_time_seconds > transfer_limit_time) {

		temp.assign("\n");

		for(index = 0; index < count; index++) {

			if(transmit_stop) return false;

			length += seconds_from_string(modem, vector_data[index], &overhead);

			temp.append(vector_data[index]);

			if(index < (count - 1)) {
				next_length = seconds_from_string(modem, vector_data[index + 1], &overhead);
			} else
				next_length = 0;

			transfer_segment_time = length + next_length;

			if (round(transfer_segment_time) >= transfer_limit_time) {
				tx_time_g = length;
				send.append(temp).append("\n").append(send_to);
				if(mode == TX_SEGMENTED) {
					send.append(" BK");
				} else {
					send.append(" ID");
				}
				send.append("\n");
				transfer(send);

				if(mode == TX_SEGMENTED) {
					if (!wait_for_rx(length, wait_factor)) return false;
					wait_seconds(thread->rx_interval_time, 100);
				}

				send.assign("\n");
				temp.clear();
				length = overhead;
				length += rsid_time;
				length += extras_time;
				length += 0.5;
			}
		}

		if(temp.size())
			send.append(temp);

		if(tail.size())
			send.append(tail);

		if(send.size()) {
			tx_time_g = seconds_from_string(modem, send, &overhead);
			tx_time_g += overhead;
			transfer(send);
			if (!wait_for_rx(transfer_segment_time, wait_factor)) return false;
		}

	} else {
		send.append("\n").append(temp);
		if(tail.size())
			send.append(tail);

		transfer(send);

		if (!wait_for_rx(data_time_seconds, wait_factor)) return false;
	}

	if(progStatus.use_header_modem) {
		if(progStatus.hamcast_mode_cycle && transmit_queue && g_event_driven) {
			std::string m;
			switch(modem_rotation_index) {
				case 0:
					send_new_modem(cbo_hamcast_mode_selection_1->value());
					break;
				case 1:
					send_new_modem(cbo_hamcast_mode_selection_2->value());
					break;
				case 2:
					send_new_modem(cbo_hamcast_mode_selection_3->value());
					break;
				case 3:
					send_new_modem(cbo_hamcast_mode_selection_4->value());
					break;
				default:
					send_new_modem(cbo_modes->value());
			}
		}
		else {
			send_new_modem(cbo_modes->value()); // Change to primary modem
		}
	}

	return true;
}

/** ********************************************************
 *
 ***********************************************************/
bool check_block_tx_time(std::vector<std::string> &header,
						 std::vector<std::string> &data, TX_FLDIGI_THREAD *thread_ptr)
{
	int index = 0;
	int count = 0;
	float max_tx_time = 0;
	float interval_time = (float) cnt_tx_interval_mins->value();
	float tx_time = 0;
	bool return_value = true;
	char str_buffer[THREAD_ERR_MSG_SIZE];
	std::string modem;

	count = header.size();

	modem.clear();
	if(progStatus.use_header_modem)
		modem.assign(cbo_header_modes->value());
	else
		modem.assign(cbo_modes->value());

	for(index = 0; index < count; index++) {
		tx_time = minutes_from_string(modem, header[index], (float *)0);
		if(tx_time > max_tx_time) {
			max_tx_time = tx_time;
		}
	}

	if(max_tx_time > interval_time) {
		memset(str_buffer, 0, sizeof(str_buffer));
		snprintf(str_buffer, sizeof(str_buffer),
				 "Header Section Modem: %s\nRequired Block Tx time: %3.1f Mins.\nUse Faster Mode, Increase Interval Time,\nor decrease block size.", \
				 modem.c_str(), max_tx_time);
		LOG_INFO("%s", str_buffer);
		return_value = false;
		if(thread_ptr) {
			memset(&thread_ptr->err_msg[0], 0, THREAD_ERR_MSG_SIZE);
			strcpy(&thread_ptr->err_msg[0], str_buffer);
			thread_ptr->err_flag = true;
		}
	}

	count = data.size();

	modem.clear();
	modem.assign(cbo_modes->value());

	max_tx_time = 0;

	for(index = 0; index < count; index++) {
		tx_time = minutes_from_string(modem, data[index], (float *)0);
		if(tx_time > max_tx_time) {
			max_tx_time = tx_time;
		}
	}

	if(max_tx_time > interval_time) {
		memset(str_buffer, 0, sizeof(str_buffer));
		snprintf(str_buffer, sizeof(str_buffer),
				 "Data Section Modem: %s\nRequired Block Tx time: %3.1f Mins.\nUse Faster Mode, Increase Interval Time,\nor decrease block size.", \
				 modem.c_str(), max_tx_time);
		LOG_INFO("%s", str_buffer);
		return_value = false;

		if(thread_ptr) {
			memset(&thread_ptr->err_msg[0], 0, THREAD_ERR_MSG_SIZE);
			strcpy(&thread_ptr->err_msg[0], str_buffer);
			thread_ptr->err_flag = true;
		}
	}

	return return_value;
}

/** ********************************************************
 *
 ***********************************************************/
bool wait_for_rx(int max_wait_seconds, float factor)
{
	
	if(max_wait_seconds < 1) return false;

	for(int i = 0; i < 2; i++) {
		if(transmit_stop) return false;

		wait_seconds(1, 500);
		if((get_trx_state() == "TX"))
			break;
	}

	max_wait_seconds += 4; // in case RSID is enabled

	for(int i = 0; i < max_wait_seconds * factor; i++) {
		if(transmit_stop) return false;

		wait_milliseconds(1000, 250);
		if((get_trx_state() == "RX")) {
			return true;
		}
	}

	return false;
}

//#define LOG_TIME_WAIT
#undef LOG_TIME_WAIT

/** ********************************************************
 *
 ***********************************************************/
static void wait_seconds(int seconds, int millisecond_increments)
{
	struct timeval		tp;
	double end_time     = 0.0;
	double current_time = 0.0;

	if(seconds < 1 || transmit_stop) return;

	if((seconds * 1000) < millisecond_increments)
		millisecond_increments = (seconds * 1000) >> 1;

	if(millisecond_increments < 1)
		millisecond_increments = 1;

	gettimeofday(&tp, NULL);
	end_time = (tp.tv_sec + seconds) + (tp.tv_usec * 0.000001);

	do {
		if(transmit_stop) break;
		MilliSleep(millisecond_increments);
		gettimeofday(&tp, NULL);
		current_time = (tp.tv_sec) + (tp.tv_usec * 0.000001);
	} while(current_time < end_time);

#ifdef LOG_TIME_WAIT
	gettimeofday(&tp, NULL);
	current_time = (tp.tv_sec) + (tp.tv_usec * 0.000001);
	LOG_DEBUG("SEC: %f DELAY", current_time - start_time);
#endif

}

/** ********************************************************
 *
 ***********************************************************/
static void wait_milliseconds(int milliseconds, int millisecond_increments)
{
	struct timeval		tp;
	double end_time = 0.0;
	double current_time = 0.0;

	if(milliseconds < 1 || transmit_stop) return;

	if(milliseconds < millisecond_increments)
		millisecond_increments = milliseconds >> 1;

	if(millisecond_increments < 1)
		millisecond_increments = 1;

	gettimeofday(&tp, NULL);
	end_time = (tp.tv_sec) + (tp.tv_usec * 0.000001) + (milliseconds * 0.001);

	do {
		if(transmit_stop) break;
		MilliSleep(millisecond_increments);
		gettimeofday(&tp, NULL);
		current_time = (tp.tv_sec) + (tp.tv_usec * 0.000001);
	} while(current_time < end_time);

#ifdef LOG_TIME_WAIT
	gettimeofday(&tp, NULL);
	current_time = (tp.tv_sec) + (tp.tv_usec * 0.000001);
	LOG_DEBUG("SEC: %f DELAY", current_time - start_time);
#endif

}

/** ********************************************************
 *
 ***********************************************************/
TX_FLDIGI_THREAD * run_in_thread(void *(*func)(void *), int mode,
								 bool queued, bool event_driven, RELAY_DATA *relay_data)
{
	TX_FLDIGI_THREAD *tx_thread = (TX_FLDIGI_THREAD *)0;
	int count = 0;

	if(transmitting) return tx_thread;

	tx_thread = new TX_FLDIGI_THREAD;

	if(tx_thread) {

		tx_thread->err_flag = false;
		tx_thread->mode = mode;
		tx_thread->que  = queued;
		tx_thread->event_driven = event_driven;
		tx_thread->rx_interval_time = cnt_rx_interval_secs->value();
		tx_thread->tx_interval_time = floor(cnt_tx_interval_mins->value() * 60);
		tx_thread->data = (void *) relay_data;

		if(relay_data)
			tx_thread->amp_type = RX_AMP;
		else
			tx_thread->amp_type = TX_AMP;

		if(progStatus.hamcast_mode_cycle && g_event_driven && queued) {
			if(modem_rotation_index >= bc_modems.size())
				modem_rotation_index = 0;

			tx_thread->modem.assign(bc_modems[modem_rotation_index]);
			modem_rotation_index++;

		} else {
			g_modem.assign(cbo_modes->value());
			tx_thread->modem.assign(g_modem);
		}

		tx_thread->header_modem.assign(g_header_modem);

		if(!pthread_mutex_init(&tx_thread->mutex, NULL)) {
			count++;
			if(!pthread_cond_init(&tx_thread->condition, NULL)) {
				count++;
				if(!pthread_attr_init(&tx_thread->attr)) {
					count++;
					if(!pthread_attr_setdetachstate(&tx_thread->attr, PTHREAD_CREATE_DETACHED)) {
						count++;
						if(!pthread_create(&tx_thread->thread, &tx_thread->attr, func, (void *) tx_thread)) {
							tx_thread_running_count++;
							return tx_thread;
						}
					}
				}
			}
		}

		run_in_thread_destroy(tx_thread, count, 0);
		LOG_ERROR("%s", "Thread creation error: run_in_thread()");
	}

	return (TX_FLDIGI_THREAD *) 0;
}

/** ********************************************************
 *
 ***********************************************************/
void * run_in_thread_destroy(TX_FLDIGI_THREAD *tx_thread, int level,
							 bool *in_use_flag)
{
	if(in_use_flag)
		*in_use_flag = false;

	if(!tx_thread) return 0;

	transmitting = false;
	transmit_queue = false;

	if(transmit_stop)
		Fl::awake(abort_tx_from_main, (void *)0);

	if(tx_thread_running_count > 0)
		tx_thread_running_count--;

	{
		static int value = TX_BUTTON;
		Fl::awake(deactivate_button, (void *) &value);
	}

	pthread_mutex_lock(&tx_thread->mutex);
	tx_thread->thread_running = 0;
	pthread_mutex_unlock(&tx_thread->mutex);

	if(level > 3 || level < 1) level = 3;

	switch (level) {
		case 3: pthread_attr_destroy(&tx_thread->attr);
		case 2:	pthread_cond_destroy(&tx_thread->condition);
		case 1: pthread_mutex_destroy(&tx_thread->mutex);
	}

	if(tx_thread->err_flag) {
		char *msg = (char *) malloc(THREAD_ERR_MSG_SIZE);

		if(msg) {
			memset(msg, 0, THREAD_ERR_MSG_SIZE);
			strcpy(msg, tx_thread->err_msg);
			Fl::awake(thread_error_msg, (void *)msg);
		}
	}

	delete tx_thread;


	Fl::awake(set_relay_button_label, (void *)0);
	Fl::awake(set_button_to_xmit, (void *)0);
	Fl::awake(clear_missing, (void *)0);


	{
		static int value = TX_BUTTON;
		Fl::awake(activate_button, (void *)&value);
		static int value2 = TX_ALL_BUTTON;
		Fl::awake(activate_button, (void *)&value2);
	}

	wait_for_rx(5);

	transmitting = false;
	transmit_queue = false;

	return 0;
}
