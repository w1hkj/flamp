// ----------------------------------------------------------------------------
// fm_ncs_control.cxx
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
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Native_File_Chooser.H>

#include "fm_msngr.h"
#include "fm_msngr_src/fm_control.h"
#include "fm_msngr_src/fm_ncs_control.h"
#include "fm_msngr_src/fm_ncs_config.h"
#include "flmisc.h"
#include "fileselect.h"

#include <string>
#include <list>
#include <vector>
#include <errno.h>

#include "gettext.h"
#include "status.h"
#include "debug.h"

#define SEND_TO_ID "SEND_TO"

typedef enum _ncs_msg_types {
	NCS_NULL = 0,
	NCS_ALL,
	NCS_OPERATOR,
	NCS_NONE
} NSC_MSG_TYPES;

typedef struct _ncs_msg_table {
	NSC_MSG_TYPES type;
	std::string header;
	std::string message;
	_ncs_msg_table() {
		type = NCS_NULL;
		header.clear();
		message.clear();
	};
} NCS_MSG_TABLE;

static std::vector<NCS_MSG_TABLE *> ncs_message_table;
static int ncs_message_table_count = 0;

/** **************************************************************
 * \brief Convert/Filter label data for display and CSV format
 * \param src_raw Original label.
 * \param display Storage for display lable.
 * \param csv Storage for csv lable.
 * \return void
 *****************************************************************/
bool convert_label(std::string src_raw, std::string &display, std::string &csv)
{
	std::string src_cooked;
	int ch = 0;

	if(src_raw.empty()) return false;

	src_cooked.clear();
	csv.clear();
	display.clear();

	// Filter out unwanted characters.
	for(int i = 0; i < src_raw.size(); i++) {
		ch = src_raw[i];
		if(ch < ' ') continue;
		src_cooked += ch;
	}

	// CSV: Copy and replace spaces with underscore.
	for(int i = 0; i < src_cooked.size(); i++) {
		ch = src_cooked[i];
		if(ch == ' ')
			ch = '_';
		csv += toupper(ch);
	}

	// Display: Copy results
	display.assign(src_cooked);

	return true;
}

/** **************************************************************
 * \brief Remove leading/trailing white spaces.
 * \param buffer Destination buffer
 * \param limit passed buffer size
 * \return void
 *****************************************************************/
void trim(char *buffer, size_t limit)
{
	char *s      = (char *)0;
	char *e      = (char *)0;
	char *dst    = (char *)0;
	size_t count = 0;

	if(!buffer || limit < 1) {
		return;
	}

	for(count = 0; count < limit; count++)
		if(buffer[count] == 0) break;

	if(count < 1) return;

	s = buffer;
	e = &buffer[count-1];

	for(size_t i = 0; i < count; i++) {
		if(*s <= ' ') s++;
		else break;
	}

	while(e > s) {
		if((*e <= ' '))
			*e = 0;
		else
			break;
		e--;
	}

	dst = buffer;
	for(; s <= e; s++) {
		*dst++ = *s;
	}
	*dst = 0;
}

/** *******************************************************************
 * \brief Load user selectable presets.
 * \param filename File to open.
 * \param data Storgae for the preset data (by reference).
 * \param labelName Storage for the user label (by reference).
 * \return 0/1/2 0=Error 1=User1 2=User2 Presets.
 **********************************************************************/
