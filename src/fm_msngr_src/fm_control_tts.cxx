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
#include <string>

#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Native_File_Chooser.H>

#include "fm_msngr.h"
#include "fm_msngr_src/fm_control_tts.h"
#include "fm_msngr_src/tts_translate_table.h"

#include "debug.h"
#include "gettext.h"
#include "status.h"
#include "gettext.h"
#include "threads.h"

static FILE *festival_fp = (FILE *)0;

pthread_t       fm_speak_thread_id;
pthread_mutex_t fm_mutex_speak_buffer = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t fm_mutex_speaker      = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t fm_mutex_send_speaker = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  fm_speak_cond         = PTHREAD_COND_INITIALIZER;

static std::string input_word_buffer;
static std::string word_parse_buffer;
static std::string word_buffer;

static bool speak_thread_running = false;
static bool speak_exit_flag      = false;

/** *******************************************************************
 * \brief Check for Data and Send data to the speech generator.
 **********************************************************************/
void word_check_send(int &word_count, std::string _data)
{
	static std::string send_data = "";
	{
		guard_lock speak_buffer(&fm_mutex_send_speaker);
		send_data.assign(_data);
	}

	if(word_count && !_data.empty()) {
		festival_speak(_data);
		word_count = 0;
	}
}

/** *******************************************************************
 * \brief Parser for words and spaces
 **********************************************************************/
void process_word_buffer(void)
{
	if(speak_exit_flag) return;

	{
		guard_lock speak_buffer(&fm_mutex_speak_buffer);
		word_parse_buffer.append(input_word_buffer);
		input_word_buffer.clear();
	}

	static int word_count = 0;
	static std::string a_word = "";

	if(word_parse_buffer.empty() && a_word.empty()) return;

	int ch    = 0;
	int last_ch = 256;
	bool done = false;
	bool word_toggle = false;

	while(!done) {

		if(speak_exit_flag) return;

		if(word_parse_buffer.empty()) return;
		if(speak_exit_flag) return;

		ch = word_parse_buffer[0];
		word_parse_buffer.erase(0,1);

		if(!ch)
			break;

		if((ch > ' ') && (word_toggle == false)) {
			word_toggle = true;
		} else if((ch <= ' ') && word_toggle == true) {
			word_count++;
			word_toggle = false;
		}

		if((ch == '\n') && (ch == last_ch)) {
			done = true;
			continue;
		}

		if((ch <= ' ') && (last_ch <= ' ')) {
			last_ch = ch;
			continue;
		}

		last_ch = ch;

		ch = (ch < ' '  ? ' ' : ch);

		if((ch == '\"') || (ch == '\''));
		else a_word += ch;

		switch(ch) {
			case '.':
			case ',':
			case '!':
			case '?':
				done = true;
				word_count++;
				break;
		}
	}

	done = false;

	if(!a_word.empty()) {
		guard_lock speak_buffer(&fm_mutex_speaker);
		word_buffer.assign(a_word);
		a_word.clear();
	}

	if(!word_buffer.empty()) {
		word_check_send(word_count, word_buffer);
		word_buffer.clear();
	}

	return;
}


/** *******************************************************************
 * \brief Start Speak Thread
 **********************************************************************/
void *speak_loop(void *ptr)
{
	speak_thread_running = true;
	speak_exit_flag = false;

	struct timespec timeout;
	memset(&timeout, 0, sizeof(timeout));

	while(!speak_exit_flag) {
		MilliSleep(100);
		process_word_buffer();
	}

	pthread_cancel(pthread_self());
	return (void *)0;
}

/** *******************************************************************
 * \brief Start Speak Thread
 **********************************************************************/
void start_speak_thread(void)
{
	if(speak_thread_running) return;

	pthread_create(&fm_speak_thread_id, NULL,
				   speak_loop, (void *)0);
}

/** *******************************************************************
 * \brief Stop Speak Thread
 **********************************************************************/
void stop_speak_thread(void)
{
	speak_exit_flag = true;
	pthread_join(fm_speak_thread_id, 0);
	speak_thread_running = false;
}

