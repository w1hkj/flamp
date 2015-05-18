// ----------------------------------------------------------------------------
//  fm_port_io.h
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

#ifndef __flfmsgr_fm_port_io__
#define __flfmsgr_fm_port_io__

extern bool numbers_and_dots_only(char *str, int expected_argc);
extern void start_io(void);
extern void arqio_loop_shutdown(void);

extern bool arqio_exit_flag;
extern bool arqio_thread_running;

#endif /* defined(__flfmsgr_fm_port_io__) */
