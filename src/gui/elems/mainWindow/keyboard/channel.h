/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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
#include "gui/elems/basics/flex.h"

namespace giada::v
{
class geDial;
class geImageButton;
class geChannelStatus;
class geChannelButton;
class geMidiActivity;
class geChannel : public geFlex
{
public:
	geChannel(int x, int y, int w, int h, c::channel::Data d);

	void draw() override;

	/* refresh
	Updates graphics. */

	virtual void refresh();

	/* getColumnId
	Returns the ID of the column this channel resides in. */

	ID getColumnId() const;

	/* handleKey
	Performs some UI-related operations when the bound key is pressed. Returns
	whether the bound key has been pressed or not. */

	bool handleKey(int e);

	/* getData
	Returns a reference to the internal data. Read-only. */

	const c::channel::Data& getData() const;

	geImageButton*   playButton;
	geImageButton*   arm;
	geChannelStatus* status;
	geChannelButton* mainButton;
	geMidiActivity*  midiActivity;
	geImageButton*   mute;
	geImageButton*   solo;
	geDial*          vol;
	geImageButton*   fx;

protected:
	/* Define some breakpoints for dynamic resize. BREAK_DELTA: base amount of
	pixels to shrink sampleButton. */

	static const int BREAK_READ_ACTIONS = 240;
	static const int BREAK_MODE_BOX     = 216;
	static const int BREAK_FX           = 192;
	static const int BREAK_ARM          = 168;

	static void cb_changeVol(Fl_Widget* /*w*/, void* p);
	void        cb_changeVol();

	/* blink
	Blinks button when channel is in wait/ending status. */

	void blink();

	/* m_channel
	Channel's data. */

	c::channel::Data m_channel;
};
} // namespace giada::v

#endif
