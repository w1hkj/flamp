// ----------------------------------------------------------------------------
// fm_ncs_control.h
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

#ifndef __fm_ncs_control__
#define __fm_ncs_control__

#include <string>
#include "fm_msngr_src/call_table.h"

extern void ncs_load_send_to_msg(bool use_browser);
extern int load_user_presets(std::string &data,	std::string &label);
extern int load_user_presets_from_file(const char *filename, std::string &data, std::string &label);
extern bool convert_label(std::string src_raw, std::string &display, std::string &csv);
extern void	tx_send_to_msg(CALL_TABLE *, int msg_idx);

#endif /* defined(__fm_ncs_control__) */
