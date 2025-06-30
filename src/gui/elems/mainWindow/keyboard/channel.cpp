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

#include "glue/channel.h"
#include "glue/layout.h"
#include "glue/main.h"
#include "gui/const.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/basics/imageButton.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "gui/elems/mainWindow/keyboard/channelButton.h"
#include "gui/elems/mainWindow/keyboard/channelProgress.h"
#include "gui/elems/mainWindow/keyboard/track.h"
#include "gui/elems/midiActivity.h"
#include "gui/ui.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>

extern giada::v::Ui* g_ui;

namespace giada::v
{
geChannel::geChannel(int x, int y, int w, int h, c::channel::Data d)
: geFlex(x, y, w, h, Direction::HORIZONTAL, G_GUI_INNER_MARGIN)
, m_channel(d)
{
}

/* -------------------------------------------------------------------------- */

void geChannel::draw()
{
	const int ny = y() + (h() / 2) - (G_GUI_UNIT / 2);

	playButton->resize(playButton->x(), ny, G_GUI_UNIT, G_GUI_UNIT);
	arm->resize(arm->x(), ny, G_GUI_UNIT, G_GUI_UNIT);
	mute->resize(mute->x(), ny, G_GUI_UNIT, G_GUI_UNIT);
	solo->resize(solo->x(), ny, G_GUI_UNIT, G_GUI_UNIT);
	vol->resize(vol->x(), ny, G_GUI_UNIT, G_GUI_UNIT);
	fx->resize(fx->x(), ny, G_GUI_UNIT, G_GUI_UNIT);

	fl_rectf(x(), y(), w(), h(), G_COLOR_GREY_1_5);

	geFlex::draw();
}

/* -------------------------------------------------------------------------- */

void geChannel::cb_changeVol(Fl_Widget* /*w*/, void* p) { ((geChannel*)p)->cb_changeVol(); }

/* -------------------------------------------------------------------------- */

void geChannel::refresh()
{
	ChannelStatus playStatus = m_channel.getPlayStatus();
	ChannelStatus recStatus  = m_channel.getRecStatus();

	if (mainButton->visible())
		mainButton->refresh();

	if (recStatus == ChannelStatus::WAIT || playStatus == ChannelStatus::WAIT)
		blink();

	playButton->setValue(playStatus == ChannelStatus::PLAY || playStatus == ChannelStatus::ENDING);
	midiActivity->redraw();
	arm->setValue(m_channel.isArmed());
	mute->setValue(m_channel.isMuted());
	solo->setValue(m_channel.isSoloed());
}

/* -------------------------------------------------------------------------- */

void geChannel::cb_changeVol()
{
	c::channel::setChannelVolume(m_channel.id, vol->value(), Thread::MAIN);
}

/* -------------------------------------------------------------------------- */

std::size_t geChannel::getTrackIndex() const
{
	return static_cast<geTrack*>(parent())->index;
}

/* -------------------------------------------------------------------------- */

bool geChannel::isGroup() const
{
	return m_channel.type == ChannelType::GROUP;
}

/* -------------------------------------------------------------------------- */

void geChannel::blink()
{
	if (g_ui->shouldBlink())
		mainButton->setPlayMode();
	else
		mainButton->setDefaultMode();
}

/* -------------------------------------------------------------------------- */

bool geChannel::handleKey(int e)
{
	if (Fl::event_key() != m_channel.key)
		return false;

	if (e == FL_KEYDOWN)
	{
		playButton->take_focus(); // Move focus to this button
		return true;
	}

	if (e == FL_KEYUP)
		return true;

	return false;
}

/* -------------------------------------------------------------------------- */

const c::channel::Data& geChannel::getData() const
{
	return m_channel;
}
} // namespace giada::v
