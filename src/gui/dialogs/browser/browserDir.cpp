/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
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

#include "browserDir.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/browser.h"
#include "utils/fs.h"

namespace giada::v
{
gdBrowserDir::gdBrowserDir(const std::string& title, const std::string& path,
    std::function<void(void*)> cb, m::Conf::Data& conf)
: gdBrowserBase(title, path, cb, 0, conf)
{
	where->size(groupTop->w() - updir->w() - 8, 20);

	browser->callback(cb_down, (void*)this);

	ok->label("Select");
	ok->callback(cb_load, (void*)this);
	ok->shortcut(FL_ENTER);

	/* On OS X the 'where' input doesn't get resized properly on startup. Let's 
	force it. */

	where->redraw();
}

/* -------------------------------------------------------------------------- */

void gdBrowserDir::cb_load(Fl_Widget* /*v*/, void* p) { ((gdBrowserDir*)p)->cb_load(); }
void gdBrowserDir::cb_down(Fl_Widget* /*v*/, void* p) { ((gdBrowserDir*)p)->cb_down(); }

/* -------------------------------------------------------------------------- */

void gdBrowserDir::cb_load()
{
	fireCallback();
}

/* -------------------------------------------------------------------------- */

void gdBrowserDir::cb_down()
{
	std::string path = browser->getSelectedItem();

	if (path.empty() || !u::fs::isDir(path)) // when click on an empty area or not a dir
		return;

	browser->loadDir(path);
	where->value(browser->getCurrentDir().c_str());
}
} // namespace giada::v