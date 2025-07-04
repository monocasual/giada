/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_RENDERING_REACTOR_H
#define G_RENDERING_REACTOR_H

#include "src/core/midiMapper.h"
#include "src/core/types.h"
#include "src/types.h"

namespace giada::m
{
class MidiEvent;
class ActionRecorder;
class KernelMidi;
} // namespace giada::m

namespace giada::m::model
{
class Model;
}

namespace giada::m::rendering
{
class Reactor
{
public:
	Reactor(model::Model&, MidiMapper<KernelMidi>&, ActionRecorder&, KernelMidi&);

	void keyPress(ID channelId, float velocity, bool canRecordActions, bool canQuantize, Frame currentFrameQuantized);
	void keyRelease(ID channelId, bool canRecordActions, Frame currentFrameQuantized);
	void keyKill(ID channelId, bool canRecordActions, Frame currentFrameQuantized);
	void processMidiEvent(ID channelId, const MidiEvent&, bool canRecordActions, Frame currentFrameQuantized);
	void toggleReadActions(ID channelId, bool seqIsRunning);
	void killReadActions(ID channelId);
	void toggleMute(ID channelId);
	void toggleSolo(ID channelId);
	void stopAll();
	void rewindAll();

private:
	model::Model&           m_model;
	KernelMidi&             m_kernelMidi;
	ActionRecorder&         m_actionRecorder;
	MidiMapper<KernelMidi>& m_midiMapper;
};
} // namespace giada::m::rendering

#endif