int load_user_presets_from_file(const char *filename, std::string &data, std::string &labelName)
{
	char  *buffer = (char *)0;
	const size_t buffer_size = 1024;
	int line_no = 0;
	int for_user= 0;
	std::string _data;

#ifdef __WOE32__
	FILE* fd = fopen(filename, "rt");
#else
	FILE* fd = fopen(filename, "r");
#endif

	if(!fd) {
		LOG_INFO("%s %s", "Unable to load file ", filename);
		return 0;
	}

	buffer = new char[buffer_size];

	if(!buffer) {
		LOG_INFO("%s", "Memory Allocation Error");
		fclose(fd);
		return 0;
	}

	memset(buffer, 0, buffer_size);

	if(!fgets(buffer, buffer_size - 1, fd)) {
		LOG_INFO("%s", "Error reading tag");
		fclose(fd);
		delete [] buffer;
		return 0;
	}

	trim(buffer, buffer_size);
	line_no++;

	for(int i = 0; i < buffer_size; i++) {
		if(!buffer[i]) break;
		buffer[i] = toupper(buffer[i]);
	}

	if(strncmp("USER1", buffer, 5) == 0)
		for_user = 1;

	if(strncmp("USER2", buffer, 5) == 0)
		for_user = 2;

	if(!for_user) {
		LOG_INFO("%s", "File missing USER1 or USER2 Tag");
		fclose(fd);
		delete [] buffer;
		return 0;
	}

	if(!fgets(buffer, buffer_size - 1, fd)) {
		LOG_INFO("%s", "Error reading label");
		fclose(fd);
		delete [] buffer;
		return 0;
	}

	trim(buffer, buffer_size);
	line_no++;

	labelName.assign(buffer);

	data.clear();
	_data.clear();

	while(1) {
		if(feof(fd)) break;

		if(!fgets(buffer, buffer_size - 1, fd)) break;
		trim(buffer, buffer_size);
		line_no++;

		_data.append(buffer).append("|");
	}

	if(ferror(fd)) {
		LOG_INFO("%s %d", "File Read Error at line ", line_no);
	} else {
		switch(for_user) {
			case 1:
				progStatus.user1_preset_file.assign(filename);
				break;
			case 2:
				progStatus.user2_preset_file.assign(filename);
				break;
		}
	}

	int ch = 0;
	for(int i = 0; i < _data.size(); i++) {
		ch = _data[i];
		if(ch == '/') { // Prevent a sub menu. Escape the character.
			data.append("\\/");
			continue;
		}
		data += ch;
	}

	delete [] buffer;
	fclose(fd);

	return for_user;
}

/** *******************************************************************
 * \brief Load user selectable presets.
 * \param data Storgae for the data (by reference).
 * \param labelName Storage for the user1 label (by reference).
 * \return 0/1/2 0=Error 1=User1 2=User2 Presets.
 **********************************************************************/
int load_user_presets(std::string &data, std::string &labelName)
{
	Fl_Native_File_Chooser browser;

	browser.title("Choose a user preset");
	browser.type(Fl_Native_File_Chooser::BROWSE_FILE);
	browser.filter("Text\t*.txt\n");
	browser.directory(HomeDir.c_str());

	switch ( browser.show() ) {
		case -1:
		case  1: return 0;
	}

	return load_user_presets_from_file(browser.filename(), data, labelName);
}

/** *******************************************************************
 * \brief Add a entry to the table
 * \param type one line of data.
 * \param index processing entry number.
 * \return bool true line extracted and store in vector table. false
 *	skipped.
 * \par FORMAT:
 * '['<type>'|'<header>'|'<Send to message>']'
 * Exceptable escape sequences in the 'send to message' section only:
 * '\]' '\[' '\|' '\\' '\n' '\t'
 * Do not include '\r' for end of line, use '\n' only.
 **********************************************************************/
static bool add_to_table(std::string type_str, std::string header_str, std::string msg_str)
{
	if(type_str.empty() || header_str.empty() || msg_str.empty())
		return false;

	NSC_MSG_TYPES _type = NCS_NULL;
	NCS_MSG_TABLE * tmp = (NCS_MSG_TABLE *)0;

	if(type_str.find("ALL") != std::string::npos)
		_type = NCS_ALL;
	else if(type_str.find("OP") != std::string::npos)
		_type = NCS_OPERATOR;
	else if(type_str.find("NONE") != std::string::npos)
		_type = NCS_NONE;
	else
		return false;

	tmp = (NCS_MSG_TABLE *) new NCS_MSG_TABLE;

	if(!tmp) return false;

	tmp->type = _type;
	tmp->header.assign(header_str);
	tmp->message.assign(msg_str);

	ncs_message_table.push_back(tmp);
	ncs_message_table_count = ncs_message_table.size();

	return true;
}

