// =====================================================================
//
// ztimer.cxx
//
// Author(s):
//  Copyright (C) 2014 Robert Stiles, KK5VD
//
// This file is part of FLAMP.
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
// =====================================================================


#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <ctime>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <libgen.h>
#include <ctype.h>
#include <sys/time.h>

#include <FL/Fl.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/x.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_File_Icon.H>

#include "debug.h"
#include "util.h"
#include "nls.h"
#include "gettext.h"
#include "flinput2.h"
#include "status.h"
#include "threads.h"
#include "flamp.h"
#include "flamp_dialog.h"
#include "xml_io.h"
#include "ztimer.h"

static char szoutTimeValue[] = "12:30:00";
static char sztime[] = "123000";
static int  ztime;

time_t ztime_current;
time_t ztime_end;

bool continuous_exception = false;
bool event_timer_on = false;
bool exit_watch_dog = false;

float tx_time_g = 0;

pthread_t *watch_dog_thread = 0;
pthread_mutex_t mutex_watch_dog = PTHREAD_MUTEX_INITIALIZER;
time_t watch_dog_seconds = 0;

time_t time_check(void);

extern bool event_bail_flag;
bool tx_ztimer_flag = false;

/** ********************************************************
 * \brief Trim leading and trailing spaces from string.
 * \param s String to modify
 * \return s modified string.
 ***********************************************************/
static inline std::string &trim(std::string &s) {
	char *buffer = (char *)0;
	char *dst    = (char *)0;
	char *end    = (char *)0;
	char *src    = (char *)0;
	long count   = s.size();

	buffer = new char[count + 1];
	if(!buffer) return s;

	memcpy(buffer, s.c_str(), count);
	buffer[count] = 0;

	dst = src = buffer;
	end = &buffer[count];

	while(src < end) {
		if(*src > ' ') break;
		src++;
	}

	if(src > dst) {
		while((dst < end) && (src < end))
			*dst++ = *src++;
		*dst = 0;
	}

	while(end >= buffer) {
		if(*end > ' ') break;
		*end-- = 0;
	}

	s.assign(buffer);

	delete [] buffer;

	return s;
}

/** ********************************************************
 *
 ***********************************************************/
void turn_off_events(void *ptr)
{
	do_events->value(0);
	do_events_flag = 0;
	cb_do_events((Fl_Light_Button *)0, (void*)0);
}

/** ********************************************************
 * \brief Stop currently running events.
 * \param void Not used
 * \return void No returned values.
 ***********************************************************/
void stop_events(void)
{
	ztime_end = 0;
	continuous_exception = false;
	if(transmitting) abort_request();
}

#if 0
/** ********************************************************
 * \brief Parse event time entries
 * \param delete_flag if true deletes the current event on completion.
 * \return true = no errors, false = errors
 ***********************************************************/
