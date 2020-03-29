/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
 *
 * This file is part of Giada - Your Hardcore Loopmachine.
 *
 * Giada - Your Hardcore Loopmachine is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * Giada - Your Hardcore Loopmachine is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------- */


#ifndef GE_BASE_ACTION_H
#define GE_BASE_ACTION_H


#include <FL/Fl_Box.H>
#include "core/recorder.h"
#include "core/types.h"


namespace giada {
namespace m 
{
struct Action;
}
namespace v
{
class geBaseAction : public Fl_Box
{
public:

	static const Pixel MIN_WIDTH    = 12;
	static const Pixel HANDLE_WIDTH = 6;

	geBaseAction(Pixel x, Pixel y, Pixel w, Pixel h, bool resizable, 
		m::Action a1, m::Action a2);

	int handle(int e) override;

	bool isOnEdges() const;

	/* setLeftEdge/setRightEdge
	Set new left/right edges position, relative range. */

	void setLeftEdge(Pixel p);
	void setRightEdge(Pixel p);

	void setPosition(Pixel p);

	bool onRightEdge = false;
	bool onLeftEdge = false;
	bool hovered = false;
	bool altered = false;
	Pixel pick = 0;

	m::Action a1;
	m::Action a2;
	
protected:
	
	bool m_resizable;
};
}} // giada::v::

#endif
