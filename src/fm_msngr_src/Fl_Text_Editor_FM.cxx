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
#include "fm_msngr_src/Fl_Text_Editor_FM.h"

#define FONT_SIZE 12

static void Copy_All_CB(Fl_Widget*a, void *b)
{
	Fl::lock();
	Fl_Text_Editor_FM *_w  = (Fl_Text_Editor_FM *) b;
	if(!b) return;
	Fl_Text_Buffer *_buf = (Fl_Text_Buffer *)  _w->buffer();
	if(_buf) {
		std::string copy_data;
		copy_data.assign(_buf->text());
		Fl::copy (copy_data.c_str(), (int) copy_data.size(), 1, Fl::clipboard_plain_text);
	}
	Fl::unlock();
}


static void Copy_CB(Fl_Widget*a, void *b)
{
	Fl::lock();
	Fl_Text_Editor_FM *_w  = (Fl_Text_Editor_FM *) b;
	if(!b) return;
	Fl_Text_Buffer *_buf = (Fl_Text_Buffer *)  _w->buffer();
	if(_buf) {
		if (_buf->selected()) {
			std::string copy_data;
			copy_data.assign(_buf->selection_text());
			Fl::copy (copy_data.c_str(), (int) copy_data.size(), 1, Fl::clipboard_plain_text);
		}
	}
	Fl::unlock();
}

static void Paste_CB(Fl_Widget*a, void *b)
{
	if(!b) return;
	Fl::lock();
	Fl_Text_Editor_FM *_w = (Fl_Text_Editor_FM *) b;
	Fl::paste(*_w, 1);
	Fl::unlock();
}

void Fl_Text_Editor_FM::copy_string(std::string &copyString)
{
	Fl_Text_Editor_FM *_w   = (Fl_Text_Editor_FM *) this;
	Fl_Text_Buffer *_buf     = (Fl_Text_Buffer *)  _w->buffer();
	if(_buf) {
		if (_buf->selected()) {
			copyString.assign(_buf->selection_text());
		}
	}
}

int Fl_Text_Editor_FM::handle(int e) {
	switch (e) {
		case FL_PUSH:
			// RIGHT MOUSE PUSHED? Popup menu on right click
			if ( Fl::event_button() == FL_RIGHT_MOUSE ) {
				Fl_Menu_Item rclick_menu[] = {
					{ "Copy",     0, Copy_CB,     (void *)this, 0, FL_NORMAL_LABEL, 0, FONT_SIZE, 0},
					{ "Copy All", 0, Copy_All_CB, (void *)this, 0, FL_NORMAL_LABEL, 0, FONT_SIZE, 0},
					{ "Paste",    0, Paste_CB,    (void *)this, 0, FL_NORMAL_LABEL, 0, FONT_SIZE, 0},
					{ 0 }
				};
				const Fl_Menu_Item *m = rclick_menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, 0);
				if ( m ) m->do_callback(0, m->user_data());
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
	return(Fl_Text_Editor::handle(e));    // let Fl_Input handle all other events
}
