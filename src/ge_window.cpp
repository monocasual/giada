/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_window
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2012 Giovanni A. Zuliani | Monocasual
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
 * ------------------------------------------------------------------ */


#include "ge_window.h"


gWindow::gWindow(int x, int y, int w, int h, const char *title, int id)
	: Fl_Double_Window(x, y, w, h, title), id(id), parent(NULL) { }


/* ------------------------------------------------------------------ */


gWindow::gWindow(int w, int h, const char *title, int id)
	: Fl_Double_Window(w, h, title), id(id), parent(NULL) { }


/* ------------------------------------------------------------------ */


gWindow::~gWindow() {

	/* delete all subwindows in order to empty the stack */

	for (unsigned i=0; i<subWindows.size; i++)
		delete subWindows.at(i);
	subWindows.clear();
}


/* ------------------------------------------------------------------ */

/* this is the default callback of each window, fired when the user closes
 * the window with the 'x'. Watch out: is the parent that calls delSubWIndow */

void gWindow::cb_closeChild(Fl_Widget *v, void *p) {
	gWindow *child = (gWindow*) v;
	if (child->getParent() != NULL)
		(child->getParent())->delSubWindow(child);
}


/* ------------------------------------------------------------------ */


void gWindow::addSubWindow(gWindow *w) {

	/** useless: delete ----------------------------------------------- */
	for (unsigned i=0; i<subWindows.size; i++)
		if (w->getId() == subWindows.at(i)->getId()) {
			//printf("[gWindow] window %p (id=%d) exists, not added (and deleted)\n", (void*)w, w->getId());
			delete w;
			return;
		}
	/** --------------------------------------------------------------- */

	w->setParent(this);
	w->callback(cb_closeChild); // you can pass params: w->callback(cb_closeChild, (void*)params)
	subWindows.add(w);
	//debug();
}


/* ------------------------------------------------------------------ */


void gWindow::delSubWindow(gWindow *w) {
	for (unsigned i=0; i<subWindows.size; i++)
		if (w->getId() == subWindows.at(i)->getId()) {
			delete subWindows.at(i);
			subWindows.del(i);
			//debug();
			return;
		}
	//debug();
}


/* ------------------------------------------------------------------ */


void gWindow::delSubWindow(int id) {
	for (unsigned i=0; i<subWindows.size; i++)
		if (subWindows.at(i)->getId() == id) {
			delete subWindows.at(i);
			subWindows.del(i);
			//debug();
			return;
		}
	//debug();
}


/* ------------------------------------------------------------------ */


int gWindow::getId() {
	return id;
}


/* ------------------------------------------------------------------ */


void gWindow::setId(int id) {
	this->id = id;
}


/* ------------------------------------------------------------------ */


void gWindow::debug() {
	printf("---- window stack (id=%d): ----\n", getId());
	for (unsigned i=0; i<subWindows.size; i++)
		printf("[gWindow] %p (id=%d)\n", (void*)subWindows.at(i), subWindows.at(i)->getId());
	puts("----");
}


/* ------------------------------------------------------------------ */


gWindow *gWindow::getParent() {
	return parent;
}


/* ------------------------------------------------------------------ */


void gWindow::setParent(gWindow *w) {
	parent = w;
}


/* ------------------------------------------------------------------ */


bool gWindow::hasWindow(int id) {
	for (unsigned i=0; i<subWindows.size; i++)
		if (id == subWindows.at(i)->getId())
			return true;
	return false;
}


/* ------------------------------------------------------------------ */


gWindow *gWindow::getChild(int id) {
	for (unsigned i=0; i<subWindows.size; i++)
		if (id == subWindows.at(i)->getId())
			return subWindows.at(i);
	return NULL;
}
