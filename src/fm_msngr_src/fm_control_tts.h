// ----------------------------------------------------------------------------
// fm_config_tts.h
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

#ifndef __fm_control_tts__
#define __fm_control_tts__

#include <string>

extern bool select_file(std::string &filename, std::string &path,
						const char *title);

extern bool open_festival(void);
extern void close_festival(void);
extern bool festival_running(void);
extern void festival_speak(std::string message);
extern void festival_test_config(void);
extern void festival_speak_buffer(std::string message);

#endif /* defined(__fm_control_tts__) */
