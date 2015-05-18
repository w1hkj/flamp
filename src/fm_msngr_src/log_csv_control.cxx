// ----------------------------------------------------------------------------
// log_csv_control.cxx
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
#include "fm_msngr_src/log_csv_control.h"
#include "fm_msngr_src/log_csv_config.h"
#include "fm_msngr_src/call_table.h"

#include <string>
#include <list>
#include <vector>

#include "gettext.h"
#include "status.h"
#include "debug.h"

static Fl_Double_Window * window_csv_log_config = (Fl_Double_Window *) 0;

extern call_table *callTable;

/** *******************************************************************
 * \brief Arragne the data in the formart requested by the user.
 * \param csv_array storage for arranging the csv data sequence.
 * \return bool true okay / false not okay
 **********************************************************************/
static bool arrange_header(DATA_REF csv_array[])
{
	if(!csv_array) return false;

	int index = progStatus.log_call_pos - 1;

	csv_array[index].data.d_char_p = progStatus.log_call_tag.c_str();
	csv_array[index].use      = progStatus.log_call_use;
	csv_array[index].type     = n_char_p;

	index = progStatus.log_name_pos - 1;

	csv_array[index].data.d_char_p = progStatus.log_name_tag.c_str();
	csv_array[index].use      = progStatus.log_name_use;
	csv_array[index].type     = n_char_p;

	index = progStatus.log_qth_pos - 1;

	csv_array[index].data.d_char_p = progStatus.log_qth_tag.c_str();
	csv_array[index].use      = progStatus.log_qth_use;
	csv_array[index].type     = n_char_p;

	index = progStatus.log_state_pos - 1;

	csv_array[index].data.d_char_p = progStatus.log_state_tag.c_str();
	csv_array[index].use      = progStatus.log_state_use;
	csv_array[index].type     = n_char_p;

	index = progStatus.log_provence_pos - 1;

	csv_array[index].data.d_char_p = progStatus.log_provence_tag.c_str();
	csv_array[index].use      = progStatus.log_provence_use;
	csv_array[index].type     = n_char_p;

	index = progStatus.log_locator_pos - 1;

	csv_array[index].data.d_char_p = progStatus.log_locator_tag.c_str();
	csv_array[index].use      = progStatus.log_locator_use;
	csv_array[index].type     = n_char_p;

	index = progStatus.log_check_in_time_pos - 1;

	csv_array[index].data.d_char_p = progStatus.log_check_in_time_tag.c_str();
	csv_array[index].use      = progStatus.log_check_in_time_use;
	csv_array[index].type     = n_char_p;

	index = progStatus.log_check_out_time_pos - 1;

	csv_array[index].data.d_char_p = progStatus.log_check_out_time_tag.c_str();
	csv_array[index].use     = progStatus.log_check_out_time_use;
	csv_array[index].type    = n_char_p;

	index = progStatus.log_user1_pos - 1;

	csv_array[index].data.d_char_p = progStatus.log_user1_tag.c_str();
	csv_array[index].use     = progStatus.log_user1_use;
	csv_array[index].type    = n_char_p;

	index = progStatus.log_user2_pos - 1;

	csv_array[index].data.d_char_p = progStatus.log_user2_tag.c_str();
	csv_array[index].use     = progStatus.log_user2_use;
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
	csv_array[index].use  = progStatus.log_call_use;
	csv_array[index].type = n_char_p;

	index = progStatus.log_name_pos - 1;

	csv_array[index].data.d_char_p = _data->name.c_str();
	csv_array[index].use  = progStatus.log_name_use;
	csv_array[index].type = n_char_p;

	index = progStatus.log_qth_pos - 1;

	csv_array[index].data.d_char_p = _data->qth.c_str();
	csv_array[index].use  = progStatus.log_qth_use;
	csv_array[index].type = n_char_p;

	index = progStatus.log_state_pos - 1;

	csv_array[index].data.d_char_p =  _data->state.c_str();
	csv_array[index].use  = progStatus.log_state_use;
	csv_array[index].type = n_char_p;

	index = progStatus.log_provence_pos - 1;

	csv_array[index].data.d_char_p = _data->provence.c_str();
	csv_array[index].use  = progStatus.log_provence_use;
	csv_array[index].type = n_char_p;

	index = progStatus.log_locator_pos - 1;

	csv_array[index].data.d_char_p = _data->provence.c_str();
	csv_array[index].use  = progStatus.log_locator_use;
	csv_array[index].type = n_char_p;

	index = progStatus.log_check_in_time_pos - 1;

	csv_array[index].data.d_time = _data->check_in_time;
	csv_array[index].use  = progStatus.log_check_in_time_use;
	csv_array[index].type = n_time_t;

	index = progStatus.log_check_out_time_pos - 1;

	csv_array[index].data.d_time = _data->check_out_time;
	csv_array[index].use = progStatus.log_check_out_time_use;
	csv_array[index].type = n_time_t;

	index = progStatus.log_user1_pos - 1;

	csv_array[index].data.d_char_p = _data->user1.c_str();
	csv_array[index].use = progStatus.log_user1_use;
	csv_array[index].type = n_char_p;

	index = progStatus.log_user2_pos - 1;

	csv_array[index].data.d_char_p = _data->user2.c_str();
	csv_array[index].use = progStatus.log_user2_use;
	csv_array[index].type = n_char_p;

	return true;
}

