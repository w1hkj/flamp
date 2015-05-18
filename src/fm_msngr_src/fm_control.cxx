// ----------------------------------------------------------------------------
// fm_control.cxx
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


#include <cstring>
#include <string>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/time.h>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/filename.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_ask.H>
#include <FL/fl_utf8.h>

#include "fm_msngr_src/fm_dialog.h"
#include "fm_msngr_src/fm_control.h"
#include "fm_msngr_src/fm_config.h"
#include "fm_msngr_src/fm_control_tts.h"
#include "fm_msngr_src/call_table.h"
#include "fm_msngr_src/fm_ncs_config.h"

#include "status.h"
#include "icons.h"
#include "gettext.h"
#include "fm_msngr.h"
#include "file_io.h"
#include "xml_io.h"
#include "threads.h"
#include "debug.h"

#define FM_FEND         '|'
#define FM_FESC         '`'
#define FM_TFEND        '{'
#define FM_TFESC        '['

#define TX_BUFFER_SIZE      1024
#define BUFFER_PADDING      32
#define FM_BUFFER_FACTOR    2
#define FM_INVALID          512
#define CALLSIGN_MAX_LEN    64
#define MAX_FRAME_SIZE      (1+1+CALLSIGN_MAX_LEN+2+2+1)
#define MAX_FRAME_DATA_SIZE 255

pthread_t fm_thread_id;
pthread_mutex_t fm_mutex_frame      = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t fm_mutex_input      = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t fm_mutex_loop       = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t fm_mutex_tx_data    = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t fm_mutex_rx_data    = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t fm_mutex_trx_update = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t fm_mutex_search     = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  fm_cond  = PTHREAD_COND_INITIALIZER;

static std::string fm_frame_buffer;
static std::string fm_frame;
static std::string transmit_data;
static std::string rx_callsign;
static std::string rx_panel_string;
static std::string last_tx_message;
static std::string last_rx_callsign;

int  trx_state = RX_STATE;
int  trx_internal_state = RX_STATE;
int  trx_button_state = RX_STATE;

call_table *callTable = (call_table *)0;

bool fm_thread_running = false;
bool fm_data_available = false;
bool fm_window_open    = false;

static bool fm_thread_idle      = false;
static bool fm_thread_exit_flag = false;
static int  fm_data_count       = 0;
static bool canceling_tx_flag   = false;

static bool decode_fm_frame(std::string one_frame, int &data_count, \
							std::string &callsign);
static void parse_fm_frame(std::string &frame_segment);
static void encode_tx_buffer(std::string &tx_panel_data, bool flag);

void fm_insert_tx_data(void);
void fm_save_rx_data(void);
void fltk_flush(void *_data);
void strip_control_characters(char *raw_data, int &raw_length);
void fm_tx_panel_append(std::string _data);
void set_trx_state(int state);

extern bool translate_callsign(std::string _call, std::string &dummy);

/** *******************************************************************
 * \brief Return the last callsign heard.
 * \return std::string callsign
 **********************************************************************/
std::string last_call_sign_heard(void)
{
	return last_rx_callsign;
}

/** *******************************************************************
 * \brief Request to clean callsign pick list.
 * \return void
 **********************************************************************/
void clear_callsign_pick_list(void)
{
	if(fl_choice(_("Reset Receive list?"), _("No"), _("Yes"), NULL)) {
		Fl::lock();
		choice_frame_last_callsign->clear();
		choice_frame_last_callsign->redraw();
		Fl::unlock();
	}
}

/** *******************************************************************
 * \brief Create call table instance
 * \return void
 **********************************************************************/
void open_calltable(void)
{
	if(!callTable)
		callTable = new call_table;
}

/** **************************************************************
 * \brief Copy the selected contents of the transmit panel into
 * the paste bin.
 * \return void
 *****************************************************************/
void fm_copy_selected_tx(void)
{

}

/** **************************************************************
 * \brief Delete the selected contents of the transmit panel.
 * \return void
 *****************************************************************/
void fm_delete_selected_tx(void)
{

}

/** **************************************************************
 * \brief Copy the entire content of the transmit window into
 * the paste bin.
 * \return void
 *****************************************************************/
void fm_copy_all_tx(void)
{

}

/** **************************************************************
 * \brief Paste stored text in to the transmit window at the
 * current cursor position.
 * \return void
 *****************************************************************/
void fm_paste_to_tx(void)
{

}

/** **************************************************************
 * \brief Clear all text in the tranmit window.
 * \return void
 *****************************************************************/
void fm_clear_tx(void)
{

}

/** **************************************************************
 * \brief Copy the selected contents of the receive panel into
 * the paste bin.
 * \return void
 *****************************************************************/
void fm_copy_selected_rx(void)
{

}

/** **************************************************************
 * \brief Copy the contents of the receive panel into the paste
 * bin.
 * \return void
 *****************************************************************/
void fm_copy_all_rx(void)
{

}

/** **************************************************************
 * \brief Clear the receive window of all text
 * \return void
 *****************************************************************/
void fm_clear_rx(void)
{

}

/** **************************************************************
 * \brief Append the last transmitted message into the tx panel.
 * \return void
 *****************************************************************/
extern void paste_last_tx(void)
{
	fm_tx_panel_append(last_tx_message);
}

/** **************************************************************
 * \brief Append the last transmitted message into the tx panel.
 * \return void
 *****************************************************************/
void fm_tx_panel_append(std::string _data)
{
	if(_data.empty()) return;

	Fl_Text_Buffer *_tx_buffer = edit_frame_tx_panel->buffer();

	if(!_tx_buffer) {
		LOG_INFO("%s", _("Missing edit/display buffers"));
		return;
	}

	fm_append_tx(_data);
}

/** **************************************************************
 * \brief Paste a short log list to TX panel.
 * \return void
 *****************************************************************/
void fm_log_list_to_tx(void)
{
	std::string log_list;

}

/** **************************************************************
 * \brief Configure window attributes to previous state or
 * initialize if new.
 * \return void
 *****************************************************************/
void set_fm_window_defaults(void)
{
	int w = progStatus.fm_window_w;
	int h = progStatus.fm_window_w;

	if(w < FM_MIN_WINDOW_WIDTH)
		progStatus.fm_window_w = FM_MIN_WINDOW_WIDTH;

	if(h < FM_MIN_WINDOW_HEIGHT)
		progStatus.fm_window_h = FM_MIN_WINDOW_HEIGHT;
}

/** **************************************************************
 * \brief
 * \return void
 *****************************************************************/
void save_fm_window_defaults(void)
{
	progStatus.fm_window_x = window_frame->x_root();
	progStatus.fm_window_y = window_frame->y_root();
	progStatus.fm_window_w = window_frame->w();
	progStatus.fm_window_h = window_frame->h();
}

