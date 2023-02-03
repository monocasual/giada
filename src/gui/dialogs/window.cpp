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

#include "gui/dialogs/window.h"
#include "gui/graphics.h"
#include "utils/gui.h"
#include "utils/log.h"
#include <FL/Fl.H>

namespace giada::v
{
gdWindow::gdWindow(int x, int y, int w, int h, const char* title, int id)
: Fl_Double_Window(x, y, w, h, title)
, id(id)
, parent(nullptr)
, m_icon(nullptr, graphics::giadaIcon)
{
	end();
	icon(&m_icon);
}

/* -------------------------------------------------------------------------- */

gdWindow::gdWindow(geompp::Rect<int> r, const char* title, int id)
: gdWindow(r.x, r.y, r.w, r.h, title, id)
{
}

/* -------------------------------------------------------------------------- */

gdWindow::~gdWindow()
{
	/* delete all subwindows in order to empty the stack */

	for (unsigned j = 0; j < subWindows.size(); j++)
		delete subWindows.at(j);
	subWindows.clear();
}

/* -------------------------------------------------------------------------- */

/* this is the default callback of each window, fired when the user closes
 * the window with the 'x'. Watch out: is the parent that calls delSubWIndow */

void gdWindow::cb_closeChild(Fl_Widget* w, void* /*p*/)
{
	/* Disable default FLTK behavior where 'escape' closes the window. */
	if (Fl::event() == FL_SHORTCUT && Fl::event_key() == FL_Escape)
		return;

	gdWindow* child = (gdWindow*)w;
	if (child->getParent() != nullptr)
		(child->getParent())->delSubWindow(child);
}

/* -------------------------------------------------------------------------- */

void gdWindow::addSubWindow(gdWindow* w)
{
	w->setParent(this);
	w->callback(cb_closeChild); // you can pass params: w->callback(cb_closeChild, (void*)params)
	subWindows.push_back(w);
	//debug();
}

/* -------------------------------------------------------------------------- */

void gdWindow::delSubWindow(gdWindow* w)
{
	for (unsigned j = 0; j < subWindows.size(); j++)
		if (w->getId() == subWindows.at(j)->getId())
		{
			delete subWindows.at(j);
			subWindows.erase(subWindows.begin() + j);
			return;
		}
}

/* -------------------------------------------------------------------------- */

void gdWindow::delSubWindow(int wid)
{
	for (unsigned j = 0; j < subWindows.size(); j++)
		if (subWindows.at(j)->getId() == wid)
		{
			delete subWindows.at(j);
			subWindows.erase(subWindows.begin() + j);
			return;
		}
}

/* -------------------------------------------------------------------------- */

int gdWindow::getId() const
{
	return id;
}

void gdWindow::setId(int wid)
{
	id = wid;
}

/* -------------------------------------------------------------------------- */

void gdWindow::debug() const
{
	/* TODO - use G_DEBUG
	u::log::print("---- window stack (id=%d): ----\n", getId());
	for (unsigned i=0; i<subWindows.size(); i++)
		u::log::print("[gdWindow] %p (id=%d)\n", (void*)subWindows.at(i), subWindows.at(i)->getId());
	u::log::print("----\n");
	*/
}

/* -------------------------------------------------------------------------- */

geompp::Rect<int> gdWindow::getContentBounds() const
{
	return {0, 0, w(), h()};
}

geompp::Rect<int> gdWindow::getBounds() const
{
	return {x(), y(), w(), h()};
}

/* -------------------------------------------------------------------------- */

gdWindow* gdWindow::getParent()
{
	return parent;
}

void gdWindow::setParent(gdWindow* w)
{
	parent = w;
}

/* -------------------------------------------------------------------------- */

bool gdWindow::hasWindow(int wid) const
{
	for (unsigned j = 0; j < subWindows.size(); j++)
		if (wid == subWindows.at(j)->getId())
			return true;
	return false;
}

/* -------------------------------------------------------------------------- */

gdWindow* gdWindow::getChild(int wid)
{
	for (unsigned j = 0; j < subWindows.size(); j++)
		if (wid == subWindows.at(j)->getId())
			return subWindows.at(j);
	return nullptr;
}

/* -------------------------------------------------------------------------- */

void gdWindow::setBounds(geompp::Rect<int> r)
{
	resize(r.x, r.y, r.w, r.h);
}
} // namespace giada::v
