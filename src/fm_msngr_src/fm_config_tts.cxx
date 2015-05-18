// ----------------------------------------------------------------------------
// fm_config_tts.cxx
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

#include <FL/fl_show_colormap.H>
#include <FL/Fl_Color_Chooser.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Multiline_Input.H>

#include "fm_msngr_src/fm_config_tts.h"
#include "fm_msngr_src/fm_control_tts.h"
#include "fm_msngr_src/fm_config.h"
#include "fm_msngr_src/fm_control.h"
#include "fm_msngr_src/fm_dialog.h"

#include "status.h"
#include "font_browser.h"
#include "flslider2.h"
#include "util.h"
#include "gettext.h"
#include "debug.h"

Fl_Double_Window * config_tts_window      = (Fl_Double_Window *) 0;
Fl_Check_Button  * chk_enable_tts         = (Fl_Check_Button *) 0;
Fl_Check_Button  * chk_voice_all          = (Fl_Check_Button *) 0;
Fl_Input         * file_festival_path     = (Fl_Input *) 0;
Fl_Input         * input_festival_cl      = (Fl_Input *) 0;
Fl_Multiline_Input * input_festival_opt   = (Fl_Multiline_Input *) 0;
Fl_Input         * file_audio_player_path = (Fl_Input *) 0;
Fl_Input         * input_audio_player_opt = (Fl_Input *) 0;
Fl_Button        * btn_test_audio_player  = (Fl_Button *) 0;
Fl_Button        * btn_close_config       = (Fl_Button *) 0;
Fl_Button        * btn_festival_path      = (Fl_Button *) 0;
Fl_Button        * btn_audio_player_path  = (Fl_Button *) 0;

static Fl_Double_Window * fm_config_tts_window(void);


/** *******************************************************************
 * \brief Enable/Disable Text to Speech Translations
 **********************************************************************/
void cb_chk_enable_tts(Fl_Widget *a, void *b)
{
	bool value = chk_enable_tts->value();

	progStatus.festival_enabled = value;

	if(value) {
		open_festival();
	} else {
		close_festival();
	}
}

/** *******************************************************************
 * \brief Enable/Disable Text to Speech Translations of both framed and
 * unframed ARQ data.
 **********************************************************************/
void cb_chk_voice_all(Fl_Widget *a, void *b)
{
	progStatus.festival_voice_all_enabled = chk_voice_all->value();
}

/** *******************************************************************
 * \brief Test Festival Configuration.
 **********************************************************************/
void cb_btn_test_audio_player(Fl_Widget *a, void *b)
{
	festival_test_config();
}

/** *******************************************************************
 * \brief Festival executable path.
 **********************************************************************/
void cb_file_festival_path(Fl_Widget *a, void *b)
{
	progStatus.festival_path.assign(file_festival_path->value());
}

/** *******************************************************************
 * \brief Select festival executable path.
 **********************************************************************/
void cb_btn_festival_path(Fl_Widget *a, void *b)
{
	bool selected = false;
	std::string filename  = "";
	std::string path = "";

	selected = select_file(filename, path, (const char *) _("Select Festival Executable"));
	if(selected) {
		file_festival_path->value(filename.c_str());
		file_festival_path->do_callback();
	}
}

/** *******************************************************************
 * \brief Festival commandline options.
 **********************************************************************/
void cb_input_festival_cl(Fl_Widget *a, void *b)
{
	progStatus.input_festival_cl.assign(input_festival_cl->value());
}


/** *******************************************************************
 * \brief Festival commandline options.
 **********************************************************************/
void cb_input_festival_opt(Fl_Widget *a, void *b)
{
	progStatus.festival_path_opts.assign(input_festival_opt->value());
}

/** *******************************************************************
 * \brief Select Audo Player executable path.
 **********************************************************************/
void cb_file_audio_player_path(Fl_Widget *a, void *b)
{
	progStatus.audio_player_path.assign(file_audio_player_path->value());
}

/** *******************************************************************
 * \brief Select Audo Player executable path.
 **********************************************************************/
