/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * midiInputChannel
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


#ifndef GD_MIDI_INPUT_CHANNEL_H
#define GD_MIDI_INPUT_CHANNEL_H


#include "midiInputBase.h"


class geScroll;
class geCheck;
class geChoice;


namespace giada {
namespace v 
{
class geMidiLearner;
class gdMidiInputChannel : public gdMidiInputBase
{
public:

	gdMidiInputChannel(ID channelId);

private:

	static void cb_enable(Fl_Widget* w, void* p);
	static void cb_setChannel(Fl_Widget* w, void* p);
	static void cb_veloAsVol(Fl_Widget* w, void* p);
	void cb_enable();
	void cb_setChannel();
	void cb_veloAsVol();

	void addChannelLearners();

#ifdef WITH_VST

	void addPluginLearners();

#endif

	ID m_channelId;

	geScroll* m_container;
	geCheck*  m_veloAsVol;
};
}} // giada::v::


#endif
