// ----------------------------------------------------------------------------
// fm_dialog.cxx
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
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/filename.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_File_Icon.H>
#include <FL/Fl_Menu_Button.H>

#include <stdio.h>
#include <string>
#include <cstring>
#include <errno.h>

#include "fm_msngr.h"
#include "fm_msngr_src/fm_control.h"
#include "fm_msngr_src/fm_config.h"
#include "fm_msngr_src/fm_config_tts.h"
#include "fm_msngr_src/log_csv_control.h"
#include "fm_msngr_src/display_list_control.h"
#include "fm_msngr_src/fm_ncs_config.h"
#include "fm_msngr_src/Fl_Text_Editor_FM.h"
#include "fm_msngr_src/Fl_Text_Display_FM.h"
#include "fm_msngr_src/fm_dialog.h"

#include "xml_io.h"
#include "gettext.h"
#include "status.h"
#include "config.h"
#include "icons.h"
#include "fileselect.h"
#include "debug.h"

Fl_Window          *window_frame               = (Fl_Window *)0;
Fl_Menu_Bar        *menubar_frame              = (Fl_Menu_Bar *)0;
Fl_Button          *btn_frame_trx              = (Fl_Button *)0;
Fl_Choice          *choice_frame_last_callsign = (Fl_Choice *)0;
Fl_Choice          *choice_frame_packet_size   = (Fl_Choice *)0;
Fl_Group           *group_rxtx_panel           = (Fl_Group *)0;
Fl_Text_Display_FM *display_frame_rx_panel     = (Fl_Text_Display_FM *)0;
Fl_Text_Editor_FM  *edit_frame_tx_panel        = (Fl_Text_Editor_FM *)0;
Fl_Text_Buffer     *text_rx_buffer             = (Fl_Text_Buffer *)0;
Fl_Text_Buffer     *text_tx_buffer             = (Fl_Text_Buffer *)0;
Fl_Input           *drop_file                  = (Fl_Input *)0;
Fl_Output          *trx_indictor               = (Fl_Output *)0;

#define STATIC

/** ********************************************************
 *
 ***********************************************************/
STATIC  int default_handler(int event)
{
	if (event != FL_SHORTCUT)
		return 0;

	else if (Fl::event_ctrl())  {
		Fl_Widget* w = Fl::focus();
		return w->handle(FL_KEYBOARD);
	}

	return 0;
}

#if !defined(__APPLE__) && !defined(__WOE32__) && USE_X
Pixmap  fm_msngr_icon_pixmap;

#define KNAME "fm_msngr"

/** ********************************************************
 *
 ***********************************************************/
void make_pixmap(Pixmap *xpm, const char **data)
{
	Fl_Window w(0,0, KNAME);
	w.xclass(KNAME);
	w.show();
	w.make_current();
	Fl_Pixmap icon(data);
	int maxd = (icon.w() > icon.h()) ? icon.w() : icon.h();
	*xpm = fl_create_offscreen(maxd, maxd);
	fl_begin_offscreen(*xpm);
	fl_color(FL_BACKGROUND_COLOR);
	fl_rectf(0, 0, maxd, maxd);
	icon.draw(maxd - icon.w(), maxd - icon.h());
	fl_end_offscreen();
}

#endif

/** **************************************************************
 * \brief
 * \param a Pointer from the calling widget
 * \param b Pointer data passed from calling widget
 * \return void
 *****************************************************************/
STATIC  void cb_menu_insert_tx_data(Fl_Widget *a, void *b)
{
	fm_insert_tx_data();
}

/** **************************************************************
 * \brief
 * \param a Pointer from the calling widget
 * \param b Pointer data passed from calling widget
 * \return void
 *****************************************************************/
STATIC  void cb_menu_save_rx_data(Fl_Widget *a, void *b)
{
	fm_save_rx_data();
}

/** **************************************************************
 * \brief
 * \param a Pointer from the calling widget
 * \param b Pointer data passed from calling widget
 * \return void
 *****************************************************************/
