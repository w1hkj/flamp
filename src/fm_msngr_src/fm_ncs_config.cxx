
// ----------------------------------------------------------------------------
// fm_ncs_config.cxx
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
#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Box.H>
#include <FL/filename.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/fl_show_colormap.H>
#include <FL/Fl_Color_Chooser.H>
#include <FL/Fl_Help_Dialog.H>

#include "fm_msngr_src/fm_ncs_control.h"
#include "fm_msngr_src/display_list_config.h"
#include "fm_msngr_src/log_csv_config.h"
#include "fm_msngr_src/fm_ncs_config.h"
#include "fm_msngr_src/fm_config.h"
#include "fm_msngr_src/fm_control.h"
#include "fm_msngr_src/fm_dialog.h"
#include "fm_msngr_src/call_table.h"
#include "status.h"
#include "font_browser.h"
#include "flslider2.h"
#include "util.h"
#include "gettext.h"
#include "debug.h"
#include "call_table.h"

Fl_Double_Window *create_ncs_window(void);
Fl_Double_Window *ncs_window = (Fl_Double_Window *)0;

Fl_Button *btn_add  = (Fl_Button *)0;
Fl_Button *btn_close = (Fl_Button *)0;
Fl_Button *btn_next = (Fl_Button *)0;
Fl_Button *btn_prev = (Fl_Button *)0;
Fl_Button *btn_logout = (Fl_Button *)0;
Fl_Button *btn_send_to_msg = (Fl_Button *)0;
Fl_Button *btn_load_send_to_msg = (Fl_Button *)0;
Fl_Button *btn_load_user_n = (Fl_Button *)0;
Fl_Button *btn_set_user_n = (Fl_Button *)0;

Fl_Check_Button *chk_btn_link_fldigi    = (Fl_Check_Button *)0;
Fl_Check_Button *chk_btn_link_flnet     = (Fl_Check_Button *)0;
Fl_Check_Button *chk_btn_print_lables   = (Fl_Check_Button *)0;
Fl_Check_Button *chk_btn_handled        = (Fl_Check_Button *)0;
Fl_Check_Button *chk_btn_use_call       = (Fl_Check_Button *)0;
Fl_Check_Button *chk_btn_use_name       = (Fl_Check_Button *)0;
Fl_Check_Button *chk_btn_use_qth        = (Fl_Check_Button *)0;
Fl_Check_Button *chk_btn_use_state      = (Fl_Check_Button *)0;
Fl_Check_Button *chk_btn_traffic        = (Fl_Check_Button *)0;
Fl_Check_Button *chk_btn_use_traffic    = (Fl_Check_Button *)0;
Fl_Check_Button *chk_btn_use_user1      = (Fl_Check_Button *)0;
Fl_Check_Button *chk_btn_use_user2      = (Fl_Check_Button *)0;
Fl_Check_Button *chk_btn_send_to_op_msg = (Fl_Check_Button *)0;

Fl_Choice *choice_queue          = (Fl_Choice *)0;
Fl_Choice *choice_send_to_op_msg = (Fl_Choice *)0;
Fl_Choice *choice_user1          = (Fl_Choice *)0;
Fl_Choice *choice_user2          = (Fl_Choice *)0;

Fl_Input  *input_call            = (Fl_Input *)0;
Fl_Input  *input_name            = (Fl_Input *)0;
Fl_Input  *input_qth             = (Fl_Input *)0;
Fl_Input  *input_state           = (Fl_Input *)0;
Fl_Input  *input_user1           = (Fl_Input *)0;
Fl_Input  *input_user2           = (Fl_Input *)0;

Fl_Output *output_link_to_msg    = (Fl_Output *)0;

pthread_mutex_t fm_mutex_choice = PTHREAD_MUTEX_INITIALIZER;

static std::string static_user1_label_name;
static std::string static_user2_label_name;

static bool global_logout_flag = false;

/** *******************************************************************
 * \brief Find Call sign, if found update with the widget data. If not
 * found create a new entry.
 * \param callsign The callsign to search for.
 * \return void.
 **********************************************************************/
void save_ncs_call_data(void *handle, std::string call_raw)
{
	if(!handle) return;

	CALL_TABLE data;
	int count = 0;
	int ch = 0;
	std::string _call;
	time_t _time = 0;

	_call.clear();
	count = call_raw.size();
	for(int i = 0; i < count; i++) {
		ch = call_raw[i];
		if(!ch) break;
		if(ch <= ' ') continue;
		_call += toupper(ch);
	}

	bool found = callTable->find_call(handle, _call);

	if(!found) {
		time(&_time);

		data.call    = _call;
		data.name    = input_name->value();
		data.qth     = input_qth->value();
		data.state   = input_state->value();
		data.user1   = input_user1->value();
		data.user2   = input_user2->value();
		data.traffic = chk_btn_traffic->value();
		data.handled = chk_btn_handled->value();
		data.check_in_time = _time;

		callTable->add(handle, &data);
		set_marker(SET_NCS_LOGIN, _call, last_call_sign_heard());

	} else {
		callTable->read(handle, &data);

		data.name    = input_name->value();
		data.qth     = input_qth->value();
		data.state   = input_state->value();
		data.user1   = input_user1->value();
		data.user2   = input_user2->value();
		data.traffic = chk_btn_traffic->value();
		data.handled = chk_btn_handled->value();

		callTable->write(handle, &data);
	}
}

