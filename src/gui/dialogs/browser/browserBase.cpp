/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/gui/dialogs/browser/browserBase.h"
#include "src/gui/const.h"
#include "src/gui/elems/basics/box.h"
#include "src/gui/elems/basics/check.h"
#include "src/gui/elems/basics/flex.h"
#include "src/gui/elems/basics/imageButton.h"
#include "src/gui/elems/basics/input.h"
#include "src/gui/elems/basics/progress.h"
#include "src/gui/elems/basics/textButton.h"
#include "src/gui/elems/fileBrowser.h"
#include "src/gui/graphics.h"
#include "src/gui/ui.h"
#include "src/utils/fs.h"
#include "src/utils/gui.h"

extern giada::v::Ui* g_ui;

namespace giada::v
{
gdBrowserBase::gdBrowserBase(const std::string& title, const std::string& path,
    std::function<void(void*)> callback, ID channelId, const Model& model)
: gdWindow(u::gui::getCenterWinBounds(model.browserBounds), title.c_str(), WID_FILE_BROWSER)
, m_callback(callback)
, m_channelId(channelId)
{
	geFlex* container = new geFlex(getContentBounds().reduced({G_GUI_OUTER_MARGIN}), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		geFlex* header = new geFlex(Direction::HORIZONTAL);
		{
			hiddenFiles = new geCheck(0, 0, 0, 0, g_ui->getI18Text(LangMap::BROWSER_SHOWHIDDENFILES));
			header->addWidget(hiddenFiles, 180);
			header->addWidget(new geBox());
			header->end();
		}

		pathArea = new geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN);
		{
			where = new geInput();
			name  = new geInput();
			updir = new geImageButton(graphics::upOff, graphics::upOn);
			pathArea->addWidget(where);
			pathArea->addWidget(name);
			pathArea->addWidget(updir, G_GUI_UNIT);
			pathArea->end();
		}

		browser = new geFileBrowser();

		geFlex* footer = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			ok     = new geTextButton("");
			cancel = new geTextButton(g_ui->getI18Text(LangMap::COMMON_CANCEL));
			footer->addWidget(new geBox());
			footer->addWidget(cancel, 80);
			footer->addWidget(ok, 80);
			footer->end();
		}

		container->addWidget(header, G_GUI_UNIT);
		container->addWidget(pathArea, G_GUI_UNIT);
		container->addWidget(browser);
		container->addWidget(footer, G_GUI_UNIT);
		container->end();
	}

	add(container);
	resizable(container);

	hiddenFiles->callback(cb_toggleHiddenFiles, (void*)this);

	where->setReadonly(true);
	where->setCursorColor(G_COLOR_BLACK);
	where->setValue(path.c_str());

	updir->onClick = [this]()
	{
		browser->loadDir(u::fs::getUpDir(browser->getCurrentDir()));
	};

	browser->onChangeDir = [this]
	{ where->setValue(browser->getCurrentDir()); };
	browser->onChooseItem = [this]()
	{ fireCallback(); };
	browser->loadDir(path);
	if (path == model.browserLastPath)
		browser->preselect(model.browserPosition, model.browserLastValue);

	cancel->onClick = [this]()
	{ do_callback(); };

	set_non_modal();
	size_range(320, 200);
	show();
}

/* -------------------------------------------------------------------------- */

gdBrowserBase::~gdBrowserBase()
{
	g_ui->model.browserBounds    = getBounds();
	g_ui->model.browserPosition  = browser->vposition();
	g_ui->model.browserLastValue = browser->value();
	g_ui->model.browserLastPath  = browser->getCurrentDir();
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