time_t parse_repeat_times(bool delete_flag, unsigned int zt)
{
	bool local_flag = false;
	char etime[10];
	char ttime[10];

	time_t diff         = 0;
	time_t diff_hs      = 0;
	time_t diff_ms      = 0;
	time_t e_h          = 0;
	time_t e_m          = 0;
	time_t hm_zt        = 0;
	time_t s_h          = 0;
	time_t s_m          = 0;
	time_t time_end     = 0;
	time_t time_seconds = 0;
	time_t time_start   = 0;

	snprintf(ttime, sizeof(ttime), "%06d", zt);

	ttime[4] = '-';
	ttime[5] = 0;

	hm_zt = atoi(ttime);

	size_t s = progStatus.repeat_times.length();
	size_t p = progStatus.repeat_times.find(ttime);

	if(p != std::string::npos) {
		if(progStatus.repeat_every != 8) return false;
		delete_flag = false;
		continuous_exception = true;
		local_flag = true;
	} else {
		ttime[4] = 0;
		p = progStatus.repeat_times.find(ttime);
		if(p == std::string::npos) return 0;
		if(p > 0)
			if(progStatus.repeat_times[p - 1] == '-')
				return 0;
		local_flag = false;
		time_end = time_check();
	}

	int len = 4;
	while (((p + len) < s) && !isdigit(progStatus.repeat_times[p + len])) len++;

	int count = 0;

	if(continuous_exception && local_flag) {

		while (((p + len) < s) && isdigit(progStatus.repeat_times[p + len])) {
			etime[count++] = progStatus.repeat_times[p + len];
			if(count > 4) break;
			if((p + len) < s) len++;
		}

		etime[4] = 0;
		time_end = (time_t) atoi(etime);
		time_start = hm_zt;

		e_h = time_end / 100;
		e_m = time_end - (e_h * 100);

		s_h = time_start / 100;
		s_m = time_start - (s_h * 100);

		if(time_end < time_start) {
			e_h += 23;
			diff_ms = (60 - s_m) + e_m;
		} else {
			diff_ms = e_m - s_m;
		}

		diff_hs = e_h - s_h;

		diff_hs     *= 3600;
		diff_ms     *= 60;
		time_seconds = diff_hs + diff_ms;
		time_end     = time_check() + ((time_t) time_seconds);

#ifdef CONSOLE_OUTPUT
		printf("    e_h = %ld      e_m = %ld\n", e_h, e_m);
		printf("    s_h = %ld      s_m = %ld\n", s_h, s_m);
		printf("diff_hs = %ld  diff_ms = %ld\n", diff_hs, diff_ms);
		printf("  hm_zt = %ld duration = %f\n",  hm_zt, time_seconds/3600.0);
#endif // CONSOLE_OUTPUT

	}

	if(delete_flag) {
		if((p + len) > s) {
			len = s - p;
			if(len < 0) len = 0;
		}

		if(len)
			progStatus.repeat_times.erase(p, len);

		txt_repeat_times->value(progStatus.repeat_times.c_str());

		if (progStatus.repeat_times.empty()) {
			do_events->value(0);
			do_events_flag = 0;
			cb_do_events((Fl_Light_Button *)0, (void*)0);
		}
	}

	return time_end;
}
#else


/** ********************************************************
 *
 ***********************************************************/
char *skip_spaces(char *cPtr, char *ePtr)
{
	if(!cPtr || !ePtr)
		return (char *)0;

	while(1) {
		if(*cPtr == 0)
			return (char *)0;

		if(cPtr >= ePtr)
			return (char *)0;

		if(*cPtr > ' ') break;
		cPtr++;
	}

	return cPtr;
}


/** ********************************************************
 *
 ***********************************************************/
char *skip_numbers(char *cPtr, char *ePtr)
{
	if(!cPtr || !ePtr)
		return (char *)0;

	while(1) {
		if(*cPtr == 0)
			return (char *)0;

		if(cPtr >= ePtr)
			return (char *)0;

		if(*cPtr == '-' || isdigit(*cPtr)) {
			cPtr++;
			continue;
		}

		break;
	}

	return cPtr;
}

/** ********************************************************
 * \brief Parse event time entries
 * \param delete_flag if true deletes the current event on completion.
 * \return true = no errors, false = errors
 ***********************************************************/
bool parse_repeat_times(bool delete_flag, unsigned int zt, int mode)
{
	time_t time_end       = 0;
	time_t time_start     = 0;
	//time_t time_diff      = 0;
	time_t time_current   = 0;
	//time_t seconds_in_day = 0;

	//int index = 0;
	int count = 0;
	char *cPtr = (char *)0;
	char *ePtr = (char *)0;

	char ttime[10];
	//char etime[10];

	size_t s = 0;
	size_t p = 0;
	int len = 0;

	snprintf(ttime, sizeof(ttime), "%06d", zt);
	ttime[4] = 0;
	time_current = atoi(ttime);

	switch(mode) {
		case 6:
		case 7:

			s = progStatus.repeat_times.length();
			p = progStatus.repeat_times.find(ttime);

			if(p == std::string::npos) {
				return false;
			}

			len = 4;
			while (((p + len) < s) &&
				   !isdigit(progStatus.repeat_times[p + len]) &&
				   progStatus.repeat_times[p + len] == '-')
				len++;

			if(delete_flag) {
				if((p + len) > s) {
					len = s - p;
					if(len < 0) len = 0;
				}

				if(len)
					progStatus.repeat_times.erase(p, len);

				progStatus.repeat_times = trim(progStatus.repeat_times);

				txt_repeat_times->value(progStatus.repeat_times.c_str());

				if (progStatus.repeat_times.empty()) {
					Fl::awake(turn_off_events, (void *)0);
				}
			}
			break;

		case 8:
			cPtr = (char *) progStatus.repeat_times.c_str();
			ePtr = &cPtr[progStatus.repeat_times.size()];

			while(1) {
				cPtr = skip_spaces(cPtr, ePtr);
				if(cPtr == (char *)0) return 0;

				count = sscanf(cPtr, "%ld-%ld", &time_start, &time_end);
				if(count < 2) return false;

				if(time_end < time_start) {
					if(((time_current >= time_start) && (time_current <= 2400)) ||
					   ((time_current >= 0)          && (time_current <= time_end)))
						return true;
				}

				if((time_current >= time_start) && (time_current <= time_end))
					return true;

				cPtr = skip_numbers(cPtr, ePtr);
				if(cPtr == (char *)0) return false;
			}

			break;

		default:;
	}

	return true;
}

