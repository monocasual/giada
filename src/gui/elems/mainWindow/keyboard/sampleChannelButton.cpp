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


#include <FL/Fl.H>
#include "core/channels/sampleChannel.h"
#include "core/const.h"
#include "core/model/model.h"
#include "core/wave.h"
#include "core/mixer.h"
#include "core/recorder.h"
#include "core/recManager.h"
#include "utils/string.h"
#include "utils/fs.h"
#include "glue/channel.h"
#include "gui/dialogs/mainWindow.h"
#include "sampleChannel.h"
#include "keyboard.h"
#include "sampleChannelButton.h"


extern giada::v::gdMainWindow* G_MainWin;


namespace giada {
namespace v
{
geSampleChannelButton::geSampleChannelButton(int x, int y, int w, int h, ID channelId)
: geChannelButton(x, y, w, h, channelId)
{
	m::model::onGet(m::model::channels, m_channelId, [&](m::Channel& c)
	{
		const m::SampleChannel& sc = static_cast<m::SampleChannel&>(c);
		
		switch (sc.playStatus) {
			case ChannelStatus::EMPTY:
				label("-- no sample --");
				break;
			case ChannelStatus::MISSING:
			case ChannelStatus::WRONG:
				label("* file not found! *");
				break;
			default:
				if (sc.name.empty()) {
					m::model::onGet(m::model::waves, sc.waveId, [&](m::Wave& w)
					{
						label(w.getBasename(false).c_str());
					});
				}
				else
					label(sc.name.c_str());
				break;
		}
	});
}


/* -------------------------------------------------------------------------- */


void geSampleChannelButton::refresh()
{
	geChannelButton::refresh();
	
	m::model::onGet(m::model::channels, m_channelId, [&](m::Channel& c)
	{
		if (m::recManager::isRecordingInput() && c.armed)
			setInputRecordMode();
		else
		if (m::recManager::isRecordingAction() && c.hasData())
			setActionRecordMode();
	});

	redraw();
}


/* -------------------------------------------------------------------------- */


int geSampleChannelButton::handle(int e)
{
	int ret = geButton::handle(e);
	switch (e) {
		case FL_DND_ENTER:
		case FL_DND_DRAG:
		case FL_DND_RELEASE: {
			ret = 1;
			break;
		}
		case FL_PASTE: {
			c::channel::loadChannel(m_channelId, u::string::trim(u::fs::stripFileUrl(Fl::event_text())));
			ret = 1;
			break;
		}
	}
	return ret;
}

}} // giada::v::