/** **************************************************************
 * \par UTF-8 Encoding Sequence
 *
 * 7    U+0000    U+007F     1	0xxxxxxx
 * 11   U+0080    U+07FF     2	110xxxxx 10xxxxxx
 * 16   U+0800    U+FFFF     3	1110xxxx 10xxxxxx 10xxxxxx
 * 21   U+10000   U+1FFFFF   4	11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 * 26   U+200000  U+3FFFFFF  5	111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
 * 31   U+4000000 U+7FFFFFFF 6	1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
 *                               Byte 1   Byte 2   Byte 3   Byte 4   Byte 5   Byte 6
 *
 * 0000=0 0001=1 0010=2 0011=3 0100=4 0101=5 0110=6 0111=7
 * 1000=8 1001=9 1010=A 1011=B 1100=C 1101=D 1110=E 1111=F
 *****************************************************************/

#define BYTE_MASK1 0x80
#define BYTE_VAL1  0x00

#define BYTE_MASK2 0xE0
#define BYTE_VAL2  0xC0

#define BYTE_MASK3 0xF0
#define BYTE_VAL3  0xE0

#define BYTE_MASK4 0xF8
#define BYTE_VAL4  0xF0

#define BYTE_MASK5 0xFC
#define BYTE_VAL5  0xF8

#define BYTE_MASK6 0xFE
#define BYTE_VAL6  0xE0

#define BYTE_DATA_MASK 0xC0
#define BYTE_DATA_VAL  0x80

/** **************************************************************
 * \brief Test for UTF-8 file content.
 * \return bool true/false
 *****************************************************************/
bool test_utf8(char *buf, size_t count)
{
	if(!buf || !count) return false;

	unsigned char *cur = (unsigned char *) buf;
	unsigned char *end = (unsigned char *) &buf[count];

	unsigned int utf8_count = 0;
	unsigned int results    = 0;

	// Check for UTF-8 BOM.
	if(count > 2) {
		if((cur[0] == 0xEF) && (cur[1] == 0xBB) && (cur[2] == 0xBF)) {
			cur += 3;
		}
	}

	while(cur < end) {

		if((*cur & BYTE_MASK1) == BYTE_VAL1)
			utf8_count = 0;
		else if((*cur & BYTE_MASK2) == BYTE_VAL2)
			utf8_count = 1;
		else if((*cur & BYTE_MASK3) == BYTE_VAL3)
			utf8_count = 2;
		else if((*cur & BYTE_MASK4) == BYTE_VAL4)
			utf8_count = 3;
		else if((*cur & BYTE_MASK5) == BYTE_VAL5)
			utf8_count = 4;
		else if((*cur & BYTE_MASK6) == BYTE_VAL6)
			utf8_count = 5;
		else
			return false;

		if(++cur >= end) {
			if(utf8_count == 0) return true;
			else return false;
		}

		switch(utf8_count) {
			case 0:
				continue;

			case 1:
				if((*cur++ & BYTE_DATA_MASK) != BYTE_DATA_VAL) return false;
				break;

			case 2:
				results |= (*cur++ & BYTE_DATA_MASK);
				if(cur >= end) return false;

				results |= (*cur++ & BYTE_DATA_MASK);

				results &= 0xFF;
				if(results != BYTE_DATA_VAL) return false;
				break;

			case 3:
				results |= (*cur++ & BYTE_DATA_MASK);
				if(cur >= end) return false;

				results |= (*cur++ & BYTE_DATA_MASK);
				if(cur >= end) return false;

				results |= (*cur++ & BYTE_DATA_MASK);
				results &= 0xFF;
				if(results != BYTE_DATA_VAL) return false;

				break;

			case 4:
				results |= (*cur++ & BYTE_DATA_MASK);
				if(cur >= end) return false;

				results |= (*cur++ & BYTE_DATA_MASK);
				if(cur >= end) return false;

				results |= (*cur++ & BYTE_DATA_MASK);
				if(cur >= end) return false;

				results |= (*cur++ & BYTE_DATA_MASK);
				results &= 0xFF;
				if(results != BYTE_DATA_VAL) return false;

				break;

			case 5:
				results |= (*cur++ & BYTE_DATA_MASK);
				if(cur >= end) return false;

				results |= (*cur++ & BYTE_DATA_MASK);
				if(cur >= end) return false;

				results |= (*cur++ & BYTE_DATA_MASK);
				if(cur >= end) return false;

				results |= (*cur++ & BYTE_DATA_MASK);
				if(cur >= end) return false;

				results |= (*cur++ & BYTE_DATA_MASK);
				results &= 0xFF;
				if(results != BYTE_DATA_VAL) return false;

				break;

			default:
				return false;
		}

		results    = 0;
		utf8_count = 0;
	}

	return true;
}

/** **************************************************************
 * \brief Test for 7 bit Ascii file content.
 * \return bool true/false
 *****************************************************************/
bool test_ascii(char *buf, size_t count)
{
#define _STRIDE 16
	if(!buf || !count) return false;

	unsigned char *end   = (unsigned char *) 0;
	unsigned char *cur   = (unsigned char *) 0;
	unsigned int rem_count = 0;

	cur = (unsigned char *) buf;
	end = (unsigned char *) &buf[count];

	if(count >= _STRIDE) {
		rem_count = count % _STRIDE;
		for(; cur < end; cur += _STRIDE) {
			if(*cur++ & BYTE_MASK1) { return false; }
			if(*cur++ & BYTE_MASK1) { return false; }
			if(*cur++ & BYTE_MASK1) { return false; }
			if(*cur++ & BYTE_MASK1) { return false; }
			if(*cur++ & BYTE_MASK1) { return false; }
			if(*cur++ & BYTE_MASK1) { return false; }
			if(*cur++ & BYTE_MASK1) { return false; }
			if(*cur++ & BYTE_MASK1) { return false; }
			if(*cur++ & BYTE_MASK1) { return false; }
			if(*cur++ & BYTE_MASK1) { return false; }
			if(*cur++ & BYTE_MASK1) { return false; }
			if(*cur++ & BYTE_MASK1) { return false; }
			if(*cur++ & BYTE_MASK1) { return false; }
			if(*cur++ & BYTE_MASK1) { return false; }
			if(*cur++ & BYTE_MASK1) { return false; }
			if(*cur++ & BYTE_MASK1) { return false; }
		}
	} else {
		rem_count = count;
	}

	for(int index = 0; index < rem_count; index++) {
		if(*cur++ & BYTE_MASK1) { return false; }
		if(cur >= end) break;
	}

	return true;
}

/** **************************************************************
 * \brief close fd and remove buf allocation
 * \return void
 *****************************************************************/
inline void i_m_free(FILE *fd, char *buf)
{
	if(fd) fclose(fd);
	if(buf) delete [] buf;
}

/** **************************************************************
 * \brief Load a file into the TX panel. Ensure content safe to
 * load.
 * \return void
 *****************************************************************/
