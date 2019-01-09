/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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


#include "../../../utils/fs.h"
#include "../../elems/browser.h"
#include "../../elems/basics/button.h"
#include "../../elems/basics/input.h"
#include "browserSave.h"


using std::string;
using namespace giada::m;


gdBrowserSave::gdBrowserSave(int x, int y, int w, int h, const string& title,
		const string& path, const string& _name, void (*cb)(void*), Channel* ch)
	:	gdBrowserBase(x, y, w, h, title, path, cb)
{
	channel = ch;

	where->size(groupTop->w()-236, 20);

	name = new geInput(where->x()+where->w()+8, where->y(), 200, 20);
	name->value(_name.c_str());
	groupTop->add(name);

	browser->callback(cb_down, (void*) this);

	ok->label("Save");
	ok->callback(cb_save, (void*) this);
	ok->shortcut(FL_ENTER);

	/* On OS X the 'where' and 'name' inputs don't get resized properly on startup. 
	Let's force them. */

	where->redraw();
	name->redraw();
}


/* -------------------------------------------------------------------------- */


void gdBrowserSave::cb_save(Fl_Widget* v, void* p) { ((gdBrowserSave*)p)->cb_save(); }
void gdBrowserSave::cb_down(Fl_Widget* v, void* p) { ((gdBrowserSave*)p)->cb_down(); }


/* -------------------------------------------------------------------------- */


void gdBrowserSave::cb_down()
{
	string path = browser->getSelectedItem();

	if (path.empty())  // when click on an empty area
		return;

	/* if the selected item is a directory just load its content. If it's a file
	 * use it as the file name (i.e. fill name->value()). */

	if (gu_isDir(path)) {
		browser->loadDir(path);
		where->value(browser->getCurrentDir().c_str());
	}
	else
		name->value(browser->getSelectedItem(false).c_str());
}


/* -------------------------------------------------------------------------- */


string gdBrowserSave::getName() const
{
  return name->value();
}


/* -------------------------------------------------------------------------- */


void gdBrowserSave::cb_save()
{
	callback((void*) this);
}