STATIC  void cb_menu_copy_selected_tx(Fl_Widget *a, void *b)
{
	Fl::lock();
	int c = 0;
	Fl_Text_Editor::kf_copy(c, edit_frame_tx_panel);
	Fl::unlock();
}

/** **************************************************************
 * \brief
 * \param a Pointer from the calling widget
 * \param b Pointer data passed from calling widget
 * \return void
 *****************************************************************/
STATIC  void cb_menu_undo_tx(Fl_Widget *a, void *b)
{
	Fl::lock();
	int c = 0;
	Fl_Text_Editor::kf_undo(c, edit_frame_tx_panel);
	Fl::unlock();
}

/** **************************************************************
 * \brief
 * \param a Pointer from the calling widget
 * \param b Pointer data passed from calling widget
 * \return void
 *****************************************************************/
STATIC  void cb_menu_delete_selected_tx(Fl_Widget *a, void *b)
{
	Fl::lock();
	int c = 0;
	Fl_Text_Editor::kf_delete(c, edit_frame_tx_panel);
	Fl::unlock();
}

/** **************************************************************
 * \brief
 * \param a Pointer from the calling widget
 * \param b Pointer data passed from calling widget
 * \return void
 *****************************************************************/
STATIC  void cb_menu_copy_all_tx(Fl_Widget *a, void *b)
{
	Fl::lock();
	int c = 0;
	Fl_Text_Editor::kf_select_all(c, edit_frame_tx_panel);
	Fl_Text_Editor::kf_copy(c, edit_frame_tx_panel);
	Fl::unlock();
}

/** **************************************************************
 * \brief
 * \param a Pointer from the calling widget
 * \param b Pointer data passed from calling widget
 * \return void
 *****************************************************************/
STATIC  void cb_menu_paste_tx(Fl_Widget *a, void *b)
{
	Fl::lock();
	int c = 0;
	Fl_Text_Editor::kf_paste(c, edit_frame_tx_panel);
	Fl::unlock();
}

/** **************************************************************
 * \brief
 * \param a Pointer from the calling widget
 * \param b Pointer data passed from calling widget
 * \return void
 *****************************************************************/
STATIC  void cb_menu_paste_last_tx(Fl_Widget *a, void *b)
{
	paste_last_tx();
}

/** **************************************************************
 * \brief
 * \param a Pointer from the calling widget
 * \param b Pointer data passed from calling widget
 * \return void
 *****************************************************************/
STATIC  void cb_menu_clear_all_tx(Fl_Widget *a, void *b)
{
	Fl::lock();
	int c = 0;
	Fl_Text_Editor::kf_select_all(c, edit_frame_tx_panel);
	Fl_Text_Editor::kf_delete(c, edit_frame_tx_panel);
	Fl::unlock();
}

/** **************************************************************
 * \brief
 * \param a Pointer from the calling widget
 * \param b Pointer data passed from calling widget
 * \return void
 *****************************************************************/
