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

#include "gui/dialogs/browser/browserBase.h"
#include "core/conf.h"
#include "core/const.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/imageButton.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/basics/progress.h"
#include "gui/elems/basics/textButton.h"
#include "gui/elems/fileBrowser.h"
#include "gui/graphics.h"
#include "gui/ui.h"
#include "utils/fs.h"
#include "utils/gui.h"

extern giada::v::Ui g_ui;

namespace giada::v
{
gdBrowserBase::gdBrowserBase(const std::string& title, const std::string& path,
    std::function<void(void*)> callback, ID channelId, m::Conf::Data& c)
: gdWindow(u::gui::getCenterWinBounds(c.browserBounds), title.c_str())
, m_callback(callback)
, m_conf(c)
, m_channelId(channelId)
{
	set_non_modal();

	begin();

	groupTop    = new Fl_Group(8, 8, w() - 16, 48);
	hiddenFiles = new geCheck(groupTop->x(), groupTop->y(), 400, 20, g_ui.langMapper.get(LangMap::BROWSER_SHOWHIDDENFILES));
	where       = new geInput(groupTop->x(), hiddenFiles->y() + hiddenFiles->h() + 8, 20, 20);
	updir       = new geImageButton(groupTop->x() + groupTop->w() - 20, where->y(), 20, 20, graphics::upOff, graphics::upOn);
	groupTop->end();
	groupTop->resizable(where);

	hiddenFiles->callback(cb_toggleHiddenFiles, (void*)this);

	where->setReadonly(true);
	where->setCursorColor(G_COLOR_BLACK);
	where->setValue(path.c_str());

	updir->onClick = [this]() {
		browser->loadDir(u::fs::getUpDir(browser->getCurrentDir()));
		where->setValue(browser->getCurrentDir().c_str());
	};

	browser = new geFileBrowser(8, groupTop->y() + groupTop->h() + 8, w() - 16, h() - 101);
	browser->loadDir(path);
	if (path == m_conf.browserLastPath)
		browser->preselect(m_conf.browserPosition, m_conf.browserLastValue);

	Fl_Group* groupButtons = new Fl_Group(8, browser->y() + browser->h() + 8, w() - 16, 20);
	ok                     = new geTextButton(w() - 88, groupButtons->y(), 80, 20, "");
	cancel                 = new geTextButton(w() - ok->w() - 96, groupButtons->y(), 80, 20, g_ui.langMapper.get(LangMap::COMMON_CANCEL));
	geBox* spacer          = new geBox(8, groupButtons->y(), cancel->x() - 16, 20);
	groupButtons->resizable(spacer);
	groupButtons->end();

	end();

	cancel->onClick = [this]() { do_callback(); };

	resizable(browser);
	size_range(320, 200);

	show();
}

/* -------------------------------------------------------------------------- */

gdBrowserBase::~gdBrowserBase()
{
	m_conf.browserBounds    = getBounds();
	m_conf.browserPosition  = browser->position();
	m_conf.browserLastPath  = browser->getCurrentDir();
	m_conf.browserLastValue = browser->value();
}

/* -------------------------------------------------------------------------- */

void gdBrowserBase::cb_toggleHiddenFiles(Fl_Widget* /*v*/, void* p) { ((gdBrowserBase*)p)->cb_toggleHiddenFiles(); }

/* -------------------------------------------------------------------------- */

void gdBrowserBase::cb_toggleHiddenFiles()
{
	browser->toggleHiddenFiles();
}

/* -------------------------------------------------------------------------- */

std::string gdBrowserBase::getCurrentPath() const
{
	return where->getValue();
}

ID gdBrowserBase::getChannelId() const
{
	return m_channelId;
}

std::string gdBrowserBase::getSelectedItem() const
{
	return browser->getSelectedItem();
}

/* -------------------------------------------------------------------------- */

void gdBrowserBase::fireCallback() const
{
	m_callback((void*)this);
}
} // namespace giada::v