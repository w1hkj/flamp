/** **************************************************************
 \page run_scripts Executing Script Parsing Class

 \par run_script.cxx (FLAMP)

 \par Author(s):
 Robert Stiles, KK5VD, Copyright &copy; 2014
 <br>
 <br>
 This is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version. This software is distributed in
 the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the GNU General Public License for more details. You
 should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 <br>
 <br>
 \par USE:
 This code is the interface between the parsing engine and the
 GUI code. The processing of the GUI is performed via 'C' style
 callback fuctions.
 <br>
 \par NOTE:
 Do not call process_xxx() functions directly. Must be called from the
 ScriptParsing Class.
 *******************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <ctime>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <libgen.h>
#include <ctype.h>
#include <sys/time.h>

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
//#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_File_Chooser.H>

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
#include "fileselect.h"

#ifdef WIN32
#  include "flamprc.h"
#  include "compat.h"
#endif

#include <FL/filename.H>

#include <FL/x.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Image.H>

#ifdef __WIN32__
#define PATH_SEPERATOR "\\"
#define PATH_CHAR_SEPERATOR '\\'
#else
#define PATH_SEPERATOR "/"
#define PATH_CHAR_SEPERATOR '/'
#endif

pthread_mutex_t mutex_script_io = PTHREAD_MUTEX_INITIALIZER;

extern const char *s_basic_modes[];
extern const char *event_types[];

extern std::string flamp_script_dir;
extern std::string flamp_script_default_dir;

extern void addfile(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
extern void auto_load_tx_queue_from_tx_directory(void);

static int process_auto_load_queue(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_base(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_blocks(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_callfrom(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_clear_missing(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_clear_rx_queue(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_clear_tx_queue(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_compression(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_event_forever(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_event_timed(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_event_times(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_event_type(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_event(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_file(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_hamcast_modem(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_hamcast(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_header_modem(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_header_repeat(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_info(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_inhibit_header(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_interval(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_load_txdir(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_modem(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_queue_filepath(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_reset(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_rx_interval(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_sync_with(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_tx_interval(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_tx_report(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_unproto_markers(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_warn_user(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_xmit_repeat(ScriptParsing *sp, SCRIPT_COMMANDS *sc);

#if 0 // Unsued fuctions
static int process_callto(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_header(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_path(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
static int process_proto(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
#endif

typedef struct _call_script {
	char filename[FL_PATH_MAX];
	bool queueflag;
} CALL_SCRIPT;

CALL_SCRIPT call_script;

void script_execute(void *);
void script_execute(const char *filename, bool queue_flag);

//! @struct command_funcs
//! An array of script commands to their respect call functions. Used
//! to assign the call back routine.

typedef struct command_funcs {
	char * command;
	int (*func)(ScriptParsing *sp, SCRIPT_COMMANDS *sc);
} COMMAND_FUNCS;

//! @typedef COMMAND_FUNCS
//! @see struct command_funcs
//! Storage for the command to function call backs.

static COMMAND_FUNCS callback_functions[] = {
	{ (char *) CMD_AUTO_LOAD_QUEUE, process_auto_load_queue },
	{ (char *) CMD_BASE,            process_base },
	{ (char *) CMD_BLOCKS,          process_blocks },
	{ (char *) CMD_CALLFROM,        process_callfrom },
	{ (char *) CMD_CLEAR_MISSING,   process_clear_missing },
	{ (char *) CMD_CLEAR_RXQ,       process_clear_rx_queue },
	{ (char *) CMD_CLEAR_TXQ,       process_clear_tx_queue },
	{ (char *) CMD_COMP,            process_compression },
	{ (char *) CMD_EVENT_FOREVER,   process_event_forever },
	{ (char *) CMD_EVENT_TIMED,     process_event_timed },
	{ (char *) CMD_EVENT_TIMES,     process_event_times },
	{ (char *) CMD_EVENT_TYPE,      process_event_type },
	{ (char *) CMD_EVENT,           process_event },
	{ (char *) CMD_FILE,            process_file },
	{ (char *) CMD_HAMCAST_MODEM,   process_hamcast_modem },
	{ (char *) CMD_HAMCAST,         process_hamcast },
	{ (char *) CMD_HDR_REPEAT,      process_header_repeat },
	{ (char *) CMD_HEADER_MODEM,    process_header_modem },
	{ (char *) CMD_INFO,            process_info },
	{ (char *) CMD_INHIBIT_HEADER,  process_inhibit_header},
	{ (char *) CMD_INTERVAL,        process_interval },
	{ (char *) CMD_LOAD_TXDIR,      process_load_txdir},
	{ (char *) CMD_MODEM,           process_modem },
	{ (char *) CMD_QUEUE_FILEPATH,  process_queue_filepath },
	{ (char *) CMD_RESET,           process_reset },
	{ (char *) CMD_RX_INTERVAL,     process_rx_interval },
	{ (char *) CMD_SYNC_WITH,       process_sync_with },
	{ (char *) CMD_TX_INTERVAL,     process_tx_interval },
	{ (char *) CMD_TX_REPORT,       process_tx_report },
	{ (char *) CMD_UNPROTO_MARKERS, process_unproto_markers },
	{ (char *) CMD_WARN_USER,       process_warn_user },
	{ (char *) CMD_XMIT_REPEAT,     process_xmit_repeat },
#if 0 // Currently not used at this level
	{ (char *) CMD_CALLTO,          process_callto }, // Handled in the GUI code
	{ (char *) CMD_HEADER,          process_header }, // Handled in HEADER MODEM COMMAND
	{ (char *) CMD_PATH,            process_path },   // Handled internally
	{ (char *) CMD_PROTO,           process_proto },  // Handled in the GUI code
#endif
	{ (char *) 0, 0}
};

#if 0 // Unused Functions, Do not remove code.
/** ********************************************************
 * \brief Enable/Disable protocol use (AMP-2).
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_proto(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	bool flag = sp->proto(); // Logic inverted for syntax reasons.

	if(flag) {
		btn_enable_tx_unproto->value(false);
		progStatus.enable_tx_unproto = false;
	} else {
		btn_enable_tx_unproto->value(true);
		progStatus.enable_tx_unproto = true;
	}

	return 0;
}

/** ********************************************************
 * \brief Not used.
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_header(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	btn_enable_header_modem->value(sp->header_modem_enable());
	progStatus.use_header_modem = sp->header_modem_enable();

	return 0;
}

/** ********************************************************
 * \brief Assign Call to.
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 * \par Note:
 * This string storage can be assigned to anything. User
 * should follow the limitations imposed by the rules
 * of the host country.
 ***********************************************************/
