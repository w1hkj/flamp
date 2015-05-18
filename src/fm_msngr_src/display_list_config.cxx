// ----------------------------------------------------------------------------
// display_list_config.cxx
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

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_File_Chooser.H>
#include "fm_msngr_src/display_list_config.h"
#include "fm_msngr_src/display_list_control.h"
#include "fm_msngr_src/log_csv_control.h"
#include "fm_msngr_src/log_csv_config.h"
#include "fm_msngr_src/call_table.h"
#include "gettext.h"
#include "status.h"

static Fl_Double_Window * window_display_list_config = (Fl_Double_Window *) 0;

static Fl_Output *enable_label   = (Fl_Output *) 0;
static Fl_Output *tag_name_label = (Fl_Output *) 0;
static Fl_Output *tag_pos_label  = (Fl_Output *) 0;

static Fl_Check_Button * ck_btn_enable_field[CSV_TAG_COUNT+1] = {0};
static Fl_Input_Choice * input_choice_field_pos[CSV_TAG_COUNT+1] = {0};
static Fl_Input        * input_text_field_tag[CSV_TAG_COUNT+1] = {0};
static Fl_Button       * bth_close = (Fl_Button *) 0;
static Fl_Check_Button * ck_btn_time_utc_local = (Fl_Check_Button *) 0;
static Fl_Check_Button * ck_btn_enable_list_header = (Fl_Check_Button *) 0;
static Fl_Check_Button * ck_btn_enable_display_list_names = (Fl_Check_Button *) 0;
static Fl_Input        * input_display_list_header = (Fl_Input *) 0;
static Fl_Check_Button * ck_btn_time = (Fl_Check_Button *) 0;
static Fl_Check_Button * ck_btn_date = (Fl_Check_Button *) 0;

static bool item_use[CSV_TAG_COUNT + 1] = {0};
static char * item_name[CSV_TAG_COUNT + 1] = {0};
static char * tag_name[CSV_TAG_COUNT + 1]  = {0};
static int tag_pos[CSV_TAG_COUNT + 1]      = {0};

/** *******************************************************************
 * \brief Set user one tag name
 * \return void
 **********************************************************************/
void set_display_user1_tag_name(std::string tag)
{
	if(tag.empty()) return;
	if(input_text_field_tag[user1_index])
		input_text_field_tag[user1_index]->value(tag.c_str());
	progStatus.display_user1_tag = tag;

}

/** *******************************************************************
 * \brief Set user two tag name
 * \return void
 **********************************************************************/
void set_display_user2_tag_name(std::string tag)
{
	if(tag.empty()) return;
	if(input_text_field_tag[user2_index])
		input_text_field_tag[user2_index]->value(tag.c_str());
	progStatus.display_user2_tag = tag;
}

/** *******************************************************************
 * \brief Verify there are no duplicate tag positions.
 * \return void
 **********************************************************************/
static bool check_duplicate_tag_pos(void)
{
	int _flags[CSV_TAG_COUNT];
	char _buf[4];
	bool _err_flag = false;

	std::string _msg = "";
	std::string _missing = "";
	std::string _duplicate = "";

	memset(_flags, 0, sizeof(_flags));

	_flags[progStatus.display_call_pos - 1]++;
	_flags[progStatus.display_name_pos - 1]++;
	_flags[progStatus.display_qth_pos - 1]++;
	_flags[progStatus.display_state_pos - 1]++;
	_flags[progStatus.display_provence_pos - 1]++;
	_flags[progStatus.display_locator_pos - 1]++;
	_flags[progStatus.display_check_in_time_pos - 1]++;
	_flags[progStatus.display_check_out_time_pos - 1]++;
	_flags[progStatus.display_user1_pos - 1]++;
	_flags[progStatus.display_user2_pos - 1]++;

	_buf[1] = 0;

	for(int i = 0; i < CSV_TAG_COUNT; i++) {
		sprintf(_buf, "%d", i+1);
		if(_flags[i] < 1) {
			_missing.append(_buf).append(" ");
			_err_flag = true;
		} else if(_flags[i] > 1) {
			_duplicate.append(_buf).append(" ");
			_err_flag = true;
		}
	}

	if(_err_flag) {
		_msg.assign(_("Missing or Duplicate Entries\n"));
		_msg.append(_("Missing")).append(" ").append(_missing).append("\n");
		_msg.append(_("Duplicate")).append(" ").append(_duplicate).append("\n");

		_err_flag = (bool) fl_choice("%s", _("Okay"), _("Not Okay"), NULL, _msg.c_str());
	}

	return _err_flag;
}

