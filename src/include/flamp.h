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

#define DEBUG 1

using namespace std;

#define THREAD_ERR_MSG_SIZE 256

typedef struct {
	pthread_t thread;
	pthread_mutex_t mutex;
	pthread_cond_t  condition;
	pthread_attr_t  attr;
	void *data;
	char err_msg[THREAD_ERR_MSG_SIZE];
	bool err_flag;
	int mode;
	int que;
	int exit_thread;
	int thread_running;
} TX_FLDIGI_THREAD;

typedef struct {
	void *widget;
	string strValue;
} STRING_VALUE;

typedef struct {
	void *widget;
	int intValue;
} INT_VALUE;


#define TX_SEGMENTED    0x01
#define TX_CONTINIOUS   0x02
#define TX_MODEM_SAME   0x04
#define TX_SINGLE_MODEM 0x08
#define TX_MULTI_MODEM  0x10

#define TX_BUTTON     1
#define TX_ALL_BUTTON 2

#define HEADER_MODEM  1
#define DATA_MODEM    2

#define ID_TIME_MINUTES (8)
#define ID_TIME_SECONDS (ID_TIME_MINUTES * 60)

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

extern void cb_exit(void);
extern void cb_folders(void);
extern void addfile(string, void *);
extern void readfile(void);
extern void drop_file_changed(void);
extern void writefile(int);
extern int valid_block_size(int value);
extern void tx_removefile(void);
extern void show_selected_xmt(int);
extern void show_selected_rcv(int);
extern void update_selected_xmt();
extern void estimate(void);
extern void transfer_time(std::string modem_name, float &cps, int &transfer_size, std::string buffer);
extern void transmit_queued(void);
extern int  alt_receive_data_stream(void *);
extern int  process_que(void *que);
extern void show_help(void);
extern void process_data_stream(void);
extern bool wait_for_rx(int max_wait_seconds);
extern void wait_seconds(int seconds);
extern void send_missing_report(void);
extern void recv_missing_report(void);
extern void receive_remove_from_queue(void);
extern void transmit_current(void);
extern void transmit_queued(void);
extern void preamble_detected(void);
extern void * transmit_serial_queued(void *);
extern void * transmit_interval(void *);
extern void * transmit_header_current(void *);
extern void * transmit_serial_current(void *);
extern bool send_vector_to_fldigi(std::string modem, std::string &send, std::string &tail, std::vector<std::string> vector_data, int mode);
extern TX_FLDIGI_THREAD * run_in_thread(void *(*func)(void *), int mode, bool queued);
extern void * run_in_thread_destroy(TX_FLDIGI_THREAD *tx_thread, int level);
extern void abort_request(void);

extern pthread_t *xmlrpc_thread;
extern pthread_mutex_t mutex_xmlrpc;
extern pthread_mutex_t mutex_file_io;
extern int xmlrpc_errno;
extern int file_io_errno;


#endif
