/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef GE_RESIZER_BAR_H
#define GE_RESIZER_BAR_H

#include <FL/Fl_Box.H>
#include <functional>

/* geResizerBar
A 'resizer bar' between widgets inside a Fl_Scroll. Thanks to Greg Ercolano from
FLTK dev team (http://seriss.com/people/erco/fltk/). It also shows a resize 
cursor when hovered over.

Assumes:
	- Parent is an Fl_Group;
	- The widget before us has an edge touching our edge;
	  ie. w->y() + w->h() == this->y() if Direction::VERTICAL.

When this widget is dragged:
	- The widget before us (with a common edge) will be resized;
	- if Mode == MOVE 
		All children after us will be moved.
	- else if Mode == RESIZE
		The child after us is resized. */

namespace giada::v
{
class geResizerBar : public Fl_Box
{
public:
	enum class Direction
	{
		HORIZONTAL,
		VERTICAL
	};

	enum class Mode
	{
		MOVE,
		RESIZE
	};

	geResizerBar(int x, int y, int w, int h, int minSize, Direction dir, Mode m = Mode::MOVE);

	int  handle(int e) override;
	void draw() override;
	void resize(int x, int y, int w, int h) override;

	void moveTo(int p);

	std::function<void(const Fl_Widget&)> onDrag    = nullptr;
	std::function<void(const Fl_Widget&)> onRelease = nullptr;

private:
	/* isBefore
	True if widget 'w' is before the drag bar. */

	bool isBefore(const Fl_Widget& w) const;

	/* isBefore
	True if widget 'w' is after the drag bar. */
	bool isAfter(const Fl_Widget& w) const;

	/* findWidgets
	Returns a vector of widgets according to a certain logic specified in the
	lambda function. Limits the output to 'howmany' widgets if 'howmany' != -1. */

	std::vector<Fl_Widget*> findWidgets(std::function<bool(const Fl_Widget&)> f, int howmany = -1) const;

	/* handleDrag
	Main entrypoint for the dragging operation. */

	void handleDrag(int diff);

	/* move
	Resize the first widget and shift all others. */

	void move(int diff);

	/* resize 
	Resize the first and the second widget, leaving all others untouched. */

	void resize(int diff);

	/* getFirstWidget
	Returns a ref to the first widget before the drag bar. */

	Fl_Widget& getFirstWidget();

	Direction m_direction;
	Mode      m_mode;
	int       m_minSize;
	int       m_lastPos;
	bool      m_hover;
};
} // namespace giada::v

#endif
