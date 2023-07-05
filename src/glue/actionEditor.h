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

#ifndef G_GLUE_ACTION_EDITOR_H
#define G_GLUE_ACTION_EDITOR_H

#include "core/types.h"
#include <optional>
#include <string>
#include <vector>

namespace giada::m
{
struct Action;
class SamplePlayer;
class Channel;
} // namespace giada::m

/* giada::c::actionEditor
Functions to interact with the Action Editor. Only the main thread can use these! */

namespace giada::c::actionEditor
{
struct SampleData
{
	SampleData(const m::SamplePlayer&);

	SamplePlayerMode channelMode;
	bool             isLoopMode;
};

struct Data
{
	Data() = default;
	Data(const m::Channel&);

	Frame getCurrentFrame() const;
	bool  isChannelPlaying() const;

	ID                     channelId;
	std::string            channelName;
	Frame                  framesInSeq;
	Frame                  framesInBeat;
	Frame                  framesInBar;
	Frame                  framesInLoop;
	std::vector<m::Action> actions;

	std::optional<SampleData> sample;
};

Data getData(ID channelId);

const m::Action* findAction(ID);

/* MIDI actions.  */

void recordMidiAction(ID channelId, int note, int velocity, Frame f1,
    Frame f2 = 0);
void deleteMidiAction(ID channelId, const m::Action& a);
void updateMidiAction(ID channelId, const m::Action& a, int note, int velocity,
    Frame f1, Frame f2);
void updateVelocity(const m::Action& a, int value);

/* Sample Actions. */

void recordSampleAction(ID channelId, int type, Frame f1, Frame f2 = 0);
void deleteSampleAction(ID channelId, const m::Action& a);
void updateSampleAction(ID channelId, const m::Action& a, int type,
    Frame f1, Frame f2 = 0);

/* Envelope actions (only volume for now). */

void recordEnvelopeAction(ID channelId, Frame f, int value);
void deleteEnvelopeAction(ID channelId, const m::Action& a);
void updateEnvelopeAction(ID channelId, const m::Action& a, Frame f, int value);
} // namespace giada::c::actionEditor

#endif