/** *******************************************************************
 * \brief Quick and dirty call table update.
 * \return void.
 **********************************************************************/
void update_call_table(void)
{
	void *handle = callTable->handle();
	if(!handle) return;

	std::string call;
	call.assign(input_call->value());

	save_ncs_call_data(handle, call);

	callTable->delete_handle(handle);
}

/** *******************************************************************
 * \brief Quick and dirty call table update.
 * \para handle handle reference to call table data.
 * \return void.
 **********************************************************************/
void update_call_table(void *handle)
{
	if(!handle) return;

	std::string call;
	call.assign(input_call->value());

	save_ncs_call_data(handle, call);
}

/** *******************************************************************
 * \brief Find Call sign, if found populate the widgets. Chenge the
 * visable state of the widget id cal is logged out.
 * \param callsign The callsign to search for.
 * \return void.
 **********************************************************************/
void recall_ncs_call_data(void *handle, std::string callsign)
{
	if(!handle) return;

	CALL_TABLE data;
	std::string _call_label;
	std::string _call;
	int count = 0;
	int ch = 0;

	_call_label.assign(callsign);
	_call.clear();
	count =_call_label.size();

	for(int i = 0; i < count; i++) {
		ch = _call_label[i];
		if(!ch) break;
		if(ch <= ' ') break;
		_call += ch;
	}

	bool found = callTable->find_call(handle, _call);
	if(found) {
		callTable->read(handle, &data);

		if(data.logged_out) {
			input_call->deactivate();
			input_name->deactivate();
			input_qth->deactivate();
			input_state->deactivate();
			input_user1->deactivate();
			input_user2->deactivate();
			chk_btn_traffic->deactivate();
			chk_btn_handled->deactivate();
		} else {
			input_call->activate();
			input_name->activate();
			input_qth->activate();
			input_state->activate();
			input_user1->activate();
			input_user2->activate();
			chk_btn_traffic->activate();
			chk_btn_handled->activate();
		}

		input_call->value(data.call.c_str());
		input_name->value(data.name.c_str());
		input_qth->value(data.qth.c_str());
		input_state->value(data.state.c_str());
		input_user1->value(data.user1.c_str());
		input_user2->value(data.user2.c_str());
		chk_btn_traffic->value(data.traffic);
		chk_btn_handled->value(data.handled);
	}
}

/** *******************************************************************
 * \brief Create a string reperesentaion of state flags
 * \param traffic The traffic flag.
 * \param handled The handled flag.
 * \param logged_out The logged out flag.
 * \param storage A place to but the data (by appending).
 * \return void
 **********************************************************************/
inline void assign_flags(bool traffic, bool handled, bool logged_out, std::string &storage)
{
	if(logged_out) {
		storage.append(" [LO]");
		return;
	}

	if(traffic || handled) {
		storage.append(" [");
		if(traffic)
			storage.append("T");
		if(handled)
			storage.append("H");
		storage.append("]");
	}
}

/** *******************************************************************
 * \brief Check to see if there is a new or changed entry. Update or
 * add if required.
 * \param _call the callsigne to match.
 * \param _qh change the label name with this.
 * \return void
 **********************************************************************/
void check_choice_menu_item(std::string _call, std::string _qh)
{
	guard_lock choice_lock(&fm_mutex_choice);

	int match = 0;
	bool found = false;
	int count = choice_queue->size();
	char *_label = (char *)0;
	for(int i = 0; i < count; i++) {
		const Fl_Menu_Item &item = choice_queue->menu()[i];
		_label = (char *) item.label();
		if(!_label) break;
		match = strncmp(_call.c_str(), _label, _call.size());
		if(match == 0) {
			found = true;
			choice_queue->replace(i, _qh.c_str());
			choice_queue->redraw();
			break;
		}
	}

	if(!found) {
		choice_queue->add(_qh.c_str(), 0, 0);
	}
}

/** *******************************************************************
 * \brief Set the loggout flag and update queue list on all entires.
 * \return void.
 **********************************************************************/
void log_out_all_calls(void)
{
	if(!callTable)   return;
	if(!ncs_window)  return;

	void *handle = (void *)0;
	bool flag = true;
	std::string queue_h;
	std::string call;
	time_t t;

	handle = callTable->handle();
	if(!handle) return;

	call.clear();

	if(!callTable->top(handle)) {
		callTable->delete_handle(handle);
		return;
	}
	time(&t);
	global_logout_flag = global_logout_flag ? false : true;

	while(flag) {
		queue_h.clear();
		callTable->check_out_time(handle, t);
		callTable->logged_out(handle, global_logout_flag);
		call =  callTable->call(handle);
		queue_h.assign(call);
		assign_flags(callTable->traffic(handle), callTable->handled(handle), \
					 callTable->logged_out(handle), queue_h);
		check_choice_menu_item(call, queue_h);

		if(!callTable->next(handle)) break;
	}

	call = input_call->value();
	if(!call.empty()) {
		bool found = callTable->find_call(handle, call);
		if(found)
			recall_ncs_call_data(handle, call);
	}
	callTable->delete_handle(handle);
}