void cb_btn_audio_player_path(Fl_Widget *a, void *b)
{
	bool selected = false;
	std::string filename  = "";
	std::string path = "";

	selected = select_file(filename, path, (const char *) _("Select Audio Player Executable"));
	if(selected) {
		file_audio_player_path->value(filename.c_str());
		file_audio_player_path->do_callback();
	}
}

/** *******************************************************************
 * \brief Audo Player commandline options.
 **********************************************************************/
void cb_input_audio_player_opt(Fl_Widget *a, void *b)
{
	progStatus.audio_player_path_opts.assign(input_audio_player_opt->value());
}


/** *******************************************************************
 * \brief Cqllback function to close the tts configuration panel.
 **********************************************************************/
void cb_btn_close_config(Fl_Widget *a, void *b)
{
	close_tts_config();
}

/** *******************************************************************
 * \brief Close TTS configuration panel.
 **********************************************************************/
void close_tts_config(void)
{
	if(config_tts_window)
		config_tts_window->hide();
}

/** *******************************************************************
 * \brief Display the TTS configuration dialog in the center of the main
 * window.
 **********************************************************************/
void cb_open_tts_configure(Fl_Widget *a, void *b)
{
	if(!config_tts_window)
		config_tts_window = fm_config_tts_window();

	if(!config_tts_window) {
		LOG_INFO("%s", _("Config TTS Panel Open Failure"));
		return;
	}

	// Center in the main window
	int x = (window_frame->x_root() + ((window_frame->w() - config_tts_window->w()) * 0.50));
	int y = (window_frame->y_root() + ((window_frame->h() - config_tts_window->h()) * 0.50));
	int h = config_tts_window->h();
	int w = config_tts_window->w();

	config_tts_window->resize(x, y, w, h);
	config_tts_window->show();
}

/** *******************************************************************
 * \brief Configuration Dialog Creation Code
 **********************************************************************/
