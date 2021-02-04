/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * geResizerBar
 * 'resizer bar' between widgets Fl_Scroll. Thanks to Greg Ercolano from
 * FLTK dev team. http://seriss.com/people/erco/fltk/
 *
 * Shows a resize cursor when hovered over.
 * Assumes:
 *     - Parent is an Fl_Scroll
 *     - All children of Fl_Scroll are vertically arranged
 *     - The widget above us has a bottom edge touching our top edge
 *       ie. (w->y()+w->h() == this->y())
 *
 * When this widget is dragged:
 *     - The widget above us (with a common edge) will be /resized/
 *       vertically
 *     - All children below us will be /moved/ vertically
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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


#include <functional>
#include <FL/Fl_Box.H>


class geResizerBar : public Fl_Box
{
public:

	static const int HORIZONTAL = 0;
	static const int VERTICAL   = 1;

	geResizerBar(int x, int y, int w, int h, int minSize, bool type, Fl_Widget* target=nullptr);

	int handle(int e) override;
	void draw() override;
	void resize(int x, int y, int w, int h) override;

	int getMinSize() const;

	std::function<void(const Fl_Widget*)> onDrag = nullptr;
	std::function<void(const Fl_Widget*)> onRelease = nullptr;

private:

	void handleDrag(int diff);

	bool m_type;
	int  m_origSize;
	int  m_minSize;
	int  m_lastPos;
	int  m_initialPos;
	bool m_hover;

	Fl_Widget* m_target;
};


#endif