/** *******************************************************************
 * \brief Free the message list table.
 * \param void
 * \return void
 **********************************************************************/
void ncs_send_to_msg_delete(void)
{
	int count = ncs_message_table.size();

	for(int i = 0; i < count; i++) {
		if(ncs_message_table[i])
			delete ncs_message_table[i];
	}

	ncs_message_table.clear();
}

/** *******************************************************************
 * \brief Parse a single line entry.
 * \param line one line of data.
 * \param index processing entry number.
 * \return bool true line extracted and store in vector table. false
 *	skipped.
 * \par FORMAT:
 * '['<type>'|'<header>'|'<Send to message>']'
 * Exceptable escape sequences in the 'send to message' section only:
 * '\]' '\[' '\|' '\\' '\n' '\t'
 * Do not include '\r' for end of line, use '\n' only.
 **********************************************************************/
static bool extract_line(std::string line, int &entry_no, bool &reset_flag)
{
	if(line[0] != '[') return false;

	bool left_flag    = false;
	bool right_flag   = false;
	bool ignore_flag  = false;
	int  divider_flag = 0;
	int  ch = 0;

	std::string type_str;
	std::string header_str;
	std::string msg_str;

	while(!line.empty()) {
		ch = line[0];
		if(ch == '[') {
			left_flag = true;
			line.erase(0,1);
			break;
		}
	}

	divider_flag = 0;
	type_str.clear();

	while(!line.empty()) {
		ch = line[0];

		if(ch == ']') {
			LOG_INFO("Misplaced ']' (Entry No ~%d)", entry_no);
			return false;
		}

		if(ch == '[') {
			LOG_INFO("Misplaced '[' (Entry No ~%d)", entry_no);
			return false;
		}

		if(ch < ' ') {
			line.erase(0,1);
			continue;
		}

		if(ch == '|') {
			divider_flag++;
			line.erase(0,1);
			break;
		}
		type_str += toupper(ch);
		line.erase(0,1);
	}

	header_str.clear();
	while(!line.empty()) {
		ch = line[0];

		if(ch == ']') {
			LOG_INFO("Misplaced ']' (Entry No ~%d)", entry_no);
			return false;
		}

		if(ch == '[') {
			LOG_INFO("Misplaced '[' (Entry No ~%d)", entry_no);
			return false;
		}

		if(ch < ' ') {
			line.erase(0,1);
			continue;
		}

		if(ch == '|') {
			divider_flag++;
			line.erase(0,1);
			break;
		}
		header_str += ch;
		line.erase(0,1);
	}

	msg_str.clear();
	while(!line.empty()) {
		ch = line[0];

		if(ch == '|') {
			LOG_INFO("Misplaced '|' (Entry No ~%d)", entry_no);
			return false;
		}

		if(ch == '[') {
			LOG_INFO("Misplaced '[' (Entry No ~%d)", entry_no);
			return false;
		}

		if(ch == '\\') {
			line.erase(0,1);
			ch = line[0];
			switch(ch) {
				case 'n':
					ch = '\n';
					break;
				case 'r':
					line.erase(0, 1);
					continue;
				case 't':
					ch = '\t';
					break;
				case '\\':
				case '[':
				case ']':
				case '|':
					ignore_flag = true;
					break;
			}
		}

		if(ch == ']' && !ignore_flag) {
			line.erase(0,1);
			right_flag = true;
			break;
		}

		ignore_flag = false;

		msg_str += ch;
		line.erase(0, 1);
	}


	if(left_flag && right_flag && (divider_flag == 2)) {
		if(reset_flag) {
			ncs_send_to_msg_delete();
			reset_flag = false;
		}
		entry_no++;
		return add_to_table(type_str, header_str, msg_str);
	} else {
		if(!left_flag)
			LOG_INFO("Missing '[' (Entry No ~%d)", entry_no);
		if(!right_flag)
			LOG_INFO("Missing ']' (Entry No ~%d)", entry_no);
		if(divider_flag < 2)
			LOG_INFO("Missing one or both '|' (Entry No ~%d)", entry_no);
	}
	return false;
}