STATIC  void cb_menu_copy_selected_rx(Fl_Widget *a, void *b)
{
	Fl::lock();
	Fl_Text_Display *_w      = (Fl_Text_Display *) display_frame_rx_panel;
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

/** **************************************************************
 * \brief
 * \param a Pointer from the calling widget
 * \param b Pointer data passed from calling widget
 * \return void
 *****************************************************************/
STATIC  void cb_menu_copy_all_rx(Fl_Widget *a, void *b)
{
	Fl::lock();
	Fl_Text_Display *_w      = (Fl_Text_Display *) display_frame_rx_panel;
	Fl_Text_Buffer *_buf     = (Fl_Text_Buffer *)  _w->buffer();

	if(_buf) {
		std::string copy_data;
		copy_data.assign(_buf->text(), _buf->length());
		Fl::copy (copy_data.c_str(), (int) copy_data.size(), 1, Fl::clipboard_plain_text);
	}
	Fl::unlock();
}

/** **************************************************************
 * \brief
 * \param a Pointer from the calling widget
 * \param b Pointer data passed from calling widget
 * \return void
 *****************************************************************/
STATIC  void cb_menu_clear_all_rx(Fl_Widget *a, void *b)
{

	Fl::lock();
	Fl_Text_Display *_w      = (Fl_Text_Display *) display_frame_rx_panel;
	Fl_Text_Buffer *_buf     = (Fl_Text_Buffer *)  _w->buffer();

	if(!_buf || !_buf->length()) {
		Fl::unlock();
		return;
	}

	int selection = fl_choice(_("Deleting RX Window Content Warning!"), \
							  _("Save and Delete"), _("Delete"), _("Cancel"));
	switch(selection) {
		case 0:
		case 1:
			Fl::lock();
			_buf->remove(0, _buf->length());
			Fl::unlock();
			break;
		case 2:
			break;
	}
	Fl::unlock();
}

/** **************************************************************
 * \brief Close frame window
 * \param a Pointer from the calling widget
 * \param b Pointer data passed from calling widget
 * \return void
 *****************************************************************/
STATIC  void cb_log_list_to_tx(Fl_Widget *a, void *b)
{
	write_list_output_tx_panel();
}

/** **************************************************************
 * \brief Close frame window
 * \param a Pointer from the calling widget
 * \param b Pointer data passed from calling widget
 * \return void
 *****************************************************************/
STATIC  void cb_menu_close_frame_window(Fl_Widget *a, void *b)
{
	close_frame_messgeger();
}

/** **************************************************************
 * \brief Open CSV Log Config Dialog.
 * \param a Pointer from the calling widget
 * \param b Pointer data passed from calling widget
 * \return void
 *****************************************************************/
STATIC  void cb_open_csv_log_configure(Fl_Widget *a, void *b)
{
	open_csv_log_configure();
}

/** **************************************************************
 * \brief Open CSV Log Config Dialog.
 * \param a Pointer from the calling widget
 * \param b Pointer data passed from calling widget
 * \return void
 *****************************************************************/
STATIC  void cb_open_disp_log_configure(Fl_Widget *a, void *b)
{
	open_display_list_configure();
}

/** **************************************************************
 * \brief Generate CSV list to the TX edit window.
 * \param a Pointer from the calling widget
 * \param b Pointer data passed from calling widget
 * \return void
 *****************************************************************/
STATIC  void cb_log_csv_to_tx(Fl_Widget *a, void *b)
{
	write_csv_output_tx_panel();
}

/** ********************************************************
 *
 ***********************************************************/
STATIC  void open_url(const char* url)
{
	LOG_INFO("%s", url);
#ifndef __WOE32__
	const char* browsers[] = {
#  ifdef __APPLE__
		getenv("FLDIGI_BROWSER"),   // valid for any OS - set by user
		"open"                      // OS X
#  else
		"fl-xdg-open",			    // Puppy Linux
		"xdg-open",			        // other Unix-Linux distros
		getenv("FLDIGI_BROWSER"),   // force use of spec'd browser
		getenv("BROWSER"),		    // most Linux distributions
		"sensible-browser",
		"firefox",
		"mozilla"				    // must be something out there!
#  endif
	};
	switch (fork()) {
		case 0:
#  ifndef NDEBUG
			unsetenv("MALLOC_CHECK_");
			unsetenv("MALLOC_PERTURB_");
#  endif
			for (size_t i = 0; i < sizeof(browsers)/sizeof(browsers[0]); i++)
				if (browsers[i])
					execlp(browsers[i], browsers[i], url, (char*)0);
			exit(EXIT_FAILURE);
		case -1:
			fl_alert2(_("Could not run a web browser:\n%s\n\n"
						"Open this URL manually:\n%s"),
					  strerror(errno), url);
	}
#else
	if ((int)ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL) <= 32)
		fl_alert2(_("Could not open url:\n%s\n"), url);
#endif
}

/** ********************************************************
 *
 ***********************************************************/