void addTxfile(char *cFileName)
{
	if(!cFileName) return;

	FILE *fd = (FILE *)0;
	char *_buf = (char *)0;
	bool asciiflag = false;
	bool utf8flag  = false;
	int selection = 0;

	fd = fopen(cFileName, "rb");

	if(!fd) {
		const char *msgfmt = _("File %s not found");
		LOG_INFO(msgfmt, cFileName);
		return;
	}

	fseek(fd, 0, SEEK_END);
	size_t count = (size_t) ftell(fd);

	if(count < 1) {
		LOG_INFO("%s %s", _("File contains no data:"), cFileName);
		i_m_free(fd, _buf);
		return;
	}

	_buf = new char [count+1];

	if(!_buf) {
		LOG_INFO("%s %s", _("Memory allocation error:"), cFileName);
		i_m_free(fd, _buf);
		return;
	}

	memset(_buf, 0, count + 1);

	fseek(fd, 0, SEEK_SET);
	size_t read_count = fread((void *) _buf, sizeof(char), count, fd);

	if(read_count != count) {
		LOG_INFO("%s %s", _("File read error:"), cFileName);
	}


	asciiflag = test_ascii(_buf, read_count);
	if(asciiflag == false)
		utf8flag = test_utf8(_buf, read_count);

	if(!asciiflag && !utf8flag) {
		selection = fl_choice(_("Can not load binary files, Sorry."), \
							  _("Ack"),  (char *)0, (char *)0);
		i_m_free(fd, _buf);
		return;
	}

#ifdef FLDIGI_COMP
	mode = active_modem->get_mode();

	if(((mode_info[mode].iface_io & KISS_IO) == 0) && (utf8flag == true)) {
		selection = fl_choice(_("Modem selected does not support UTF-8 Transfers.\nContinue loading?"), \
							  _("Yes"), _("No"), (char *)0);
		if(selection != 0) {
			i_m_free(fd, _buf);
			return;
		}
	}
#endif

	fm_append_tx(_buf);

	i_m_free(fd, _buf);
}


/** **************************************************************
 * \brief Search text event data for file names.
 * \return void
 *****************************************************************/
void fm_dnd_file(void)
{
	std::string buffer = Fl::event_text();
	size_t n = 0;
	size_t limit = 0;
	int valid = 0;

	char *cPtr = (char *)0;
	char *cFileName = (char *)0;
	const char *cBufferEnd = (char *)0;

	drop_file->value(FM_LABLE_DND);
	drop_file->redraw();

	while(1) { // Remove URL file marker
		if ((n = buffer.find("file:///")) != std::string::npos) {
			buffer.erase(n, 7);
		} else {
			break;
		}
	}

	n = buffer.find(":\\");
	if(n == std::string::npos)
		n = buffer.find("/");

	if(n != std::string::npos) {
		valid = 1;
		limit = buffer.size();
		cBufferEnd = &((buffer.c_str())[limit]);
		cFileName = cPtr = (char *) buffer.c_str();

		while(cPtr < cBufferEnd) {

			// Skip though the current filename path to find delimters
			while(*cPtr >= ' ' && cPtr < cBufferEnd)
				cPtr++;

			// Null terminate CR/LF delimiters
			while(*cPtr == '\n' || *cPtr == '\r') {
				*cPtr = 0;
				cPtr++;
				if(cPtr >= cBufferEnd) break;
			}

			if(valid)
				addTxfile(cFileName);

			// Remove any leading spaces and control characters
			while(*cPtr <= ' ' && cPtr < cBufferEnd)
				cPtr++;

			// Reassign a possible new filename path.
			cFileName = cPtr;
			valid = 0;

			// Minimal validation check on system specific file name path sturcture
			while(cPtr < cBufferEnd) {
				if(*cPtr == '/') { // Unix/BSD type file systems
					valid = 1;
					break;
				}

				if(cPtr[0] == ':' && cPtr[1] == '\\') { // Windows file system
					valid = 1;
					break;
				}
				cPtr++;
			}
		}
	}
}

/** **************************************************************
 * \brief
 * \return void
 *****************************************************************/
void fm_insert_tx_data(void)
{

}

/** **************************************************************
 * \brief
 * \return void
 *****************************************************************/
void fm_save_rx_data()
{

}


/** **************************************************************
 * \brief Send data to the recevie panel using the main thread.
 * \param data std::string data to append to recevie buffer.
 * \return void
 *****************************************************************/
void update_rx(void *_data)
{
	if(!_data) return;

	guard_lock fm_search_lock(&fm_mutex_search);

	Fl::lock();

	Fl_Text_Buffer *_rx_buffer = display_frame_rx_panel->buffer();
	int prev_length = _rx_buffer->length();
	double prev_max   = display_frame_rx_panel->V_Scroll_Max();
	double prev_value = display_frame_rx_panel->V_Scroll_Value();
	std::string _str_data;

	_rx_buffer->append((const char *) _data);


	if(progStatus.festival_enabled) {
		_str_data.assign((const char *)_data);
		festival_speak_buffer(_str_data);
	}


	if((prev_length < 1) || (prev_value >= (prev_max - 1))) {
		double _max = display_frame_rx_panel->V_Scroll_Max();
		display_frame_rx_panel->V_Scroll_Value(_max);
	}
	else {
		display_frame_rx_panel->V_Scroll_Value(prev_value);
	}

	Fl::unlock();
	//Fl::flush();

	free(_data);
}

/** **************************************************************
 * \brief Append data to the recevie buffer.
 * \param data std::string data to append to recevie buffer.
 * \return void
 *****************************************************************/
void fm_append_rx(std::string data)
{
	guard_lock fm_rx_update(&fm_mutex_trx_update);

	char *_data = (char *)0;
	int _len = data.length();

	_data = (char *) malloc(_len + 1);

	if(!_data) return;
	memset(_data, 0, _len + 1);
	memcpy(_data, data.c_str(), _len);
	Fl::awake(update_rx, _data);
}

/** **************************************************************
 * \brief Append data to the recevie buffer.
 * \param data char * data to append to recevie buffer.
 * \return void
 *****************************************************************/
void fm_append_rx(char *data, int count)
{
	guard_lock fm_rx_update(&fm_mutex_trx_update);

	char *_data = 0;

	if(!data) return;

	_data = (char *) malloc(count + 1);
	if(!_data) return;

	memset(_data, 0, count + 1);
	memcpy(_data, data, count);

	Fl::awake(update_rx, _data);
}

/** **************************************************************
 * \brief Append data to the transmit buffer.
 * \param data std::string data to append to recevie buffer.
 * \return void
 *****************************************************************/
void fm_append_tx(std::string data)
{
	guard_lock fm_tx_update(&fm_mutex_trx_update);
	Fl::lock();
	Fl_Text_Buffer *_tx_buffer = edit_frame_tx_panel->buffer();
	_tx_buffer->append((const char *) data.c_str());
	Fl::unlock();
	edit_frame_tx_panel->redraw();
}

