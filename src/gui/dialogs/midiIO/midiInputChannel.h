/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * midiInputChannel
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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


class gdMidiInputChannel : public gdMidiInputBase
{
private:

  static const int LEARNER_WIDTH = 284;

	class Channel *ch;

  class geScroll *container;
	class gCheck   *enable;

	//gVector <geMidiLearner *> items; // plugins parameters

	static void cb_enable  (Fl_Widget *w, void *p);
	inline void __cb_enable();

  void addChannelLearners();

#ifdef WITH_VST

  void addPluginLearners();

#endif

public:

	gdMidiInputChannel(class Channel *ch);
  ~gdMidiInputChannel();
};


#endif
