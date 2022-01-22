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

#include "sampleChannelButton.h"
#include "glue/channel.h"
#include "gui/dialogs/mainWindow.h"
#include "keyboard.h"
#include "sampleChannel.h"
#include "utils/fs.h"
#include "utils/string.h"
#include <FL/Fl.H>

namespace giada::v
{
geSampleChannelButton::geSampleChannelButton(int x, int y, int w, int h, const c::channel::Data& d)
: geChannelButton(x, y, w, h, d)
{
	switch (m_channel.getPlayStatus())
	{
	case ChannelStatus::MISSING:
	case ChannelStatus::WRONG:
		label("* file not found! *");
		break;
	default:
		label(m_channel.sample->waveId == 0 ? "-- no sample --" : m_channel.name.c_str());
		break;
	}
}

/* -------------------------------------------------------------------------- */

void geSampleChannelButton::refresh()
{
	geChannelButton::refresh();

	if (m_channel.isRecordingInput() && m_channel.isArmed())
		setInputRecordMode();
	else if (m_channel.isRecordingAction() && m_channel.sample->waveId != 0 && !m_channel.sample->isLoop)
		setActionRecordMode();

	redraw();
}

/* -------------------------------------------------------------------------- */

int geSampleChannelButton::handle(int e)
{
	int ret = geButton::handle(e);
	switch (e)
	{
	case FL_DND_ENTER:
	case FL_DND_DRAG:
	case FL_DND_RELEASE:
	{
		ret = 1;
		break;
	}
	case FL_PASTE:
	{
		c::channel::loadChannel(m_channel.id, u::string::trim(u::fs::stripFileUrl(Fl::event_text())));
		ret = 1;
		break;
	}
	}
	return ret;
}
} // namespace giada::v
