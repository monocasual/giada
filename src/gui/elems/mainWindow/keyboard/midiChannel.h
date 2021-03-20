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

#ifndef GE_MIDI_CHANNEL_H
#define GE_MIDI_CHANNEL_H

#include "channel.h"
#include "channelButton.h"

namespace giada
{
namespace v
{
class geMidiChannel : public geChannel
{
public:
	geMidiChannel(int x, int y, int w, int h, c::channel::Data d);

	void resize(int x, int y, int w, int h) override;

  private:
	static void cb_playButton(Fl_Widget* /*w*/, void* p);
	static void cb_openMenu(Fl_Widget* /*w*/, void* p);
	void        cb_playButton();
	void        cb_openMenu();

	c::channel::Data m_data;
};
} // namespace v
} // namespace giada

#endif