/** *******************************************************************
 * \brief Buffer incomming data for processing
 **********************************************************************/
void festival_speak_buffer(std::string message)
{
	guard_lock speak_buffer(&fm_mutex_speak_buffer);
	input_word_buffer.append(message);
}

/** *******************************************************************
 * \brief Pass a string to Festival for speech generation.
 **********************************************************************/
extern void festival_test_config(void)
{
	if(!festival_fp) return;

	if(!progStatus.festival_path_opts.empty()) {
		fputs(progStatus.festival_path_opts.c_str(), festival_fp);
		fputc('\n', festival_fp);
		fflush(festival_fp);
	}

	std::string tmp;
	tmp.assign(_("Festival Operational"));
	festival_speak(tmp);
}

/** *******************************************************************
 * \brief Pass a string to Festival for speech generation.
 **********************************************************************/
void festival_speak(std::string message)
{
	guard_lock speaker(&fm_mutex_send_speaker);

	if(message.empty() || (!festival_fp)) return;

	static std::string _cmd = "(SayText \"";
	std::string _spk_msg;

	translate_hamspeak(message);

	_spk_msg.assign(_cmd).append(message).append("\")\n");
	printf("%s", _spk_msg.c_str());

	fputs(_spk_msg.c_str(), festival_fp);
	fflush(festival_fp);
}


/** *******************************************************************
 * \brief Festival Running Check.
 **********************************************************************/
bool festival_running(void)
{
	if(festival_fp)
		return true;
	return false;
}

/** *******************************************************************
 * \brief Open pipe to festival.
 **********************************************************************/
bool open_festival(void)
{

	if(festival_fp || speak_thread_running) {
		LOG_INFO("%s", _("Festival already running"));
		return true;
	}

	if(progStatus.festival_path.empty()) {
		LOG_INFO("%s", _("Path to Festival not set."));
		return false;
	}

	std::string _path;
	_path.assign(progStatus.festival_path);

	if(progStatus.input_festival_cl.empty()) {
		progStatus.input_festival_cl.assign("--pipe");
	}

	_path.append(" ").append(progStatus.input_festival_cl);

	/* Create one way pipe line with call to popen() */

	if ((festival_fp = popen(_path.c_str(), "w")) == NULL) {
		LOG_INFO("%s", _("Festival pipe creation failure."));
		return false;
	}

	translate_loader();

	if(!speak_thread_running)
		start_speak_thread();

	festival_test_config();

	return(0);
}

/** *******************************************************************
 * \brief Close pipe to festival.
 **********************************************************************/
void close_festival(void)
{
	if(speak_thread_running)
		stop_speak_thread();

	if(festival_fp) {
		LOG_INFO("%s", _("Closing Festival TTS"));
		pclose(festival_fp);
		festival_fp = (FILE *)0;
	}

	close_translate_hamspeak();
}


/** *******************************************************************
 * \brief Select a file.
 **********************************************************************/
bool select_file(std::string &filename,
				 std::string &path,
				 const char *title = (const char *) "Select a File")
{
#if 0
	// Create the file chooser, and show it
	Fl_File_Chooser chooser(".",                        // directory
							"*",                        // filter
							Fl_File_Chooser::SINGLE,    // chooser type
							"Select Executable");       // title
	chooser.show();

	// Block until user picks something.
	//     (The other way to do this is to use a callback())
	//
	while(chooser.shown())
	{ Fl::wait(); }

	// User hit cancel?
	if ( chooser.value() == NULL )
		return false;

	filename.assign(chooser.value());

#else

	Fl_Native_File_Chooser chooser;

	chooser.title(title);
	chooser.type(Fl_Native_File_Chooser::BROWSE_FILE);
	chooser.filter("ALL\t*\n");
	chooser.directory("");
	
	switch ( chooser.show() ) {
		case -1:
		case  1:
			return false;
	}
	
	filename.assign(chooser.filename());
#endif
	
	path.assign(chooser.directory());
	
	int c = path[path.length() - 1];
	if((c != '\\') || (c != '/')) {
		path += '/';
	}
	
	return true;
}