/** *******************************************************************
 * \brief Create CSV log configuration window.
 * \return void
 **********************************************************************/
static void cb_input_text_field_tag(Fl_Input *a, void *b)
{
	if(!a) return;

	int index = (int) a->user_data();
	const char *str = a->value();

	if(!str) return;

	switch(index) {
		case call_index:
			progStatus.display_call_tag = str;
			break;

		case name_index:
			progStatus.display_name_tag = str;
			break;

		case qth_index:
			progStatus.display_qth_tag = str;
			break;

		case state_index:
			progStatus.display_state_tag = str;
			break;

		case prov_index:
			progStatus.display_provence_tag = str;
			break;

		case loc_index:
			progStatus.display_locator_tag = str;
			break;

		case check_in_index:
			progStatus.display_check_in_time_tag = str;
			break;

		case check_out_index:
			progStatus.display_check_out_time_tag = str;
			break;

		case user1_index:
			progStatus.display_user1_tag = str;
			break;

		case user2_index:
			progStatus.display_user2_tag = str;
			break;
	}
}

/** *******************************************************************
 * \brief Swap position data to maintain individuality.
 * \return void
 **********************************************************************/
static void swap_pos_values(int index, int val, int oval)
{
	if((index != call_index) && (progStatus.display_call_pos == val)) {
		input_choice_field_pos[call_index]->value(oval - 1);
		progStatus.display_call_pos = oval;
		return;
	}

	if((index != name_index) && (progStatus.display_name_pos == val)) {
		input_choice_field_pos[name_index]->value(oval - 1);
		progStatus.display_name_pos = oval;
		return;
	}

	if((index != qth_index) && (progStatus.display_qth_pos == val)) {
		input_choice_field_pos[qth_index]->value(oval - 1);
		progStatus.display_qth_pos = oval;
		return;
	}

	if((index != state_index) && (progStatus.display_state_pos == val)) {
		input_choice_field_pos[state_index]->value(oval - 1);
		progStatus.display_state_pos = oval;
		return;
	}

	if((index != prov_index) && (progStatus.display_provence_pos == val)) {
		input_choice_field_pos[prov_index]->value(oval - 1);
		progStatus.display_provence_pos = oval;
		return;
	}

	if((index != loc_index) && (progStatus.display_locator_pos == val)) {
		input_choice_field_pos[loc_index]->value(oval - 1);
		progStatus.display_locator_pos = oval;
		return;
	}

	if((index != check_in_index) && (progStatus.display_check_in_time_pos == val)) {
		input_choice_field_pos[check_in_index]->value(oval - 1);
		progStatus.display_check_in_time_pos = oval;
		return;
	}

	if((index != check_out_index) && (progStatus.display_check_out_time_pos == val)) {
		input_choice_field_pos[check_out_index]->value(oval - 1);
		progStatus.display_check_out_time_pos = oval;
		return;
	}

	if((index != user1_index) && (progStatus.display_user1_pos == val)) {
		input_choice_field_pos[user1_index]->value(oval - 1);
		progStatus.display_user1_pos = oval;
		return;
	}

	if((index != user2_index) && (progStatus.display_user2_pos == val)) {
		input_choice_field_pos[user2_index]->value(oval - 1);
		progStatus.display_user2_pos = oval;
		return;
	}

}

/** *******************************************************************
 * \brief Create CSV log configuration window.
 * \return void
 **********************************************************************/
