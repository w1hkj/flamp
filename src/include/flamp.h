// =====================================================================
//
// flamp.h
//
// Author: Dave Freese, W1HKJ
// Copyright: 2010
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
#include <FL/Fl.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Double_Window.H>

#include "crc16.h"
#include "threads.h"

#define DEBUG 1

using namespace std;

extern const char *flamp_beg;
extern const char *flamp_end;

extern Fl_Double_Window *mainwindow;
extern Fl_Double_Window *optionswindow;
extern Fl_Double_Window *config_files_window;
extern Fl_Double_Window *socket_window;

extern string title;
extern string flampHomeDir;
extern string flamp_dir;
extern string buffer;

extern void cb_exit();
extern void readfile();
extern void writefile();
extern void tx_removefile();
extern void show_selected_xmt(int);
extern void show_selected_rcv(int);
extern void update_selected_xmt();
extern void estimate();

extern void send_missing_report();
extern void recv_missing_report();

extern bool transmit_selected;
extern bool transmit_queue;

extern pthread_t *xmlrpc_thread;
extern pthread_mutex_t mutex_xmlrpc;

#endif