#endif

/** ********************************************************
 * \brief Thread safe time() function.
 * \param void No paramters (void).
 * \return time_t in seconds.
 ***********************************************************/
time_t time_check(void)
{
	pthread_mutex_lock(&mutex_watch_dog);
	time_t the_time = time(0);
	pthread_mutex_unlock(&mutex_watch_dog);

	return the_time;
}

/** ********************************************************
 * \brief Start/Continue FLTK's event timer.
 * \param flag true initialize, false reset to continue.
 * \param tv struct timeval storage by reference.
 * \return void
 ***********************************************************/
void execute_ztimer(bool flag, struct timeval &tv)
{
	pthread_mutex_lock(&mutex_watch_dog);

	gettimeofday(&tv, NULL);

	if (flag) {
		double st = 1.0 - tv.tv_usec / 1e6;
		Fl::repeat_timeout(st, ztimer);
	} else {
		Fl::repeat_timeout(1.0, ztimer);
	}

	pthread_mutex_unlock(&mutex_watch_dog);
}

/** *******************************************************
 * \brief Watch dog for ztimer.
 *
 * In the event the user adjusts the system time there is
 * a possiblility the event timer will timeout and not
 * continue.  The watch dog loop monitors the condition and
 * will restart the event timer if needed.
 *
 * \param *p unused. Required for pthread call.
 * \return void* null unused. Required for pthread call.
 *
 ***********************************************************/
void *watch_dog_loop(void *p)
{
	time_t current_time = 0;
	double diff_time = 0;
	double limit = 10.0;
	int counter = 0;
	struct timeval tv;

	exit_watch_dog = false;

	while(!exit_watch_dog) {
		MilliSleep(250);
		if((counter++) < 40) continue; // Check every 10 seconds
		else counter = 0;

		current_time = time_check();
		diff_time = (double) (watch_dog_seconds - current_time);
		if((diff_time > limit) || (diff_time < (-limit))) {
			execute_ztimer(false, tv);
		}
	}

	return (void *)0;
}

/** ********************************************************
 * \brief FLAMP event timer.
 * \param *first_call. if true initialzation occures. Otherwise, the
 * count down timer is reset.
 * \return void (nothing)
 ***********************************************************/
