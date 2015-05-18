// ----------------------------------------------------------------------------
//      debug.cxx
//
// Copyright (C) 2008, 2012
//              Stelios Bounanos, M0GLD, Dave Freese, W1HKJ
//
//
// This file is part of FM_MSNGR.
//
// This is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ----------------------------------------------------------------------------

#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <string>
#include <iostream>
#include <fstream>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Menu_Button.H>

#include <FL/Fl_Browser.H>

#include "debug.h"
#include "icons.h"
#include "gettext.h"
#include "fm_msngr.h"

using namespace std;

#define MAX_LINES 65536

static FILE* wfile;
static FILE* rfile;
static int rfd;

static Fl_Double_Window*	window;
static Fl_Browser*			btext;
static string dbg_buffer;

debug* debug::inst = 0;
debug::level_e debug::level = debug::INFO_LEVEL;
//debug::level_e debug::level = debug::DEBUG_LEVEL;
uint32_t debug::mask = ~0u;

const char* prefix[] = { _("Quiet"), _("Error"), _("Warning"), _("Info"), _("Debug") };

pthread_mutex_t mutex_log = PTHREAD_MUTEX_INITIALIZER;

static void slider_cb(Fl_Widget* w, void*);
static void clear_cb(Fl_Widget *w, void*);
static void save_cb(Fl_Widget *w, void*);
static void synctext(void *);

/** ********************************************************
 *
 ***********************************************************/
int strlen_n(char *buf, size_t limit)
{
	size_t index = 0;
	int count = 0;
	int value = 0;

	if(limit < 1) return 0;
	if(!buf) return 0;

	for(index = 0; index < limit; index++) {
		value = *buf++;
		if(value == 0) break;
		count++;
	}
	return count;
}

/** ********************************************************
 *
 ***********************************************************/
void debug::start(const char* filename)
{
	if (debug::inst)
		return;
	inst = new debug(filename);

	window = new Fl_Double_Window(600, 256, _("Debug log"));

	int pad = 2;

	Fl_Slider* slider = new Fl_Slider(pad, pad, 128, 20, prefix[level]);
	slider->tooltip(_("Change log level"));
	slider->align(FL_ALIGN_RIGHT);
	slider->type(FL_HOR_NICE_SLIDER);
	slider->range(0.0, LOG_NLEVELS - 1);
	slider->step(1.0);
	slider->value(level);
	slider->callback(slider_cb);

	Fl_Button* savebtn  = new Fl_Button(window->w() - 124, pad, 60, 20, "save");
	savebtn->callback(save_cb);

	Fl_Button* clearbtn = new Fl_Button(window->w() - 60, pad, 60, 20, "clear");
	clearbtn->callback(clear_cb);

	btext = new Fl_Browser(pad,  slider->h()+pad, window->w()-2*pad, window->h()-slider->h()-2*pad, 0);
	btext->textfont(FL_COURIER);
	window->resizable(btext);

	dbg_buffer.clear();

	window->end();
}

/** ********************************************************
 *
 ***********************************************************/
void debug::stop(void)
{
	delete inst;
	inst = 0;
	delete window;
}

static char fmt[1024];
static char sztemp[1024];
static string estr = "";
bool   debug_in_use = false;

/** ********************************************************
 *
 ***********************************************************/
void debug::log(level_e level, const char* func, const char* srcf, int line, const char* format, ...)
{
	if (!inst)
		return;

	pthread_mutex_lock(&mutex_log);

	snprintf(fmt, sizeof(fmt), "%c: %s: %s\n", *prefix[level], func, format);

	while(debug_in_use) MilliSleep(10);

	va_list args;
	va_start(args, format);

	vsnprintf(sztemp, sizeof(sztemp), fmt, args);
	estr.append(sztemp);

	va_end(args);

	fprintf(wfile, "%s", sztemp);
	fflush(wfile);

	append_dbg_buffer(sztemp);

	pthread_mutex_unlock(&mutex_log);

	Fl::awake(synctext, 0);
	MilliSleep(10);
}

/** ********************************************************
 *
 ***********************************************************/
