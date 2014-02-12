//======================================================================
//	transmit_camp.cxx
//
//  Author(s):
//	Dave Freese, W1HKJ, Copyright (C) 2010, 2011, 2012, 2013
//	Robert Stiles, KK5VD, Copyright (C) 2013
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

#ifndef __flamp_transmit_camp__
#define __flamp_transmit_camp__

#define TX_SEGMENTED    0x01  //!< @brief Interval Timer
#define TX_CONTINIOUS   0x02  //!< @brief No breaks in the transmitted data.
#define TX_MODEM_SAME   0x04  //!< @brief Header modem not used.
#define TX_SINGLE_MODEM 0x08  //!< @brief Create a time table for a single modem.
#define TX_MULTI_MODEM  0x10  //!< @brief Create a time table for all modems.

#define TX_BUTTON     1       //!< @brief Flag indicating transmit a single file.
#define TX_ALL_BUTTON 2       //!< @brief Flag indicating transmit all queued file.

#define HEADER_MODEM  1       //!< @brief Indicate this data is for header modem.
#define DATA_MODEM    2       //!< @brief Indicate this data is for data modem.

#define CALLSIGN_PREAMBLE  0x01
#define CALLSIGN_POSTAMBLE 0x02

#define BROADCAST_MAX_MODEMS 4  //!< @brief Maximum number of modems (hamcast).

#define INTERVAL_TIME_BUFFER 10 // seconds

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
	bool event_driven;          //!< @brief Indicate the tx thread is event_driven
	int rx_interval_time;       //!< @brief Delay period between transmits
	int mode;                   //!< @brief Mode (modem index) used in time table generation
	int que;                    //!< @brief Flag to determine if a single file or multiple files to be sent.
	int exit_thread;            //!< @brief Setting to true causes thread to exit.
	int thread_running;         //!< @brief Flag indicating thread is running.
	int amp_type;               //!< @brief What type of instance in this? RX_AMP or TX_AMP.
	vector<std::string> bc_modems; //!< @brief Local storage for hamcast modems.
	std::string modem;          //!< @brief Local storage for current selected modem
	std::string header_modem;   //!< @brief Local storage for current selected header modem

	_tx_fldigi_thread() {       //!< @brief Clear struct _tx_fldigi_thread memory on allocation.
		memset(&thread,    0, sizeof(thread));
		memset(&mutex,     0, sizeof(mutex));
		memset(&condition, 0, sizeof(condition));
		memset(&attr,      0, sizeof(attr));
		memset(&err_msg,   0, sizeof(err_msg));
		data = (void *)0;
		mode = 0;
		que  = 0;
		event_driven = false;
		exit_thread    = 0;
		thread_running = 0;
		bc_modems.clear();
		modem.clear();
		header_modem.clear();
	}

} TX_FLDIGI_THREAD;

extern unsigned int modem_rotation_index;
extern vector<std::string> bc_modems;
extern std::string g_modem;
extern std::string g_header_modem;

extern class cAmpGlobal tx_amp;
extern class cAmpGlobal rx_amp;

extern bool active_data_io;
extern bool transmit_queue;
extern bool event_bail_flag;

extern int last_selected_tx_file;
extern int g_event_driven;
extern int tx_thread_running_count;

extern void * transmit_serial_relay(void *ptr);
extern void * transmit_relay_interval(void *ptr);
extern void clear_missing(void *ptr);
extern TX_FLDIGI_THREAD * run_in_thread(void *(*func)(void *), int mode, bool queued, bool event_driven, RELAY_DATA *relay_data);
extern void * run_in_thread_destroy(TX_FLDIGI_THREAD *tx_thread, int level, bool *in_use_flag);
extern void * transmit_header_current(void *);
extern void * transmit_header(void * ptr);
extern void * transmit_interval(void *);
extern void * transmit_serial_current(void *);
extern void * transmit_serial_queued(void *);
extern bool check_block_tx_time(std::vector<std::string> &header, std::vector<std::string> &data, TX_FLDIGI_THREAD *thread_ptr);
extern bool send_vector_to_fldigi(std::string modem, std::string &tail, std::vector<std::string> vector_data, int mode, cAmp *tx);
extern bool wait_for_rx(int max_wait_seconds);


#endif /* defined(__flamp_transmit_camp__) */
