// ----------------------------------------------------------------------------
// fm_config.cxx
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

#include "fm_msngr_src/fm_config.h"
#include "fm_msngr_src/fm_control.h"
#include "fm_msngr_src/fm_dialog.h"
#include "status.h"
#include "font_browser.h"
#include "flslider2.h"
#include "util.h"
#include "gettext.h"
#include "debug.h"

extern Font_Browser *font_browser;

static Fl_Double_Window *configure_window = (Fl_Double_Window *)0;

Fl_Output *txt_tx_font_display = (Fl_Output *)0;
Fl_Button *btn_tx_bkgrnd_color = (Fl_Button *)0;
Fl_Button *btn_tx_font         = (Fl_Button *)0;
Fl_Button *btn_tx_default      = (Fl_Button *)0;

Fl_Output *txt_rx_font_display = (Fl_Output *)0;
Fl_Button *btn_rx_bkgrnd_color = (Fl_Button *)0;
Fl_Button *btn_rx_font         = (Fl_Button *)0;
Fl_Button *btn_rx_default      = (Fl_Button *)0;

Fl_Check_Button *chk_enable_flnet_io      = (Fl_Check_Button *)0;
Fl_Check_Button *chk_enable_timestamp     = (Fl_Check_Button *)0;
Fl_Check_Button *chk_enable_utc_timestamp = (Fl_Check_Button *)0;

Fl_Input *inp_callsign = (Fl_Input *)0;

Fl_Button *btn_close_config_window = (Fl_Button *)0;

Fl_Color_Chooser *bkGndColor = (Fl_Color_Chooser *)0;

/** *******************************************************************
 * \brief Update the configure rx display font/color attributes
 **********************************************************************/
void update_config_rx_font(void)
{
	txt_rx_font_display->value(_("RX Window Font"));
	txt_rx_font_display->color(fl_rgb_color(progStatus.RxColor_red,
											progStatus.RxColor_green,
											progStatus.RxColor_blue));
	txt_rx_font_display->textcolor(fl_rgb_color(progStatus.RxFontcolor_red,
												progStatus.RxFontcolor_green,
												progStatus.RxFontcolor_blue));
	txt_rx_font_display->textsize(progStatus.RxFontsize);
	txt_rx_font_display->textfont(progStatus.RxFontnbr);
}

/** *******************************************************************
 * \brief Update the configure tx display font/color attributes
 **********************************************************************/
void update_config_tx_font(void)
{
	txt_tx_font_display->value(_("TX Window Font"));
	txt_tx_font_display->color(fl_rgb_color(progStatus.TxColor_red,
											progStatus.TxColor_green,
											progStatus.TxColor_blue));
	txt_tx_font_display->textcolor(fl_rgb_color(progStatus.TxFontcolor_red,
												progStatus.TxFontcolor_green,
												progStatus.TxFontcolor_blue));
	txt_tx_font_display->textsize(progStatus.TxFontsize);
	txt_tx_font_display->textfont(progStatus.TxFontnbr);
}

/** *******************************************************************
 * \brief Convert Fl_Color into RGB color channels
 **********************************************************************/
inline void color_rgb(Fl_Color &color, short int &r, short int &g, short int &b)
{
	Fl_Color shift = color;

	r = (shift >> 24) & 0xFF;
	g = (shift >> 16) & 0xFF;
	b = (shift >> 8)  & 0xFF;
}

/** *******************************************************************
 * \brief Callback for user selected TX font.
 **********************************************************************/
static void cb_btn_tx_font_proc(Fl_Widget * a, void * b)
{
	Fl_Font font = font_browser->fontNumber();
	int size = font_browser->fontSize();
	Fl_Color color = font_browser->fontColor();

	edit_frame_tx_panel->textfont(font);
	edit_frame_tx_panel->textsize(size);
	edit_frame_tx_panel->textcolor(color);
	edit_frame_tx_panel->redraw();

	progStatus.TxFontnbr = font;
	progStatus.TxFontsize = size;

	color_rgb(color, progStatus.TxFontcolor_red, \
			  progStatus.TxFontcolor_green, \
			  progStatus.TxFontcolor_blue);

	font_browser->hide();

	update_config_tx_font();
	txt_tx_font_display->redraw();
}


