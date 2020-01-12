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


#include "utils/log.h"
#include "window.h"


namespace giada {
namespace v 
{
void cb_window_closer(Fl_Widget* v, void* p)
{
  delete (Fl_Window*) p;
}


/* -------------------------------------------------------------------------- */


gdWindow::gdWindow(int x, int y, int w, int h, const char* title, int id)
	: Fl_Double_Window(x, y, w, h, title), id(id), parent(nullptr)
{
}


/* -------------------------------------------------------------------------- */


gdWindow::gdWindow(int w, int h, const char* title, int id)
	: Fl_Double_Window(w, h, title), id(id), parent(nullptr)
{
}


/* -------------------------------------------------------------------------- */


gdWindow::~gdWindow()
{
	/* delete all subwindows in order to empty the stack */

	for (unsigned i=0; i<subWindows.size(); i++)
		delete subWindows.at(i);
	subWindows.clear();
}


/* -------------------------------------------------------------------------- */

/* this is the default callback of each window, fired when the user closes
 * the window with the 'x'. Watch out: is the parent that calls delSubWIndow */

void gdWindow::cb_closeChild(Fl_Widget* v, void* p)
{
	gdWindow* child = (gdWindow*) v;
	if (child->getParent() != nullptr)
		(child->getParent())->delSubWindow(child);
}


/* -------------------------------------------------------------------------- */


void gdWindow::addSubWindow(gdWindow* w)
{
	/** TODO - useless: delete ---------------------------------------- */
	for (unsigned i=0; i<subWindows.size(); i++)
		if (w->getId() == subWindows.at(i)->getId()) {
			//u::log::print("[gdWindow] window %p (id=%d) exists, not added (and deleted)\n", (void*)w, w->getId());
			delete w;
			return;
		}
	/** --------------------------------------------------------------- */

	w->setParent(this);
	w->callback(cb_closeChild); // you can pass params: w->callback(cb_closeChild, (void*)params)
	subWindows.push_back(w);
	//debug();
}


/* -------------------------------------------------------------------------- */


void gdWindow::delSubWindow(gdWindow* w)
{
	for (unsigned i=0; i<subWindows.size(); i++)
		if (w->getId() == subWindows.at(i)->getId()) {
			delete subWindows.at(i);
			subWindows.erase(subWindows.begin() + i);
			//debug();
			return;
		}
	//debug();
}


/* -------------------------------------------------------------------------- */


void gdWindow::delSubWindow(int id)
{
	for (unsigned i=0; i<subWindows.size(); i++)
		if (subWindows.at(i)->getId() == id) {
			delete subWindows.at(i);
			subWindows.erase(subWindows.begin() + i);
			//debug();
			return;
		}
	//debug();
}


/* -------------------------------------------------------------------------- */


int gdWindow::getId() const
{
	return id;
}


void gdWindow::setId(int id)
{
	this->id = id;
}


/* -------------------------------------------------------------------------- */


void gdWindow::debug() const
{
	u::log::print("---- window stack (id=%d): ----\n", getId());
	for (unsigned i=0; i<subWindows.size(); i++)
		u::log::print("[gdWindow] %p (id=%d)\n", (void*)subWindows.at(i), subWindows.at(i)->getId());
	u::log::print("----\n");
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


bool gdWindow::hasWindow(int id) const
{
	for (unsigned i=0; i<subWindows.size(); i++)
		if (id == subWindows.at(i)->getId())
			return true;
	return false;
}


/* -------------------------------------------------------------------------- */


gdWindow* gdWindow::getChild(int id)
{
	for (unsigned i=0; i<subWindows.size(); i++)
		if (id == subWindows.at(i)->getId())
			return subWindows.at(i);
	return nullptr;
}
}} // giada::v::