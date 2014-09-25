// =====================================================================
//
// ztimer.cxx
//
// Author(s):
//  Copyright (C) 2014 Robert Stiles, KK5VD
//
// This file is part of FLLNK.
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
#include "gettext.h"
#include "flinput2.h"
#include "status.h"
#include "threads.h"
#include "fllnk.h"
#include "fllnk_dialog.h"
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
extern std::string g_modem;

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
 * \brief FLLNK event timer.
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

	if((tx_state == "TX") && tx_toggle == false)  {
		tx_start = watch_dog_seconds;
		tx_toggle = true;
	}

	if((tx_state == "RX") && tx_toggle == true) {
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
}
