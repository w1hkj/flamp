// =====================================================================
//
// status.cxx
//
// Author(s):
//	Dave Freese, W1HKJ Copyright (C) 2010
//  Robert Stiles, KK5VD Copyright (C) 2013
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


#include <iostream>
#include <fstream>
#include <string>

#include <FL/Fl_Preferences.H>

#include "status.h"
#include "config.h"
#include "flamp.h"
#include "flamp_dialog.h"
#include "file_io.h"

status progStatus = {
	50,				// int mainX;
	50,				// int mainY;

	"",				// my_call
	"",				// my_info
	"127.0.0.1",	// fldigi socket address
	"7322",			// fldigi socket port
	"127.0.0.1",	// fldigi xmlrpc socket address
	"7362",			// fldigi xmlrpc socket port

					// User Assigned addr/ports not saved.
	"",				// User assigned fldigi socket address
	"",				// User assigned fldigi socket port
	"",				// User assigned fldigi xmlrpc socket address
	"",				// User assigned fldigi xmlrpc socket port

	true,			// use_compression
	BASE256,		// encoder
	1,				// selected_mode
	64,				// blocksize
	1,				// repeatNN
	1,				// repeat_header

	false,			// bool sync_mode_flamp_fldigi;
	false,			// bool sync_mode_fldigi_flamp;
	false,			// bool fldigi_xmt_mode_change;

	1,				// int repeat_every;
	false,			// bool repeat_at_times;
	"",				// string repeat_times;
	false,			// bool repeat_at_times;

	false, 			// bool use_txrx_interval;
	3, 				// int  tx_interval_minutes;
	10, 			// int  rx_interval_seconds;

	false, 			// bool use_header_modem;
	1,				// int  header_selected_mode;
	false,          // bool disable_header_modem_on_block_fills;

	0,				// int  use_tx_on_report;

	false,			// bool clear_tosend_on_tx_blocks;

	false           // bool enable_tx_unproto
};

void status::saveLastState()
{
	Fl_Preferences FLAMPpref(flampHomeDir.c_str(), "w1hkj.com",  PACKAGE_NAME);

	int mX = main_window->x();
	int mY = main_window->y();
	if (mX >= 0 && mX >= 0) {
		mainX = mX;
		mainY = mY;
	}

	FLAMPpref.set("version", PACKAGE_VERSION);
	FLAMPpref.set("mainx", mX);
	FLAMPpref.set("mainy", mY);

	my_call = txt_tx_mycall->value();
	my_info = txt_tx_myinfo->value();

	FLAMPpref.set("mycall", my_call.c_str());
	FLAMPpref.set("myinfo", my_info.c_str());
	FLAMPpref.set("socket_address", socket_addr.c_str());
	FLAMPpref.set("socket_port", socket_port.c_str());
	FLAMPpref.set("xmlrpc_address", xmlrpc_addr.c_str());
	FLAMPpref.set("xmlrpc_port", xmlrpc_port.c_str());
	FLAMPpref.set("blocksize", blocksize);
	FLAMPpref.set("repeatNN", repeatNN);
	FLAMPpref.set("repeat_header", repeat_header);
	FLAMPpref.set("selected_mode", selected_mode);
	FLAMPpref.set("compression", use_compression);
	FLAMPpref.set("encoder", encoder);
	FLAMPpref.set("sync_mode_flamp_fldigi", sync_mode_flamp_fldigi);
	FLAMPpref.set("sync_mode_fldigi_flamp", sync_mode_fldigi_flamp);
	FLAMPpref.set("fldigi_xmt_mode_change", fldigi_xmt_mode_change);

	FLAMPpref.set("repeat_every", repeat_every);
	FLAMPpref.set("repeat_at_times", repeat_at_times);
	FLAMPpref.set("repeat_times", repeat_times.c_str());

	FLAMPpref.set("repeat_forever", repeat_forever);

	FLAMPpref.set("use_repeater_interval", use_txrx_interval);
	FLAMPpref.set("repeater_tx_minutes", tx_interval_minutes);
	FLAMPpref.set("repeater_rx_seconds", rx_interval_seconds);

	FLAMPpref.set("disable_header_modem_on_block_fills", disable_header_modem_on_block_fills);
	FLAMPpref.set("use_header_modem", use_header_modem);
	FLAMPpref.set("header_selected_mode", header_selected_mode);

	FLAMPpref.set("use_tx_on_report", use_tx_on_report);

	FLAMPpref.set("clear_tosend_on_tx_blocks", clear_tosend_on_tx_blocks);

	FLAMPpref.set("enable_tx_unproto", enable_tx_unproto);
}

