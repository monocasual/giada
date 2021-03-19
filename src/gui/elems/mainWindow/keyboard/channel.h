/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#ifndef GE_CHANNEL_H
#define GE_CHANNEL_H

#include "core/types.h"
#include "glue/channel.h"
#include <FL/Fl_Group.H>

class geChannelStatus;
class geButton;
class geDial;
class geStatusButton;

namespace giada::v
{
class geChannelButton;
class geChannel : public Fl_Group
{
  public:
	geChannel(int x, int y, int w, int h, c::channel::Data d);

	void draw() override;

	/* refresh
	Updates graphics. */

	virtual void refresh();

	/* getColumnId
	Returns the ID of the column this channel resides in. */

	ID getColumnId();

	/* handleKey
	Performs some UI-related operations when the bound key is pressed. Returns
	whether the bound key has been pressed or not. */

	bool handleKey(int e);

	/* getData
	Returns a reference to the internal data. Read-only. */

	const c::channel::Data& getData() const;

	geStatusButton*  playButton;
	geButton*        arm;
	geChannelStatus* status;
	geChannelButton* mainButton;
	geStatusButton*  mute;
	geStatusButton*  solo;
	geDial*          vol;
#ifdef WITH_VST
	geStatusButton* fx;
#endif

  protected:
	/* Define some breakpoints for dynamic resize. BREAK_DELTA: base amount of
	pixels to shrink sampleButton. */

#ifdef WITH_VST
	static const int BREAK_READ_ACTIONS = 240;
	static const int BREAK_MODE_BOX     = 216;
	static const int BREAK_FX           = 192;
	static const int BREAK_ARM          = 168;
#else
	static const int BREAK_READ_ACTIONS = 216;
	static const int BREAK_MODE_BOX     = 192;
	static const int BREAK_ARM          = 168;
#endif

	static const int MIN_ELEM_W = 20;

	static void cb_arm(Fl_Widget* /*w*/, void* p);
	static void cb_mute(Fl_Widget* /*w*/, void* p);
	static void cb_solo(Fl_Widget* /*w*/, void* p);
	static void cb_changeVol(Fl_Widget* /*w*/, void* p);
#ifdef WITH_VST
	static void cb_openFxWindow(Fl_Widget* /*w*/, void* p);
#endif
	void cb_mute();
	void cb_arm();
	void cb_solo();
	void cb_changeVol();
#ifdef WITH_VST
	void cb_openFxWindow();
#endif

	/* blink
	Blinks button when channel is in wait/ending status. */

	void blink();

	/* packWidgets
	Spread widgets across available space. */

	void packWidgets();

	/* m_channel
	Channel's data. */

	c::channel::Data m_channel;
};
} // namespace giada::v

#endif