void debug::slog(level_e level, const char* func, const char* srcf, int line, const char* format, ...)
{
	if (!inst)
		return;

	pthread_mutex_lock(&mutex_log);

	snprintf(fmt, sizeof(fmt), "%c:%s\n", *prefix[level], format);

	while(debug_in_use) MilliSleep(10);

	va_list args;
	va_start(args, format);

	vsnprintf(sztemp, sizeof(sztemp), fmt, args);
	estr.append(sztemp);

	va_end(args);
	fflush(wfile);

	pthread_mutex_unlock(&mutex_log);

	Fl::awake(synctext, 0);
	MilliSleep(10);
}

/** ********************************************************
 *
 ***********************************************************/
void debug::elog(const char* func, const char* srcf, int line, const char* text)
{
	log(ERROR_LEVEL, func, srcf, line, "%s: %s", text, strerror(errno));
}

/** ********************************************************
 *
 ***********************************************************/
void debug::show(void)
{
	window->show();
}

/** ********************************************************
 *
 ***********************************************************/
void debug::sync_text(void *arg)
{
	debug_in_use = true;
	size_t p0 = 0, p1 = estr.find('\n');
	while (p1 != string::npos) {
		btext->insert(1, estr.substr(p0,p1-p0).c_str());
		dbg_buffer.append(estr.substr(p0, p1 - p0)).append("\n");
		p0 = p1 + 1;
		p1 = estr.find('\n', p0);
	}
	estr = "";
	debug_in_use = false;
}

/** ********************************************************
 *
 ***********************************************************/
void debug::append_dbg_buffer(char * message)
{
	debug_in_use = true;
	std::string msg;
	char strTime[64];
	char *cPtr = (char *)0;
	int len = 0;
	int index = 0;
	int strIndex = 0;
	time_t current_time = 0;

	if(!message) return;

	msg.assign(message);

	if(msg.size() < 1) return;

	size_t p1 = msg.find('\n');

	if(p1 == string::npos) {
		msg.append("\n");
	}

	memset(strTime, 0, sizeof(strTime));
	current_time = time(0);
	cPtr = ctime(&current_time);

	len = strlen_n(cPtr, sizeof(strTime) - 1);

	if(len) {
		strIndex = 0;
		for(index = 0; index < len; index++) {
			if(cPtr[index] == '\r' || cPtr[index] == '\n')
				strTime[strIndex++] = 0;
			else
				strTime[strIndex++] = cPtr[index];
		}
		dbg_buffer.append(strTime).append("  ");
	}

	dbg_buffer.append(msg);

	debug_in_use = false;
}

/** ********************************************************
 *
 ***********************************************************/
debug::debug(const char* filename)
{
	if ((wfile = fopen(filename, "w")) == NULL)
		throw strerror(errno);
	setvbuf(wfile, (char*)NULL, _IOLBF, 0);

	if ((rfile = fopen(filename, "r")) == NULL)
		throw strerror(errno);
	rfd = fileno(rfile);
#ifndef __WIN32__
	int f;
	if ((f = fcntl(rfd, F_GETFL)) == -1)
		throw strerror(errno);
	if (fcntl(rfd, F_SETFL, f | O_NONBLOCK) == -1)
		throw strerror(errno);
#endif
}

/** ********************************************************
 *
 ***********************************************************/
debug::~debug()
{
	fclose(wfile);
	fclose(rfile);
}

/** ********************************************************
 *
 ***********************************************************/
static void synctext(void *d)
{
	debug_in_use = true;
	size_t p0 = 0, p1 = estr.find('\n');
	while (p1 != string::npos) {
		btext->insert(1, estr.substr(p0,p1-p0).c_str());
		p0 = p1 + 1;
		p1 = estr.find('\n', p0);
	}
	estr = "";
	debug_in_use = false;
}

/** ********************************************************
 *
 ***********************************************************/
static void slider_cb(Fl_Widget* w, void*)
{
	debug::level = (debug::level_e)((Fl_Slider*)w)->value();
	w->label(prefix[debug::level]);
	w->parent()->redraw();
}

/** ********************************************************
 *
 ***********************************************************/
static void clear_cb(Fl_Widget* w, void*)
{
	btext->clear();
	dbg_buffer.clear();
}

/** ********************************************************
 *
 ***********************************************************/
static void save_cb(Fl_Widget* w, void*)
{
	if (!btext->size()) return;
	string filename = HomeDir;
	filename.append("events.txt");
	ofstream out;
	out.open(filename.c_str(), ios::app);
	out << dbg_buffer;
	out.close();
	fl_alert2("Saved in %s", filename.c_str());
}
