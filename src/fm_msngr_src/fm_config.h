// ----------------------------------------------------------------------------
// fm_config.h
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

#ifndef __fm_config__
#define __fm_config__

#include <FL/Fl_Widget.H>

extern void cb_open_configure(Fl_Widget *a, void *b);
extern void update_config_tx_font(void);
extern void update_config_rx_font(void);
extern void close_config();

#endif /* defined(__fm_config__) */