void status::loadLastState()
{
	Fl_Preferences FLAMPpref(flampHomeDir.c_str(), "w1hkj.com", PACKAGE_NAME);

	if (FLAMPpref.entryExists("version")) {
		char *defbuffer;

		FLAMPpref.get("mainx", mainX, mainX);
		FLAMPpref.get("mainy", mainY, mainY);

		FLAMPpref.get("mycall", defbuffer, "");
		my_call = defbuffer; free(defbuffer);

		FLAMPpref.get("myinfo", defbuffer, "");
		my_info = defbuffer; free(defbuffer);

		FLAMPpref.get("socket_address", defbuffer, socket_addr.c_str());
		socket_addr = defbuffer; free(defbuffer);
		FLAMPpref.get("socket_port", defbuffer, socket_port.c_str());
		socket_port = defbuffer; free(defbuffer);

		FLAMPpref.get("xmlrpc_address", defbuffer, xmlrpc_addr.c_str());
		xmlrpc_addr = defbuffer; free(defbuffer);
		FLAMPpref.get("xmlrpc_port", defbuffer, xmlrpc_port.c_str());
		xmlrpc_port = defbuffer; free(defbuffer);

		FLAMPpref.get("blocksize", blocksize, blocksize);
		FLAMPpref.get("repeatNN", repeatNN, repeatNN);
		FLAMPpref.get("repeat_header", repeat_header, repeat_header);

		FLAMPpref.get("selected_mode", selected_mode, selected_mode);

		int i = 0;
		FLAMPpref.get("compression", i, use_compression);
		use_compression = i;

		FLAMPpref.get("sync_mode_flamp_fldigi", i, sync_mode_flamp_fldigi);
		sync_mode_flamp_fldigi = i;

		FLAMPpref.get("sync_mode_fldigi_flamp", i, sync_mode_fldigi_flamp);
		sync_mode_fldigi_flamp = i;


		FLAMPpref.get("fldigi_xmt_mode_change", i, fldigi_xmt_mode_change);
		fldigi_xmt_mode_change = i;

		FLAMPpref.get("encoder", encoder, encoder);

		FLAMPpref.get("repeat_every", repeat_every, repeat_every);
		FLAMPpref.get("repeat_at_times", i, repeat_at_times);
		repeat_at_times = i;
		FLAMPpref.get("repeat_times", defbuffer, repeat_times.c_str());
		repeat_times = defbuffer; free(defbuffer);
		FLAMPpref.get("repeat_forever", i, repeat_forever);
		repeat_forever = i;

		FLAMPpref.get("use_repeater_interval", i, use_txrx_interval);
		use_txrx_interval = (bool) i;

		FLAMPpref.get("repeater_tx_minutes", i, tx_interval_minutes);
		tx_interval_minutes = i;

		FLAMPpref.get("repeater_rx_seconds", i, rx_interval_seconds);
		rx_interval_seconds = i;

		FLAMPpref.get("disable_header_modem_on_block_fills", i, disable_header_modem_on_block_fills);
		disable_header_modem_on_block_fills = (bool) i;

		FLAMPpref.get("use_header_modem", i, use_header_modem);
		use_header_modem = (bool) i;

		FLAMPpref.get("header_selected_mode", i, header_selected_mode);
		header_selected_mode = i;
		
		FLAMPpref.get("use_tx_on_report", i, use_tx_on_report);
		use_tx_on_report = i;

		FLAMPpref.get("clear_tosend_on_tx_blocks", i, clear_tosend_on_tx_blocks);
		clear_tosend_on_tx_blocks = (bool) i;

		FLAMPpref.get("clear_tosend_on_tx_blocks", i, enable_tx_unproto);
		enable_tx_unproto = (bool) i;

		if(enable_tx_unproto) {
			use_txrx_interval = false;
			use_header_modem = false;
		}
	} 
}
