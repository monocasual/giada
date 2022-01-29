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

#include "glue/channel.h"
#include "core/graphics.h"
#include "glue/events.h"
#include "glue/layout.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/basics/statusButton.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "gui/elems/mainWindow/keyboard/channelButton.h"
#include "gui/elems/mainWindow/keyboard/channelStatus.h"
#include "gui/elems/mainWindow/keyboard/column.h"
#include "gui/elems/mainWindow/keyboard/midiActivity.h"
#include "gui/ui.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>

extern giada::v::Ui g_ui;

namespace giada::v
{
geChannel::geChannel(int X, int Y, int W, int H, c::channel::Data d)
: Fl_Group(X, Y, W, H)
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
#ifdef WITH_VST
	fx->resize(fx->x(), ny, G_GUI_UNIT, G_GUI_UNIT);
#endif

	fl_rectf(x(), y(), w(), h(), G_COLOR_GREY_1_5);

	Fl_Group::draw();
}

/* -------------------------------------------------------------------------- */

void geChannel::cb_arm(Fl_Widget* /*w*/, void* p) { ((geChannel*)p)->cb_arm(); }
void geChannel::cb_mute(Fl_Widget* /*w*/, void* p) { ((geChannel*)p)->cb_mute(); }
void geChannel::cb_solo(Fl_Widget* /*w*/, void* p) { ((geChannel*)p)->cb_solo(); }
void geChannel::cb_changeVol(Fl_Widget* /*w*/, void* p) { ((geChannel*)p)->cb_changeVol(); }
#ifdef WITH_VST
void geChannel::cb_openFxWindow(Fl_Widget* /*w*/, void* p)
{
	((geChannel*)p)->cb_openFxWindow();
}
#endif

/* -------------------------------------------------------------------------- */

void geChannel::refresh()
{
	ChannelStatus playStatus = m_channel.getPlayStatus();
	ChannelStatus recStatus  = m_channel.getRecStatus();

	if (mainButton->visible())
		mainButton->refresh();

	if (recStatus == ChannelStatus::WAIT || playStatus == ChannelStatus::WAIT)
		blink();

	playButton->setStatus(playStatus == ChannelStatus::PLAY || playStatus == ChannelStatus::ENDING);
	midiActivity->redraw();
	mute->setStatus(m_channel.getMute());
	solo->setStatus(m_channel.getSolo());
}

/* -------------------------------------------------------------------------- */

void geChannel::cb_arm()
{
	c::events::toggleArmChannel(m_channel.id, Thread::MAIN);
}

/* -------------------------------------------------------------------------- */

void geChannel::cb_mute()
{
	c::events::toggleMuteChannel(m_channel.id, Thread::MAIN);
}

/* -------------------------------------------------------------------------- */

void geChannel::cb_solo()
{
	c::events::toggleSoloChannel(m_channel.id, Thread::MAIN);
}

/* -------------------------------------------------------------------------- */

void geChannel::cb_changeVol()
{
	c::events::setChannelVolume(m_channel.id, vol->value(), Thread::MAIN);
}

/* -------------------------------------------------------------------------- */

#ifdef WITH_VST
void geChannel::cb_openFxWindow()
{
	c::layout::openChannelPluginListWindow(m_channel.id);
}
#endif

/* -------------------------------------------------------------------------- */

int geChannel::getColumnId() const
{
	return static_cast<geColumn*>(parent())->id;
}

/* -------------------------------------------------------------------------- */

void geChannel::blink()
{
	if (g_ui.shouldBlink())
		mainButton->setPlayMode();
	else
		mainButton->setDefaultMode();
}

/* -------------------------------------------------------------------------- */

void geChannel::packWidgets()
{
	/* Compute how much space is visible for the main button, then resize it
	according to that amount. */

	int visible = w();
	for (int i = 0; i < children(); i++)
	{
		if (child(i)->visible() && child(i) != mainButton)
			visible -= child(i)->w() + G_GUI_INNER_MARGIN;
	}

	mainButton->size(visible, mainButton->h());

	/* Reposition everything else */

	for (int i = 1, p = 0; i < children(); i++)
	{
		if (!child(i)->visible())
			continue;
		for (int k = i - 1; k >= 0; k--) // Get the first visible item prior to i
			if (child(k)->visible())
			{
				p = k;
				break;
			}
		child(i)->position(child(p)->x() + child(p)->w() + G_GUI_INNER_MARGIN, child(i)->y());
	}

	init_sizes(); // Resets the internal array of widget sizes and positions
}

/* -------------------------------------------------------------------------- */

bool geChannel::handleKey(int e)
{
	if (Fl::event_key() != m_channel.key)
		return false;

	if (e == FL_KEYDOWN && !playButton->value())
	{                             // Key not already pressed
		playButton->take_focus(); // Move focus to this playButton
		playButton->value(1);
		return true;
	}

	if (e == FL_KEYUP)
	{
		playButton->value(0);
		return true;
	}

	return false;
}

/* -------------------------------------------------------------------------- */

const c::channel::Data& geChannel::getData() const
{
	return m_channel;
}
} // namespace giada::v