/** *******************************************************************
 * \brief Callback for user selected TX font.
 **********************************************************************/
static void cb_btn_tx_font(Fl_Button * a, void * b)
{
	font_browser->fontNumber(progStatus.TxFontnbr);
	font_browser->fontSize(progStatus.TxFontsize);
	Fl_Color c = fl_rgb_color(progStatus.TxFontcolor_red,
							  progStatus.TxFontcolor_green,
							  progStatus.TxFontcolor_blue);
	font_browser->fontColor(c);
	//font_browser->fontFilter(Font_Browser::FIXED_WIDTH);
	font_browser->callback(cb_btn_tx_font_proc);
	font_browser->show();
}

/** *******************************************************************
 * \brief Callback for user selected RX font.
 **********************************************************************/
static void cb_btn_rx_font_proc(Fl_Widget * a, void * b)
{
	Fl_Font font = font_browser->fontNumber();
	int size = font_browser->fontSize();
	Fl_Color color = font_browser->fontColor();

	display_frame_rx_panel->textfont(font);
	display_frame_rx_panel->textsize(size);
	display_frame_rx_panel->textcolor(color);
	display_frame_rx_panel->redraw();

	progStatus.RxFontnbr = font;
	progStatus.RxFontsize = size;

	color_rgb(color, progStatus.RxFontcolor_red, \
			  progStatus.RxFontcolor_green, \
			  progStatus.RxFontcolor_blue);

	font_browser->hide();

	update_config_rx_font();
	txt_rx_font_display->redraw();
}

/** *******************************************************************
 * \brief Callback for user selected RX font.
 **********************************************************************/
static void cb_btn_rx_font(Fl_Button * a, void * b)
{
	font_browser->fontNumber(progStatus.RxFontnbr);
	font_browser->fontSize(progStatus.RxFontsize);
	Fl_Color c = fl_rgb_color(progStatus.RxFontcolor_red,
							  progStatus.RxFontcolor_green,
							  progStatus.RxFontcolor_blue);
	font_browser->fontColor(c);
	//font_browser->fontFilter(Font_Browser::FIXED_WIDTH);
	font_browser->callback(cb_btn_rx_font_proc);
	font_browser->show();
}

/** *******************************************************************
 * \brief Callback to enable/disable FLNET IO comms
 **********************************************************************/
static void cb_chk_enable_flnet_io(Fl_Check_Button * a, void * b)
{
	bool value = chk_enable_flnet_io->value() ? true : false;
	progStatus.fm_enable_flnet = value;
}

/** *******************************************************************
 * \brief Callback to enable/disable the TX of empty dtat frames.
 **********************************************************************/
static void cb_fm_utc_timestamp(Fl_Check_Button * a, void * b)
{
	bool value = chk_enable_utc_timestamp->value() ? true : false;
	progStatus.fm_utc_timestamp = value;
}

/** *******************************************************************
 * \brief Callback to change the RX window back ground color.
 **********************************************************************/
static void cb_btn_rx_bkgnd_color(Fl_Button * a, void * ptr)
{
	unsigned char r = progStatus.RxColor_red;
	unsigned char g = progStatus.RxColor_green;
	unsigned char b = progStatus.RxColor_blue;

	const char * label = (const char *) _("Rx Window BkGrn Color");

	int value = fl_color_chooser(label, r, g, b, 1);

	if(value) {
		progStatus.RxColor_red   = r;
		progStatus.RxColor_green = g;
		progStatus.RxColor_blue  = b;

		Fl_Color color = fl_rgb_color(r, g, b);

		display_frame_rx_panel->color(color);
		display_frame_rx_panel->redraw();

		update_config_rx_font();
		txt_rx_font_display->redraw();
	}
}

/** *******************************************************************
 * \brief Callback to change the TX window back ground color.
 **********************************************************************/
static void cb_btn_tx_bkgnd_color(Fl_Button * a, void * ptr)
{
	unsigned char r = progStatus.TxColor_red;
	unsigned char g = progStatus.TxColor_green;
	unsigned char b = progStatus.TxColor_blue;

	const char * label = (const char *) _("Tx Window BkGrn Color");

	int value = fl_color_chooser(label, r, g, b, 1);

	if(value) {
		progStatus.TxColor_red   = r;
		progStatus.TxColor_green = g;
		progStatus.TxColor_blue  = b;

		Fl_Color color = fl_rgb_color(r, g, b);

		edit_frame_tx_panel->color(color);
		edit_frame_tx_panel->redraw();

		update_config_tx_font();
		txt_tx_font_display->redraw();
	}
}

