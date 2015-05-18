// ----------------------------------------------------------------------------
// tts_translate_table.h
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
#ifndef __write_default_tts_table__
#define __write_default_tts_table__

#include <stdio.h>
#include <cstring>
#include <string>
#include <vector>

#define TRANSLATE_FILE_TAG "HAMSPEAK"
#define TRANSLATE_FILENAME "translate_table.txt"
#define MAX_TABLE_ENTIES   32767

extern void translate_hamspeak(std::string &message);
extern void translate_loader(void);
extern void close_translate_hamspeak(void);


#endif /* defined(__write_default_tts_table__) */