static int process_callto(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	return 0; // Leave this in place. cAmp panel data update conflict.

	if(!sp->call_to().empty()) {
		txt_tx_send_to->value(sp->call_to().c_str());
	}

	return 0;
}
#endif // #if 0 Unused Functions, Do not remove code.

/** ********************************************************
 * \brief Trim leading and trailing spaces from string.
 * \param s String to modify
 * \return s modified string.
 ***********************************************************/
static inline std::string &trim(std::string &s) {
	char *buffer = (char *)0;
	char *dst    = (char *)0;
	char *end    = (char *)0;
	char *src    = (char *)0;
	long count   = s.size();

	buffer = new char[count + 1];
	if(!buffer) return s;

	memcpy(buffer, s.c_str(), count);
	buffer[count] = 0;

	dst = src = buffer;
	end = &buffer[count];

	while(src < end) {
		if(*src > ' ') break;
		src++;
	}

	if(src > dst) {
		while((dst < end) && (src < end))
			*dst++ = *src++;
		*dst = 0;
	}

	while(end >= buffer) {
		if(*end > ' ') break;
		*end-- = 0;
	}

	s.assign(buffer);

	delete [] buffer;

	return s;
}

/** ********************************************************
 * \brief Assign auto load queue flag for timed events.
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_auto_load_queue(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	bool value = sp->auto_load_queue();
	progStatus.auto_load_queue = value;
	btn_auto_load_queue->value(value);

	return 0;
}

/** ********************************************************
 * \brief Assign base conversion type.
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_base(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	return 0; // Leave this in place. cAmp panel data update conflict.

	char *base_string = (char *)0;
	int base = sp->base();
	int index = 0;

	switch(base) {
		case 64:
			base_string = (char *) "base64";
			index = 1;
			break;

		case 128:
			base_string = (char *) "base128";
			index = 2;
			break;

		case 256:
			base_string = (char *) "base256";
			index = 3;
			break;

		default:
			LOG_INFO("Unknown base encoder selected (%d)", base);
			return -1;
	}

	progStatus.encoder = index;
	encoders->value(base_string);

	return 0;
}

/** ********************************************************
 * \brief Assign block size.
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_blocks(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	int blocks = sp->blocks();
	int mask   = ~0x0f;
	int value = 0;

	if((blocks >= CNT_BLOCK_SIZE_MINIMUM) && (blocks <= CNT_BLOCK_SIZE_MAXIMUM)) {
		value = blocks & mask;
		cnt_blocksize->value(value);
		progStatus.blocksize = value;
	}

	return 0;
}

/** ********************************************************
 * \brief Assign Call from.
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 * \par Note:
 * This string storage can be assigned to anything. User
 * should follow the limitations imposed by the rules
 * of the host country.
 ***********************************************************/