/** *******************************************************************
 * \brief Conver time_t into string.
 * \param ttmp time struct tm data to comvert.
 * \param stmp Storage for the time string.
 * \return bool true okay / false not okay
 **********************************************************************/
void time_to_string(struct tm * ttmp, std::string &stmp, bool verbose, int flag)
{
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

	char time_buffer[128];
	memset(time_buffer, 0, sizeof(time_buffer));

	stmp.clear();


	if(((DATE_F | TIME_F | DAY_F) & flag) == 0)
		flag |= TIME_F;

	if(verbose) {
		if(flag & DAY_F) {
			stmp.append(day[ttmp->tm_wday]);
			if(flag & (TIME_F | DATE_F)) stmp.append(" ");
		}

		if(flag & DATE_F) {
			snprintf(time_buffer, sizeof(time_buffer)-1, "%02d %s %04d",
					 ttmp->tm_mday, month[ttmp->tm_mon], ttmp->tm_year + 1900);

			stmp.append(time_buffer);
			if(flag & TIME_F) stmp.append(" ");
		}

		if(flag & TIME_F) {
			snprintf(time_buffer, sizeof(time_buffer)-1, "%02d:%02d:%02d",
					 ttmp->tm_hour, ttmp->tm_min, ttmp->tm_sec);
			stmp.append(time_buffer);
		}
	} else {
		if(flag & DATE_F) {
			snprintf(time_buffer, sizeof(time_buffer)-1, "%02d/%02d/%04d",
					 ttmp->tm_mday, ttmp->tm_mon, ttmp->tm_year + 1900);

			stmp.append(time_buffer);
			if(flag & TIME_F) stmp.append(" ");
		}

		if(flag & TIME_F) {
			snprintf(time_buffer, sizeof(time_buffer)-1, "%02d:%02d:%02d",
					 ttmp->tm_hour, ttmp->tm_min, ttmp->tm_sec);
			stmp.append(time_buffer);
		}
	}

	if(flag & UTC_F)
		stmp.append("z");
}

/** *******************************************************************
 * \brief Check to see if quotes are needed.
 * \param stmp storage string.
 * \return void
 **********************************************************************/
void add_quotes(std::string &stmp)
{
	int count = 0;
	int index = 0;
	int delimiter = progStatus.log_csv_delimitor[0];
	bool q_flag = false;
	std::string tmp = "";

	count = stmp.size();

	for(index = 0; index < count; index++) {
		if(stmp[index] <= ' ') {
			q_flag = true;
			break;
		}
		if(stmp[index] == delimiter) {
			q_flag = true;
			break;
		}
	}

	if(q_flag) {
		tmp.assign("\"").append(stmp).append("\"");
		stmp.assign(tmp);
	}
}

