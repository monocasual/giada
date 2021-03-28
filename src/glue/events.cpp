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

#include "events.h"
#include "core/clock.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/eventDispatcher.h"
#include "core/midiEvent.h"
#include "core/mixer.h"
#include "core/mixerHandler.h"
#include "core/model/model.h"
#include "core/plugins/pluginHost.h"
#include "core/recManager.h"
#include "core/sequencer.h"
#include "core/types.h"
#include "glue/main.h"
#include "glue/plugin.h"
#include "glue/sampleEditor.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/dialogs/warnings.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/elems/mainWindow/mainIO.h"
#include "gui/elems/mainWindow/mainTimer.h"
#include "gui/elems/sampleEditor/panTool.h"
#include "gui/elems/sampleEditor/pitchTool.h"
#include "gui/elems/sampleEditor/volumeTool.h"
#include "utils/log.h"
#include <FL/Fl.H>
#include <cassert>

extern giada::v::gdMainWindow* G_MainWin;

namespace giada::c::events
{
namespace
{
void pushEvent_(m::eventDispatcher::Event e, Thread t)
{
	bool res = true;
	if (t == Thread::MAIN)
		res = m::eventDispatcher::UIevents.push(e);
	else if (t == Thread::MIDI)
		res = m::eventDispatcher::MidiEvents.push(e);
	else
		assert(false);

	if (!res)
		G_DEBUG("[events] Queue full!\n");
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void pressChannel(ID channelId, int velocity, Thread t)
{
	m::MidiEvent e;
	e.setVelocity(velocity);
	pushEvent_({m::eventDispatcher::EventType::KEY_PRESS, 0, channelId, velocity}, t);
}

void releaseChannel(ID channelId, Thread t)
{
	pushEvent_({m::eventDispatcher::EventType::KEY_RELEASE, 0, channelId, {}}, t);
}

void killChannel(ID channelId, Thread t)
{
	pushEvent_({m::eventDispatcher::EventType::KEY_KILL, 0, channelId, {}}, t);
}

/* -------------------------------------------------------------------------- */

void setChannelVolume(ID channelId, float v, Thread t)
{
	v = std::clamp(v, 0.0f, G_MAX_VOLUME);

	pushEvent_({m::eventDispatcher::EventType::CHANNEL_VOLUME, 0, channelId, v}, t);

	sampleEditor::onRefresh(t == Thread::MAIN, [v](v::gdSampleEditor& e) { e.volumeTool->update(v); });

	if (t != Thread::MAIN)
	{
		Fl::lock();
		G_MainWin->keyboard->getChannel(channelId)->vol->value(v);
		Fl::unlock();
	}
}

/* -------------------------------------------------------------------------- */

void setChannelPitch(ID channelId, float v, Thread t)
{
	v = std::clamp(v, G_MIN_PITCH, G_MAX_PITCH);

	pushEvent_({m::eventDispatcher::EventType::CHANNEL_PITCH, 0, channelId, v}, t);

	sampleEditor::onRefresh(t == Thread::MAIN, [v](v::gdSampleEditor& e) { e.pitchTool->update(v); });
}

/* -------------------------------------------------------------------------- */

void sendChannelPan(ID channelId, float v)
{
	v = std::clamp(v, 0.0f, G_MAX_PAN);

	/* Pan event is currently triggered only by the main thread. */
	pushEvent_({m::eventDispatcher::EventType::CHANNEL_PAN, 0, channelId, v}, Thread::MAIN);

	sampleEditor::onRefresh(/*gui=*/true, [v](v::gdSampleEditor& e) { e.panTool->update(v); });
}

/* -------------------------------------------------------------------------- */

void toggleMuteChannel(ID channelId, Thread t)
{
	pushEvent_({m::eventDispatcher::EventType::CHANNEL_MUTE, 0, channelId, {}}, t);
}

void toggleSoloChannel(ID channelId, Thread t)
{
	pushEvent_({m::eventDispatcher::EventType::CHANNEL_SOLO, 0, channelId, {}}, t);
}

/* -------------------------------------------------------------------------- */

void toggleArmChannel(ID channelId, Thread t)
{
	pushEvent_({m::eventDispatcher::EventType::CHANNEL_TOGGLE_ARM, 0, channelId, {}}, t);
}

void toggleReadActionsChannel(ID channelId, Thread t)
{
	pushEvent_({m::eventDispatcher::EventType::CHANNEL_TOGGLE_READ_ACTIONS, 0, channelId, {}}, t);
}

void killReadActionsChannel(ID channelId, Thread t)
{
	pushEvent_({m::eventDispatcher::EventType::CHANNEL_KILL_READ_ACTIONS, 0, channelId, {}}, t);
}

/* -------------------------------------------------------------------------- */

void sendMidiToChannel(ID channelId, m::MidiEvent e, Thread t)
{
	pushEvent_({m::eventDispatcher::EventType::MIDI, 0, channelId, m::Action{0, channelId, 0, e}}, t);
}

/* -------------------------------------------------------------------------- */

void toggleMetronome()
{
	m::sequencer::toggleMetronome();
}

/* -------------------------------------------------------------------------- */

void setMasterInVolume(float v, Thread t)
{
	pushEvent_({m::eventDispatcher::EventType::CHANNEL_VOLUME, 0, m::mixer::MASTER_IN_CHANNEL_ID, v}, t);

	if (t != Thread::MAIN)
	{
		Fl::lock();
		G_MainWin->mainIO->setInVol(v);
		Fl::unlock();
	}
}

void setMasterOutVolume(float v, Thread t)
{
	pushEvent_({m::eventDispatcher::EventType::CHANNEL_VOLUME, 0, m::mixer::MASTER_OUT_CHANNEL_ID, v}, t);

	if (t != Thread::MAIN)
	{
		Fl::lock();
		G_MainWin->mainIO->setOutVol(v);
		Fl::unlock();
	}
}

/* -------------------------------------------------------------------------- */

void multiplyBeats()
{
	main::setBeats(m::clock::getBeats() * 2, m::clock::getBars());
}

void divideBeats()
{
	main::setBeats(m::clock::getBeats() / 2, m::clock::getBars());
}

/* -------------------------------------------------------------------------- */

void startSequencer(Thread t)
{
	pushEvent_({m::eventDispatcher::EventType::SEQUENCER_START, 0, 0, {}}, t);
	m::conf::conf.recTriggerMode = RecTriggerMode::NORMAL;
}

void stopSequencer(Thread t)
{
	pushEvent_({m::eventDispatcher::EventType::SEQUENCER_STOP, 0, 0, {}}, t);
}

void toggleSequencer(Thread t)
{
	m::clock::isRunning() ? stopSequencer(t) : startSequencer(t);
}

void rewindSequencer(Thread t)
{
	pushEvent_({m::eventDispatcher::EventType::SEQUENCER_REWIND, 0, 0, {}}, t);
}

/* -------------------------------------------------------------------------- */

void toggleActionRecording()
{
	m::recManager::toggleActionRec(m::conf::conf.recTriggerMode);
}

void toggleInputRecording()
{
	m::recManager::toggleInputRec(m::conf::conf.recTriggerMode, m::conf::conf.inputRecMode);
}

/* -------------------------------------------------------------------------- */

#ifdef WITH_VST
void setPluginParameter(ID pluginId, int paramIndex, float value, bool gui)
{
	m::pluginHost::setPluginParameter(pluginId, paramIndex, value);
	c::plugin::updateWindow(pluginId, gui);
}
#endif
} // namespace giada::c::events