/** **************************************************************
 * \brief Append data to the transmit buffer.
 * \param data char * data to append to recevie buffer.
 * \return void
 *****************************************************************/
void fm_append_tx(char *data)
{
	guard_lock fm_tx_update(&fm_mutex_trx_update);
	Fl::lock();
	Fl_Text_Buffer *_tx_buffer = edit_frame_tx_panel->buffer();
	_tx_buffer->append((const char *) data);
	Fl::unlock();
	edit_frame_tx_panel->redraw();
}


/** **************************************************************
 * \brief Reset global variables
 * \return void
 *****************************************************************/
static void fm_reset(void)
{
	fm_frame_buffer.clear();
	fm_frame.clear();
	transmit_data.clear();
	rx_callsign.clear();
	rx_panel_string.clear();

	fm_thread_running   = false;
	fm_data_available   = false;
	fm_window_open      = false;
	fm_thread_idle      = false;
	fm_thread_exit_flag = false;

	fm_data_count = 0;
}

/** **************************************************************
 * \brief Move the reveive panel data to the selected callsign.
 * \return void
 *****************************************************************/
void move_to_callsign(std::string _call)
{
	guard_lock fm_search_lock(&fm_mutex_search);

	void *hndl = (void *) callTable->handle();
	if(!hndl) return;

	if(!callTable->find_call(hndl, _call)) return;

	Fl_Text_Buffer *_rx_buffer = display_frame_rx_panel->buffer();
	int pos = _rx_buffer->length();
	int found = 0;
	size_t loc = 0;
	std::string search_string;

	bool valid = callTable->mstr(hndl, search_string);

	callTable->delete_handle(hndl);

	if(!valid) return;

	while(1) {
		loc = search_string.find("\n");
		if(loc != std::string::npos) {
			search_string.erase(loc, 1);
		} else {
			break;
		}
	}

	if(pos > 0)
		found = text_rx_buffer->search_forward(0, search_string.c_str(), &pos, 0);

	if(found) {
		int ln = text_rx_buffer->count_lines(0, pos);
		display_frame_rx_panel->scroll(ln, 0);
		text_rx_buffer->select(pos, pos + search_string.length());
		display_frame_rx_panel->insert_position(pos + search_string.length());
		display_frame_rx_panel->show_insert_position();
	}
	Fl::flush();
}

typedef struct _call_index {
	std::string call;
	std::string match;
	time_t time;
	_call_index() {
		call.clear();
		match.clear();
		time = 0;
	};
} _CALL_INDEX;

/** **************************************************************
 * \brief Update Callsign index information
 * \return void
 *****************************************************************/
void update_callsign_index_main(void *ptr)
{
	if(!ptr) return;

	guard_lock fm_rx_update(&fm_mutex_search);

	_CALL_INDEX *ci = (_CALL_INDEX *) ptr;

	if(callTable) {
		callTable->add(ci->call, ci->match, ci->time);
		update_ncs_list();
	}

	int count = choice_frame_last_callsign->size();
	int index = 0;
	std::string _temp  = "";
	std::string _match = "";
	std::string _selected = "";

	if(count < 1) {
		choice_frame_last_callsign->add(ci->call.c_str());
		choice_frame_last_callsign->value(0);
	} else {

		_selected.assign(choice_frame_last_callsign->text());
		_temp.append(ci->call);

		for(index = 0; index < (count - 1); index++) {
			_match.assign(choice_frame_last_callsign->text(index));
			if(_match == ci->call) continue;
			if(_match == FM_CLEAR_LIST) continue;
			_temp.append("|").append(_match);
		}

		_temp.append("|").append(FM_CLEAR_LIST);

		choice_frame_last_callsign->clear();
		choice_frame_last_callsign->add(_temp.c_str());
		choice_frame_last_callsign->redraw();
		choice_frame_last_callsign->value(choice_frame_last_callsign->find_index(_selected.c_str()));
	}

	Fl::flush();
	delete ci;
}

/** **************************************************************
 * \brief Update Callsign index information
 * \return void
 *****************************************************************/
void update_callsign_index(std::string _call, std::string _match_str, time_t _time)
{
	// guard_lock fm_rx_update(&fm_mutex_trx_update);

	_CALL_INDEX *_ci = new _CALL_INDEX;
	if(!_ci) return;

	_ci->call.assign(_call);
	_ci->match.assign(_match_str);
	_ci->time = _time;

	Fl::awake(update_callsign_index_main, _ci);
}
/** **************************************************************
 * \brief Set receive text field maker.
 * \return void
 *****************************************************************/
std::string time_stamp(std::string _trx, time_t &t)
{
	char time_buffer[128];
	struct tm *tm_time;
	std::string time_stamp;
	char *utc = (char *)0;

	static char *month[12] = {
		(char *) _("January"),   (char *) _("Febuary"), (char *) _("March"),    (char *) _("April"),
		(char *) _("May"),       (char *) _("June"),    (char *) _("July"),     (char *) _("August"),
		(char *) _("September"), (char *) _("October"), (char *) _("November"), (char *) _("December")
	};

	static char *day[7] = {
		(char *) _("Sunday"),    (char *) _("Monday"),    (char *) _("Tuesday"),
		(char *) _("Wednesday"), (char *) _("Thursday"),  (char *) _("Friday"),
		(char *) _("Saturday"),
	};

	time(&t);

	if(progStatus.fm_utc_timestamp) {
		tm_time = gmtime(&t);
		utc = (char *)"UTC";
	} else {
		utc = (char *)"";
		tm_time = localtime(&t);
	}

	memset(time_buffer, 0, sizeof(time_buffer));
	snprintf(time_buffer, sizeof(time_buffer)-1, "%s %02d %s %04d %02d:%02d:%02d %s\n",
			 day[tm_time->tm_wday], tm_time->tm_mday, month[tm_time->tm_mon],
			 tm_time->tm_year + 1900, tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec, utc);

	if(progStatus.festival_enabled) {
		if(_trx == SET_TX)
			time_stamp.append(_("Tramsmitted by "));
		else if(_trx == SET_RX)
			time_stamp.append(_("Received from "));
		else if(_trx == SET_NCS_LOGIN)
			time_stamp.append(_("NCS added check in"));
		else if(_trx == SET_LOGIN)
			time_stamp.append(_("Received login from "));
		else
			time_stamp.append(_("Unassigned Login "));
	} else {
		time_stamp.assign("[ ").append(_trx).append(" ] ");
	}

	time_stamp.append(time_buffer);

	return time_stamp;
}

/** **************************************************************
 * \brief Set receive text field maker.
 * \return void
 *****************************************************************/