static void cb_input_choice_call_pos(Fl_Input_Choice *a, void *b)
{
	if(!a) return;

	Fl::lock();
	int index = (int) a->user_data();
	const char *str = a->value();

	int val = 0;
	int oval = 0;

	if(!str) {
		Fl::unlock();
		return;
	}

	val = atoi(str);

	if((val > 0) && (val <= max_item_index)) {
		switch(index) {
			case call_index:
				oval = progStatus.display_call_pos;
				progStatus.display_call_pos = val;
				break;

			case name_index:
				oval = progStatus.display_name_pos;
				progStatus.display_name_pos = val;
				break;

			case qth_index:
				oval = progStatus.display_qth_pos;
				progStatus.display_qth_pos = val;
				break;

			case state_index:
				oval = progStatus.display_state_pos;
				progStatus.display_state_pos = val;
				break;

			case prov_index:
				oval = progStatus.display_provence_pos;
				progStatus.display_provence_pos = val;
				break;

			case loc_index:
				oval = progStatus.display_locator_pos;
				progStatus.display_locator_pos = val;
				break;

			case check_in_index:
				oval = progStatus.display_check_in_time_pos;
				progStatus.display_check_in_time_pos = val;
				break;

			case check_out_index:
				oval = progStatus.display_check_out_time_pos;
				progStatus.display_check_out_time_pos = val;
				break;

			case user1_index:
				oval = progStatus.display_user1_pos;
				progStatus.display_user1_pos = val;
				break;

			case user2_index:
				oval = progStatus.display_user2_pos;
				progStatus.display_user2_pos = val;
				break;
		}

		swap_pos_values(index, val, oval);
	}

	Fl::unlock();
}

/** *******************************************************************
 * \brief Create CSV log configuration window.
 * \return void
 **********************************************************************/
static void cb_ck_btn_enable_field(Fl_Check_Button *a, void *b)
{
	if(!a) return;

	int index = (int) a->user_data();
	bool val = a->value();

	switch(index) {
		case call_index:
			progStatus.display_call_use = val;
			break;

		case name_index:
			progStatus.display_name_use = val;
			break;

		case qth_index:
			progStatus.display_qth_use = val;
			break;

		case state_index:
			progStatus.display_state_use = val;
			break;

		case prov_index:
			progStatus.display_provence_use = val;
			break;

		case loc_index:
			progStatus.display_locator_use = val;
			break;

		case check_in_index:
			progStatus.display_check_in_time_use = val;
			break;

		case check_out_index:
			progStatus.display_check_out_time_use = val;
			break;

		case user1_index:
			progStatus.display_user1_use = val;
			break;

		case user2_index:
			progStatus.display_user2_use = val;
			break;
	}
}

/** *******************************************************************
 * \brief UTC/Local time selection callback.
 * \return void
 **********************************************************************/
static void cb_ck_btn_display_time_utc_local(Fl_Check_Button *a, void *b)
{
	progStatus.display_time_utc_local = a->value() ? true : false;
}

/** *******************************************************************
 * \brief Enable Header message callback
 * \return void
 **********************************************************************/
static void cb_enable_display_list_header(Fl_Check_Button *a, void *b)
{
	progStatus.enable_display_list_header = a->value();
}

/** *******************************************************************
 * \brief Display list header callback.
 * \return void
 **********************************************************************/
static void cb_input_display_list_header(Fl_Input *a, void *b)
{
	progStatus.display_list_header = a->value();
}

/** *******************************************************************
 * \brief Display list header callback.
 * \return void
 **********************************************************************/
static void cb_enable_display_list_names(Fl_Check_Button *a, void *b)
{
	progStatus.enable_display_list_names = a->value() ? true : false;
}

/** *******************************************************************
 * \brief Display list include date callback.
 * \return void
 **********************************************************************/
static void cb_ck_btn_display_date(Fl_Check_Button *a, void *b)
{
	progStatus.display_date = a->value() ? true : false;
}

/** *******************************************************************
 * \brief Display list include date callback.
 * \return void
 **********************************************************************/
static void cb_ck_btn_display_time(Fl_Check_Button *a, void *b)
{
	progStatus.display_time = a->value() ? true : false;
}

/** *******************************************************************
 * \brief Initialize index arrays.
 * \return void
 **********************************************************************/