static int process_callfrom(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	if(!sp->call_from().empty()) {
		txt_tx_mycall->value(sp->call_from().c_str());
		progStatus.my_call.assign(sp->call_from());
	}

	return 0;
}

/** ********************************************************
 * \brief Set flag to clear the missing report queue after
 * the retransmitting of the missed data.
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_clear_missing(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	bool value = sp->clear_missing();
	progStatus.clear_tosend_on_tx_blocks = value;
	btn_clear_tosend_on_tx_blocks->value(value);

	return 0;
}

/** ********************************************************
 * \brief Clear the receive queue.
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_clear_rx_queue(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	int count = rx_queue->size();
	if(count) {
		rx_queue->select(1);
		receive_remove_from_queue(true);
	}

	return 0;
}

/** ********************************************************
 * \brief Clear the transmit queue.
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_clear_tx_queue(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	int count = tx_queue->size();
	if(count) {
		tx_queue->select(1);
		tx_removefile(true);
	}

	return 0;
}

/** ********************************************************
 * \brief Enable/Disable Compression. Compression may or may
 * not be enabled as file size takes precedence.
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_compression(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	bool value = sp->comp();
	progStatus.use_compression = value;
	btn_use_compression->value(value);
	return 0;
}

/** ********************************************************
 * \brief
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_event_times(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	std::string event_time;

	event_time.assign(sp->event_times());
	if(event_time.empty()) return 0;

	event_time = trim(event_time);
	txt_repeat_times->value(event_time.c_str());
	progStatus.repeat_times.assign(event_time);

	return 0;
}

/** ********************************************************
 * \brief
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_event_timed(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	bool value = sp->event_timed();

	btn_repeat_at_times->value(value);
	btn_repeat_at_times->do_callback();

	if(value)
		sp->event_forever(false);

	return 0;
}

/** ********************************************************
 * \brief
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_event_type(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	int event_type = sp->event_type();
	char *cPtr = (char *)0;

	switch(event_type) {
		case et_5_min:
		case et_15_min:
		case et_30_min:
		case et_hourly:
		case et_even_hours:
		case et_odd_hours:
		case et_repeat_at:
		case et_one_time_at:
		case et_continious_at:
			progStatus.repeat_every = event_type;
			cPtr = (char *) event_types[event_type];
			if(cPtr) {
				cbo_repeat_every->value(cPtr);
			}
	}

	return 0;
}

/** ********************************************************
 * \brief Enable/Disable Events (all kinds)
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 **********************************************************/
static int process_event(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	bool value = sp->event();
	int results = 1;
	int warn = 0;

	warn = sp->event_forever() | btn_repeat_forever->value();

	if(value && warn)
		results = fl_choice2(_("Enable Events?\nIMMEDIATE TRANSMISSION WILL OCCUR!"),
							 _("No"), _("Yes"), NULL);

	warn = sp->event_timed() | btn_repeat_at_times->value();

	if(value && warn)
		results = fl_choice2(_("Enable Events?\nImmediate Transmission is possible!"),
							 _("No"), _("Yes"), NULL);

	if(results) {
		do_events->value(value);
		do_events->do_callback();
	}

	return 0;
}

