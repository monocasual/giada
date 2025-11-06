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

#include "src/gui/elems/mainWindow/keyboard/sampleChannelButton.h"
#include "src/glue/channel.h"
#include "src/gui/dialogs/mainWindow.h"
#include "src/gui/elems/mainWindow/keyboard/keyboard.h"
#include "src/gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "src/gui/ui.h"
#include "src/utils/fs.h"
#include "src/utils/string.h"
#include <FL/Fl.H>

extern giada::v::Ui* g_ui;

namespace giada::v
{
geSampleChannelButton::geSampleChannelButton(const c::channel::Data& d)
: geChannelButton(d)
{
	switch (m_channel.getPlayStatus())
	{
	case ChannelStatus::MISSING:
	case ChannelStatus::WRONG:
		label(g_ui->getI18Text(LangMap::MAIN_CHANNEL_SAMPLENOTFOUND));
		break;
	default:
		label(m_channel.sample->waveId == 0 ? g_ui->getI18Text(LangMap::MAIN_CHANNEL_NOSAMPLE) : m_channel.name.c_str());
		break;
	}
}

/* -------------------------------------------------------------------------- */

void geSampleChannelButton::refresh()
{
	geChannelButton::refresh();

	if (m_channel.isRecordingInput() && m_channel.isArmed())
		setInputRecordState();
	else if (m_channel.isRecordingActions() && m_channel.sample->waveId != 0 && !m_channel.sample->isLoop)
		setActionRecordState();

	redraw();
}

/* -------------------------------------------------------------------------- */

int geSampleChannelButton::handle(int e)
{
	int ret = geTextButton::handle(e);
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