void set_marker(std::string _trx, std::string _call, std::string _last)
{
	guard_lock fm_search_lock(&fm_mutex_search);

	std::string dummy = "";
	bool valid_call = translate_callsign(_call, dummy);
	if(!valid_call) return;

	if(_call != _last) {
		std::string _marker = "";
		static std::string _divider = "--------------------------------------------------------------------------\n";
		std::string _stamp = "";
		time_t t_time;
		std::string time_buffer;

		_stamp.clear();

		_stamp.append(_call);
		_stamp.append(" ").append(time_stamp(_trx, t_time));

		if(progStatus.festival_enabled) {
			_marker.assign("\n\n").append(_stamp).append("\n\n");
		} else {
			_marker.assign("\n\n").append(_divider).append(_stamp).append(_divider).append("\n\n");
		}

		fm_append_rx(_marker);

		last_rx_callsign.assign(_call);

		update_callsign_index(_call, _stamp, t_time);
	}
}

typedef struct _rx_out {
	size_t count;
	char *data;
} RX_OUT;

/** **************************************************************
 * \brief Output character to the recevive panel. Must be called
 * from the main thread. REQ(output_rx_string, data, count)
 * \param data pointer to an allocated byte array. (new char[])
 * \param count number of bytes in the array.
 * \return void
 *****************************************************************/
void output_rx_string(void *ptr)
{
	if(!ptr) return;
	RX_OUT *rx_data = (RX_OUT *)ptr;

	if(rx_data->count > MAX_FRAME_DATA_SIZE) {
		if(rx_data->data) free(rx_data->data);
		free(rx_data);
		return;
	}

	if(rx_data->data && (rx_data->count < 1)) {
		free(rx_data->data);
		free(rx_data);
		return;
	}

	fm_append_rx(rx_data->data);

	free(rx_data->data);
	free(rx_data);
}

/** **************************************************************
 * \brief Output character to the recevive panel.
 * \return void
 *****************************************************************/
void output_rx(int ch)
{
	guard_lock fm_rx_lock(&fm_mutex_rx_data);
	rx_panel_string += (ch & 0xff);
}

/** **************************************************************
 * \brief Output character to the recevive panel.
 * \return void
 *****************************************************************/
void output_rx(const char *str, size_t count)
{
	guard_lock fm_rx_lock(&fm_mutex_rx_data);
	std::string temp;
	temp.assign(str, count);
	rx_panel_string.append(temp);
}

/** **************************************************************
 * \brief Output buffered string to the text window. Must be called
 * from the main thread. REQ(out_rx_buffered_string)
 * \return void
 *****************************************************************/
void out_rx_buffered_string(std::string &_frame_data, size_t _frame_count)
{
	//guard_lock fm_rx_lock(&fm_mutex_rx_data);

	if(_frame_data.empty()) return;

	const char *data = _frame_data.c_str();
	size_t count = _frame_data.size();

	if(!data || (_frame_count > count))
		return;

	char *buf = (char *) malloc(_frame_count + 1);
	if(!buf) return;

	// Make sure the data is null terminated.
	memcpy(buf, data, _frame_count);
	buf[_frame_count] = 0;

	RX_OUT *_strc = (RX_OUT *) malloc(sizeof(RX_OUT));
	if(!_strc) {
		free(buf);
		return;
	}

	_strc->count = _frame_count;
	_strc->data  = buf;

	Fl::awake(output_rx_string, _strc);
}

/** **************************************************************
 * \brief Frame encoder
 * \param src Pointer to data source.
 * \param src_size Number of bytes in the src to process
 * \param dst Pointer to store converted data. Must call delete[]
 * after use.
 * \return size_t count in the dst buffer.
 *****************************************************************/
static size_t fm_encode(char *src, size_t src_size, char **dst)
{
	if(!src || !dst || src_size < 1) return 0;

	size_t index = 0;
	int count = 0;
	int buffer_size = 0;
	int byte = 0;
	char *buffer = (char *)0;

	buffer_size = (src_size * FM_BUFFER_FACTOR) + BUFFER_PADDING;
	buffer = new char[buffer_size];

	if(!buffer) {
		LOG_DEBUG("Memory allocation error near line %d", __LINE__);
		*dst = (char *)0;
		return 0;
	}

	memset(buffer, 0, buffer_size);
	count = 0;
	buffer[count++] = FM_FEND;

	for(index = 0; index < src_size; index++) {
		byte = (int) src[index] & 0xFF;
		switch(byte) {
			case FM_FESC:
				buffer[count++] = FM_FESC;
				buffer[count++] = FM_TFESC;
				break;

			case FM_FEND:
				buffer[count++] = FM_FESC;
				buffer[count++] = FM_TFEND;
				break;

			default:
				buffer[count++] = byte;
		}
	}

	buffer[count++] = FM_FEND;
	*dst = (char *) buffer;

	return count;
}

/** **************************************************************
 * \brief Frame decoder
 * \param src Pointer to data source.
 * \param src_size Number of bytes in the src to process
 * \param dst Pointer to store converted data. Must call delete[]
 * after use.
 * \return size_t count in the dst buffer.
 *****************************************************************/
static size_t fm_decode(char *src, size_t src_size, char **dst)
{
	if(!src || !dst || src_size < 1) return 0;

	size_t index = 0;
	int count = 0;
	int buffer_size = 0;
	int byte = 0;
	int last_byte = 0;
	char *buffer = (char *)0;

	buffer_size = src_size + FM_BUFFER_FACTOR;
	buffer = new char[buffer_size];

	if(!buffer) {
		LOG_DEBUG("Memory allocation error near line %d", __LINE__);
		*dst = (char *)0;
		return 0;
	}

	memset(buffer, 0, buffer_size);
	count = 0;
	last_byte = FM_INVALID;

	for(index = 0; index < src_size; index++) {

		byte = src[index] & 0xFF;

		switch(byte) {
			case FM_FEND:
				continue;

			case FM_FESC:
				break;

			case FM_TFEND:
				if(last_byte == FM_FESC)
					buffer[count++] = FM_FEND;
				else
					buffer[count++] = byte;
				break;

			case FM_TFESC:
				if(last_byte == FM_FESC)
					buffer[count++] = FM_FESC;
				else
					buffer[count++] = byte;
				break;

			default:
				buffer[count++] = byte;
		}

		last_byte = byte;
	}

	*dst = (char *) buffer;
	return count;
}


/** **************************************************************
 * \brief encode tx data for transmit.
 * \param flag true = allow empty data frame
 * \return void
 * \par Frame Format
 * FRAME_MARKER, CALLSIGN_LENGTH, CALLSIGN, DATA_LENGTH, XOR_CHECKSUM,
 * FRAME_MARKER, DATA
 * Example with 16 byte data size:
 * "~%w1hkj100b~NBEMS net operat"
 * Packet header usable with any 7/8 bit modems.
 *****************************************************************/