/** ********************************************************
 * \brief Enable/Disable forever events
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 **********************************************************/
static int process_event_forever(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{

	bool repeat_forever = sp->event_forever();
	btn_repeat_forever->value(repeat_forever);
	btn_repeat_forever->do_callback();
	if(repeat_forever)
		sp->event_timed(false);

	return 0;
}

/** ********************************************************
 * \brief Add a file to the transmit queue.
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_file(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	addfile(sp, sc);

	return 0;
}

/** ********************************************************
 * \brief Select the Hamcast modem for each of the 4 positions
 * available.
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_hamcast_modem(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	if(!sc->args[0]) return -1;

	int modem_index = atoi(sc->args[0]);

	if(modem_index < 1 || modem_index > 4) return -1;

	switch(modem_index) {
		case 1:
			btn_hamcast_mode_enable_1->value(sp->hamcast_modem_1_enable());
			progStatus.hamcast_mode_enable_1 = sp->hamcast_modem_1_enable();

			if(!sp->hamcast_modem_1().empty()) {
				cbo_hamcast_mode_selection_1->value(sp->hamcast_modem_1().c_str());
				progStatus.hamcast_mode_selection_1 = cbo_hamcast_mode_selection_1->index();
			}
			break;

		case 2:
			btn_hamcast_mode_enable_2->value(sp->hamcast_modem_2_enable());
			progStatus.hamcast_mode_enable_2 = sp->hamcast_modem_2_enable();

			if(!sp->hamcast_modem_2().empty()) {
				cbo_hamcast_mode_selection_2->value(sp->hamcast_modem_2().c_str());
				progStatus.hamcast_mode_selection_2 = cbo_hamcast_mode_selection_2->index();
			}
			break;

		case 3:
			btn_hamcast_mode_enable_3->value(sp->hamcast_modem_3_enable());
			progStatus.hamcast_mode_enable_3 = sp->hamcast_modem_3_enable();

			if(!sp->hamcast_modem_3().empty()) {
				cbo_hamcast_mode_selection_3->value(sp->hamcast_modem_3().c_str());
				progStatus.hamcast_mode_selection_3 = cbo_hamcast_mode_selection_3->index();
			}
			break;

		case 4:
			btn_hamcast_mode_enable_4->value(sp->hamcast_modem_4_enable());
			progStatus.hamcast_mode_enable_4 = sp->hamcast_modem_4_enable();

			if(!sp->hamcast_modem_4().empty()) {
				cbo_hamcast_mode_selection_4->value(sp->hamcast_modem_4().c_str());
				progStatus.hamcast_mode_selection_4 = cbo_hamcast_mode_selection_4->index();
			}
			break;

	}

	return 0;
}

/** ********************************************************
 * \brief Enable/Disable Hamcast operation during event
 triggered operations.
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_hamcast(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	btn_hamcast_mode_cycle->value(sp->hamcast());
	progStatus.hamcast_mode_cycle = sp->hamcast();
	return 0;
}

/** ********************************************************
 * \brief Enable/Disable Header modem and select the header
 * modem used for transmitting.
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_header_modem(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	btn_enable_header_modem->value(sp->header_modem_enable());
	progStatus.use_header_modem = sp->header_modem_enable();

	if(!sp->header_modem().empty()) {
		cbo_header_modes->value(sp->header_modem().c_str());
		progStatus.header_selected_mode = cbo_header_modes->index();
	}

	return 0;
}

/** ********************************************************
 * \brief Number of times the header data is repeated.
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_header_repeat(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	int value = sp->hdr_repeat();
	int min = cnt_repeat_header->minimum();
	int max = cnt_repeat_header->maximum();

	if(value > max) value = max;
	if(value < min) value = min;

	cnt_repeat_header->value(value);
	progStatus.repeat_header = value;

	return 0;
}

/** ********************************************************
 * \brief Set the information field in the configuration panel
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_info(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	if(!sp->info().empty()) {
		txt_tx_myinfo->value(sp->info().c_str());
		progStatus.my_info.assign(sp->info());
	}

	return 0;
}

/** ********************************************************
 * \brief Enable/Diable Inhibit header modem on fills.
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_inhibit_header(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	btn_disable_header_modem_on_block_fills->value(sp->inhibit_header());
	progStatus.disable_header_modem_on_block_fills = sp->inhibit_header();

	return 0;
}

/** ********************************************************
 * \brief Enable/Disable Interval timer
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_interval(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	btn_enable_txrx_interval->value(sp->interval());
	progStatus.use_txrx_interval = sp->interval();

	return 0;
}

/** ********************************************************
 * \brief Flag event load queue to load from tx/ direcotry
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_load_txdir(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	btn_load_from_tx_folder->value(sp->load_txdir());
	progStatus.load_from_tx_folder = sp->load_txdir();

	return 0;
}

/** ********************************************************
 * \brief Select transmit modem
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_modem(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	if(!sp->modem().empty()) {
		cbo_modes->put_value(sp->modem().c_str());
		progStatus.selected_mode = cbo_modes->index();
		cbo_modes->do_callback();
	}

	return 0;
}

/** ********************************************************
 * \brief Set the filename and path of the event queue load script.
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_queue_filepath(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	if(sp->queue_filepath().empty()) return 0;

	txt_auto_load_queue_path->value(sp->queue_filepath().c_str());
	progStatus.auto_load_queue_path.assign(sp->queue_filepath());
	return 0;
}

/** ********************************************************
 * \brief Reset Configuration panel attributes
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_reset(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	int reset_type = sp->reset();

	switch(reset_type) {
		case RESET_ALL:

			sp->call_to("");
			txt_tx_mycall->value("");
			progStatus.my_call.clear();

			sp->info("");
			txt_tx_myinfo->value("");
			progStatus.my_info.clear();

		case RESET_PARTIAL:

			sp->interval(false);
			btn_enable_txrx_interval->value(false);
			progStatus.use_txrx_interval = false;

			sp->sync_with_flamp(false);
			btn_sync_mode_flamp_fldigi->value(false);
			progStatus.sync_mode_flamp_fldigi = false;

			sp->sync_with_fldigi(false);
			btn_sync_mode_fldigi_flamp->value(false);
			progStatus.sync_mode_fldigi_flamp = false;

			sp->sync_with_prior(false);
			btn_fldigi_xmt_mode_change->value(false);
			progStatus.fldigi_xmt_mode_change = false;

			sp->tx_report(false);
			btn_enable_tx_on_report->value(false);
			progStatus.use_txrx_interval = false;

			sp->warn_user(false);
			btn_enable_delete_warning->value(false);
			progStatus.use_tx_on_report = false;

			sp->clear_missing(false);
			btn_clear_tosend_on_tx_blocks->value(false);
			progStatus.clear_tosend_on_tx_blocks = false;

			sp->inhibit_header(false);
			btn_disable_header_modem_on_block_fills->value(false);
			progStatus.disable_header_modem_on_block_fills = false;

			sp->header_modem_enable(false);
			btn_enable_header_modem->value(false);
			progStatus.use_header_modem = false;

			sp->proto(true);
			btn_enable_tx_unproto->value(false);
			progStatus.enable_tx_unproto = false;

			sp->unproto_markers(false);
			btn_enable_unproto_markers->value(false);
			progStatus.enable_unproto_markers = false;

			sp->load_txdir(false);
			btn_load_from_tx_folder->value(false);
			progStatus.load_from_tx_folder = false;

			sp->event(false);
			btn_repeat_at_times->value(false);
			progStatus.repeat_at_times = false;

			sp->queue_filepath("");
			txt_auto_load_queue_path->value("");
			progStatus.auto_load_queue_path.clear();

			sp->auto_load_queue(false);
			btn_auto_load_queue->value(false);
			progStatus.auto_load_queue = false;

			sp->event_times("");
			txt_repeat_times->value("");
			progStatus.repeat_times.clear();

			sp->hamcast_modem_1_enable(false);
			btn_hamcast_mode_enable_1->value(false);
			progStatus.hamcast_mode_enable_1 = false;

			sp->hamcast_modem_2_enable(false);
			btn_hamcast_mode_enable_2->value(false);
			progStatus.hamcast_mode_enable_2 = false;

			sp->hamcast_modem_3_enable(false);
			btn_hamcast_mode_enable_3->value(false);
			progStatus.hamcast_mode_enable_3 = false;

			sp->hamcast_modem_4_enable(false);
			btn_hamcast_mode_enable_4->value(false);
			progStatus.hamcast_mode_enable_4 = false;

			sp->hamcast(false);
			btn_hamcast_mode_cycle->value(false);
			progStatus.hamcast_mode_cycle = false;

			sp->event_forever(false);
			btn_repeat_forever->value(false);
			btn_repeat_forever->do_callback(); // Requires callback()

			sp->event_timed(false);
			btn_repeat_at_times->value(false);
			btn_repeat_at_times->do_callback(); // Requires callback()

			do_events->value(false);
			do_events->do_callback(); // Requires callback()

		default:;
	}

	return 0;
}

/** ********************************************************
 * \brief Set the receive interval in seconds
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_rx_interval(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	int max   = cnt_rx_internval_secs->maximum();
	int min   = cnt_rx_internval_secs->minimum();
	int value = sp->rx_interval();

	if(value > max) value = max;
	if(value < min) value = min;

	cnt_rx_internval_secs->value(value);
	progStatus.rx_interval_seconds = value;

	return 0;
}

/** ********************************************************
 * \brief Set the sync modes between FLAMP and FLDIGI
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_sync_with(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	std::string str;

	str.assign(sc->args[0]);

	if(str.empty()) return 0;

	int i = 0;
	int c = (int) str.size();

	for(i = 0; i < c; i++)
		str[i] = toupper(str[i]);

	if(strncmp(str.c_str(), "FLDIGI", 6) == 0) {
		btn_sync_mode_flamp_fldigi->value(sp->sync_with_fldigi());
		progStatus.sync_mode_flamp_fldigi = sp->sync_with_fldigi();
		btn_sync_mode_flamp_fldigi->do_callback();
		return 0;
	}

	if(strncmp(str.c_str(), "FLAMP", 5) == 0) {
		btn_sync_mode_fldigi_flamp->value(sp->sync_with_flamp());
		progStatus.sync_mode_fldigi_flamp = sp->sync_with_flamp();
		btn_sync_mode_fldigi_flamp->do_callback();
		return 0;
	}

	if(strncmp(str.c_str(), "PRIOR", 5) == 0) {
		btn_fldigi_xmt_mode_change->value(sp->sync_with_prior());
		progStatus.fldigi_xmt_mode_change = sp->sync_with_prior();
		btn_fldigi_xmt_mode_change->do_callback();
		return 0;
	}

	return -1;
}

/** ********************************************************
 * \brief Set the transmit interval in minutes.
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_tx_interval(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	int max   = cnt_tx_internval_mins->maximum();
	int min   = cnt_tx_internval_mins->minimum();
	int value = sp->tx_interval();

	if(value > max) value = max;
	if(value < min) value = min;

	cnt_tx_internval_mins->value(value);
	progStatus.tx_interval_minutes = value;

	return 0;
}

/** ********************************************************
 * \brief Enable/Disable transmit on report
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_tx_report(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	btn_enable_tx_on_report->value(sp->tx_report());
	progStatus.use_tx_on_report = sp->tx_report();

	return 0;
}

/** ********************************************************
 * \brief Enable/Disable unproto start and end markers.
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_unproto_markers(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	btn_enable_unproto_markers->value(sp->unproto_markers());
	progStatus.enable_unproto_markers = sp->unproto_markers();

	return 0;
}

/** ********************************************************
 * \brief Enable/Disable dialog box warning when deleting
 * transmit queue items.
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_warn_user(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	btn_enable_delete_warning->value(sp->warn_user());
	progStatus.enable_delete_warning = sp->warn_user();

	return 0;
}

/** ********************************************************
 * \brief Set the number of time the non header data is sent.
 * \param sp Access to ScritpParsing members.
 * \param sc Access to SCRIPT_COMMANDS structure variables.
 * \return 0 (no error) Other (error)
 ***********************************************************/
static int process_xmit_repeat(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
{
	int max   = cnt_repeat_nbr->maximum();
	int min   = cnt_repeat_nbr->minimum();
	int value = sp->xmit_repeat();

	if(value > max) value = max;
	if(value < min) value = min;

	cnt_repeat_nbr->value(value);
	progStatus.repeatNN = value;


	return 0;
}

/** ********************************************************
 * \brief Call back function when executing a configuration script.
 * Called from the File->Execute Config Script menu item.
 * \param void
 ***********************************************************/
void script_execute(const char *filename, bool queue_flag)
{
	int count = 0;
	int index = 0;
	SCRIPT_CODES error = script_no_errors;
	ScriptParsing *sp = 0;
	static std::string script_filename = "";

	if(!filename) {
		LOG_INFO("Script file name (path) null pointer");
		return;
	}

	script_filename.assign(filename);

	if(script_filename.empty()) {
		LOG_INFO("Script file name (path) invalid");
		return;
	}

	sp = new ScriptParsing;

	if(!sp) {
		LOG_INFO("ScriptParsing Class Allocation Fail (%s)", script_filename.c_str());
		return;
	}

	index = 0;
	// All commands support callback functions.
	while(callback_functions[index].command) {
		sp->assign_callback(callback_functions[index].command, callback_functions[index].func);
		index++;
	}

	count = 0;
	while(s_modes[count]) {
		if(count > 100) break;
		count++;
	}

	// Assign "validation parameters" to the following functions.
	sp->assign_valid_parameters("MODEM",         (const char **) s_modes, count);
	sp->assign_valid_parameters("HEADER MODEM",  (const char **) s_modes, count);
	sp->assign_valid_parameters("HAMCAST MODEM", (const char **) s_modes, count);

	// Limit command set depending on queue_flag

	if(queue_flag) {
		sp->file_type(QUEUE_COMMAND);
	} else {
		sp->file_type(SCRIPT_COMMAND);
	}

	// LOG_INFO("Executing Script:%s", script_filename.c_str());

	error = sp->parse_commands((char *) script_filename.c_str());

	if(error != script_no_errors) {
		LOG_INFO("Error(s) in processing script file: %s", script_filename.c_str());
		fl_alert("%s", "Script File contains Error(s)\nSee Log file for details.");
	}

	if(sp)
		delete sp;
}

/** ********************************************************
 * \brief Call back function when executing a configuration script.
 * Called from the File->Execute Config Script menu item.
 * \param void
 ***********************************************************/
void cb_scripts(bool reset_path = false)
{
	pthread_mutex_lock(&mutex_script_io);

	static bool first_time = true;
	static char script_filename[FL_PATH_MAX + 1];
	std::string new_path = "";

	if(reset_path || first_time) {
		memset(script_filename, 0, sizeof(script_filename));
		strncpy(script_filename, flamp_script_dir.c_str(), FL_PATH_MAX);
		int len = strnlen(script_filename, FL_PATH_MAX);

		if(len > 0) {
			len--;
			if(script_filename[len] == PATH_CHAR_SEPERATOR);
			else strncat(script_filename, PATH_SEPERATOR, FL_PATH_MAX);
		} else {
			return;
		}

		first_time = false;
	}

	const char *p = FSEL::select((char *)"Script Files", (char *)"*.txt", \
		script_filename);

	if(p) {
		memset(script_filename, 0, sizeof(script_filename));
		strncpy(script_filename, p, FL_PATH_MAX);

		Fl::lock();
		script_execute(script_filename, false);
		Fl::unlock();

		show_selected_xmt(1);
	}

	pthread_mutex_unlock(&mutex_script_io);
}

/** ********************************************************
 * \brief Call back function when executing a queue script.
 * \param void
 ***********************************************************/
void cb_load_tx_queue(void)
{
	std::string script_filename;
	script_filename.assign(txt_auto_load_queue_path->value());

	if(script_filename.empty()) {
		LOG_INFO("Queue Load file list (path) not assigned");
		return;
	}

	if(progStatus.load_from_tx_folder) {
		auto_load_tx_queue_from_tx_directory();
	} else {
		pthread_mutex_lock(&mutex_script_io);
		strncpy(call_script.filename, script_filename.c_str(), FL_PATH_MAX-1);
		call_script.queueflag = true;
		Fl::awake(script_execute, (void *)&call_script);
		pthread_mutex_unlock(&mutex_script_io);
	}

	show_selected_xmt(0);
}


/** ********************************************************
 * \brief Hander for FL:awake call.
 * \param void
 ***********************************************************/
void script_execute(void *v) {
	CALL_SCRIPT *s = (CALL_SCRIPT *)v;
	if(!s) return;
	Fl::lock();
	script_execute(s->filename, s->queueflag);
	Fl::unlock();
}