/** *******************************************************************
 * \brief Set the loggout flag and update queue list for this call.
 * \return void.
 **********************************************************************/
void log_out_call(std::string call)
{
	void *handle = callTable->handle();
	if(!handle) return;

	bool found = callTable->find_call(handle, call);
	time_t t = 0;
	std::string queue_label = "";
	bool log_state = false;

	if(found) {
		time(&t);
		log_state = callTable->logged_out(handle);
		log_state = log_state ? false : true;
		callTable->logged_out(handle, log_state);
		callTable->check_out_time(handle, t);
		queue_label.assign(call);
		assign_flags(callTable->traffic(handle), callTable->handled(handle), \
					 callTable->logged_out(handle), queue_label);
		recall_ncs_call_data(handle, queue_label);
		check_choice_menu_item(call, queue_label);
	}
}

/** *******************************************************************
 * \brief Update the NCS queue list when visable.
 * \param void
 * \return void
 **********************************************************************/
void update_ncs_list(void)
{
	if(!ncs_window) return;
	if(!ncs_window->visible()) return;
	if(!choice_queue) return;

	ncs_load_current_list();
}

/** *******************************************************************
 * \brief Passed logged in ops and place them in the NCS queue.
 * \return void
 **********************************************************************/
void ncs_load_current_list(void)
{
	if(!callTable)   return;
	if(!ncs_window)  return;

	void *rh = (void *)0;
	CALL_TABLE data;
	bool flag = true;
	std::string queue_h;
	std::string list;

	rh = callTable->handle();
	if(!rh) return;

	list.clear();

	if(!callTable->top(rh)) {
		callTable->delete_handle(rh);
		return;
	}

	while(flag) {
		queue_h.clear();

		flag = callTable->read(rh, &data);

		if(!data.call.empty()) {
			queue_h.assign(data.call);
			assign_flags(data.traffic, data.handled, data.logged_out, queue_h);
			check_choice_menu_item(data.call, queue_h);
		}

		if(!callTable->next(rh)) break;
	}

	callTable->delete_handle(rh);
}