STATIC  void cb_show_debug(Fl_Widget *a, void *b)
{
	debug::show();
}

/** ********************************************************
 *
 ***********************************************************/
STATIC  void cb_show_about(Fl_Widget *a, void *b)
{
	fl_choice2("Frame Messenger Version %s\n\nAuthors:\n\tRobert Stiles, KK5VD",
			   _("Close"), NULL, NULL, VERSION);
}

/** ********************************************************
 *
 ***********************************************************/
STATIC  void cb_show_help(Fl_Widget *a, void *b)
{
	open_url("http://www.w1hkj.com/fm_msngr-help/index.html");
}

/** ********************************************************
 *
 ***********************************************************/
extern std::string HomeDir;

STATIC  void cb_folders(Fl_Widget *a, void *b)
{
	open_url(HomeDir.c_str());
}

/** **************************************************************
 * \brief Menu Sturcture initialization.
 *****************************************************************/

STATIC  Fl_Menu_Item menu_menubar_frame[] = {
	{ _("Files"),              0,  0,                           0, FL_SUBMENU,      FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ _("Folders"),            0,  cb_folders,                  0, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ _("Insert TX"),          0,  cb_menu_insert_tx_data,      0, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ _("Save RX"),            0,  cb_menu_save_rx_data,        0, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ _("Close"),              0,  cb_menu_close_frame_window,  0, 0,               FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ 0,0,0,0,0,0,0,0,0 },
	{ _("Edit TX"),            0,  0,                           0, FL_SUBMENU,      FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ _("Copy &Selected"),     0,  cb_menu_copy_selected_tx,    0, 0,               FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ _("Copy &All"),          0,  cb_menu_copy_all_tx,         0, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ _("&Paste"),             0,  cb_menu_paste_tx,            0, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ _("Paste Last TX &Msg"), 0,  cb_menu_paste_last_tx,       0, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ _("D&elete Selected"),   0,  cb_menu_delete_selected_tx,  0, 0,               FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ _("&Delete All"),        0,  cb_menu_clear_all_tx,        0, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ _("&Undo"),              0,  cb_menu_undo_tx,             0, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ 0,0,0,0,0,0,0,0,0 },
	{ _("Edit RX"),            0,  0,                           0, FL_SUBMENU,      FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ _("Copy &Selected"),     0,  cb_menu_copy_selected_rx,    0, 0,               FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ _("Copy &All"),          0,  cb_menu_copy_all_rx,         0, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ _("&Delete All"),        0,  cb_menu_clear_all_rx,        0, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ 0,0,0,0,0,0,0,0,0 },
	{ _("Log"),                0,  0,                           0, FL_SUBMENU,      FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ _("List Checks-Ins"),    0,  cb_log_list_to_tx,           0, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ _("List CSV to TX"),     0,  cb_log_csv_to_tx,            0, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ _("Net Control"),        0,  cb_open_ncs,                 0, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ 0,0,0,0,0,0,0,0,0 },
	{ _("Configure"),          0,  0,                           0, FL_SUBMENU,      FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ _("Settings"),           0,  cb_open_configure,           0, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ _("Log Display"),        0,  cb_open_disp_log_configure,  0, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ _("CSV Write Format"),   0,  cb_open_csv_log_configure,   0, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ _("Text to Speech"),     0,  cb_open_tts_configure,       0, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ 0,0,0,0,0,0,0,0,0 },
	{ _("Help"),               0,  0,                           0, FL_SUBMENU,      FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ _("Online Document"),    0,  cb_show_help,                0, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ _("Debug Log"),          0,  cb_show_debug,               0, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ _("About"),              0,  cb_show_about,               0, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, FM_LABEL_FONT_SIZE, 0 },
	{ 0,0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,0 }
};

/** **************************************************************
 * \brief User request - Transmit tx panel data.
 * \param a Pointer from the calling widget
 * \param b Pointer data passed from calling widget
 * \return void
 *****************************************************************/
