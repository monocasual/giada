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

#ifndef G_RENDERING_SAMPLE_ADVANCE_H
#define G_RENDERING_SAMPLE_ADVANCE_H

#include "core/sequencer.h"
#include "core/types.h"

namespace giada::m
{
struct Action;
struct ChannelShared;
class Channel;
} // namespace giada::m

namespace giada::m::rendering
{
void advanceSampleChannel(const Channel&, const Sequencer::Event&);
void onSampleEnd(const Channel&, bool seqIsRunning, bool natural);
} // namespace giada::m::rendering

#endif
