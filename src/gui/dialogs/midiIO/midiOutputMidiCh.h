/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * midiOutputMidiCh
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

#ifndef GD_MIDI_OUTPUT_MIDI_CH_H
#define GD_MIDI_OUTPUT_MIDI_CH_H

#include "midiOutputBase.h"

namespace giada
{
namespace v
{
class geChoice;
class gdMidiOutputMidiCh : public gdMidiOutputBase
{
  public:
	gdMidiOutputMidiCh(ID channelId);

	void rebuild() override;

  private:
	static void cb_enableOut(Fl_Widget* /*w*/, void* p);
	static void cb_setChannel(Fl_Widget* /*w*/, void* p);
	void        cb_enableOut();
	void        cb_setChannel();

	geCheck*  m_enableOut;
	geChoice* m_chanListOut;
};
} // namespace v
} // namespace giada

#endif
