// ----------------------------------------------------------------------------
// fm_dialog.h
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

#ifndef _fm_dialog_h
#define _fm_dialog_h

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>

#include "fm_msngr_src/Fl_Text_Display_FM.h"
#include "fm_msngr_src/Fl_Text_Editor_FM.h"
#include <FL/Fl_Text_Buffer.H>

#include "gettext.h"

#define FM_CLEAR_LIST _("Clear List")

extern void open_frame_messgeger(void);
extern Fl_Window *open_frame_window(int argc, char **argv);

extern Fl_Window          *window_frame;
extern Fl_Menu_Bar        *menubar_frame;
extern Fl_Button          *btn_frame_trx;
extern Fl_Choice          *choice_frame_last_callsign;
extern Fl_Choice          *choice_frame_packet_size;
extern Fl_Group           *group_rxtx_panel;
extern Fl_Text_Display_FM *display_frame_rx_panel;
extern Fl_Text_Editor_FM  *edit_frame_tx_panel;
extern Fl_Text_Buffer     *text_rx_buffer;
extern Fl_Text_Buffer     *text_tx_buffer;
extern Fl_Input           *drop_file;
extern Fl_Output	      *trx_indictor;

#endif // _fm_dialog_h