/** *******************************************************************
 * \brief Toggle FLDIGI link usage.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_link_fldigi(Fl_Widget *a, void *b)
{
	progStatus.ncs_link_to_fldigi = chk_btn_link_fldigi->value();
}

/** *******************************************************************
 * \brief Toggle FLNET link usage.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_link_flnet(Fl_Widget *a, void *b)
{
	progStatus.ncs_link_to_flnet = chk_btn_link_flnet->value();
}

/** *******************************************************************
 * \brief Print labels in message.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_print_lables(Fl_Widget *a, void *b)
{
	progStatus.ncs_print_labels = chk_btn_print_lables->value();
}

/** *******************************************************************
 * \brief Enbale/Disable the printing of this field.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_use_user1(Fl_Widget *a, void *b)
{
	progStatus.ncs_user1_print = chk_btn_use_user1->value();
}

/** *******************************************************************
 * \brief Enbale/Disable the printing of this field.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_use_user2(Fl_Widget *a, void *b)
{
	progStatus.ncs_user2_print = chk_btn_use_user2->value();
}

/** *******************************************************************
 * \brief Enbale/Disable the printing of this field.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_use_call(Fl_Widget *a, void *b)
{
	progStatus.ncs_call_print = chk_btn_use_call->value();
}

/** *******************************************************************
 * \brief Enbale/Disable the printing of this field.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_use_name(Fl_Widget *a, void *b)
{
	progStatus.ncs_name_print = chk_btn_use_name->value();
}

/** *******************************************************************
 * \brief Enbale/Disable the printing of this field.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_use_qth(Fl_Widget *a, void *b)
{
	progStatus.ncs_qth_print = chk_btn_use_qth->value();
}

/** *******************************************************************
 * \brief Enbale/Disable the printing of this field.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_use_state(Fl_Widget *a, void *b)
{
	progStatus.ncs_state_print = chk_btn_use_state->value();
}

/** *******************************************************************
 * \brief Enbale/Disable the printing of this field.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_use_traffic(Fl_Widget *a, void *b)
{
	progStatus.ncs_traffic_print = chk_btn_use_traffic->value();
}

/** *******************************************************************
 * \brief Enbale/Disable the printing of this field.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_chk_btn_send_to_op_msg(Fl_Widget *a, void *b)
{
	progStatus.ncs_send_to_print = chk_btn_send_to_op_msg->value();
}

/** *******************************************************************
 * \brief Choice Sent to callback.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_choice_send_to_op_msg(Fl_Widget *a, void *b)
{
	update_call_table();
}

/** *******************************************************************
 * \brief Inputs User 1 callback.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_input_user1(Fl_Widget *a, void *b)
{
	update_call_table();
}


/** *******************************************************************
 * \brief Inputs User 2 callback.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_input_user2(Fl_Widget *a, void *b)
{
	update_call_table();
}

/** *******************************************************************
 * \brief Set current log enter to previous one callback.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_btn_prev(Fl_Widget *a, void *b)
{
	std::string callsign = input_call->value();
	std::string next_callsign = "";

	void *handle = callTable->handle();
	if(!handle) return;

	if(!callsign.empty())
		save_ncs_call_data(handle, callsign);

	if(!callTable->prev(handle)) return;

	next_callsign = callTable->call(handle);
	if((next_callsign != callsign) && !next_callsign.empty())
		recall_ncs_call_data(handle, next_callsign);

	callTable->delete_handle(handle);
}

/** *******************************************************************
 * \brief Set current log enter to next one callback.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_btn_next(Fl_Widget *a, void *b)
{
	std::string callsign = input_call->value();
	std::string next_callsign = "";

	void *handle = callTable->handle();
	if(!handle) return;

	if(!callsign.empty())
		save_ncs_call_data(handle, callsign);

	callTable->next(handle);

	next_callsign = callTable->call(handle);
	if((next_callsign != callsign) && !next_callsign.empty())
		recall_ncs_call_data(handle, next_callsign);

	callTable->delete_handle(handle);
}

/** *******************************************************************
 * \brief Add/Update Log entry callback.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_btn_add(Fl_Widget *a, void *b)
{
	std::string call_raw = input_call->value();
	if(call_raw.empty()) return;
	void *handle = callTable->handle();
	if(!handle) return;
	save_ncs_call_data(handle, call_raw);
	callTable->delete_handle(handle);
}

/** *******************************************************************
 * \brief Set/unset logout status for log entry.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_btn_logout(Fl_Widget *a, void *b)
{
	std::string call = input_call->value();
	if(call.empty()) return;

	update_call_table();

	int select = fl_choice("Log Out", _("ALL"), call.c_str(), _("Cancel"));

	switch(select) {
		case 0:
			log_out_all_calls();
			break;
		case 1:
			log_out_call(call);
			break;
	}
}

/** *******************************************************************
 * \brief Transfer a Send To message to the tx panel callback.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_btn_send_to_msg(Fl_Widget *a, void *b)
{
	std::string call = input_call->value();
	int msg_idx = choice_send_to_op_msg->value();
	void *handle = callTable->handle();
	if(!handle) return;
	CALL_TABLE data;

	update_call_table(handle);

	bool found = callTable->find_call(handle, call);

	if(found) {
		callTable->read(handle, &data);
	}

	tx_send_to_msg(&data, msg_idx);

	callTable->delete_handle(handle);
}

/** *******************************************************************
 * \brief Callsign field callback.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_input_call(Fl_Widget *a, void *b)
{
	update_call_table();
}

/** *******************************************************************
 * \brief Name field callback.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_input_name(Fl_Widget *a, void *b)
{
	update_call_table();
}

/** *******************************************************************
 * \brief QTH field callback.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_input_qth(Fl_Widget *a, void *b)
{
	update_call_table();
}

/** *******************************************************************
 * \brief State field callback.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_input_state(Fl_Widget *a, void *b)
{
	update_call_table();
}


/** *******************************************************************
 * \brief User 1 defined field callback.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_choice_user1(Fl_Widget *a, void *b)
{
	input_user1->value(choice_user1->text());
	update_call_table();
}

/** *******************************************************************
 * \brief User 2 defined field callback.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_choice_user2(Fl_Widget *a, void *b)
{
	input_user2->value(choice_user2->text());
	update_call_table();
}

/** *******************************************************************
 * \brief Set current log entry based on queue selection callback.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_choice_users(Fl_Widget *a, void *b)
{
	input_user1->value(choice_user1->text());
	input_user2->value(choice_user2->text());
	update_call_table();
}

/** *******************************************************************
 * \brief Set user 1 label.
 * \param label Text data
 * \return void
 **********************************************************************/
void update_user1_labels(std::string label)
{
	std::string csv;
	std::string display;

	if(convert_label(label, display, csv)) {
		static_user1_label_name.assign(display);
		input_user1->label(static_user1_label_name.c_str());
		input_user1->redraw();
		set_display_user1_tag_name(display);
		set_log_user1_tag_name(csv);
	}
}

/** *******************************************************************
 * \brief Set user 2 label.
 * \param label Text data
 * \return void
 **********************************************************************/
void update_user2_labels(std::string label)
{
	std::string csv;
	std::string display;

	if(convert_label(label, display, csv)) {
		static_user2_label_name.assign(display);
		input_user2->label(static_user2_label_name.c_str());
		input_user2->redraw();
		set_display_user2_tag_name(display);
		set_log_user2_tag_name(csv);
	}
}

/** *******************************************************************
 * \brief Load User 1/2 presets.
 * \param use_broswer Use browser to locate file.
 * \param filename Optional filename path to the preset data.
 * \return void
 **********************************************************************/
void load_presets(bool use_broswer, const char *filename = (char *)0)
{
	std::string data;
	std::string label;
	int usern = 0;

	if(use_broswer) {
		usern = load_user_presets(data, label);
	} else {
		usern = load_user_presets_from_file(filename, data, label);
	}

	switch(usern) {
		case 1:
			update_user1_labels(label);
			choice_user1->clear();
			choice_user1->add(data.c_str());
			break;

		case 2:
			update_user2_labels(label);
			choice_user2->clear();
			choice_user2->add(data.c_str());
			break;
	}
}

