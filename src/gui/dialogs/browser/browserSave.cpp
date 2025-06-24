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

#include "gui/dialogs/browser/browserSave.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/basics/textButton.h"
#include "gui/elems/fileBrowser.h"
#include "gui/ui.h"
#include "utils/fs.h"

extern giada::v::Ui* g_ui;

namespace giada::v
{
gdBrowserSave::gdBrowserSave(const std::string& title, const std::string& path,
    const std::string& name_, std::function<void(void*)> cb, ID channelId,
    const Model& model)
: gdBrowserBase(title, path, cb, channelId, model)
{
	name->setValue(name_.c_str());

	browser->onSelectItem = [this]
	{
		name->setValue(u::fs::basename(browser->getSelectedItem()));
	};

	ok->label(g_ui->getI18Text(LangMap::COMMON_SAVE));
	ok->onClick = [this]()
	{ fireCallback(); };
}

/* -------------------------------------------------------------------------- */

std::string gdBrowserSave::getName() const
{
	return name->getValue();
}
} // namespace giada::v