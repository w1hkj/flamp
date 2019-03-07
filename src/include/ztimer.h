// =====================================================================
//
// ztimer.h
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

extern bool  continuous_exception;
extern bool  event_timer_on;
extern bool  exit_watch_dog;
extern float tx_time_g;

extern time_t watch_dog_seconds;
extern time_t ztime_current;
extern time_t ztime_end;

extern pthread_t * watch_dog_thread;
extern void      * watch_dog_loop(void *p);

extern time_t time_check(void);
extern void stop_events(void);
extern void ztimer(void* first_call);
extern bool parse_repeat_times(bool delete_flag, unsigned int zt, int mode);