/** *******************************************************************
 * \brief Callback to revert to the defualt TX Window Font, Font Color
 * and back ground color.
 **********************************************************************/
static void cb_btn_tx_default(Fl_Button * a, void * ptr)
{

	if(fl_choice(_("Reset TX Window Attributes?"), _("No"), _("Yes"), NULL)) {
		progStatus.TxColor_red   = 189;
		progStatus.TxColor_green = 230;
		progStatus.TxColor_blue  = 255;

		progStatus.TxFontcolor_red   = 0;
		progStatus.TxFontcolor_green = 0;
		progStatus.TxFontcolor_blue  = 0;

		progStatus.TxFontnbr  = 0;
		progStatus.TxFontsize = 13;

		Fl_Color color = fl_rgb_color(progStatus.TxColor_red, \
									  progStatus.TxColor_green, \
									  progStatus.TxColor_blue);

		edit_frame_tx_panel->color(color);

		color = fl_rgb_color(progStatus.TxFontcolor_red, \
							 progStatus.TxFontcolor_green, \
							 progStatus.TxFontcolor_blue);

		edit_frame_tx_panel->textfont(progStatus.TxFontnbr);
		edit_frame_tx_panel->textsize(progStatus.TxFontsize);
		edit_frame_tx_panel->textcolor(color);
		edit_frame_tx_panel->redraw();

		update_config_tx_font();
		txt_tx_font_display->redraw();
	}
}

/** *******************************************************************
 * \brief Callback to revert to the defualt RX Window Font, Font Color
 * and back ground color.
 **********************************************************************/
static void cb_btn_rx_default(Fl_Button * a, void * ptr)
{
	if(fl_choice(_("Reset RX Window Attributes?"), _("No"), _("Yes"), NULL)) {
		progStatus.RxColor_red   = 255;
		progStatus.RxColor_green = 240;
		progStatus.RxColor_blue  = 177;

		progStatus.RxFontcolor_red   = 0;
		progStatus.RxFontcolor_green = 0;
		progStatus.RxFontcolor_blue  = 0;

		progStatus.RxFontnbr  = 0;
		progStatus.RxFontsize = 13;

		Fl_Color color = fl_rgb_color(progStatus.RxColor_red, \
									  progStatus.RxColor_green, \
									  progStatus.RxColor_blue);

		display_frame_rx_panel->color(color);

		color = fl_rgb_color(progStatus.RxFontcolor_red, \
							 progStatus.RxFontcolor_green, \
							 progStatus.RxFontcolor_blue);

		display_frame_rx_panel->textfont(progStatus.RxFontnbr);
		display_frame_rx_panel->textsize(progStatus.RxFontsize);
		display_frame_rx_panel->textcolor(color);
		display_frame_rx_panel->redraw();

		update_config_rx_font();
		txt_rx_font_display->redraw();
	}
}

/** *******************************************************************
 * \brief Callback to assign the callsign
 **********************************************************************/
static void cb_inp_callsign(Fl_Input * a, void * b)
{
	progStatus.fm_inp_callsign.assign(inp_callsign->value());
}


/** *******************************************************************
 * \brief Callback to Close configuration dialog.
 **********************************************************************/
static void cb_btn_close_config_window(Fl_Button * a, void * b)
{
	close_config();
}

/** *******************************************************************
 * \brief Close configuration dialog.
 **********************************************************************/
void close_config(void)
{
	if(configure_window) {
		configure_window->hide();
	}
}

/** *******************************************************************
 * \brief Configuration Dialog Creation Code
 **********************************************************************/