static Fl_Double_Window * fm_config_tts_window(void)
{
	int w_h     = 360;
	int w_w     = 512;
	int w_os    = 16;
	int _x      = w_os;
	int _y      = w_os;
	int _h1     = 0;
	int _w1     = 0;
	int _ws     = (w_os >> 2);
	int _btn_h  = 24;
	int _btn_w  = 88;
	int _opt_h  = 24;
	int _mopt_h = 144;
	int _exe_h  = 24;

	Fl_Double_Window * config_tts_window = new Fl_Double_Window(w_w, w_h);

	_x  = w_os;
	_y  = w_os;
	_h1 = w_os;

	chk_enable_tts = new Fl_Check_Button(_x, _y, _h1 + 3, _h1, _("Enable TTS (Festival)"));
	chk_enable_tts->tooltip(_("Translate all received framed data to speech"));
	chk_enable_tts->callback((Fl_Callback*)cb_chk_enable_tts);
	chk_enable_tts->when(FL_WHEN_CHANGED);
	chk_enable_tts->value(progStatus.festival_enabled);

	chk_voice_all = new Fl_Check_Button((w_w >> 1), _y, _h1 + 3, _h1, _("TTS All Text"));
	chk_voice_all->tooltip(_("Translate all received framed and unframed ARQ data"));
	chk_voice_all->callback((Fl_Callback*)cb_chk_voice_all);
	chk_voice_all->when(FL_WHEN_CHANGED);
	chk_voice_all->value(progStatus.festival_voice_all_enabled);

	_x  = w_os;
	_y  += (_h1 + w_os + w_os);
	_h1 = _exe_h;
	_w1 = w_w - (w_os << 1);

	file_festival_path = new Fl_Input(_x, _y, _w1, _h1, _("Festival Exec Path"));
	file_festival_path->align(Fl_Align(FL_ALIGN_TOP_LEFT));
	file_festival_path->tooltip(_("Festival executable path."));
	file_festival_path->callback((Fl_Callback*)cb_file_festival_path);
	file_festival_path->value(progStatus.festival_path.c_str());

	_x  = w_w - _btn_w - w_os;
	_y  += (_h1 + _ws);
	_h1 = _btn_h;
	_w1 = _btn_w;

	btn_festival_path       = new Fl_Button(_x, _y, _w1, _h1, _("Select Path"));
	btn_festival_path->tooltip(_("Select Festival executable path."));
	btn_festival_path->callback((Fl_Callback*)cb_btn_festival_path);

	_x  = w_os;
	_y  += (_h1 + _ws);
	_h1 = _opt_h;
	_w1 = w_w - (w_os << 1);

	input_festival_cl = new Fl_Input(_x, _y, _w1, _h1, _("Festival Command line"));
	input_festival_cl->align(Fl_Align(FL_ALIGN_TOP_LEFT));
	input_festival_cl->tooltip(_("Festival command line options"));
	input_festival_cl->callback((Fl_Callback*)cb_input_festival_cl);
	input_festival_cl->value(progStatus.input_festival_cl.c_str());

	_x  = w_os;
	_y  += (_h1 + w_os + _ws);
	_h1 = _mopt_h;
	_w1 = w_w - (w_os << 1);

	input_festival_opt = new Fl_Multiline_Input(_x, _y, _w1, _h1, _("Festival Scheme Config"));
	input_festival_opt->align(Fl_Align(FL_ALIGN_TOP_LEFT));
	input_festival_opt->tooltip(_("Festival command line options"));
	input_festival_opt->callback((Fl_Callback*)cb_input_festival_opt);
	input_festival_opt->value(progStatus.festival_path_opts.c_str());


#if 0
	_x  = w_os;
	_y  += (_h1 + w_os + w_os);
	_h1 = _exe_h;
	_w1 = w_w - (w_os << 1);

	file_audio_player_path = new Fl_Input(_x, _y, _w1, _h1, _("Audio Player Exec Path"));
	file_audio_player_path->align(Fl_Align(FL_ALIGN_TOP_LEFT));
	file_audio_player_path->tooltip(_("Path to the Audio Player."));
	file_audio_player_path->callback((Fl_Callback*)cb_file_audio_player_path);
	file_audio_player_path->value(progStatus.audio_player_path.c_str());

	_x  = w_w - _btn_w - w_os;
	_y  += (_h1 + _ws);
	_h1 = _btn_h;
	_w1 = _btn_w;

	btn_audio_player_path       = new Fl_Button(_x, _y, _w1, _h1, _("Select Path"));
	btn_audio_player_path->tooltip(_("Select the path to the Audio Player"));
	btn_audio_player_path->callback((Fl_Callback*)cb_btn_audio_player_path);

	_x  = w_os;
	_y  += (_h1 + _ws);
	_h1 = _opt_h;
	_w1 = w_w - (w_os << 1);

	input_audio_player_opt = new Fl_Input(_x, _y, _w1, _h1, _("Audio Player Option(s)"));
	input_audio_player_opt->align(Fl_Align(FL_ALIGN_TOP_LEFT));
	input_audio_player_opt->tooltip(_("Audio player command line options"));
	input_audio_player_opt->callback((Fl_Callback*)cb_input_audio_player_opt);
	input_audio_player_opt->value(progStatus.audio_player_path_opts.c_str());
#endif

	_x  = (w_w - (_btn_w + _btn_w + w_os)) >> 1;
	_y  = w_h - w_os - _btn_h;

	btn_test_audio_player  = new Fl_Button(_x, _y, _btn_w, _btn_h, _("Test TTS"));
	btn_test_audio_player->tooltip(_("Translate all received framed and unframed ARQ data"));
	btn_test_audio_player->callback((Fl_Callback*)cb_btn_test_audio_player);

	_x += _btn_w + w_os;

	btn_close_config       = new Fl_Button(_x, _y, _btn_w, _btn_h, _("Close"));
	btn_close_config->tooltip(_("Translate all received framed and unframed ARQ data"));
	btn_close_config->callback((Fl_Callback*)cb_btn_close_config);
	
	return config_tts_window;
}
