/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "gui/dialogs/browser/browserLoad.h"
#include "gui/elems/basics/imageButton.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/basics/textButton.h"
#include "gui/elems/fileBrowser.h"
#include "gui/ui.h"
#include "utils/fs.h"

extern giada::v::Ui g_ui;

namespace giada::v
{
gdBrowserLoad::gdBrowserLoad(const std::string& title, const std::string& path,
    std::function<void(void*)> cb, ID channelId, m::Conf::Data& conf)
: gdBrowserBase(title, path, cb, channelId, conf)
{
	where->size(groupTop->w() - updir->w() - 8, 20);

	browser->callback(cb_down, (void*)this);

	ok->label(g_ui.langMapper.get(LangMap::COMMON_LOAD));
	ok->shortcut(FL_ENTER);
	ok->onClick = [this]() { fireCallback(); };

	/* On OS X the 'where' input doesn't get resized properly on startup. Let's 
	force it. */

	where->redraw();
}

/* -------------------------------------------------------------------------- */

void gdBrowserLoad::cb_down(Fl_Widget* /*v*/, void* p) { ((gdBrowserLoad*)p)->cb_down(); }

/* -------------------------------------------------------------------------- */

void gdBrowserLoad::cb_down()
{
	std::string path = browser->getSelectedItem();

	if (path.empty() || !u::fs::isDir(path)) // when click on an empty area or not a dir
		return;

	browser->loadDir(path);
	where->setValue(browser->getCurrentDir().c_str());
}
} // namespace giada::v