void ztimer(void* first_call)
{
	struct timeval tv;
	static bool tx_toggle = false;
	static time_t tx_start = 0;
	static time_t tx_time_seconds = 0;
	static time_t tx_time_minutes = 0;
	static std::string tx_state;
	static std::string tx_duration;
	bool flag = false;

	if (first_call) {
		flag = true;
		execute_ztimer(flag, tv);
	} else {
		execute_ztimer(flag, tv);
	}

	if(generate_time_table) return; // No Events Allow in Time Table Generation Mode.

	struct tm tm;
	time_t t_temp;

	t_temp = (time_t) tv.tv_sec;
	gmtime_r(&t_temp, &tm);

	if (!strftime(sztime, sizeof(sztime), "%H%M%S", &tm))
		strcpy(sztime, "000000");

	ztime = atoi(sztime);

	if (!strftime(szoutTimeValue, sizeof(szoutTimeValue), "%H:%M:%S", &tm))
		memset(szoutTimeValue, 0, sizeof(szoutTimeValue));

	tx_state = get_trx_state();

	watch_dog_seconds = time_check();

	if((tx_state == "RX"))
		tx_ztimer_flag = false;
	else
		tx_ztimer_flag = true;

	//LOG_DEBUG("tx_ztimer_flag=%s", tx_ztimer_flag ? "TRUE" : "FALSE");

	if(tx_ztimer_flag && tx_toggle == false)  {
		tx_start = watch_dog_seconds;
		tx_toggle = true;
	}

	if(!tx_ztimer_flag && tx_toggle == true) {
		tx_time_seconds = watch_dog_seconds - tx_start;
		tx_start = tx_time_seconds;
		tx_time_minutes = tx_time_seconds / 60.0;
		tx_time_seconds -= (tx_time_minutes * 60);
		tx_toggle = false;
		if(tx_time_g <= 0.0) tx_time_g = 1;

		LOG_DEBUG("TX time [mm:ss]: %02ld:%02ld [m:%ld c:%1.3f r:%1.6f] %s", \
				  tx_time_minutes, tx_time_seconds, tx_start, tx_time_g, \
				  tx_start/tx_time_g, g_modem.c_str());
	}

	outTimeValue->value(szoutTimeValue);

	if (do_events_flag && !tx_ztimer_flag && !transmitting) {
		if (progStatus.repeat_at_times && (ztime % 100 == 0) && (progStatus.repeat_every != 8)) {
			switch (progStatus.repeat_every) {
				case 0 : // every 5 minutes
					if (ztime % 500 == 0) 	Fl::awake(transmit_queue_main_thread, (void *)0);
					break;
				case 1 : // every 15 minutes
					if (ztime % 1500 == 0) Fl::awake(transmit_queue_main_thread, (void *)0);
					break;
				case 2 : // every 30 minutes
					if (ztime % 3000 == 0) Fl::awake(transmit_queue_main_thread, (void *)0);
					break;
				case 3 : // hourly
					if (ztime % 10000 == 0) Fl::awake(transmit_queue_main_thread, (void *)0);
					break;
				case 4 : // even hours
					if (ztime == 0 || ztime == 20000 || ztime == 40000 ||
						ztime == 60000 || ztime == 80000 || ztime == 100000 ||
						ztime == 120000 || ztime == 140000 || ztime == 160000 ||
						ztime == 180000 || ztime == 200000 || ztime == 220000 )
						Fl::awake(transmit_queue_main_thread, (void *)0);
					break;
				case 5 : // odd hours
					if (ztime == 10000 || ztime == 30000 || ztime == 50000 ||
						ztime == 70000 || ztime == 90000 || ztime == 110000 ||
						ztime == 130000 || ztime == 150000 || ztime == 170000 ||
						ztime == 190000 || ztime == 210000 || ztime == 230000 )
						Fl::awake(transmit_queue_main_thread, (void *)0);
					break;
				case 6 : // at specified times
					flag = parse_repeat_times(false, ztime, progStatus.repeat_every);
					if (flag)
						Fl::awake(transmit_queue_main_thread, (void *)0);
					break;

				case 7 : // One time scheduled
					flag = parse_repeat_times(true, ztime, progStatus.repeat_every);
					if (flag)
						Fl::awake(transmit_queue_main_thread, (void *)0);
					break;

				default : // do nothing
					break;
			}
		} else if(progStatus.repeat_at_times && (progStatus.repeat_every == 8)) {

			ztime_current = time_check();

			ztime_end = parse_repeat_times(false, ztime, progStatus.repeat_every);

			if(ztime_end == 0) {
				continuous_exception = false;
				event_bail_flag = true;
			} else {
				continuous_exception = true;
				event_bail_flag = false;
			}

			if(continuous_exception) {
				try {
					Fl::awake(transmit_queue_main_thread, (void *)0);
				}
				catch (...) {
				}
			}
		} else if (progStatus.repeat_forever) {
			try {
				Fl::awake(transmit_queue_main_thread, (void *)0);
			}
			catch (...) {
			}
		}
	}
}
