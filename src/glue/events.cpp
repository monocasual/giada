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
#include "core/channels/channelManager.h"
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
void notifyChannelForMidiIn_(Thread t, ID channelId)
{
	if (t == Thread::MIDI)
		g_ui.pumpEvent([channelId]() { g_ui.mainWindow->keyboard->notifyMidiIn(channelId); });
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void pressChannel(ID channelId, int velocity, Thread t)
{
	const bool  canRecordActions = g_engine.recorder.canRecordActions();
	const bool  canQuantize      = g_engine.sequencer.canQuantize();
	const Frame currentFrameQ    = g_engine.sequencer.getCurrentFrameQuantized();
	g_engine.channelManager.keyPress(channelId, velocity, canRecordActions, canQuantize, currentFrameQ);
	notifyChannelForMidiIn_(t, channelId);
}

void releaseChannel(ID channelId, Thread t)
{
	const bool  canRecordActions = g_engine.recorder.canRecordActions();
	const Frame currentFrameQ    = g_engine.sequencer.getCurrentFrameQuantized();
	g_engine.channelManager.keyRelease(channelId, canRecordActions, currentFrameQ);
	notifyChannelForMidiIn_(t, channelId);
}

void killChannel(ID channelId, Thread t)
{
	const bool  canRecordActions = g_engine.recorder.canRecordActions();
	const Frame currentFrameQ    = g_engine.sequencer.getCurrentFrameQuantized();
	g_engine.channelManager.keyKill(channelId, canRecordActions, currentFrameQ);
	notifyChannelForMidiIn_(t, channelId);
}

/* -------------------------------------------------------------------------- */

float setChannelVolume(ID channelId, float v, Thread t, bool repaintMainUi)
{
	g_engine.channelManager.setVolume(channelId, v);
	notifyChannelForMidiIn_(t, channelId);

	if (t != Thread::MAIN || repaintMainUi)
		g_ui.pumpEvent([channelId, v]() { g_ui.mainWindow->keyboard->setChannelVolume(channelId, v); });

	return v;
}

/* -------------------------------------------------------------------------- */

float setChannelPitch(ID channelId, float v, Thread t)
{
	g_engine.channelManager.setPitch(channelId, v);
	g_ui.pumpEvent([v]() {
		if (auto* w = sampleEditor::getWindow(); w != nullptr)
			w->pitchTool->update(v);
	});
	notifyChannelForMidiIn_(t, channelId);
	return v;
}

/* -------------------------------------------------------------------------- */

float sendChannelPan(ID channelId, float v)
{
	g_engine.channelManager.setPan(channelId, v);
	notifyChannelForMidiIn_(Thread::MAIN, channelId); // Currently triggered only by the main thread
	return v;
}

/* -------------------------------------------------------------------------- */

void toggleMuteChannel(ID channelId, Thread t)
{
	g_engine.channelManager.toggleMute(channelId);
	notifyChannelForMidiIn_(t, channelId);
}

void toggleSoloChannel(ID channelId, Thread t)
{
	g_engine.channelManager.toggleSolo(channelId);
	g_engine.mixer.updateSoloCount(g_engine.channelManager.hasSolos()); // TOD - move to channelManager
	notifyChannelForMidiIn_(t, channelId);
}

/* -------------------------------------------------------------------------- */

void toggleArmChannel(ID channelId, Thread t)
{
	g_engine.channelManager.toggleArm(channelId);
	notifyChannelForMidiIn_(t, channelId);
}

void toggleReadActionsChannel(ID channelId, Thread t)
{
	g_engine.channelManager.toggleReadActions(channelId, g_engine.sequencer.isRunning());
	notifyChannelForMidiIn_(t, channelId);
}

void killReadActionsChannel(ID channelId, Thread t)
{

	g_engine.channelManager.killReadActions(channelId);
	notifyChannelForMidiIn_(t, channelId);
}

/* -------------------------------------------------------------------------- */

void sendMidiToChannel(ID channelId, m::MidiEvent e, Thread t)
{
	const bool  canRecordActions = g_engine.recorder.canRecordActions();
	const Frame currentFrameQ    = g_engine.sequencer.getCurrentFrameQuantized();
	g_engine.channelManager.processMidiEvent(channelId, e, canRecordActions, currentFrameQ);
	notifyChannelForMidiIn_(t, channelId);
}

/* -------------------------------------------------------------------------- */

void toggleMetronome()
{
	g_engine.getMainEngine().toggleMetronome();
}

/* -------------------------------------------------------------------------- */

void setMasterInVolume(float v, Thread t)
{
	g_engine.getMainEngine().setMasterInVolume(v);

	if (t != Thread::MAIN)
		g_ui.pumpEvent([v]() { g_ui.mainWindow->mainIO->setInVol(v); });
}

void setMasterOutVolume(float v, Thread t)
{
	g_engine.getMainEngine().setMasterOutVolume(v);

	if (t != Thread::MAIN)
		g_ui.pumpEvent([v]() { g_ui.mainWindow->mainIO->setOutVol(v); });
}

/* -------------------------------------------------------------------------- */

void setBpm(float v) { g_engine.getMainEngine().setBpm(v); }

/* -------------------------------------------------------------------------- */

void multiplyBeats() { g_engine.getMainEngine().multiplyBeats(); }
void divideBeats() { g_engine.getMainEngine().divideBeats(); }

/* -------------------------------------------------------------------------- */

void goToBeat(int beat) { g_engine.getMainEngine().goToBeat(beat); }

/* -------------------------------------------------------------------------- */

void startSequencer() { g_engine.getMainEngine().startSequencer(); }
void stopSequencer() { g_engine.getMainEngine().stopSequencer(); }
void toggleSequencer() { g_engine.getMainEngine().toggleSequencer(); }
void rewindSequencer() { g_engine.getMainEngine().rewindSequencer(); }

/* -------------------------------------------------------------------------- */

void stopActionRecording() { g_engine.getMainEngine().stopActionRecording(); }
void stopInputRecording() { g_engine.getMainEngine().stopInputRecording(); }
void toggleActionRecording() { g_engine.getMainEngine().toggleActionRecording(); }
void toggleInputRecording() { g_engine.getMainEngine().toggleInputRecording(); }

/* -------------------------------------------------------------------------- */

void setPluginParameter(ID channelId, ID pluginId, int paramIndex, float value, Thread t)
{
	g_engine.pluginHost.setPluginParameter(pluginId, paramIndex, value);
	notifyChannelForMidiIn_(t, channelId);

	g_ui.pumpEvent([pluginId, t]() { c::plugin::updateWindow(pluginId, t); });
}
} // namespace giada::c::events