static void encode_tx_buffer(std::string &tx_panel_data, bool flag = false)
{
	int size = tx_panel_data.size();
	int frame_size = 0;
	size_t copy_count = 0;
	int xor_crc = 0;
	int i = 0;
	int h = 0;
	int data = 0;
	int erase_size = 0;
	char *tx_buffer = new char[TX_BUFFER_SIZE];
	char *tx_frame = (char *) 0;
	char _callsign[CALLSIGN_MAX_LEN+1];
	char tmp[16];

	if(!tx_buffer) {
		LOG_INFO("%s", _("tx_buffer memory allocation error"));
		return;
	}

	memset(_callsign, 0, sizeof(_callsign));
	strncpy(_callsign, progStatus.fm_inp_callsign.c_str(), CALLSIGN_MAX_LEN);

	// Lower case characters require less tx time.
	for(int i = 0; i < sizeof(_callsign); i++) {
		if(_callsign[i]) _callsign[i] = tolower(_callsign[i]);
		else break;
	}

	size = tx_panel_data.size();
	sscanf((const char *) choice_frame_packet_size->text(), "%d", &frame_size);

	while(size > 0 || flag) {
		xor_crc = 0;
		memset(tx_buffer, 0, TX_BUFFER_SIZE);
		i = 0;

		if(size >= frame_size)
			copy_count = frame_size;
		else
			copy_count = size;

		erase_size = copy_count;

		h = strnlen(_callsign, CALLSIGN_MAX_LEN);
		data = (h + (int) ' ') & 0xff;
		xor_crc ^= data;
		tx_buffer[i++] = data;

		for(int j = 0; j < h; j++) {
			data = _callsign[j];
			if(data) {
				xor_crc ^= data;
				tx_buffer[i++] = data;
			} else break;
		}

		memset(tmp, 0, sizeof(tmp));
		snprintf(tmp, 3, "%02x", (unsigned int) (copy_count & 0xff));

		data = tmp[0];
		xor_crc ^= data;
		tx_buffer[i++] = data;

		data = tmp[1];
		xor_crc ^= data;
		tx_buffer[i++] = data;

		memset(tmp, 0, sizeof(tmp));
		snprintf(tmp, 3, "%02x", xor_crc & 0xff);

		tx_buffer[i++] = tmp[0];
		tx_buffer[i++] = tmp[1];

		h = fm_encode(tx_buffer, (size_t) i, &tx_frame);

		if(h && tx_frame) {
			for(i = 0; i < h; i++) {
				if(tx_frame[i]) {
					tx_buffer[i] = tx_frame[i];
				} else break;
			}

			delete [] tx_frame;

			for(size_t j = 0; j < copy_count; j++) {
				if(tx_panel_data[j]) {
					tx_buffer[i++] = tx_panel_data[j];
				} else break;
			}

			{
				guard_lock fm_tx_lock(&fm_mutex_tx_data);
				transmit_data.append(tx_buffer, i);
			}
		}

		if(erase_size)
			tx_panel_data.erase(0, erase_size);
		size = tx_panel_data.size();

		if(size < 1 && flag) break;
	}

	if(!transmit_data.empty()) {
		{
			guard_lock fm_tx_lock(&fm_mutex_tx_data);
			send_via_fldigi(transmit_data);
			transmit_data.clear();
		}
	}

	delete [] tx_buffer;
}

/** **************************************************************
 * \brief Remove non printable characters from the message.
 * \param raw_data the source material.
 * \param raw_length number of bytes to process.
 * \return void
 *****************************************************************/
void strip_control_characters(char *raw_data, int &raw_length)
{
	if(!raw_data || raw_length < 1)
		return;

	int count = 0;
	int ch = 0;
	char last_ch = 0;
	char *assign_pos = raw_data;
	char *read_pos = raw_data;
	char *last_char = &raw_data[raw_length];

	while(read_pos < last_char) {
		ch = *read_pos++;
		ch &= 0xff;

		if(ch < ' ') {
			switch(ch) {
				case '\n':
					if(last_ch == '\r') {
						last_ch = ch;
						continue;
					}
					break;
				case '\t': // Tab
				case '\b': // Back space
					break;

				default:
					last_ch = ch;
					continue;
			}
		}

		if(ch == '\r')
			*assign_pos++ = '\n';
		else
			*assign_pos++ = ch;

		last_ch = ch;
		count++;

		if(count > raw_length) {
			count = raw_length;
			break;
		}
	}

	raw_length = count;
}

/** **************************************************************
 * \brief Append a period to each transmission. This is used to
 * help the Text to Speech processor determine the end of sentence
 * or in the case of ham operators the whole paragraph.
 * \param raw_data charater buffer.
 * \para
 * \return void
 *****************************************************************/
std::string append_period(char *raw_data, int raw_length)
{
	if(!raw_data || (raw_length < 1))
		return "";

	std::string tmpbuf = "";
	int count = raw_length;
	bool dot_flag = false;

	tmpbuf.assign(raw_data, raw_length);

	// Append a period only if we need to.
	while(count > 0) {
		if(tmpbuf[count] == '.') {
			dot_flag = true;
			break;
		}

		if(tmpbuf[count] > ' ')
			break;

		count--;
	}

	if((count > 0) && !dot_flag)
		tmpbuf.append(".");

	return tmpbuf;
}

/** **************************************************************
 * \brief Clear transmit buffers.
 * \param none
 * \return void
 *****************************************************************/
void clear_tx_buffer(void)
{
	{
		guard_lock fm_tx_lock(&fm_mutex_tx_data);
		transmit_data.clear();
	}

	{
		guard_lock fm_input_lock(&fm_mutex_input);
		fm_frame_buffer.clear();
	}
}

/** **************************************************************
 * \brief GUI interface call - Format and Transmit Data
 * \param none
 * \return void
 *****************************************************************/
void fm_transmit(void)
{
	if((trx_internal_state == TX_STATE || trx_state == TX_STATE))
		return;

	trx_internal_state = TX_STATE;

	Fl_Text_Buffer *_tx_buffer = edit_frame_tx_panel->buffer();
	Fl_Text_Buffer *_rx_buffer = display_frame_rx_panel->buffer();

	if(!_rx_buffer || !_tx_buffer) {
		LOG_INFO("%s", _("Missing edit/display buffers"));
		return;
	}

	std::string _tx_panel_data;
	std::string _tmp;
	char *raw_data = _tx_buffer->text();
	int raw_length = _tx_buffer->length();

	strip_control_characters(raw_data, raw_length);

	set_trx_state(trx_internal_state);

	_tmp.assign(progStatus.fm_inp_callsign);

	int size = _tmp.size() * 4;
	char *_utf8_buf = new char[size];
	if(!_utf8_buf) return;

	int prev_length   = _rx_buffer->length();
	double prev_max   = display_frame_rx_panel->V_Scroll_Max();
	double prev_value = display_frame_rx_panel->V_Scroll_Value();

	std::string rd = append_period(raw_data, raw_length);

	if(raw_data && raw_length > 1) {
		memset(_utf8_buf, 0, size);
		fl_utf_toupper((const unsigned char *)_tmp.c_str(), size, _utf8_buf);
		_tmp.assign(_utf8_buf);
		set_marker(SET_TX, _tmp, last_rx_callsign);
		fm_append_rx(rd);
	}

	if((prev_length < 1) || (prev_value >= prev_max)) {
		double _max = display_frame_rx_panel->V_Scroll_Max();
		display_frame_rx_panel->V_Scroll_Value(_max);
	} else {
		display_frame_rx_panel->V_Scroll_Value(prev_value);
	}

	// For the TTS.

	_tx_panel_data.assign(rd);
	last_tx_message.assign(_tx_panel_data);

	encode_tx_buffer(_tx_panel_data, true);

	free(raw_data);
	delete [] _utf8_buf;

	trx_internal_state = RX_STATE;
}

