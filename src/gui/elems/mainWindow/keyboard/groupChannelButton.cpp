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

#include "src/gui/elems/mainWindow/keyboard/groupChannelButton.h"
#include "src/glue/channel.h"
#include "src/gui/ui.h"
#include <string>

extern giada::v::Ui* g_ui;

namespace giada::v
{
geGroupChannelButton::geGroupChannelButton(const c::channel::Data& d)
: geChannelButton(d)
{
}

/* -------------------------------------------------------------------------- */

void geGroupChannelButton::refresh()
{
	const std::string l = m_channel.name.empty() ? g_ui->getI18Text(LangMap::MAIN_CHANNEL_DEFAULTGROUPNAME) : m_channel.name;
	copy_label(l.c_str());
	redraw();
}
} // namespace giada::v