/** *******************************************************************
 * \brief Parse the read file. Assigned to GUI interface
 * \param char *data file read data.
 * \param size_t Number of bytes in the buffer.
 * \return bool true valid file, false otherwise.
 **********************************************************************/
static bool parse_file(char *data, size_t count, bool &reset_flag)
{
	if(!data || count < 1) return false;

	std::string tmp = "";
	std::string line_item = "";

	tmp.assign(data, count);
	int state = tmp.find(SEND_TO_ID);
	int index = 0;
	int entry_no = 1;
	bool left_flag = false;
	bool right_flag = false;

	if(state == std::string::npos) {
		LOG_INFO("%s (%s)", "Send To file missing ID tag", (char *) SEND_TO_ID);
		return false;
	}

	tmp.erase(0, state + sizeof(SEND_TO_ID) - 1);

	while(!tmp.empty()) {
		while(!tmp.empty()) {
			if(tmp[0] == '[') {
				left_flag = true;
				break;
			}
			else tmp.erase(0, 1);
		}

		if(tmp.empty()) break;

		index = 0;
		while(tmp[index]) {
			line_item += tmp[index];
			if(tmp[index] == ']') {
				right_flag = true;
				break;
			}
			index++;
		}

		if(left_flag && right_flag) {
			extract_line(line_item, entry_no, reset_flag);
		} else {
			if(!left_flag)
				LOG_INFO("Missing '[' (Entry No ~%d)", entry_no);
			if(!right_flag)
				LOG_INFO("Missing ']' (Entry No ~%d)", entry_no);
			return false;
		}

		left_flag = right_flag = false;
		line_item.clear();
		tmp.erase(0, index);
	}

	return true;
}

/** *******************************************************************
 * \brief Send NCS selected message.
 * \param ctable The callsign and associated data.
 * \param msg_idx The message to send.
 * \return void
 **********************************************************************/
void tx_send_to_msg(CALL_TABLE *ctable, int msg_idx)
{
	int count = ncs_message_table.size();

	if(!ctable || (msg_idx < 0) || (msg_idx > count)) return;

	NCS_MSG_TABLE *_table = ncs_message_table[msg_idx];
	if(!_table) return;

	std::string tx_msg = "";
	std::string delim = " ";
	std::string msg = _table->message;
	int msg_type = _table->type;

	tx_msg.assign("\n");

	if(msg_type == NCS_OPERATOR) {

		if(progStatus.ncs_print_labels) {
			delim.assign("\t");

			if(progStatus.ncs_call_print)
				tx_msg.append(progStatus.display_call_tag).append(delim);

			if(progStatus.ncs_name_print)
				tx_msg.append(progStatus.display_name_tag).append(delim);

			if(progStatus.ncs_qth_print)
				tx_msg.append(progStatus.display_qth_tag).append(delim);

			if(progStatus.ncs_state_print)
				tx_msg.append(progStatus.display_state_tag).append(delim);

			if(progStatus.ncs_traffic_print) {
				if(ctable->traffic)
					tx_msg.append("Traffic").append(delim);
			}

			if(progStatus.ncs_user1_print)
				tx_msg.append(progStatus.display_user1_tag).append(delim);

			if(progStatus.ncs_user2_print)
				tx_msg.append(progStatus.display_user2_tag);

			tx_msg += '\n';
		}

		if(progStatus.ncs_call_print)
			tx_msg.append(ctable->call).append(delim);

		if(progStatus.ncs_name_print)
			tx_msg.append(ctable->name).append(delim);

		if(progStatus.ncs_qth_print)
			tx_msg.append(ctable->qth).append(delim);

		if(progStatus.ncs_state_print)
			tx_msg.append(ctable->state).append(delim);

		if(progStatus.ncs_traffic_print) {
			if(ctable->traffic)
				tx_msg.append(_("Yes")).append(delim);
			else
				tx_msg.append(_("No")).append(delim);
		}


		if(progStatus.ncs_user1_print)
			tx_msg.append(ctable->user1).append(delim);

		if(progStatus.ncs_user2_print)
			tx_msg.append(ctable->user2);

	}

	if(msg_type == NCS_ALL) {
		tx_msg.append(_("All Stations")).append(" ");
	}

	tx_msg += '\n';

	tx_msg.append(msg);

	fm_append_tx(tx_msg);
}