STATIC  void cb_btn_frame_trx(Fl_Widget *a, void *b)
{
	extern int trx_state;
	if (trx_state == RX_STATE) {
		fm_transmit();
	}
	else {
		cancel_tx();
		btn_frame_trx->label(_("Canceling"));
		btn_frame_trx->redraw();
	}
}

/** **************************************************************
 * \brief Set the check box state of the menu item (initalize).
 * \param menu_menubar_frame Fl_Menu_Item sturcture.
 * \param nof_menu_items number of menu items to search in the array.
 * \param menu_name Lable name of the menu item to modify.
 * \param flag The state to set.
 * \return void
 *****************************************************************/
STATIC  void set_menu_item_value(Fl_Menu_Item menu[], size_t nof_menu_items, const char *menu_name, bool flag)
{
	int found = 0;

	if(!menu_name) return;

	for(size_t index = 0; index < nof_menu_items; index++) {
		if(menu[index].text) {
			found = strncmp((const char *) menu_name, (const char *) menu[index].text, 80);
			if(found == 0) {
				if(flag) menu[index].flags |= FL_MENU_VALUE;
				else menu[index].flags &= (~FL_MENU_VALUE);
				return;
			}
		}
	}
}

/** **************************************************************
 * \brief Drag and Drop a file name path.
 * \param a Pointer from the calling widget
 * \param b Pointer data passed from calling widget
 * \return void
 *****************************************************************/
STATIC  void cb_drop_file(Fl_Widget *a, void *b)
{
	Fl_Input *c = (Fl_Input *)a;
	STATIC  bool in_use = false;

	if(in_use) {
		c->value(FM_LABLE_DND);
		return;
	}

	in_use = true;
	if(c) fm_dnd_file();
	in_use = false;
}

/** **************************************************************
 * \brief Select RX callsign and move recevie panel position
 * to the most recent received text from that callsign.
 * \param a Pointer from the calling widget
 * \param b Pointer data passed from calling widget
 * \return void
 *****************************************************************/
STATIC  void cb_choice_frame_last_callsign(Fl_Widget *a, void *b)
{
	std::string cl = FM_CLEAR_LIST;
	std::string _m = choice_frame_last_callsign->text();

	if(cl == _m) {
		clear_callsign_pick_list();
	} else {
		move_to_callsign(_m);
	}
}

/** **************************************************************
 * \brief Setnumber of bytes per frame (callback)
 * \param b Pointer data passed from calling widget
 * \return void
 *****************************************************************/
STATIC  void cb_frame_count(Fl_Widget *a, void *b)
{
	progStatus.fm_frame_count.assign(choice_frame_packet_size->text());
}

/** **************************************************************
 * \brief Initialize/Create Frame Messenger Window
 * \param none
 * \return Pointer to window structure.
 *****************************************************************/
