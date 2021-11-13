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

#ifndef G_CHANNEL_SAMPLE_ADVANCER_H
#define G_CHANNEL_SAMPLE_ADVANCER_H

#include "core/sequencer.h"

namespace giada::m::channel
{
struct Data;
}

namespace giada::m
{
class SampleAdvancer final
{
public:
	void onLastFrame(const channel::Data& ch, bool seqIsRunning) const;
	void advance(const channel::Data& ch, const Sequencer::Event& e) const;

private:
	void rewind(const channel::Data& ch, Frame localFrame) const;
	void stop(const channel::Data& ch, Frame localFrame) const;
	void play(const channel::Data& ch, Frame localFrame) const;
	void wait(const channel::Data& ch) const;
	void onFirstBeat(const channel::Data& ch, Frame localFrame) const;
	void onBar(const channel::Data& ch, Frame localFrame) const;
	void onNoteOn(const channel::Data& ch, Frame localFrame) const;
	void parseActions(const channel::Data& ch, const std::vector<Action>& as, Frame localFrame) const;
};
} // namespace giada::m

#endif
