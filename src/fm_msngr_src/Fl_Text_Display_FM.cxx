// ----------------------------------------------------------------------------
// Fl_Text_Display_FM.cxx
//
// Copyright (C) 2015  Robert Stiles, KK5VD
//
// This file is part of fldigi.
//
// Fldigi is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Fldigi is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fldigi.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------

#include <FL/Fl_Menu_Item.H>
#include "fm_msngr_src/Fl_Text_Display_FM.h"
#include "fm_msngr_src/display_list_config.h"
#include "fm_msngr_src/fm_ncs_config.h"
#include "status.h"

#define FONT_SIZE 12


void Copy_CB(Fl_Widget*a, void *b)
{
	Fl::lock();
	Fl_Text_Display *_w      = (Fl_Text_Display *) b;
	if(!b) return;
	Fl_Text_Buffer *_buf     = (Fl_Text_Buffer *)  _w->buffer();
	if(_buf) {
		if (_buf->selected()) {
			std::string copy_data;
			copy_data.assign(_buf->selection_text());
			Fl::copy (copy_data.c_str(), (int) copy_data.size(), 1, Fl::clipboard_plain_text);
		}
	}
	Fl::unlock();
}

void Copy_Call_CB(Fl_Widget*a, void *b)
{
	if(b) {
		Fl_Text_Display_FM *c =  (Fl_Text_Display_FM *) b;
		std::string str;
		c->copy_string(str);
		input_call->value(str.c_str());
		input_call->do_callback();
	}
}

void Copy_Name_CB(Fl_Widget*a, void *b)
{
	if(b) {
		Fl_Text_Display_FM *c =  (Fl_Text_Display_FM *) b;
		std::string str;
		c->copy_string(str);
		input_name->value(str.c_str());
		input_name->do_callback();
	}
}

void Copy_Qth_CB(Fl_Widget*a, void *b)
{
	if(b) {
		Fl_Text_Display_FM *c =  (Fl_Text_Display_FM *) b;
		std::string str;
		c->copy_string(str);
		input_qth->value(str.c_str());
		input_qth->do_callback();
	}
}

void Copy_State_CB(Fl_Widget*a, void *b)
{
	if(b) {
		Fl_Text_Display_FM *c =  (Fl_Text_Display_FM *) b;
		std::string str;
		c->copy_string(str);
		input_state->value(str.c_str());
		input_state->do_callback();
	}
}

void Copy_User1_CB(Fl_Widget*a, void *b)
{
	if(b) {
		Fl_Text_Display_FM *c =  (Fl_Text_Display_FM *) b;
		std::string str;
		c->copy_string(str);
		input_user1->value(str.c_str());
		input_user1->do_callback();
	}
}

void Copy_User2_CB(Fl_Widget*a, void *b)
{
	if(b) {
		Fl_Text_Display_FM *c =  (Fl_Text_Display_FM *) b;
		std::string str;
		c->copy_string(str);
		input_user2->value(str.c_str());
		input_user2->do_callback();
	}
}

void Fl_Text_Display_FM::copy_string(std::string &copyString)
{
	Fl::lock();
	Fl_Text_Display *_w      = (Fl_Text_Display *) this;
	Fl_Text_Buffer *_buf     = (Fl_Text_Buffer *)  _w->buffer();
	if(_buf) {
		if (_buf->selected()) {
			copyString.assign(_buf->selection_text());
		}
	}
	Fl::unlock();
}

double Fl_Text_Display_FM::V_Scroll_Max(void)
{
	return (double) mVScrollBar->maximum();
}

double Fl_Text_Display_FM::V_Scroll_Value(void)
{
	return (double) mVScrollBar->value();
}

void Fl_Text_Display_FM::V_Scroll_Value(double value)
{
	double _max = mVScrollBar->maximum();
	if(value > _max) value = _max;
	if(value < 0.0) value = 0.0;

	mVScrollBar->value(value);
	mVScrollBar->do_callback();
}

double Fl_Text_Display_FM::V_Slider_Height(void)
{
	return (double) mVScrollBar->h();
}

int Fl_Text_Display_FM::handle(int e) {
	switch (e) {
		case FL_PUSH:
			// RIGHT MOUSE PUSHED? Popup menu on right click
			if ( Fl::event_button() == FL_RIGHT_MOUSE ) {
				if(ncs_window && ncs_window->visible()) {
					Fl_Menu_Item rclick_menu[] = {
						{ "Copy",    0, Copy_CB,        (void *)this, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, FONT_SIZE, 0},
						{ "Call",    0, Copy_Call_CB,   (void *)this, 0, FL_NORMAL_LABEL, 0, FONT_SIZE, 0},
						{ "Name",    0, Copy_Name_CB,   (void *)this, 0, FL_NORMAL_LABEL, 0, FONT_SIZE, 0},
						{ "Qth",     0, Copy_Qth_CB,    (void *)this, 0, FL_NORMAL_LABEL, 0, FONT_SIZE, 0},
						{ "State",   0, Copy_State_CB,  (void *)this, 0, FL_NORMAL_LABEL, 0, FONT_SIZE, 0},
						{ progStatus.display_user1_tag.c_str(), 0, Copy_User1_CB, (void *)this, 0, FL_NORMAL_LABEL, 0, FONT_SIZE, 0},
						{ progStatus.display_user2_tag.c_str(), 0, Copy_User2_CB, (void *)this, 0, FL_NORMAL_LABEL, 0, FONT_SIZE, 0},
						{ 0 }
					};
					const Fl_Menu_Item *m = rclick_menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, 0);
					if ( m ) m->do_callback(0, m->user_data());
				} else {
					Fl_Menu_Item rclick_menu[] = {
						{ "Copy",    0, Copy_CB,  (void *)this, 0, FL_NORMAL_LABEL, 0, FONT_SIZE, 0},
						{ 0 }
					};
					const Fl_Menu_Item *m = rclick_menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, 0);
					if ( m ) m->do_callback(0, m->user_data());
				}
				return(1);
			}
			break;
		case FL_RELEASE:
			// RIGHT MOUSE RELEASED? Mask it from Fl_Input
			if ( Fl::event_button() == FL_RIGHT_MOUSE ) {
				return(1);
			}
			break;
	}
	return(Fl_Text_Display::handle(e));    // let Fl_Input handle all other events
}
