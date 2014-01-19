// =====================================================================
//
// flamp.h
//
//  Author(s):
//    Robert Stiles, KK5VD, Copyright (C) 2013
//    Dave Freese, W1HKJ, Copyright (C) 2013
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  It is
// copyright under the GNU General Public License.
//
// You should have received a copy of the GNU General Public License
// along with the program; if not, write to the Free Software
// Foundation, Inc.
// 59 Temple Place, Suite 330
// Boston, MA  02111-1307 USA
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

#include "crc16.h"
#include "threads.h"
#include "timeops.h"
#include "amp.h"

#define DEBUG 1

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
	bool err_flag;              //!< @brief Indiacted a error occured
	int mode;                   //!< @brief Mode (modem index) used in time table generation
	int que;                    //!< @brief Flag to determine if a single file or multiple files to be sent.
	int exit_thread;            //!< @brief Setting to true causes thread to exit.
	int thread_running;         //!< @brief Flag indicating thread is running.
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
		exit_thread    = 0;
		thread_running = 0;
		bc_modems.clear();
		modem.clear();
		header_modem.clear();
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

//! Use to sync class cAmp global variables via a threaded mutex lock access.
class cAmpGlobal {

protected:

	cAmp *rx_amp;                //!< @brief Current selected receive cAmp pointer
	cAmp *tx_amp;                //!< @brief Current selected transmit cAmp pointer

	pthread_mutex_t mutex_rxAmp; //!< @brief Mutex locks for receive cAmp pointer access
	pthread_mutex_t mutex_txAmp; //!< @brief Mutex locks for pointer cAmp pointer access

public:
	cAmpGlobal();
	~cAmpGlobal();

	//! Access method for transmit cAmp pointer
	//! @param none (void)
	//! @return cAmp class pointer for current selected tx queue item.
	cAmp *tx_cAmp(void);

	//! Access method for transmit cAmp pointer.
	//! @param Set current selected TX cAmp ponter for global access.
	//! @return bool false if cAmp pointer NULL.
	bool tx_cAmp(cAmp *amp);

	//! Access method for transmit cAmp pointer
	//! @param none (void)
	//! @return cAmp class pointer for current selected rx queue item.
	cAmp *rx_cAmp(void);

	//! Access method for receive cAmp pointer.
	//! @param Set current selected RX cAmp ponter for global access.
	//! @return bool false if cAmp pointer was NULL.
	bool rx_cAmp(cAmp *amp);

	//! Free allocaled cAmp memory.
	//! @param The cAmp pointer to be freed.
	//! @return none (void)
	void free_cAmp(cAmp *amp) { if(amp) delete amp; }

};

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

#define ID_TIME_MINUTES (8)
#define ID_TIME_SECONDS (ID_TIME_MINUTES * 60)

#define BROADCAST_MAX_MODEMS 4

#define INTERVAL_TIME_BUFFER 10 // seconds

void alt_receive_data_stream(void);

extern const char *flamp_beg;
extern const char *flamp_end;

extern Fl_Double_Window *mainwindow;
extern Fl_Double_Window *optionswindow;
extern Fl_Double_Window *config_files_window;
extern Fl_Double_Window *socket_window;

extern const char *options[];
extern string title;
extern string flampHomeDir;
extern string flamp_dir;
extern string buffer;

extern bool transmitting;
extern bool transmit_stop;

extern void auto_load_tx_queue(void);
extern void cb_exit(void);
extern void cb_folders(void);
extern void addfile(string, void *, bool, char *);
extern void readfile(void);
extern void drop_file_changed(void);
extern void writefile(int);
extern int  valid_block_size(int value);
extern void tx_removefile(void);
extern void show_selected_xmt(int);
extern void show_selected_rcv(int);
extern void update_selected_xmt();
extern void estimate(void);
extern void transfer_time(std::string modem_name, float &cps, int &transfer_size, std::string buffer);
extern int  alt_receive_data_stream(void *);
extern int  process_que(void *que);
extern void show_help(void);
extern void process_data_stream(void);
extern void process_missing_stream(void);
extern bool wait_for_rx(int max_wait_seconds);
extern void wait_seconds(int seconds);
extern void send_missing_report(void);
extern void recv_missing_report(void);
extern void receive_remove_from_queue(void);
extern void transmit_current(void);
extern void transmit_queued(bool);
extern void preamble_detected(void);
extern void * transmit_serial_queued(void *);
extern void * transmit_interval(void *);
extern void * transmit_header_current(void *);
extern void * transmit_serial_current(void *);
extern bool send_vector_to_fldigi(std::string modem, std::string &send, std::string &tail, std::vector<std::string> vector_data, int mode);
extern TX_FLDIGI_THREAD * run_in_thread(void *(*func)(void *), int mode, bool queued);
extern void * run_in_thread_destroy(TX_FLDIGI_THREAD *tx_thread, int level);
extern void abort_request(void);

extern void thread_error_msg(void *data);
extern void set_xmit_label(void *data);
extern bool check_block_tx_time(cAmp *tx, TX_FLDIGI_THREAD *thread_ptr);
extern void set_button_to_xmit(void *);
extern void set_button_to_cancel(void *);
extern void deactivate_button(void *ptr);
extern void activate_button(void *ptr);
extern void send_fldigi_modem(void *ptr);
extern void get_int_value(void *ptr);
extern void get_string_value(void *ptr);
extern void get_c_string_value(void *ptr);
extern void * transmit_header(void * ptr);
extern void transmit_queue_main_thread(void *ptr);
extern void send_via_fldigi_in_main_thread(void *ptr);
extern void get_trx_state_in_main_thread(void *ptr);
extern void turn_rsid_on(void);
extern void turn_rsid_off(void);
extern void abort_and_id(void);

extern pthread_t *xmlrpc_thread;
extern pthread_mutex_t mutex_xmlrpc;
extern pthread_mutex_t mutex_file_io;
extern int xmlrpc_errno;
extern int file_io_errno;
extern bool generate_time_table;

#endif
