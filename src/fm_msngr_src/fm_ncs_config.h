// ----------------------------------------------------------------------------
// fm_ncs_config.h
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
#ifndef __fm_ncs_config__
#define __fm_ncs_config__

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Widget.H>

extern Fl_Double_Window *create_ncs_window(void);
extern Fl_Double_Window *ncs_window;
extern Fl_Button *btn_add;
extern Fl_Button *btn_close;
extern Fl_Button *btn_next;
extern Fl_Button *btn_prev;
extern Fl_Button *btn_logout;
extern Fl_Button *btn_send_to_msg;
extern Fl_Button *btn_load_send_to_msg;
extern Fl_Check_Button *chk_btn_link_fldigi;
extern Fl_Check_Button *chk_btn_link_flnet;
extern Fl_Check_Button *chk_btn_print_lables;
extern Fl_Check_Button *chk_btn_reply;
extern Fl_Check_Button *chk_btn_use_call;
extern Fl_Check_Button *chk_btn_use_name;
extern Fl_Check_Button *chk_btn_use_qth;
extern Fl_Check_Button *chk_btn_use_state;
extern Fl_Check_Button *chk_btn_traffic;
extern Fl_Check_Button *chk_btn_use_traffic;
extern Fl_Check_Button *chk_btn_use_user1;
extern Fl_Check_Button *chk_btn_use_user2;
extern Fl_Check_Button *chk_btn_send_to_op_msg;
extern Fl_Choice *choice_queue;
extern Fl_Choice *choice_send_to_op_msg;
extern Fl_Input  *input_call;
extern Fl_Input  *input_name;
extern Fl_Input  *input_qth;
extern Fl_Input  *input_state;
extern Fl_Input  *input_user1;
extern Fl_Input  *input_user2;
extern Fl_Output *output_link_to_msg;


extern void cb_open_ncs(Fl_Widget *a, void *b);
extern void cb_close_ncs(Fl_Widget *a, void *b);
extern void ncs_load_current_list(void);
extern void update_ncs_list(void);

#endif /* defined(__fm_ncs_config__) */
