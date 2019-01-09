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


#ifndef GE_MIDI_CHANNEL_H
#define GE_MIDI_CHANNEL_H


#include "../../../../core/midiChannel.h"
#include "channel.h"
#include "channelButton.h"


class geMidiChannel : public geChannel
{
private:

	static void cb_button(Fl_Widget* v, void* p);
	static void cb_openMenu(Fl_Widget* v, void* p);
	void cb_button();
	void cb_openMenu();

public:

	geMidiChannel(int x, int y, int w, int h, giada::m::MidiChannel* ch);

	void resize(int x, int y, int w, int h) override;

	void reset() override;
	void update() override;
	void refresh() override;

	int keyPress(int event);  // TODO - move to base class
};


#endif
