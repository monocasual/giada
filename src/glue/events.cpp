/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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
#include "core/conf.h"
#include "core/const.h"
#include "core/engine.h"
#include "core/eventDispatcher.h"
#include "core/kernelAudio.h"
#include "core/midiEvent.h"
#include "core/mixer.h"
#include "core/model/model.h"
#include "core/plugins/pluginHost.h"
#include "core/recorder.h"
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
#include "gui/elems/sampleEditor/pitchTool.h"
#include "gui/ui.h"
#include "src/gui/elems/panTool.h"
#include "src/gui/elems/volumeTool.h"
#include "utils/gui.h"
#include "utils/log.h"
#include <FL/Fl.H>
#include <cassert>

extern giada::v::Ui     g_ui;
extern giada::m::Engine g_engine;

namespace giada::c::events
{
namespace
{
void pushEvent_(m::EventDispatcher::Event e, Thread t)
{
	bool res = true;
	if (t == Thread::MAIN)
	{
		res = g_engine.eventDispatcher.UIevents.push(e);
	}
	else if (t == Thread::MIDI)
	{
		res = g_engine.eventDispatcher.MidiEvents.push(e);
		u::gui::ScopedLock lock;
		g_ui.mainWindow->keyboard->notifyMidiIn(e.channelId);
	}
	else
	{
		assert(false);
	}

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
	pushEvent_({m::EventDispatcher::EventType::KEY_PRESS, 0, channelId, velocity}, t);
}

void releaseChannel(ID channelId, Thread t)
{
	pushEvent_({m::EventDispatcher::EventType::KEY_RELEASE, 0, channelId, {}}, t);
}

void killChannel(ID channelId, Thread t)
{
	pushEvent_({m::EventDispatcher::EventType::KEY_KILL, 0, channelId, {}}, t);
}

/* -------------------------------------------------------------------------- */

void setChannelVolume(ID channelId, float v, Thread t, bool repaintMainUi)
{
	v = std::clamp(v, 0.0f, G_MAX_VOLUME);

	pushEvent_({m::EventDispatcher::EventType::CHANNEL_VOLUME, 0, channelId, v}, t);

	if (t != Thread::MAIN || repaintMainUi)
	{
		u::gui::ScopedLock lock;
		g_ui.mainWindow->keyboard->setChannelVolume(channelId, v);
	}
}

/* -------------------------------------------------------------------------- */

void setChannelPitch(ID channelId, float v, Thread t)
{
	v = std::clamp(v, G_MIN_PITCH, G_MAX_PITCH);

	pushEvent_({m::EventDispatcher::EventType::CHANNEL_PITCH, 0, channelId, v}, t);

	sampleEditor::onRefresh(t, [v](v::gdSampleEditor& e) { e.pitchTool->update(v); });
}

/* -------------------------------------------------------------------------- */

void sendChannelPan(ID channelId, float v)
{
	v = std::clamp(v, 0.0f, G_MAX_PAN);

	/* Pan event is currently triggered only by the main thread. */
	pushEvent_({m::EventDispatcher::EventType::CHANNEL_PAN, 0, channelId, v}, Thread::MAIN);
}

/* -------------------------------------------------------------------------- */

void toggleMuteChannel(ID channelId, Thread t)
{
	pushEvent_({m::EventDispatcher::EventType::CHANNEL_MUTE, 0, channelId, {}}, t);
}

void toggleSoloChannel(ID channelId, Thread t)
{
	pushEvent_({m::EventDispatcher::EventType::CHANNEL_SOLO, 0, channelId, {}}, t);
}

/* -------------------------------------------------------------------------- */

void toggleArmChannel(ID channelId, Thread t)
{
	pushEvent_({m::EventDispatcher::EventType::CHANNEL_TOGGLE_ARM, 0, channelId, {}}, t);
}

void toggleReadActionsChannel(ID channelId, Thread t)
{
	pushEvent_({m::EventDispatcher::EventType::CHANNEL_TOGGLE_READ_ACTIONS, 0, channelId, {}}, t);
}

void killReadActionsChannel(ID channelId, Thread t)
{
	pushEvent_({m::EventDispatcher::EventType::CHANNEL_KILL_READ_ACTIONS, 0, channelId, {}}, t);
}

/* -------------------------------------------------------------------------- */

void sendMidiToChannel(ID channelId, m::MidiEvent e, Thread t)
{
	pushEvent_({m::EventDispatcher::EventType::MIDI, 0, channelId, m::Action{0, channelId, 0, e}}, t);
}

/* -------------------------------------------------------------------------- */

void toggleMetronome()
{
	g_engine.sequencer.toggleMetronome();
}

/* -------------------------------------------------------------------------- */

void setMasterInVolume(float v, Thread t)
{
	pushEvent_({m::EventDispatcher::EventType::CHANNEL_VOLUME, 0, m::Mixer::MASTER_IN_CHANNEL_ID, v}, t);

	if (t != Thread::MAIN)
	{
		u::gui::ScopedLock lock;
		g_ui.mainWindow->mainIO->setInVol(v);
	}
}

void setMasterOutVolume(float v, Thread t)
{
	pushEvent_({m::EventDispatcher::EventType::CHANNEL_VOLUME, 0, m::Mixer::MASTER_OUT_CHANNEL_ID, v}, t);

	if (t != Thread::MAIN)
	{
		u::gui::ScopedLock lock;
		g_ui.mainWindow->mainIO->setOutVol(v);
	}
}

/* -------------------------------------------------------------------------- */

void multiplyBeats()
{
	main::setBeats(g_engine.sequencer.getBeats() * 2, g_engine.sequencer.getBars());
}

void divideBeats()
{
	main::setBeats(g_engine.sequencer.getBeats() / 2, g_engine.sequencer.getBars());
}

/* -------------------------------------------------------------------------- */

void startSequencer(Thread t)
{
	pushEvent_({m::EventDispatcher::EventType::SEQUENCER_START, 0, 0, {}}, t);
}

void stopSequencer(Thread t)
{
	pushEvent_({m::EventDispatcher::EventType::SEQUENCER_STOP, 0, 0, {}}, t);
}

void toggleSequencer(Thread t)
{
	g_engine.sequencer.isRunning() ? stopSequencer(t) : startSequencer(t);
}

void rewindSequencer(Thread t)
{
	pushEvent_({m::EventDispatcher::EventType::SEQUENCER_REWIND, 0, 0, {}}, t);
}

/* -------------------------------------------------------------------------- */

void stopActionRecording()
{
	if (g_engine.kernelAudio.isReady() && g_engine.recorder.isRecordingAction())
		g_engine.recorder.stopActionRec(g_engine.actionRecorder);
}

/* -------------------------------------------------------------------------- */

void toggleActionRecording()
{
	if (!g_engine.kernelAudio.isReady())
		return;
	if (g_engine.recorder.isRecordingAction())
		g_engine.recorder.stopActionRec(g_engine.actionRecorder);
	else
		g_engine.recorder.prepareActionRec(g_engine.conf.data.recTriggerMode);
}

/* -------------------------------------------------------------------------- */

void stopInputRecording()
{
	if (g_engine.kernelAudio.isReady() && g_engine.recorder.isRecordingInput())
		g_engine.recorder.stopInputRec(g_engine.conf.data.inputRecMode, g_engine.kernelAudio.getSampleRate());
}

/* -------------------------------------------------------------------------- */

void toggleInputRecording()
{
	if (!g_engine.kernelAudio.isReady() || !g_engine.kernelAudio.isInputEnabled() || !g_engine.channelManager.hasInputRecordableChannels())
		return;
	if (g_engine.recorder.isRecordingInput())
		g_engine.recorder.stopInputRec(g_engine.conf.data.inputRecMode, g_engine.kernelAudio.getSampleRate());
	else
		g_engine.recorder.prepareInputRec(g_engine.conf.data.recTriggerMode, g_engine.conf.data.inputRecMode);
}

/* -------------------------------------------------------------------------- */

void setPluginParameter(ID channelId, ID pluginId, int paramIndex, float value, Thread t)
{
	if (t == Thread::MIDI)
	{
		u::gui::ScopedLock lock;
		g_ui.mainWindow->keyboard->notifyMidiIn(channelId);
	}
	g_engine.pluginHost.setPluginParameter(pluginId, paramIndex, value);
	c::plugin::updateWindow(pluginId, t);
}
} // namespace giada::c::events