/** **************************************************************
 * \brief Frame decoding processor
 * \param one_frame One frame of data.
 * \param data_count number of data bytes to receive outside of frame.
 * \param callsign extracted callsign from the frame.
 * \return true passes checksum, false fails checksum.
 * \par Frame Format
 * FRAME_MARKER, CALLSIGN_LENGTH, CALLSIGN, DATA_LENGTH, XOR_CHECKSUM,
 * FRAME_MARKER, DATA
 * Example with 16 byte data size:
 * "~%w1hkj100b~NBEMS net operat"
 * Packet header usable with any 7/8 bit modems.
 *****************************************************************/
static bool decode_fm_frame(std::string one_frame, int &_data_count, \
							std::string &callsign_)
{
	int xor_cksum = 0;
	int frame_xor_cksum = 0;
	int data_length = 0;
	int callsign_length = 0;
	int frame_size = 0;
	int i = 0;
	char buf[8];
	std::string _callsign;

	if(one_frame.empty()) return false;

	frame_size = one_frame.size();

	if(one_frame[i] == FM_FEND) i++;
	if(i > frame_size) return false;

	callsign_length = one_frame[i] - (int) ' ';
	xor_cksum ^= one_frame[i++];
	if(i > frame_size) return false;

	_callsign.clear();
	if(callsign_length >= 0 && callsign_length < CALLSIGN_MAX_LEN) {
		for(int j = 0; j < callsign_length; j++) {
			xor_cksum ^= tolower(one_frame[i]); // Contestia uses upper case only. Must be lower case for chksum.
			_callsign += toupper(one_frame[i++]);
			if(i > frame_size) return false;
		}
	} else return false;

	xor_cksum ^= one_frame[i];
	buf[0] = toupper(one_frame[i++]);
	if(i > frame_size) return false;

	xor_cksum ^= one_frame[i];
	buf[1] = toupper(one_frame[i++]);
	if(i > frame_size) return false;

	buf[2] = 0;

	if (isxdigit(buf[0]) && isxdigit(buf[1]))
		sscanf(buf, "%X", &data_length);
	else
		return false;

	data_length &= 0xFF;

	buf[0] = toupper(one_frame[i++]);
	if(i > frame_size) return false;

	buf[1] = toupper(one_frame[i++]);
	if(i > frame_size) return false;

	buf[2] = 0;

	if (isxdigit(buf[0]) && isxdigit(buf[1]))
		sscanf(buf, "%X", &frame_xor_cksum);
	else
		return false;

	frame_xor_cksum &= 0xff;
	xor_cksum &= 0xff;

	if(frame_xor_cksum != xor_cksum)
		return false;

	callsign_.assign(_callsign);
	_data_count = (data_length & 0xFF);

	return true;
}

#if 0
/** **************************************************************
 * \brief Extern interface - Return a character for transmit.
 * \return int character.
 *****************************************************************/
void fm_flush_tx_buffer(void)
{
	guard_lock fm_tx_lock(&fm_mutex_tx_data);
	fm_data_available = false;
	transmit_data.clear();
}
#endif

/** **************************************************************
 * \brief Extern interface - Return a character for transmit.
 * \return int character.
 *****************************************************************/
int fm_getchar(void)
{
	guard_lock fm_tx_lock(&fm_mutex_tx_data);

	int size = transmit_data.size();
	int ch = ' ';

	if(size < 1) {
		fm_data_available = false;
		//idling = true;
		//active_modem->set_stopflag(true);
	} else {
		ch = (transmit_data[0] & 0xff);
		transmit_data.erase(0, 1);

		if(ch < ' ') { // Restrict control character types
			switch(ch) {
				case '\r': // Carrage return
				case '\n': // New Line
				case '\t': // Tab
				case '\b': // Back space
					break;
				default:
					ch = ' ';
			}
		}
	}

	return ch;
}

/** **************************************************************
 * \brief Extern interface - buffer a string of characters. All
 * characters lost if thread not running.
 * \return void
 *****************************************************************/
void fm_putstring(std::string &string)
{
	if(!fm_thread_running) return;

	guard_lock fm_input_lock(&fm_mutex_input);
	fm_frame_buffer.append(string);

	if(!fm_frame_buffer.empty())
		pthread_cond_signal(&fm_cond);
}

/** **************************************************************
 * \brief Extern interface - buffer a single character. All
 * characters lost if thread not running.
 * \return void
 *****************************************************************/
void fm_putchar(int ch)
{
	if(!fm_thread_running) return;

	guard_lock fm_input_lock(&fm_mutex_input);
	fm_frame_buffer += (ch & 0xff);

	if(fm_thread_idle)
		if((fm_frame_buffer.size() & 0x7) == 0)
			pthread_cond_signal(&fm_cond);
}

/** **************************************************************
 * \brief Proccess the input buffer into frames.
 * character or every two second to process the input buffer.
 * \return void
 *****************************************************************/
void process_input_buffer(std::string _data)
{
	std::string temp = "";
	{
		guard_lock fm_input_lock(&fm_mutex_input);
		temp.append(_data);
	}

	if(temp.size() > 0) {
		parse_fm_frame(_data);
		temp.clear();
	}
}

/** **************************************************************
 * \brief Proccess the input buffer into frames.
 * character or every two second to process the input buffer.
 * \return void
 *****************************************************************/
static void process_input_buffer(void)
{
	static std::string temp = "";

	{
		guard_lock fm_input_lock(&fm_mutex_input);
		temp.append(fm_frame_buffer);
		fm_frame_buffer.clear();
	}

	if(temp.size() > 0) {
		parse_fm_frame(temp);
		temp.clear();
	}
}

/** **************************************************************
 * \brief Search for frame signature.
 * \param frame_segment append data for the frame buffer.
 * \return void
 *****************************************************************/
