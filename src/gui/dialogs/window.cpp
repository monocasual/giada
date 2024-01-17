/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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
gdWindow::gdWindow(int x, int y, int w, int h, const char* title, ID id)
: Fl_Double_Window(x, y, w, h, title)
, m_id(id)
, m_parent(nullptr)
, m_icon(nullptr, graphics::giadaIcon)
{
	end();
	icon(&m_icon);
}

/* -------------------------------------------------------------------------- */

gdWindow::gdWindow(geompp::Rect<int> r, const char* title, ID id)
: gdWindow(r.x, r.y, r.w, r.h, title, id)
{
}

/* -------------------------------------------------------------------------- */

gdWindow::~gdWindow()
{
	deleteChildren();
}

/* -------------------------------------------------------------------------- */

/* this is the default callback of each window, fired when the user closes
 * the window with the 'x'. Watch out: is the parent that calls delSubWIndow */

void gdWindow::cb_closeChild(Fl_Widget* w, void* /*p*/)
{
	/* Disable default FLTK behavior where 'escape' closes the window. */
	if (Fl::event() == FL_SHORTCUT && Fl::event_key() == FL_Escape)
		return;

	gdWindow* child = static_cast<gdWindow*>(w);
	if (child->getParent() != nullptr)
		(child->getParent())->deleteChild(child);
}

/* -------------------------------------------------------------------------- */

void gdWindow::addChild(gdWindow* w)
{
	if (hasChild(w->getId()))
	{
		G_DEBUG("Parent has subwindow with id={}, deleting", w->getId());
		deleteChild(w->getId());
	}
	w->setParent(this);
	w->callback(cb_closeChild); // you can pass params: w->callback(cb_closeChild, (void*)params)
	m_children.push_back(w);
}

/* -------------------------------------------------------------------------- */

void gdWindow::deleteChild(gdWindow* w)
{
	deleteChild(w->getId());
}

/* -------------------------------------------------------------------------- */

void gdWindow::deleteChild(ID wid)
{
	for (unsigned j = 0; j < m_children.size(); j++)
	{
		if (m_children.at(j)->getId() != wid)
			continue;
		delete m_children.at(j);
		m_children.erase(m_children.begin() + j);
		return;
	}
}

/* -------------------------------------------------------------------------- */

void gdWindow::deleteChildren()
{
	for (auto* child : m_children)
		delete child;
	m_children.clear();
}

/* -------------------------------------------------------------------------- */

ID gdWindow::getId() const
{
	return m_id;
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
	return m_parent;
}

void gdWindow::setParent(gdWindow* w)
{
	m_parent = w;
}

/* -------------------------------------------------------------------------- */

bool gdWindow::hasChild(ID wid) const
{
	for (unsigned j = 0; j < m_children.size(); j++)
		if (wid == m_children.at(j)->getId())
			return true;
	return false;
}

/* -------------------------------------------------------------------------- */

gdWindow* gdWindow::getChild(ID wid)
{
	for (unsigned j = 0; j < m_children.size(); j++)
		if (wid == m_children.at(j)->getId())
			return m_children.at(j);
	return nullptr;
}

/* -------------------------------------------------------------------------- */

void gdWindow::setBounds(geompp::Rect<int> r)
{
	resize(r.x, r.y, r.w, r.h);
}
} // namespace giada::v