static void assign_data(void)
{
	item_name[call_index]      = (char *) CALL_TAG;
	item_name[name_index]      = (char *) NAME_TAG;
	item_name[qth_index]       = (char *) QTH_TAG;
	item_name[state_index]     = (char *) STATE_TAG;
	item_name[prov_index]      = (char *) PROV_TAG;
	item_name[loc_index]       = (char *) LOC_TAG;
	item_name[check_in_index]  = (char *) CHECK_IN_TAG;
	item_name[check_out_index] = (char *) CHECK_OUT_TAG;
	item_name[user1_index]     = (char *) USER1_TAG;
	item_name[user2_index]     = (char *) USER2_TAG;

	item_use[call_index]       = progStatus.display_call_use;
	item_use[name_index]       = progStatus.display_name_use;
	item_use[qth_index]        = progStatus.display_qth_use;
	item_use[state_index]      = progStatus.display_state_use;
	item_use[prov_index]       = progStatus.display_provence_use;
	item_use[loc_index]        = progStatus.display_locator_use;
	item_use[check_in_index]   = progStatus.display_check_in_time_use;
	item_use[check_out_index]  = progStatus.display_check_out_time_use;
	item_use[user1_index]      = progStatus.display_user1_use;
	item_use[user2_index]      = progStatus.display_user2_use;

	tag_pos[call_index]        = progStatus.display_call_pos;
	tag_pos[name_index]        = progStatus.display_name_pos;
	tag_pos[qth_index]         = progStatus.display_qth_pos;
	tag_pos[state_index]       = progStatus.display_state_pos;
	tag_pos[prov_index]        = progStatus.display_provence_pos;
	tag_pos[loc_index]         = progStatus.display_locator_pos;
	tag_pos[check_in_index]    = progStatus.display_check_in_time_pos;
	tag_pos[check_out_index]   = progStatus.display_check_out_time_pos;
	tag_pos[user1_index]       = progStatus.display_user1_pos;
	tag_pos[user2_index]       = progStatus.display_user2_pos;

	tag_name[call_index]       = (char *) progStatus.display_call_tag.c_str();
	tag_name[name_index]       = (char *) progStatus.display_name_tag.c_str();
	tag_name[qth_index]        = (char *) progStatus.display_qth_tag.c_str();
	tag_name[state_index]      = (char *) progStatus.display_state_tag.c_str();
	tag_name[prov_index]       = (char *) progStatus.display_provence_tag.c_str();
	tag_name[loc_index]        = (char *) progStatus.display_locator_tag.c_str();
	tag_name[check_in_index]   = (char *) progStatus.display_check_in_time_tag.c_str();
	tag_name[check_out_index]  = (char *) progStatus.display_check_out_time_tag.c_str();
	tag_name[user1_index]      = (char *) progStatus.display_user1_tag.c_str();
	tag_name[user2_index]      = (char *) progStatus.display_user2_tag.c_str();

}

/** *******************************************************************
 * \brief Create CSV log configuration window.
 * \return void
 **********************************************************************/
static void cb_close_window(Fl_Widget *a, void *b)
{
	if(!check_duplicate_tag_pos())
		close_display_log_configure();
}

/** *******************************************************************
 * \brief Create CSV log configuration window.
 * \return void
 **********************************************************************/