/** *******************************************************************
 * \brief Convert raw data into a csv line data.
 * \param csv_array storage for arranging the csv data sequence.
 * \return bool true okay / false not okay
 **********************************************************************/
bool assemble_line(DATA_REF csv_array[], std::string &line, std::string delimiter, \
				   int dt_flag, bool q_flag)
{
	if(!csv_array) return false;
	std::vector<std::string> vtmp;
	std::string stmp;
	struct tm * ttmp;

	for(int index = 0; index < CSV_TAG_COUNT; index++) {

		if(!csv_array[index].use) continue;

		if(!stmp.empty()) stmp.clear();

		switch(csv_array[index].type) {
			case n_null:
			case n_char:
			case n_short:
			case n_int:
			case n_long:
			case n_void_p:
				// Not using these. Skip.
				continue;

			case n_bool:
				if(csv_array[index].data.d_bool)
					stmp.assign("TRUE");
				else
					stmp.assign("FALSE");
				break;

			case n_time_t:
				if(dt_flag & UTC_F)
					ttmp = gmtime((const time_t *) &csv_array[index].data.d_time);
				else
					ttmp = localtime((const time_t *) &csv_array[index].data.d_time);

				time_to_string(ttmp, stmp, false, dt_flag);

				break;

			case n_char_p:
				stmp.assign(csv_array[index].data.d_char_p);
				break;

			default:
				stmp.clear();
		}


		if(q_flag) add_quotes(stmp);
		vtmp.push_back(stmp);
	}

	int count = vtmp.size();
	int index = 0;
	line.clear();

	for(index = 0; index < (count-1); index++) {
		stmp = vtmp[index];
		line.append(stmp).append(delimiter);
	}

	if(count) {
		line.append(vtmp[index]).append("\n");
	}

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

	if(!arrange_header(csv_array))
		return false;

	dt_flag = (progStatus.log_time_utc_local ? UTC_F : 0);
	dt_flag |= (DATE_F|TIME_F);

	if(!assemble_line(csv_array, tmp, delimiter, dt_flag, true))
		return false;

	storage.assign(tmp);

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

		assemble_line(csv_array, tmp, delimiter, dt_flag, true);
		storage.append(tmp);

		if(!callTable->next(handle))
			break;

	} while(1);

	callTable->delete_handle(handle);

	delete _data;

	return true;
}

/** *******************************************************************
 * \brief Write CSV log to the tx panel.
 * \return void
 **********************************************************************/
void write_csv_output_tx_panel(void)
{
	std::string storage;

	build_csv_list(storage, progStatus.log_csv_delimitor);
	fm_append_tx(storage);
}

/** *******************************************************************
 * \brief Write CSV Log to a file.
 * \return void
 **********************************************************************/
void write_csv_output_to_file(void)
{

}

/** *******************************************************************
 * \brief Open csv log configuration window.
 * \return void
 **********************************************************************/
void open_csv_log_configure(void)
{
	if(!window_csv_log_config) {
		window_csv_log_config = create_window_csv_log_config();
		if(window_csv_log_config) {
			int x = (window_frame->x_root() + ((window_frame->w() - window_csv_log_config->w()) * 0.50));
			int y = (window_frame->y_root() + ((window_frame->h() - window_csv_log_config->h()) * 0.50));
			int h = window_csv_log_config->h();
			int w = window_csv_log_config->w();
			window_csv_log_config->resize(x, y, w, h);
		}
	}
	
	if(!window_csv_log_config) {
		LOG_INFO("%s", "Open csv log configuration window");
		return;
	}
	
	window_csv_log_config->show();
}

/** *******************************************************************
 * \brief Open csv log configuration window.
 * \return void
 **********************************************************************/
void close_csv_log_configure(void)
{
	if(window_csv_log_config)
		window_csv_log_config->hide();
}