/** *******************************************************************
 * \brief Load user Presets callback.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_btn_load_user_n(Fl_Widget *a, void *b)
{
	load_presets(true, (char *)0);
}

/** *******************************************************************
 * \brief Toggle Traffic state for selected call callback.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_btn_traffic(Fl_Widget *a, void *b)
{
	std::string _call;
	_call.assign(input_call->value());
	if(_call.empty()) return;

	void *rh = callTable->handle();
	if(!rh) return;

	bool found = callTable->find_call(rh, _call);
	if(found) {
		callTable->traffic(rh, chk_btn_traffic->value());
		std::string str_flags;
		str_flags.assign(_call);
		assign_flags(callTable->traffic(rh), callTable->handled(rh), callTable->logged_out(rh), str_flags);
		check_choice_menu_item(_call, str_flags);
		choice_queue->redraw();
	}

	callTable->delete_handle(rh);
}

/** *******************************************************************
 * \brief Toggle Handled state for selected call callback.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_btn_handled(Fl_Widget *a, void *b)
{
	std::string _call;
	_call.assign(input_call->value());
	if(_call.empty()) return;

	void *rh = callTable->handle();
	if(!rh) return;

	bool found = callTable->find_call(rh, _call);
	if(found) {
		callTable->handled(rh, chk_btn_handled->value());
		std::string str_flags;
		str_flags.assign(_call);
		assign_flags(callTable->traffic(rh), callTable->handled(rh), callTable->logged_out(rh), str_flags);
		check_choice_menu_item(_call, str_flags);
		choice_queue->redraw();
	}

	callTable->delete_handle(rh);

}

/** *******************************************************************
 * \brief Select current operator from queoe list callback.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_choice_queue(Fl_Widget *a, void *b)
{
	const Fl_Menu_Item &item = choice_queue->menu()[choice_queue->value()];
	char *_label = (char *) item.label();
	if(!_label) return;

	std::string callsign;
	std::string callsign_raw = "";
	std::string next_callsign = "";

	void *handle = callTable->handle();
	if(!handle) return;

	callsign.assign(input_call->value());
	callsign_raw.assign(_label);

	int ch = 0;

	for(int i = 0; i < callsign_raw.size(); i++) {
		ch = callsign_raw[i];
		if(ch == ' ' || ch == '[') break;
		next_callsign += ch;
	}

	if(!callsign.empty())
		save_ncs_call_data(handle, callsign);

	if((next_callsign != callsign) && !next_callsign.empty())
		recall_ncs_call_data(handle, next_callsign);

	callTable->delete_handle(handle);
}

/** *******************************************************************
 * \brief GUI Callback function for loading message list.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_btn_load_send_to_msg(Fl_Widget *a, void *b)
{
	ncs_load_send_to_msg(true);
}

/** *******************************************************************
 * \brief Display the navigation dialog in the center of the main
 * window.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_open_ncs(Fl_Widget *a, void *b)
{
	if(!ncs_window) {
		ncs_window = create_ncs_window();
		if(ncs_window) {
			int x = 0;
			int y = 0;
			int w = 0;
			int h = 0;

			if((progStatus.ncs_window_pos_x < 1) && (progStatus.ncs_window_pos_y < 1)) {
				x = (window_frame->x_root() + ((window_frame->w() - ncs_window->w()) * 0.50));
				y = (window_frame->y_root() + ((window_frame->h() - ncs_window->h()) * 0.50));
			} else {
				x = progStatus.ncs_window_pos_x;
				y = progStatus.ncs_window_pos_y;
			}

			h = ncs_window->h();
			w = ncs_window->w();
			ncs_window->resize(x, y, w, h);

			if(!progStatus.user1_preset_file.empty())
				load_presets(false, progStatus.user1_preset_file.c_str());
			if(!progStatus.user2_preset_file.empty())
				load_presets(false, progStatus.user2_preset_file.c_str());
			ncs_load_send_to_msg(false);
			ncs_load_current_list();
		}
	}

	if(!ncs_window) {
		LOG_INFO("%s", _("Navigator Panel Open Failure"));
		return;
	}

	ncs_window->show();
}

/** *******************************************************************
 * \brief Close the navigator window.
 * \param a Calling widget.
 * \param b variable data pointer
 * \return void
 **********************************************************************/
void cb_close_ncs(Fl_Widget *a, void *b)
{
	if(ncs_window) {
		progStatus.ncs_window_pos_x = ncs_window->x();
		progStatus.ncs_window_pos_y = ncs_window->y();
		ncs_window->hide();
	}
}

/** *******************************************************************
 * \brief Create the Net Control Window
 * \param void
 * \return Fl_Double_Window window pointer.
 **********************************************************************/