Fl_Window *open_frame_window(int argc, char **argv)
{
	int y_disp = 0;
	int x_disp = 0;
	int fm_x = 0;
	int fm_y = 0;
	int fm_w = FM_MIN_WINDOW_WIDTH;
	int fm_h = FM_MIN_WINDOW_HEIGHT;

	// You need this to initialize the font database in FLTK.
	// Do not remove or the program might crash on a font selection.
	// Keep the unused warning at bay by double assignement (LLVM, clang)
	// #pragma unused for GCC.
	int nbr_fonts = 0;
	nbr_fonts = Fl::set_fonts(0);
#pragma unused (nbr_fonts)


	set_fm_window_defaults();

	window_frame = new Fl_Window(fm_x, fm_y, fm_w, fm_h, _("Frame Messenger"));
	window_frame->box(FL_ENGRAVED_BOX);
	window_frame->callback(cb_menu_close_frame_window);

	menubar_frame = new Fl_Menu_Bar(fm_x, fm_y, fm_w, 20, "");
	menubar_frame->tooltip(_("Open Text file for Insertion"));
	menubar_frame->menu(menu_menubar_frame);

	y_disp = 24;
	x_disp = 5;

	int w = 70;
	int h = 24;

	y_disp = fm_h - (h + x_disp);

	btn_frame_trx = new Fl_Button(fm_w - (w + x_disp), y_disp, w, h, _("Transmit"));
	btn_frame_trx->callback(cb_btn_frame_trx);
	btn_frame_trx->labelsize(FM_LABEL_FONT_SIZE);

	y_disp = fm_h - (h + x_disp);

	int d_w = 40;
	int d_pos = fm_w - 160;

	drop_file = new Fl_Input(d_pos, y_disp, d_w, h);
	drop_file->box(FL_BORDER_BOX);
	drop_file->textcolor(fl_rgb_color(200, 0, 0) );
	drop_file->value(FM_LABLE_DND);
	drop_file->color(fl_rgb_color(200, 255, 255));
	drop_file->cursor_color(fl_rgb_color(200, 255, 255));
	drop_file->label("");
	drop_file->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	drop_file->labelcolor(fl_rgb_color(200, 0, 0) );
	drop_file->tooltip(_("drag and drop tx files here ..."));
	drop_file->callback((Fl_Callback*)cb_drop_file);
	drop_file->when(FL_WHEN_CHANGED);
	drop_file->labelsize(FM_LABEL_FONT_SIZE);

	d_w = h;
	d_pos = (d_pos) + d_w + h + 8;

	trx_indictor = new Fl_Output(d_pos, y_disp, d_w, h, "");
	trx_indictor->box(FL_BORDER_BOX);
	trx_indictor->value("");
	trx_indictor->color(fl_rgb_color(0, 255, 0));
	trx_indictor->set_output();
	trx_indictor->labelsize(FM_LABEL_FONT_SIZE);

	choice_frame_last_callsign = new Fl_Choice(x_disp, y_disp, 125, 24,  _("Last Callsign RX"));
	choice_frame_last_callsign->tooltip( _("Displays the last callsign received"));
	choice_frame_last_callsign->align(Fl_Align(FL_ALIGN_RIGHT));
	//choice_frame_last_callsign->labelfont(0);
	choice_frame_last_callsign->labelsize(FM_LABEL_FONT_SIZE);
	choice_frame_last_callsign->callback(cb_choice_frame_last_callsign);

	choice_frame_packet_size = new Fl_Choice(x_disp + 260, y_disp, 85, 24,  _("Bytes Per Frame"));
	choice_frame_last_callsign->tooltip(_("Limit data frame size to 'n' bytes"));
	choice_frame_packet_size->down_box(FL_BORDER_BOX);
	choice_frame_packet_size->align(Fl_Align(FL_ALIGN_RIGHT));
	choice_frame_packet_size->add("32");
	choice_frame_packet_size->add("64");
	choice_frame_packet_size->add("80");
	choice_frame_packet_size->add("96");
	choice_frame_packet_size->add("128");
	choice_frame_packet_size->add("224");
	choice_frame_packet_size->add("255");
	choice_frame_packet_size->when(FL_WHEN_CHANGED);
	int index = choice_frame_packet_size->find_index(progStatus.fm_frame_count.c_str());
	choice_frame_packet_size->value(index);
	choice_frame_packet_size->labelsize(FM_LABEL_FONT_SIZE);
	choice_frame_packet_size->callback((Fl_Callback*)cb_frame_count);

	group_rxtx_panel = new Fl_Group(0, 32, fm_w - 5, fm_h - 60);

	display_frame_rx_panel = new Fl_Text_Display_FM(x_disp, 24, fm_w - 10, 210);
	display_frame_rx_panel->box(FL_DOWN_BOX);
	display_frame_rx_panel->color(fl_rgb_color(progStatus.RxColor_red,
											   progStatus.RxColor_green,
											   progStatus.RxColor_blue));
	display_frame_rx_panel->selection_color(FL_BACKGROUND_COLOR);
	display_frame_rx_panel->labeltype(FL_NORMAL_LABEL);
	//display_frame_rx_panel->labelfont(0);
	display_frame_rx_panel->labelsize(FM_LABEL_FONT_SIZE);
	display_frame_rx_panel->labelcolor(FL_FOREGROUND_COLOR);
	display_frame_rx_panel->textcolor(fl_rgb_color(progStatus.RxFontcolor_red,
												   progStatus.RxFontcolor_green,
												   progStatus.RxFontcolor_blue));

	const char *font_name = Fl::get_font(progStatus.RxFontnbr);
	progStatus.rx_font_name.assign(font_name);

	display_frame_rx_panel->textsize(progStatus.RxFontsize);
	display_frame_rx_panel->textfont(progStatus.RxFontnbr);
	display_frame_rx_panel->align(Fl_Align(FL_ALIGN_TOP));
	display_frame_rx_panel->when(FL_WHEN_RELEASE);
	display_frame_rx_panel->end();

	text_rx_buffer = new Fl_Text_Buffer();
	display_frame_rx_panel->buffer(text_rx_buffer);

	edit_frame_tx_panel = new Fl_Text_Editor_FM(x_disp, 236, fm_w - 10, 210);
	edit_frame_tx_panel->box(FL_DOWN_BOX);
	edit_frame_tx_panel->color(fl_rgb_color(progStatus.TxColor_red,
											progStatus.TxColor_green,
											progStatus.TxColor_blue));
	edit_frame_tx_panel->selection_color(FL_BACKGROUND_COLOR);
	edit_frame_tx_panel->labeltype(FL_NORMAL_LABEL);
	//edit_frame_tx_panel->labelfont(0);
	edit_frame_tx_panel->labelsize(FM_LABEL_FONT_SIZE);
	edit_frame_tx_panel->labelcolor(FL_FOREGROUND_COLOR);
	edit_frame_tx_panel->textcolor(fl_rgb_color(progStatus.TxFontcolor_red,
												progStatus.TxFontcolor_green,
												progStatus.TxFontcolor_blue));

	font_name = Fl::get_font(progStatus.TxFontnbr);
	progStatus.tx_font_name.assign(font_name);

	edit_frame_tx_panel->textsize(progStatus.TxFontsize);
	edit_frame_tx_panel->textfont(progStatus.TxFontnbr);
	edit_frame_tx_panel->align(Fl_Align(FL_ALIGN_TOP));
	edit_frame_tx_panel->when(FL_WHEN_RELEASE);
	edit_frame_tx_panel->end();

	text_tx_buffer = new Fl_Text_Buffer();
	edit_frame_tx_panel->buffer(text_tx_buffer);

	Fl_Group::current()->resizable(display_frame_rx_panel);

	group_rxtx_panel->end();
	Fl_Group::current()->resizable(group_rxtx_panel);

	window_frame->size_range(FM_MIN_WINDOW_WIDTH, FM_MIN_WINDOW_HEIGHT);
	window_frame->end();

	window_frame->resize(progStatus.fm_window_x, progStatus.fm_window_y, progStatus.fm_window_w, progStatus.fm_window_h);

	Fl::add_handler(default_handler);

	Fl_File_Icon::load_system_icons();
	FSEL::create();

#if defined(__WOE32__)
#  ifndef IDI_ICON
#	define IDI_ICON 101
#  endif
	window_frame->icon((char*)LoadIcon(fl_display, MAKEINTRESOURCE(IDI_ICON)));
	window_frame->show (argc, argv);
#elif !defined(__APPLE__)
	make_pixmap(&fm_msngr_icon_pixmap, fm_msngr_icon);
	window_frame->icon((char *)fm_msngr_icon_pixmap);
	window_frame->show(argc, argv);
#else
	window_frame->show(argc, argv);
#endif

	if (string(window_frame->label()) == "") {
		string main_label = PACKAGE_NAME;
		main_label.append(": ").append(PACKAGE_VERSION);
		window_frame->label(main_label.c_str());
	}

	open_calltable();

	return window_frame;
}
