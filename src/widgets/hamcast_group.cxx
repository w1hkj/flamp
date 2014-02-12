/** **************************************************************
 \page hamcast_group Hamcast_Group Subclass of Fl_Group

 \par hamcast_group.cxx (FLAMP)

 \par Author(s):
 Robert Stiles, KK5VD, Copyright &copy; 2014
 <br>
 <br>
 This is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version. This software is distributed in
 the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the GNU General Public License for more details. You
 should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 <br>
 <br>
 \par PURPOSE:
 Subclassed to allow the content be updated only when HAMCAST tab/panel
 has focus.
 *******************************************************************/

#include "hamcast_group.h"

extern void estimate_bc(void);

Hamcast_Group::Hamcast_Group(int x, int y, int w, int h, const char *label = 0)
: Fl_Group(x, y, w, h, label) {

}

/** **************************************************************
 * \brief Display the HAMACAST panel content. Update the time
 * measurements when the panel has focus.
 * \return void
 *****************************************************************/
void Hamcast_Group::draw()
{
	estimate_bc();
	Fl_Widget * const *widgets = array();
	int i = 0;
	if(damage() == FL_DAMAGE_CHILD) {
		for(i = children(); i--; widgets++)
			update_child(**widgets);
	} else {
		for(i = children(); i--; widgets++) {
			draw_child(**widgets);
			draw_outside_label(**widgets);
		}
	}
}