// ----------------------------------------------------------------------------
// display_list_control.cxx
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


#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Input.H>

#include "fm_msngr_src/fm_control.h"
#include "fm_msngr_src/fm_dialog.h"
#include "fm_msngr_src/display_list_control.h"
#include "fm_msngr_src/display_list_config.h"
//#include "fm_msngr_src/log_csv_control.h"
//#include "fm_msngr_src/log_csv_config.h"
#include "fm_msngr_src/call_table.h"

#include <string>
#include <list>
#include <vector>

#include "gettext.h"
#include "status.h"
#include "debug.h"

static Fl_Double_Window * window_display_log_config = (Fl_Double_Window *) 0;

extern call_table *callTable;

extern void time_to_string(struct tm * ttmp, std::string &stmp, bool verbose, int flag);
extern void add_quotes(std::string &stmp);
extern bool assemble_line(DATA_REF csv_array[], std::string &line, std::string delimiter, \
						  int flag, bool q_flag);

static bool build_csv_list(std::string &storage, std::string delimiter);

/** *******************************************************************
 * \brief Arragne the data in the formart requested by the user.
 * \param csv_array storage for arranging the csv data sequence.
 * \return bool true okay / false not okay
 **********************************************************************/
static bool arrange_header(DATA_REF csv_array[], std::string delimiter)
{
	if(!csv_array) return false;

	int index = progStatus.display_call_pos - 1;

	csv_array[index].data.d_char_p = progStatus.display_call_tag.c_str();
	csv_array[index].use      = progStatus.display_call_use;
	csv_array[index].type     = n_char_p;

	index = progStatus.display_name_pos - 1;

	csv_array[index].data.d_char_p = progStatus.display_name_tag.c_str();
	csv_array[index].use      = progStatus.display_name_use;
	csv_array[index].type     = n_char_p;

	index = progStatus.display_qth_pos - 1;

	csv_array[index].data.d_char_p = progStatus.display_qth_tag.c_str();
	csv_array[index].use      = progStatus.display_qth_use;
	csv_array[index].type     = n_char_p;

	index = progStatus.display_state_pos - 1;

	csv_array[index].data.d_char_p = progStatus.display_state_tag.c_str();
	csv_array[index].use      = progStatus.display_state_use;
	csv_array[index].type     = n_char_p;

	index = progStatus.display_provence_pos - 1;

	csv_array[index].data.d_char_p = progStatus.display_provence_tag.c_str();
	csv_array[index].use      = progStatus.display_provence_use;
	csv_array[index].type     = n_char_p;

	index = progStatus.display_locator_pos - 1;

	csv_array[index].data.d_char_p = progStatus.display_locator_tag.c_str();
	csv_array[index].use      = progStatus.display_locator_use;
	csv_array[index].type     = n_char_p;

	index = progStatus.display_check_in_time_pos - 1;

	csv_array[index].data.d_char_p = progStatus.display_check_in_time_tag.c_str();
	csv_array[index].use      = progStatus.display_check_in_time_use;
	csv_array[index].type     = n_char_p;

	index = progStatus.display_check_out_time_pos - 1;

	csv_array[index].data.d_char_p = progStatus.display_check_out_time_tag.c_str();
	csv_array[index].use     = progStatus.display_check_out_time_use;
	csv_array[index].type    = n_char_p;

	index = progStatus.display_user1_pos - 1;

	csv_array[index].data.d_char_p = progStatus.display_user1_tag.c_str();
	csv_array[index].use     = progStatus.display_user1_use;
	csv_array[index].type    = n_char_p;

	index = progStatus.display_user2_pos - 1;

	csv_array[index].data.d_char_p = progStatus.display_user2_tag.c_str();
	csv_array[index].use     = progStatus.display_user2_use;
	csv_array[index].type    = n_char_p;

	return true;
}

/** *******************************************************************
 * \brief Arragne the data in the formart requested by the user.
 * \param csv_array storage for arranging the csv data sequence.
 * \return bool true okay / false not okay
 **********************************************************************/
