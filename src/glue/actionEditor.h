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


#ifndef G_GLUE_ACTION_EDITOR_H
#define G_GLUE_ACTION_EDITOR_H


#include <vector>
#include "../core/types.h"


namespace giada {
namespace m
{
struct Action;
class SampleChannel;
class MidiChannel;
}
namespace c {
namespace actionEditor 
{
std::vector<const m::Action*> getActions(const m::Channel* ch);

/* MIDI actions.  */

void recordMidiAction(m::MidiChannel* ch, int note, int velocity, Frame f1, Frame f2=0);
void deleteMidiAction(m::MidiChannel* ch, const m::Action* a);
void updateMidiAction(m::MidiChannel* ch, const m::Action* a, int note, int velocity, 
    Frame f1, Frame f2);
void updateVelocity(const m::MidiChannel* ch, const m::Action* a, int value);

/* Sample Actions. */

void recordSampleAction(const m::SampleChannel* ch, int type, Frame f1, Frame f2=0);
void deleteSampleAction(m::SampleChannel* ch, const m::Action* a);
void updateSampleAction(m::SampleChannel* ch, const m::Action* a, int type, Frame f1, Frame f2=0);

/* Envelope actions (only volume for now). */

void recordEnvelopeAction(m::Channel* ch, int frame, int value);
void deleteEnvelopeAction(m::Channel* ch, const m::Action* a);
void updateEnvelopeAction(m::Channel* ch, const m::Action* a, int frame, int value);
}}}; // giada::c::actionEditor::

#endif
