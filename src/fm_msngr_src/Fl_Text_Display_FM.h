// ----------------------------------------------------------------------------
// Fl_Text_Display_FM.h
//
// Copyright (C) 2015  Robert Stiles, KK5VD
//
// This file is part of fldigi.
//
// Fldigi is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Fldigi is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fldigi.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------

#ifndef __Fl_Text_Display_FM__
#define __Fl_Text_Display_FM__

#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/fl_utf8.h>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/Fl_Scroll.H>
#include <string>

class FL_EXPORT Fl_Text_Display_FM : public Fl_Text_Display
{
public:
	Fl_Text_Display_FM(int x, int y, int w, int h, char *l = (char *)0):Fl_Text_Display(x, y, w, h, l){};
	int handle(int event);
	void V_Scroll_Value(double value);
	double V_Scroll_Value(void);
	double V_Scroll_Max(void);
	double V_Slider_Height(void);
	void copy_string(std::string &str);
};

#endif /* defined(__Fl_Text_Diplay_FM__) */
