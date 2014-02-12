// =====================================================================
// Mapped values to x coordinate viewer
//
// Copyright 2012 - Dave Freese, <w1hkj@w1hkj.com>
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
// Contents:
//
//	 Fl_BlockMap::draw()				- Draw the block mapping widget
//	 Fl_BlockMap::Fl_BlockMap() - Construct a Fl_BlockMap widget
//
//======================================================================

#include <stdlib.h>
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include "Fl_BlockMap.h"

//
// 'Fl_BlockMap::draw()
//

void Fl_BlockMap::draw()
{
	int tx = x();
	int tw = w();
	int ty = y();
	int th = h();

	// draw an empty widget
	fl_push_clip(x(), y(), w(), h());
	draw_box(box(), tx, ty, tw, th, color());

	if (blocks.empty() || nblocks_ == 0) {
		fl_pop_clip();
		return;
	}

	std::string working = blocks;
	int blknbr = 0;
	int x1 = 0;
	int x2 = 0;
	float delta = 1.0 * tw / nblocks_;
	size_t p = std::string::npos;

	while(!working.empty()) {
		blknbr = atoi(working.c_str());
		p = working.find(" ");
		if (p != std::string::npos) working.erase(0,p+1);
		else working.clear();
		x1 = (int) ((blknbr - 1) * delta + 0.5);
		x2 = (int) (blknbr * delta + 0.5);
		draw_box(box(), tx + x1, ty, x2-x1, th, selection_color());
	}

	fl_pop_clip();

}


/**
 The constructor creates the progress bar using the position, size, and label.

 You can set the background color with color() and the
 mapping color with selection_color(), or you can set both colors
 together with color(unsigned bg, unsigned sel).

 The default colors are FL_LIGHT3 and FL_DARK_BLUE, resp.
 */
Fl_BlockMap::Fl_BlockMap(int X, int Y, int W, int H, const char* L)
: Fl_Widget(X, Y, W, H, L) {
	align(FL_ALIGN_LEFT);
	box(FL_DOWN_BOX);
	color(FL_LIGHT2, FL_DARK_BLUE);
	nblocks_ = 0;
	blocks.clear();
}


//
// End of "$Id: Fl_BlockMap.cxx 7903 2010-11-28 21:06:39Z matt $".
//