Fl_Double_Window* create_ncs_window(void)
{
	int win_w  = 720;
	int win_h  = 480;
	int max_w  = 0;
	int max_h  = 0;
	int border = 8;
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
	int text_height  = 0;
	int text_width   = 0;
	Fl_Font font_nbr = FL_HELVETICA;
	Fl_Fontsize font_size = 13;
	char *msg = (char *)0;
	int tmp = 0;
	int call_width     = 90;
	int name_width     = 90;
	int qth_width      = 120;
	int state_width    = 74;
	int tfc_rep_width  = 76;
	int custom_width   = 90;
	int send_to_width  = 400;
	int ckbox_sqr      = 16;
	int std_height     = font_size + border;
	int button_start_x = 0;
	int button_start_y = 0;
	int button_end_y   = 0;
	int button_width   = 0;
	int button_height  = 0;
	int load_button_width = 0;
	int set_button_width = 0;


	ncs_window = new Fl_Double_Window(win_w, win_h);
	ncs_window->label(_("NCS"));
	ncs_window->callback((Fl_Callback*)cb_close_ncs);

	fl_font(font_nbr, font_size);
	text_height = ::fl_height(font_nbr, font_size);

	msg = (char *) _("Link To:");
	text_width = (int) fl_width(msg);

	x = border;
	y = border;
	w = text_width + border;
	h = text_height;

	output_link_to_msg = new Fl_Output(x, y, w, h);
	output_link_to_msg->box(FL_NO_BOX);
	output_link_to_msg->value(msg);
	output_link_to_msg->textfont(font_nbr);
	output_link_to_msg->textsize(font_size);
	output_link_to_msg->set_output();

	x += (border + text_width);
	w = std_height;
	h = std_height;

	msg = (char *) _("FLDIGI");
	text_width = (int) fl_width(msg);

	chk_btn_link_fldigi = new Fl_Check_Button(x, y, ckbox_sqr, ckbox_sqr, msg);
	chk_btn_link_fldigi->labelfont(font_nbr);
	chk_btn_link_fldigi->labelsize(font_size);
	chk_btn_link_fldigi->value(progStatus.ncs_link_to_fldigi);
	chk_btn_link_fldigi->callback((Fl_Callback*)cb_link_fldigi);

	x += (border + text_width) + w;
	w = std_height;
	h = std_height;

	msg = (char *) _("FLNET");
	text_width = (int) fl_width(msg);

	chk_btn_link_flnet = new Fl_Check_Button(x, y, ckbox_sqr, ckbox_sqr, msg);
	chk_btn_link_flnet->labelfont(font_nbr);
	chk_btn_link_flnet->labelsize(font_size);
	chk_btn_link_flnet->value(progStatus.ncs_link_to_flnet);
	chk_btn_link_flnet->callback((Fl_Callback*)cb_link_flnet);

	x += (border + text_width) + w;

	msg = (char *) _("Print Labels");
	text_width = (int) fl_width(msg);

	chk_btn_print_lables = new Fl_Check_Button(x, y, ckbox_sqr, ckbox_sqr, msg);
	chk_btn_print_lables->labelfont(font_nbr);
	chk_btn_print_lables->labelsize(font_size);
	chk_btn_print_lables->value(progStatus.ncs_print_labels);
	chk_btn_print_lables->callback((Fl_Callback*)cb_print_lables);

	x = (call_width + border) + (name_width + border) + \
	(qth_width + border) + (state_width + border) + (tfc_rep_width + border);

	msg = (char *) _("S");
	set_button_width = (int) fl_width(msg);
	set_button_width += (border * 2);

	btn_set_user_n = new Fl_Button(x - set_button_width, y, set_button_width, std_height, msg);
	btn_set_user_n->labelfont(font_nbr);
	btn_set_user_n->labelsize(font_size);
	btn_set_user_n->callback((Fl_Callback*)cb_choice_users);

	msg = (char *) _("User Presets");
	text_width = (int) fl_width(msg);

	text_width += ((border * 2));
	x = (call_width + border) + (name_width + border) + \
	(qth_width + border) + (state_width + border) + (tfc_rep_width + border);
	tmp = x - text_width - set_button_width - border;

	btn_load_user_n = new Fl_Button(tmp, y, text_width, std_height, msg);
	btn_load_user_n->labelfont(font_nbr);
	btn_load_user_n->labelsize(font_size);
	btn_load_user_n->callback((Fl_Callback*)cb_btn_load_user_n);

	x += (border);

	choice_user1 = new Fl_Choice(x, y, custom_width, h);
	choice_user1->labelfont(font_nbr);
	choice_user1->labelsize(font_size);
	choice_user1->add("Not Set");
	choice_user1->value(0);
	choice_user1->callback((Fl_Callback*)cb_choice_user1);

	x += (custom_width + border);

	choice_user2 = new Fl_Choice(x, y, custom_width, h);
	choice_user2->labelfont(font_nbr);
	choice_user2->labelsize(font_size);
	choice_user2->add("Not Set");
	choice_user2->value(0);
	choice_user2->callback((Fl_Callback*)cb_choice_user2);

	x = (border);
	y += (std_height + text_height + (border >> 1));

	input_call = new Fl_Input(x, y, call_width, h, _("Call"));
	input_call->align(FL_ALIGN_TOP_LEFT);
	input_call->labelfont(font_nbr);
	input_call->labelsize(font_size);
	input_call->callback((Fl_Callback*)cb_input_call);

	chk_btn_use_call = new Fl_Check_Button(x + call_width - ckbox_sqr, y - ckbox_sqr, ckbox_sqr, ckbox_sqr);
	chk_btn_use_call->labelfont(font_nbr);
	chk_btn_use_call->labelsize(font_size);
	chk_btn_use_call->value(progStatus.ncs_call_print);
	chk_btn_use_call->callback((Fl_Callback*)cb_use_call);

	x += (call_width + border);

	input_name = new Fl_Input(x, y, name_width, std_height,  _("Name"));
	input_name->align(FL_ALIGN_TOP_LEFT);
	input_name->labelfont(font_nbr);
	input_name->labelsize(font_size);
	input_name->callback((Fl_Callback*)cb_input_name);

	chk_btn_use_name = new Fl_Check_Button(x + name_width - ckbox_sqr, y - ckbox_sqr, ckbox_sqr, ckbox_sqr, "");
	chk_btn_use_name->labelfont(font_nbr);
	chk_btn_use_name->labelsize(font_size);
	chk_btn_use_name->value(progStatus.ncs_name_print);
	chk_btn_use_name->callback((Fl_Callback*)cb_use_name);

	x += (name_width + border);

	input_qth = new Fl_Input(x, y, qth_width, std_height,  _("QTH"));
	input_qth->align(FL_ALIGN_TOP_LEFT);
	input_qth->labelfont(font_nbr);
	input_qth->labelsize(font_size);
	input_qth->callback((Fl_Callback*)cb_input_qth);

	chk_btn_use_qth = new Fl_Check_Button(x + qth_width - ckbox_sqr, y - ckbox_sqr, ckbox_sqr, ckbox_sqr, "");
	chk_btn_use_qth->labelfont(font_nbr);
	chk_btn_use_qth->labelsize(font_size);
	chk_btn_use_qth->value(progStatus.ncs_qth_print);
	chk_btn_use_qth->callback((Fl_Callback*)cb_use_qth);

	x += (qth_width + border);

	input_state = new Fl_Input(x, y, state_width, std_height,  _("State"));
	input_state->align(FL_ALIGN_TOP_LEFT);
	input_state->labelfont(font_nbr);
	input_state->labelsize(font_size);
	input_state->callback((Fl_Callback*)cb_input_state);

	chk_btn_use_state = new Fl_Check_Button(x + state_width - ckbox_sqr, y - ckbox_sqr, ckbox_sqr, ckbox_sqr, "");
	chk_btn_use_state->labelfont(font_nbr);
	chk_btn_use_state->labelsize(font_size);
	chk_btn_use_state->value(progStatus.ncs_state_print);
	chk_btn_use_state->callback((Fl_Callback*)cb_use_state);

	tmp = y;
	x += (state_width + border);

	chk_btn_use_traffic = new Fl_Check_Button(x + state_width - ckbox_sqr, y - ckbox_sqr, ckbox_sqr, ckbox_sqr, "");
	chk_btn_use_traffic->labelfont(font_nbr);
	chk_btn_use_traffic->labelsize(font_size);
	chk_btn_use_traffic->value(progStatus.ncs_traffic_print);
	chk_btn_use_traffic->callback((Fl_Callback*)cb_use_traffic);

	y -= (ckbox_sqr >> 1);

	chk_btn_traffic = new Fl_Check_Button(x, y, ckbox_sqr, ckbox_sqr, _("Traffic"));
	chk_btn_traffic->labelfont(font_nbr);
	chk_btn_traffic->labelsize(font_size);
	chk_btn_traffic->callback((Fl_Callback*)cb_btn_traffic);

	y += (ckbox_sqr);

	chk_btn_handled = new Fl_Check_Button(x, y, ckbox_sqr, ckbox_sqr, _("Handled"));
	chk_btn_handled->labelfont(font_nbr);
	chk_btn_handled->labelsize(font_size);
	chk_btn_handled->callback((Fl_Callback*)cb_btn_handled);

	send_to_width = (x + tfc_rep_width - border);

	x += (tfc_rep_width + border);
	y = tmp;

	button_start_x = x;
	button_start_y = y + std_height;

	input_user1 = new Fl_Input(x, y, custom_width, h, "User1");
	input_user1->align(FL_ALIGN_TOP_LEFT);
	input_user1->labelfont(font_nbr);
	input_user1->labelsize(font_size);
	input_user1->label(_("User1"));
	input_user1->callback((Fl_Callback*)cb_input_user1);

	chk_btn_use_user1 = new Fl_Check_Button(x + custom_width - ckbox_sqr, y - ckbox_sqr, ckbox_sqr, ckbox_sqr, "");
	chk_btn_use_user1->labelfont(font_nbr);
	chk_btn_use_user1->labelsize(font_size);
	chk_btn_use_user1->value(progStatus.ncs_user1_print);
	chk_btn_use_user1->callback((Fl_Callback*)cb_use_user1);

	x += (custom_width + border);

	input_user2 = new Fl_Input(x, y, custom_width, h, "User2");
	input_user2->align(FL_ALIGN_TOP_LEFT);
	input_user2->labelfont(font_nbr);
	input_user2->labelsize(font_size);
	input_user2->callback((Fl_Callback*)cb_input_user1);

	chk_btn_use_user2 = new Fl_Check_Button(x + custom_width - ckbox_sqr, y - ckbox_sqr, ckbox_sqr, ckbox_sqr, "");
	chk_btn_use_user2->labelfont(font_nbr);
	chk_btn_use_user2->labelsize(font_size);
	chk_btn_use_user2->value(progStatus.ncs_user2_print);
	chk_btn_use_user2->callback((Fl_Callback*)cb_use_user2);

	max_w = x + custom_width + border;

	x = (border);
	y = (tmp + std_height + text_height + (border >> 1));

	choice_queue = new Fl_Choice(x, y, send_to_width, h, _("Queue"));
	choice_queue->align(FL_ALIGN_TOP_LEFT);
	choice_queue->labelfont(font_nbr);
	choice_queue->labelsize(font_size);
	choice_queue->callback((Fl_Callback*)cb_choice_queue);

	ncs_load_current_list();

	x = (border);
	y += (std_height + text_height + border);

	msg = (char *) _("Load STMs");
	load_button_width = (int) fl_width(msg);
	load_button_width += (border * 2);

	tmp = send_to_width - border - load_button_width;

	choice_send_to_op_msg = new Fl_Choice(x, y, tmp, h, _("Send To Message"));
	choice_send_to_op_msg->align(FL_ALIGN_TOP_LEFT);
	choice_send_to_op_msg->labelfont(font_nbr);
	choice_send_to_op_msg->labelsize(font_size);
	choice_send_to_op_msg->callback((Fl_Callback*)cb_choice_send_to_op_msg);

	chk_btn_send_to_op_msg = new Fl_Check_Button(x + tmp - ckbox_sqr, y - ckbox_sqr - 1, ckbox_sqr, ckbox_sqr, "");
	chk_btn_send_to_op_msg->labelfont(font_nbr);
	chk_btn_send_to_op_msg->labelsize(font_size);
	chk_btn_send_to_op_msg->value(progStatus.ncs_send_to_print);
	chk_btn_send_to_op_msg->callback((Fl_Callback*)cb_chk_btn_send_to_op_msg);

	btn_load_send_to_msg = new Fl_Button(x + tmp + border, y, load_button_width, h, msg);
	btn_load_send_to_msg->labelfont(font_nbr);
	btn_load_send_to_msg->labelsize(font_size);
	btn_load_send_to_msg->callback((Fl_Callback*)cb_btn_load_send_to_msg);

	button_end_y = y + std_height;

	int no_of_h_buttons = 2;
	int no_of_v_buttons = 3;

	button_width = max_w - button_start_x;
	button_width /= no_of_h_buttons;
	button_width -= border;

	button_height = button_end_y - button_start_y;
	button_height /= no_of_v_buttons;
	button_height -= border;

	x = button_start_x;
	y = button_start_y + border - 2;

	btn_prev = new Fl_Button(x, y, button_width, h, _("Prev"));
	btn_prev->labelfont(font_nbr);
	btn_prev->labelsize(font_size);
	btn_prev->callback((Fl_Callback*)cb_btn_prev);

	x += (button_width + border);

	btn_add = new Fl_Button(x, y, button_width, h, _("Add"));
	btn_add->labelfont(font_nbr);
	btn_add->labelsize(font_size);
	btn_add->callback((Fl_Callback*)cb_btn_add);
	
	x = button_start_x;
	y += (button_height + border);
	
	btn_next = new Fl_Button(x, y, button_width, h, _("Next"));
	btn_next->labelfont(font_nbr);
	btn_next->labelsize(font_size);
	btn_next->callback((Fl_Callback*)cb_btn_next);
	
	x += (button_width + border);
	
	btn_logout = new Fl_Button(x, y, button_width, h, _("Logout"));
	btn_logout->labelfont(font_nbr);
	btn_logout->labelsize(font_size);
	btn_logout->callback((Fl_Callback*)cb_btn_logout);
	
	x = button_start_x;
	y += (button_height + border);
	
	btn_send_to_msg = new Fl_Button(x, y, button_width, h, _("Send To"));
	btn_send_to_msg->labelfont(font_nbr);
	btn_send_to_msg->labelsize(font_size);
	btn_send_to_msg->callback((Fl_Callback*)cb_btn_send_to_msg);
	
	x += (button_width + border);
	
	btn_close = new Fl_Button(x, y, button_width, h, _("Close"));
	btn_close->labelfont(font_nbr);
	btn_close->labelsize(font_size);
	btn_close->callback((Fl_Callback*)cb_close_ncs);
	
	max_h = y + h + border;
	ncs_window->size(max_w, max_h);
	
	// Change the tab order
	ncs_window->insert(*input_call,  1);
	ncs_window->insert(*input_name,  2);
	ncs_window->insert(*input_qth,   3);
	ncs_window->insert(*input_state, 4);
	ncs_window->insert(*input_user1, 5);
	ncs_window->insert(*input_user2, 6);
	
	ncs_window->end();
	
	return ncs_window;
}