Fl_Double_Window * create_window_display_list_config(void)
{
	if(window_display_list_config)
		return window_display_list_config;

	int w_h = 370;
	int w_w = 512;
	int btn_close_w = 100;
	int btn_close_h = 26;
	int os = 8;
	int font_nbr = 0;
	int font_size = 12;
	int y_spacing = font_size + os;
	int x = 0;
	int y = 0;
	int ic_w = 100;
	int ic_h = y_spacing;
	int ic_x = 0;
	int i_w = 100;
	int i_h = y_spacing;
	int i_x = 0;
	int ck_x = 0;
	int ck_h = y_spacing;
	int ck_w = 19;
	int index = 0;
	int prev_font_nbr = 0;
	int prev_font_size = 12;
	int data_width = 0;
	int tmp = 0;
	int dummy = 0;
	char *str = (char *) 0;
	char buf[4];

	buf[1] = 0;

	assign_data();

	window_display_list_config = new Fl_Double_Window(w_w, w_h, _("Display List Configuration"));
	window_display_list_config->callback((Fl_Callback*)cb_close_window);

	prev_font_nbr = fl_font();
	prev_font_size = fl_size();

	fl_font(font_nbr, font_size);

	for(index = 0; index < CSV_TAG_COUNT; index++) {
		str = (char *) item_name[index];
		fl_text_extents((const char *)str, dummy, dummy, tmp, dummy);
		if(tmp > data_width)
			data_width = tmp;
	}

	ck_x = os;
	i_x = ck_x + ck_w + os;
	ic_x = i_x + i_w + os;

	y = font_size;

	ck_btn_enable_display_list_names = new Fl_Check_Button(ck_x, y, ck_w, ck_h, "");
	ck_btn_enable_display_list_names->labeltype(FL_NORMAL_LABEL);
	ck_btn_enable_display_list_names->align(FL_ALIGN_RIGHT);
	ck_btn_enable_display_list_names->labelfont(font_nbr);
	ck_btn_enable_display_list_names->labelsize(font_size);
	ck_btn_enable_display_list_names->labelcolor(FL_FOREGROUND_COLOR);
	ck_btn_enable_display_list_names->value(progStatus.enable_display_list_names);
	ck_btn_enable_display_list_names->callback((Fl_Callback *) cb_enable_display_list_names);
	ck_btn_enable_display_list_names->value(progStatus.enable_display_list_header);
	ck_btn_enable_display_list_names->callback((Fl_Callback *) cb_enable_display_list_header);


	input_display_list_header = new Fl_Input(i_x, y, (i_w * 2) + os, i_h, _("List Hdr"));
	input_display_list_header->labeltype(FL_NORMAL_LABEL);
	input_display_list_header->align(FL_ALIGN_RIGHT);
	input_display_list_header->labelfont(font_nbr);
	input_display_list_header->labelsize(font_size);
	input_display_list_header->labelcolor(FL_FOREGROUND_COLOR);
	input_display_list_header->value(progStatus.display_list_header.c_str());
	input_display_list_header->callback((Fl_Callback *) cb_input_display_list_header);


	y += (y_spacing + (os >> 1));

	ck_btn_enable_list_header = new Fl_Check_Button(ck_x, y, ck_w, ck_h, _("Enable List Names"));
	ck_btn_enable_list_header->labeltype(FL_NORMAL_LABEL);
	ck_btn_enable_list_header->align(FL_ALIGN_RIGHT);
	ck_btn_enable_list_header->labelfont(font_nbr);
	ck_btn_enable_list_header->labelsize(font_size);
	ck_btn_enable_list_header->labelcolor(FL_FOREGROUND_COLOR);
	ck_btn_enable_list_header->value(progStatus.enable_display_list_names);
	ck_btn_enable_list_header->callback((Fl_Callback *) cb_enable_display_list_names);

	y += (y_spacing + (os >> 1));

	enable_label = new Fl_Output(ck_x, y, ck_w, ck_h, "");
	enable_label->value("E");
	enable_label->set_output();
	enable_label->box(FL_NO_BOX);

	tag_name_label = new Fl_Output(i_x, y, i_w, i_h, "");
	tag_name_label->value(_("Tag Name"));
	tag_name_label->set_output();
	tag_name_label->box(FL_NO_BOX);

	tag_pos_label = new Fl_Output(ic_x, y, ic_w, ic_h, "");
	tag_pos_label->value(_("Write Position"));
	tag_pos_label->set_output();
	tag_pos_label->box(FL_NO_BOX);

	y += (y_spacing + (os >> 1));

	for(index = 0; index < CSV_TAG_COUNT; index++) {

		ck_btn_enable_field[index] = new Fl_Check_Button(ck_x, y, ck_w, ck_h, "");
		ck_btn_enable_field[index]->down_box(FL_DOWN_BOX);
		ck_btn_enable_field[index]->labeltype(FL_NORMAL_LABEL);
		ck_btn_enable_field[index]->labelfont(font_nbr);
		ck_btn_enable_field[index]->labelsize(font_size);
		ck_btn_enable_field[index]->labelcolor(FL_FOREGROUND_COLOR);
		ck_btn_enable_field[index]->user_data((void *) index);
		ck_btn_enable_field[index]->value(item_use[index]);
		ck_btn_enable_field[index]->callback((Fl_Callback *) cb_ck_btn_enable_field);

		str = (char *) tag_name[index];

		input_text_field_tag[index] = new Fl_Input(i_x, y, i_w, i_h, "");
		input_text_field_tag[index]->labeltype(FL_NORMAL_LABEL);
		input_text_field_tag[index]->labelfont(font_nbr);
		input_text_field_tag[index]->labelsize(font_size);
		input_text_field_tag[index]->labelcolor( FL_FOREGROUND_COLOR);
		input_text_field_tag[index]->value(str);
		input_text_field_tag[index]->user_data((void *) index);
		input_text_field_tag[index]->callback((Fl_Callback *) cb_input_text_field_tag);

		str = (char *) item_name[index];

		input_choice_field_pos[index] = new Fl_Input_Choice(ic_x, y, ic_w, ic_h, str);
		input_choice_field_pos[index]->labeltype(FL_NORMAL_LABEL);
		input_choice_field_pos[index]->labelfont(font_nbr);
		input_choice_field_pos[index]->labelsize(font_size);
		input_choice_field_pos[index]->labelcolor(FL_FOREGROUND_COLOR);
		input_choice_field_pos[index]->align(FL_ALIGN_RIGHT);

		for(int i = 0; i < CSV_TAG_COUNT; i++) {
			sprintf(buf, "%d", i + 1);
			input_choice_field_pos[index]->add(buf);
		}

		input_choice_field_pos[index]->value(tag_pos[index] - 1);
		input_choice_field_pos[index]->user_data((void *) index);
		input_choice_field_pos[index]->callback((Fl_Callback *) cb_input_choice_call_pos);

		y += (y_spacing + (os >> 1));
	}

	tmp = ((i_w * 2) / 3);

	ck_btn_time_utc_local = new Fl_Check_Button(i_x, y, ck_w, ck_h, _("UTC Time"));
	ck_btn_time_utc_local->down_box(FL_DOWN_BOX);
	ck_btn_time_utc_local->labeltype(FL_NORMAL_LABEL);
	ck_btn_time_utc_local->labelfont(font_nbr);
	ck_btn_time_utc_local->labelsize(font_size);
	ck_btn_time_utc_local->labelcolor(FL_FOREGROUND_COLOR);
	ck_btn_time_utc_local->value(progStatus.display_time_utc_local);
	ck_btn_time_utc_local->callback((Fl_Callback *) cb_ck_btn_display_time_utc_local);

	ck_btn_time = new Fl_Check_Button((tmp * 2) + os, y, ck_w, ck_h, _("Time"));
	ck_btn_time->down_box(FL_DOWN_BOX);
	ck_btn_time->labeltype(FL_NORMAL_LABEL);
	ck_btn_time->labelfont(font_nbr);
	ck_btn_time->labelsize(font_size);
	ck_btn_time->labelcolor(FL_FOREGROUND_COLOR);
	ck_btn_time->value(progStatus.display_time);
	ck_btn_time->callback((Fl_Callback *) cb_ck_btn_display_time);
	
	ck_btn_date = new Fl_Check_Button((tmp * 3) + os, y, ck_w, ck_h, _("Date"));
	ck_btn_date->down_box(FL_DOWN_BOX);
	ck_btn_date->labeltype(FL_NORMAL_LABEL);
	ck_btn_date->labelfont(font_nbr);
	ck_btn_date->labelsize(font_size);
	ck_btn_date->labelcolor(FL_FOREGROUND_COLOR);
	ck_btn_date->value(progStatus.display_date);
	ck_btn_date->callback((Fl_Callback *) cb_ck_btn_display_date);
	
	y += (y_spacing + (os >> 1));
	
	w_w = ic_x + ic_w + os + data_width + os;
	w_h = y + btn_close_h + os;
	
	x = w_w - btn_close_w - os;
	y = w_h - btn_close_h - os;
	
	bth_close = new Fl_Button(x, y, btn_close_w, btn_close_h, _("Close"));
	bth_close->callback((Fl_Callback*)cb_close_window);
	
	window_display_list_config->end();
	
	window_display_list_config->resize(0, 0, w_w, w_h);
	
	fl_font(prev_font_nbr, prev_font_size);
	
	return window_display_list_config;
}
