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

#include "gui/dialogs/browser/browserBase.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/engine.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/flex.h"
#include "gui/elems/basics/imageButton.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/basics/progress.h"
#include "gui/elems/basics/textButton.h"
#include "gui/elems/fileBrowser.h"
#include "gui/graphics.h"
#include "gui/ui.h"
#include "utils/fs.h"
#include "utils/gui.h"

extern giada::v::Ui     g_ui;
extern giada::m::Engine g_engine;

namespace giada::v
{
gdBrowserBase::gdBrowserBase(const std::string& title, const std::string& path,
    std::function<void(void*)> callback, ID channelId, const m::Conf& conf)
: gdWindow(u::gui::getCenterWinBounds(conf.browserBounds), title.c_str())
, m_callback(callback)
, m_channelId(channelId)
{
	geFlex* container = new geFlex(getContentBounds().reduced({G_GUI_OUTER_MARGIN}), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		geFlex* header = new geFlex(Direction::HORIZONTAL);
		{
			hiddenFiles = new geCheck(0, 0, 0, 0, g_ui.getI18Text(LangMap::BROWSER_SHOWHIDDENFILES));
			header->add(hiddenFiles, 180);
			header->add(new geBox());
			header->end();
		}

		pathArea = new geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN);
		{
			where = new geInput();
			name  = new geInput();
			updir = new geImageButton(graphics::upOff, graphics::upOn);
			pathArea->add(where);
			pathArea->add(name);
			pathArea->add(updir, G_GUI_UNIT);
			pathArea->end();
		}

		browser = new geFileBrowser();

		geFlex* footer = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			ok     = new geTextButton("");
			cancel = new geTextButton(g_ui.getI18Text(LangMap::COMMON_CANCEL));
			footer->add(new geBox());
			footer->add(cancel, 80);
			footer->add(ok, 80);
			footer->end();
		}

		container->add(header, G_GUI_UNIT);
		container->add(pathArea, G_GUI_UNIT);
		container->add(browser);
		container->add(footer, G_GUI_UNIT);
		container->end();
	}

	add(container);
	resizable(container);

	hiddenFiles->callback(cb_toggleHiddenFiles, (void*)this);

	where->setReadonly(true);
	where->setCursorColor(G_COLOR_BLACK);
	where->setValue(path.c_str());

	updir->onClick = [this]() {
		browser->loadDir(u::fs::getUpDir(browser->getCurrentDir()));
		where->setValue(browser->getCurrentDir().c_str());
	};

	browser->onSelectedElement = [this]() { fireCallback(); };
	browser->loadDir(path);
	if (path == conf.browserLastPath)
		browser->preselect(conf.browserPosition, conf.browserLastValue);

	cancel->onClick = [this]() { do_callback(); };

	set_non_modal();
	size_range(320, 200);
	show();
}

/* -------------------------------------------------------------------------- */

gdBrowserBase::~gdBrowserBase()
{
	m::Conf conf          = g_engine.getConf();
	conf.browserBounds    = getBounds();
	conf.browserPosition  = browser->position();
	conf.browserLastPath  = browser->getCurrentDir();
	conf.browserLastValue = browser->value();
	g_engine.setConf(conf);
}

/* -------------------------------------------------------------------------- */

void gdBrowserBase::cb_toggleHiddenFiles(Fl_Widget* /*v*/, void* p) { ((gdBrowserBase*)p)->cb_toggleHiddenFiles(); }

/* -------------------------------------------------------------------------- */

void gdBrowserBase::cb_toggleHiddenFiles()
{
	browser->toggleHiddenFiles();
}

/* -------------------------------------------------------------------------- */

void gdBrowserBase::hidePathName()
{
	name->hide();
	pathArea->end();
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