static void parse_fm_frame(std::string &frame_segment)
{
	unsigned int cur_byte         = FM_INVALID;
	unsigned int frame_size       = 0;
	unsigned int index            = 0;

	unsigned int fend_count       = 0;
	unsigned int first_maker_pos  = 0;
	unsigned int second_maker_pos = 0;

	bool process_one_frame = false;

	std::string one_frame;

	fm_frame.append(frame_segment);
	frame_size = fm_frame.size();


	if(fm_data_count) {
		if(frame_size >= fm_data_count) {
			out_rx_buffered_string(fm_frame, fm_data_count);
			fm_data_count = 0;
		}

		// Do not delete any data from the fm_frame buffer in the data collection stage.
		// Under poor conditions the recevied bits may not translate correctly into the
		// correct number of bytes. This miscalculation could cause the loss of the
		// next frame. This also means frame headers might be seen in the data section.
		// No way around this without employing ARQ protocol.

		return;
	}

	process_one_frame = false;
	fend_count = 0;
	one_frame.clear();

	for(index = 0; index < frame_size; index++) {
		cur_byte = fm_frame[index] & 0xFF;

		if(cur_byte == FM_FEND) {
			fend_count++;
			if(fend_count == 1)
				first_maker_pos = index;
		}

		if(fend_count) {
			one_frame += cur_byte;
		}

		if(fend_count == 2) {
			second_maker_pos = index;
			process_one_frame = true;
			break;
		}
	}

	if(!process_one_frame) { // Keep frame buffer from getting to large.
		if(fm_frame.size() > (MAX_FRAME_SIZE * 2))
			fm_frame.erase(0, MAX_FRAME_SIZE);
		return;
	}

	char *out = (char *)0;
	size_t out_size = fm_decode((char *) one_frame.c_str(), one_frame.size(), &out);
	if(out && (out_size > 0)) {
		one_frame.assign(out, out_size);
		delete [] out;
	}

	if(decode_fm_frame(one_frame, fm_data_count, rx_callsign)) {
		fm_frame.erase(0, second_maker_pos + 1);
	} else {
		fm_frame.erase(0, second_maker_pos);
	}

	if(fm_data_count > 0) {
		set_marker(SET_RX, rx_callsign, last_rx_callsign);
	} else {
		set_marker(SET_LOGIN, rx_callsign, last_rx_callsign);
	}
}

/** **************************************************************
 * \brief Frame detection and processing thread. Wait for received
 * character or every two second to process the input buffer.
 * \return void
 *****************************************************************/
static void * frame_processor(void *ptr)
{
	fm_thread_running   = true;
	fm_thread_idle      = false;
	fm_thread_exit_flag = false;

	while(!fm_thread_exit_flag) {
		MilliSleep(100);
		process_input_buffer();
	}

	pthread_cancel(pthread_self());

	return (void *)0;
}

/** **************************************************************
 * \brief Frame detection and processing thread executor.
 * \return void
 *****************************************************************/
void start_fm_thread(void)
{

	fm_reset();

	int status = pthread_create(&fm_thread_id, NULL,
								frame_processor, (void *)0);
	bool flag = true;

	fltk_flush( (void *)&flag);

	if(status) {
		char *msg = (char *) _("Frame Messenger Thread Execution Failure");
		char *msg2 = (char *) _("Closing Window");

		fl_choice("%s - %s", _("Acknowledge"), (char *)0, (char *)0, msg, msg2);
		LOG_INFO("%s", msg);

		if(window_frame->shown()) {
			window_frame->hide();
			fm_window_open = false;
		}
	}
}

#ifdef FLDIGI_COMP

/** **************************************************************
 * \brief Extern interface - Open messenger Window and start the
 * input thread.
 * \return void
 *****************************************************************/
void open_frame_messgeger(void)
{

	if(fm_window_open) {
		window_frame->show();
		return;
	}

	if(!callTable) {
		callTable = new call_table;
	}

	if(fm_thread_running) {
		LOG_INFO("%s", _("Frame Messenger Thread Already Running"));
		return;
	}

	if(!window_frame)
		return;

	if(window_frame) {
		window_frame->show();
	}

	if(window_frame->shown()) {
		start_fm_thread();
		fm_window_open = true;
	}
}

#endif // #ifdef FLDIGI_COMP

extern void cb_exit(void);

/** **************************************************************
 * \brief Extern interface - Close messenger Window and shut down
 * input buffer thread.
 * \return void
 *****************************************************************/
void close_frame_messgeger(void)
{
	void *results = (void *)0;

	save_fm_window_defaults();

	close_config();
	close_festival();

	window_frame->hide();
	fm_window_open = false;

	fm_thread_exit_flag = true;
	pthread_cond_signal(&fm_cond);

	MilliSleep(250);
	pthread_join(fm_thread_id, &results);
	fm_thread_running = false;

	if(!callTable) {
		delete callTable;
	}

	cb_exit();
}

/** **************************************************************
 * \brief Cancel transmission state.
 * \param void
 * \return void
 *****************************************************************/
void cancel_tx(void)
{
	canceling_tx_flag = true;
	clear_tx_buffer();
	send_abort();
}

/** **************************************************************
 * \brief Set the TX button state.
 * \param state Button state to set.
 * \return void
 *****************************************************************/
void set_trx_state(int state)
{
	if((trx_internal_state == RX_STATE) && (trx_state == RX_STATE)) {
		trx_indictor->color(fl_rgb_color(0, 255, 0));
		trx_indictor->redraw();
		btn_frame_trx->label(_("Transmit"));
		btn_frame_trx->redraw();
		canceling_tx_flag = false;
		return;
	}

	if((trx_internal_state == TX_STATE) && (trx_state == RX_STATE)) {
		btn_frame_trx->label(_("Sending"));
		btn_frame_trx->redraw();
		return;
	}

	if(state == TX_STATE) {
		trx_indictor->color(fl_rgb_color(255, 0, 0));
		trx_indictor->redraw();
	}

	if((state == TX_STATE) && !canceling_tx_flag) {
		btn_frame_trx->label(_("Cancel"));
		btn_frame_trx->redraw();
	}
}

/** **************************************************************
 * \brief Force screen update every second.
 * \param _data void pointer to bool reset flag.
 * \return void
 *****************************************************************/
void fltk_flush(void *_data)
{
	bool flag = 0;
	if(_data)
		flag = *(bool *)_data;

	if (flag) {
		struct timeval tv;
		gettimeofday(&tv, NULL);
		double st = 1.0 - tv.tv_usec / 1e6;
		Fl::repeat_timeout(st, fltk_flush);
	} else {
		Fl::repeat_timeout(1.0, fltk_flush);
	}

	trx_state = (get_trx_state() == "TX") ? TX_STATE : RX_STATE;

	set_trx_state(trx_state);

	Fl::lock();
	Fl::flush();
	Fl::unlock();
}
