/*****************************************************************

hamcast_group.h (FLAMP)
 
Author(s):

	Robert Stiles, KK5VD, Copyright (C) 2014

 This is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version. This software is distributed in
 the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the GNU General Public License for more details. You
 should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************/

#ifndef __flamp_hamcast_group__
#define __flamp_hamcast_group__

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

class Hamcast_Group : public Fl_Group
{
public:
	Hamcast_Group(int x, int y, int w, int h, const char *label);
private:
    void draw(void);
};

#endif /* defined(__flamp_hamcast_group__) */