static bool arrange_data(DATA_REF csv_array[],	CALL_TABLE *_data)
{
	if(!csv_array || !_data) return false;

	int index = progStatus.log_call_pos - 1;

	csv_array[index].data.d_char_p = _data->call.c_str();
	csv_array[index].use  = progStatus.display_call_use;
	csv_array[index].type = n_char_p;

	index = progStatus.log_name_pos - 1;

	csv_array[index].data.d_char_p = _data->name.c_str();
	csv_array[index].use  = progStatus.display_name_use;
	csv_array[index].type = n_char_p;

	index = progStatus.log_qth_pos - 1;

	csv_array[index].data.d_char_p = _data->qth.c_str();
	csv_array[index].use  = progStatus.display_qth_use;
	csv_array[index].type = n_char_p;

	index = progStatus.log_state_pos - 1;

	csv_array[index].data.d_char_p =  _data->state.c_str();
	csv_array[index].use  = progStatus.display_state_use;
	csv_array[index].type = n_char_p;

	index = progStatus.log_provence_pos - 1;

	csv_array[index].data.d_char_p = _data->provence.c_str();
	csv_array[index].use  = progStatus.display_provence_use;
	csv_array[index].type = n_char_p;

	index = progStatus.log_locator_pos - 1;

	csv_array[index].data.d_char_p = _data->provence.c_str();
	csv_array[index].use  = progStatus.display_locator_use;
	csv_array[index].type = n_char_p;

	index = progStatus.log_check_in_time_pos - 1;

	csv_array[index].data.d_time = _data->check_in_time;
	csv_array[index].use  = progStatus.display_check_in_time_use;
	csv_array[index].type = n_time_t;

	index = progStatus.log_check_out_time_pos - 1;

	csv_array[index].data.d_time = _data->check_out_time;
	csv_array[index].use = progStatus.display_check_out_time_use;
	csv_array[index].type = n_time_t;

	index = progStatus.log_user1_pos - 1;

	csv_array[index].data.d_char_p = _data->user1.c_str();
	csv_array[index].use = progStatus.display_user1_use;
	csv_array[index].type = n_char_p;

	index = progStatus.log_user2_pos - 1;

	csv_array[index].data.d_char_p = _data->user2.c_str();
	csv_array[index].use = progStatus.display_user2_use;
	csv_array[index].type = n_char_p;

	return true;
}

/** *******************************************************************
 * \brief Create the CSV list from call table data.
 * \param storage converted to csv data string storage.
 * \return bool true data present in the storage string otherwise
 * data invalid.
 **********************************************************************/
static bool build_csv_list(std::string &storage, std::string delimiter)
{
	if(!callTable) return false;
	void * handle = (void *)0;
	DATA_REF csv_array[CSV_TAG_COUNT];
	CALL_TABLE *_data;
	std::string tmp = "";
	int dt_flag = 0;

	handle = callTable->handle();

	if(!handle)
		return false;

	tmp.clear();

	if(progStatus.enable_display_list_header) {
		storage.assign("\n").append(progStatus.display_list_header).append("\n");
	}

	if(progStatus.enable_display_list_names)
		if(!arrange_header(csv_array, delimiter))
			return false;

	dt_flag = ((progStatus.display_date ? DATE_F : 0) \
			   | (progStatus.display_time ? TIME_F : 0) \
			   | (progStatus.display_time_utc_local ? UTC_F : 0));

	if(!assemble_line(csv_array, tmp, delimiter, dt_flag, false))
		return false;

	storage.append(tmp);

	if(!callTable->top(handle)) {
		callTable->delete_handle(handle);
		return false;
	}

	_data = new CALL_TABLE;

	if(!_data) return false;

	do {
		memset(csv_array, 0, sizeof(csv_array));

		if(!callTable->read(handle, _data))
			break;

		if(!arrange_data(csv_array, _data))
			break;

		assemble_line(csv_array, tmp, delimiter, dt_flag, false);
		storage.append(tmp);

		if(!callTable->next(handle))
			break;

	} while(1);

	callTable->delete_handle(handle);

	delete _data;

	return true;
}

/** *******************************************************************
 * \brief Write list log to the tx panel.
 * \return void
 **********************************************************************/
void write_list_output_tx_panel(void)
{
	std::string storage;

	build_csv_list(storage, "  ");
	fm_append_tx(storage);
}

/** *******************************************************************
 * \brief Open csv log configuration window.
 * \return void
 **********************************************************************/
void open_display_list_configure(void)
{
	if(!window_display_log_config) {
		window_display_log_config = create_window_display_list_config();
		if(window_display_log_config) {
			int x = (window_frame->x_root() + ((window_frame->w() - window_display_log_config->w()) * 0.50));
			int y = (window_frame->y_root() + ((window_frame->h() - window_display_log_config->h()) * 0.50));
			int h = window_display_log_config->h();
			int w = window_display_log_config->w();
			window_display_log_config->resize(x, y, w, h);
		}
	}

	if(!window_display_log_config) {
		LOG_INFO("%s", "Open csv log configuration window");
		return;
	}

	window_display_log_config->show();
}

/** *******************************************************************
 * \brief Open display log configuration window.
 * \return void
 **********************************************************************/
void close_display_log_configure(void)
{
	if(window_display_log_config)
		window_display_log_config->hide();
}

