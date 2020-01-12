/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#include <FL/fl_draw.H>
#include "core/channels/sampleChannel.h"
#include "core/model/model.h"
#include "core/mixer.h"
#include "core/clock.h"
#include "core/recorder.h"
#include "core/recManager.h"
#include "core/const.h"
#include "channelStatus.h"


namespace giada {
namespace v
{
geChannelStatus::geChannelStatus(int x, int y, int w, int h, ID channelId)
: Fl_Box(x, y, w, h), channelId(channelId)
{
}


/* -------------------------------------------------------------------------- */


void geChannelStatus::draw()
{
	fl_rect(x(), y(), w(), h(), G_COLOR_GREY_4);              // reset border
	fl_rectf(x()+1, y()+1, w()-2, h()-2, G_COLOR_GREY_2);     // reset background

	m::model::ChannelsLock l(m::model::channels);
	const m::SampleChannel& ch = static_cast<m::SampleChannel&>(m::model::get(m::model::channels, channelId));
	
	if (ch.playStatus == ChannelStatus::WAIT    || 
	    ch.playStatus == ChannelStatus::ENDING  ||
	    ch.recStatus == ChannelStatus::WAIT || 
	    ch.recStatus == ChannelStatus::ENDING)
	{
		fl_rect(x(), y(), w(), h(), G_COLOR_LIGHT_1);
	}
	else
	if (ch.playStatus == ChannelStatus::PLAY)
		fl_rect(x(), y(), w(), h(), G_COLOR_LIGHT_1);
	else
		fl_rectf(x()+1, y()+1, w()-2, h()-2, G_COLOR_GREY_2);  // status empty


	if (m::recManager::isRecordingInput() && ch.armed)
		fl_rectf(x()+1, y()+1, w()-2, h()-2, G_COLOR_RED);     // take in progress
	else
	if (m::recManager::isRecordingAction())
		fl_rectf(x()+1, y()+1, w()-2, h()-2, G_COLOR_BLUE);    // action recording

	/* Equation for the progress bar: 
	((chanTracker - chanStart) * w()) / (chanEnd - chanStart). */

	int pos = ch.getPosition();
	if (pos == -1)
		pos = 0;
	else
		pos = (pos * (w()-1)) / ((ch.getEnd() - ch.getBegin()));
	fl_rectf(x()+1, y()+1, pos, h()-2, G_COLOR_LIGHT_1);
}

}} // giada::v::