/** *******************************************************************
 * \brief Assign the widget with the loaded data.
 * \param reset_flag bool append or reset data.
 * \return void
 **********************************************************************/
void assign_to_widget(bool reset_flag)
{
	int count = ncs_message_table.size();
	if(count < 1) return;

	std::string one_liner = "";
	std::string table = "";
	NCS_MSG_TABLE *ptr = (NCS_MSG_TABLE *)0;

	if(reset_flag)
		choice_send_to_op_msg->clear();

	for(int i = 0; i < count; i++) {
		ptr = ncs_message_table[i];
		if(ptr) {
			switch(ptr->type) {
				case NCS_ALL:
					one_liner.assign("[ALL] ");
					break;
				case NCS_OPERATOR:
					one_liner.assign("[OP] ");
					break;
				case NCS_NONE:
					one_liner.assign("[NONE] ");
					break;
				default:
					continue;
			}

			one_liner.append(ptr->header);
		}

		table.append(one_liner).append("|");
	}

	choice_send_to_op_msg->add(table.c_str());
	choice_send_to_op_msg->value(0);
}

/** *******************************************************************
 * \brief Load a NCS message list.
 * \param void
 * \return void
 **********************************************************************/
void ncs_load_send_to_msg(bool use_browser)
{
	char *fn = (char *)0;
	char *data = (char *) 0;
	size_t count = 0;
	size_t read_count = 0;
	bool parse_flag = true;
	bool reset_flag = false;
	int select = 0;
	Fl_Native_File_Chooser browser;

	if(use_browser) {
		select = fl_choice("%s", _("Replace"), _("Append"), _("Cancel"), _("Select Load Type:"));

		if(select == 2) return;



		browser.title("Choose a user preset");
		browser.type(Fl_Native_File_Chooser::BROWSE_FILE);
		browser.filter("Text\t*.txt\n");
		browser.directory(HomeDir.c_str());           // default directory to use
													  // Show native chooser
		switch (browser.show() ) {
			case -1:
			case  1: return;
		}

		fn = (char *) browser.filename();
	} else {
		if(!progStatus.ncs_load_stms_file_path.empty())
			fn = (char *) progStatus.ncs_load_stms_file_path.c_str();
		parse_flag = true;
		reset_flag = true;
	}
	
	if(!fn) return;
	
#ifdef __WOE32__
	FILE* fd = fopen(fn, "rt");
#else
	FILE* fd = fopen(fn, "r");
#endif
	
	if(!fd) {
		LOG_INFO("Error %d: %s %s", errno, _("Unable to load file"), fn);
		return;
	}
	
	if(select == 0)
		reset_flag = true;
	
	fseek(fd, 0, SEEK_END);
	count = ftell(fd);
	fseek(fd, 0, SEEK_SET);
	
	data = (char *) malloc(count + 1);
	
	if(!data) {
		LOG_INFO("%s", "Read File memory allocation Error");
		parse_flag = false;
	}
	
	if(parse_flag)
		read_count = fread(data, sizeof(char), count, fd);
	
	if(ferror(fd) && !feof(fd)) {
		LOG_INFO("Error %d: %s %s", errno, "Read Error in file", fn);
		parse_flag = false;
	}
	
	if(count != read_count) {
		LOG_INFO("%s", "File Read Error: Read count does not meet expectations");
		parse_flag = false;
	}
	
	if(parse_flag) {
		if(parse_file(data, count, reset_flag))
			progStatus.ncs_load_stms_file_path.assign(fn);
	}
	
	assign_to_widget(reset_flag);
	
	if(data) free(data);
	if(fd) fclose(fd);
}