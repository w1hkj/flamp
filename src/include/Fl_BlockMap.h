// =====================================================================
// Mapped values to x coordinate viewer
//
// Copyright 2012 - Dave Freese, <w1hkj@w1hkj.com>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Contents:
//
//	 Fl_BlockMap::draw()				- Draw the block mapping widget
//	 Fl_BlockMap::Fl_BlockMap() - Construct a Fl_BlockMap widget
//
//======================================================================

#ifndef _Fl_BlockMap_H_
#	define _Fl_BlockMap_H_

#include <string>

#include <FL/Fl_Widget.H>

//
// BlockMap class...
//
/**
	Displays a BlockMap bar for the user.
*/

class Fl_BlockMap : public Fl_Widget {

	int nblocks_;
	std::string blocks;

	protected:

	virtual void draw();

	public:

	Fl_BlockMap(int x, int y, int w, int h, const char *l = 0);

	/** Sets the maximum value in the BlockMap widget.	*/
	void	nblocks(int v) { if (v < 0) v = 0; nblocks_ = v; redraw(); }
	/** Gets the maximum value in the BlockMap widget.	*/
	int		nblocks() const { return (nblocks_); }

	/** Sets the current value in the BlockMap widget.	*/
	void	value(std::string v) { blocks = v; redraw(); }
	/** Gets the current value in the BlockMap widget.	*/
	std::string	value() const { return (blocks); }

	void	set(std::string v, int blks) {
		blocks = v;
		nblocks_ = blks;
		redraw();
	}

	void clear() {
		blocks.clear();
		nblocks_ = 0;
		redraw();
	}
};

#endif // !_Fl_BlockMap_H_