Fl_Double_Window* config_window(void)
{
	int x = 0;
	int y = 0;
	int d_w = 450;
	int d_h = 158;
	int h = 0;
	int w = 115;
	int os = 15;

	font_browser = new Font_Browser;

	configure_window = new Fl_Double_Window(d_w, d_h);
	configure_window->callback((Fl_Callback*)cb_btn_close_config_window);

	x = os;
	y = os;
	h = 25;
	inp_callsign = new Fl_Input((d_w * 0.5), y, 150, h, _("Callsign"));
	inp_callsign->align(Fl_Align(FL_ALIGN_RIGHT));
	inp_callsign->callback((Fl_Callback*)cb_inp_callsign);
	inp_callsign->value(progStatus.fm_inp_callsign.c_str());

	h = 15;
	/*
	chk_enable_flnet_io = new Fl_Check_Button(x, y, h+4, h, _("Enable FLNET Interface"));
	chk_enable_flnet_io->tooltip(_("Enable bidirectional Communications with FLNET"));
	chk_enable_flnet_io->down_box(FL_DOWN_BOX);
	chk_enable_flnet_io->callback((Fl_Callback*)cb_chk_enable_flnet_io);
	chk_enable_flnet_io->value(progStatus.fm_enable_flnet);
*/
//	y += (h + 5);
	chk_enable_utc_timestamp = new Fl_Check_Button(x, y, h+4, h, _("UTC Time Stamps"));
	chk_enable_utc_timestamp->tooltip(_("Use UTC time for time stamps."));
	chk_enable_utc_timestamp->down_box(FL_DOWN_BOX);
	chk_enable_utc_timestamp->callback((Fl_Callback*)cb_fm_utc_timestamp);
	chk_enable_utc_timestamp->value(progStatus.fm_utc_timestamp);

	y += (h + 20);
	h = 25;

	txt_rx_font_display = new Fl_Output(x, y, 150, h, _("Rx"));
	txt_rx_font_display->align(Fl_Align(FL_ALIGN_RIGHT));

	btn_rx_font = new Fl_Button(x + 195, y, 65, h, _("Font"));
	btn_rx_font->callback((Fl_Callback*)cb_btn_rx_font);

	update_config_rx_font();

	btn_rx_bkgrnd_color = new Fl_Button(x + 267, y, 115, h, _("BkGnd Color"));
	btn_rx_bkgrnd_color->callback((Fl_Callback*)cb_btn_rx_bkgnd_color);

	btn_rx_default = new Fl_Button(x + 390, y, h, h, _("D"));
	btn_rx_default->callback((Fl_Callback*)cb_btn_rx_default);

	y += (h + 5);

	txt_tx_font_display = new Fl_Output(x, y, 150, h, _("Tx"));
	txt_tx_font_display->align(Fl_Align(FL_ALIGN_RIGHT));
	txt_tx_font_display->value(_("TX Window Font"));

	btn_tx_font = new Fl_Button(x + 195, y, 65, h, _("Font"));
	btn_tx_font->callback((Fl_Callback*)cb_btn_tx_font);

	update_config_tx_font();

	btn_tx_bkgrnd_color = new Fl_Button(x + 267, y, 115, h, _("BkGnd Color"));
	btn_tx_bkgrnd_color->callback((Fl_Callback*)cb_btn_tx_bkgnd_color);


	btn_tx_default = new Fl_Button(x + 390, y, h, h, _("D"));
	btn_tx_default->callback((Fl_Callback*)cb_btn_tx_default);


	h = 25;
	w = 115;
	os = 15;

	btn_close_config_window = new Fl_Button((d_w - w - os), (d_h - h - os), w, h, _("Close"));
	btn_close_config_window->callback((Fl_Callback*)cb_btn_close_config_window);

	configure_window->end();

	return configure_window;
}

/** *******************************************************************
 * \brief Display the configuration dialog in the center of the main
 * window.
 **********************************************************************/
void cb_open_configure(Fl_Widget *a, void *b)
{
	if(!configure_window) {
		configure_window = config_window();
		if(configure_window) {
			int x = (window_frame->x_root() + ((window_frame->w() - configure_window->w()) * 0.50));
			int y = (window_frame->y_root() + ((window_frame->h() - configure_window->h()) * 0.50));
			int h = configure_window->h();
			int w = configure_window->w();
			configure_window->resize(x, y, w, h);
		}
	}
	
	if(!configure_window) {
		LOG_INFO("%s", _("Config Panel Open Failure"));
		return;
	}
	
	configure_window->show();
}

