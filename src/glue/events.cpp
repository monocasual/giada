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


#include <cassert>
#include <FL/Fl.H>
#include "core/model/model.h"
#include "core/clock.h"
#include "core/mixer.h"
#include "core/midiEvent.h"
#include "core/pluginHost.h"
#include "core/sequencer.h"
#include "core/mixerHandler.h"
#include "core/conf.h"
#include "core/recManager.h"
#include "utils/log.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/warnings.h"
#include "gui/elems/mainWindow/mainIO.h"
#include "gui/elems/mainWindow/mainTimer.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "gui/elems/sampleEditor/volumeTool.h"
#include "gui/elems/sampleEditor/pitchTool.h"
#include "gui/elems/sampleEditor/panTool.h"
#include "glue/sampleEditor.h"
#include "glue/plugin.h"
#include "glue/main.h"
#include "events.h"


extern giada::v::gdMainWindow* G_MainWin;


namespace giada {
namespace c {
namespace events 
{
namespace
{
void pushEvent_(m::mixer::Event e, Thread t)
{
	bool res = true;
	if (t == Thread::MAIN)
		res = m::mixer::UIevents.push(e);
	else
	if (t == Thread::MIDI)
		res = m::mixer::MidiEvents.push(e);
	else
		assert(false);
	
	if (!res)
		G_DEBUG("[events] Queue full!\n");
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void pressChannel(ID channelId, int velocity, Thread t)
{
	m::MidiEvent e;
	e.setVelocity(velocity);
	pushEvent_({ m::mixer::EventType::KEY_PRESS, 0, {0, channelId, 0, e} }, t);
}


void releaseChannel(ID channelId, Thread t)
{
	pushEvent_({ m::mixer::EventType::KEY_RELEASE, 0, {0, channelId} }, t);
}


void killChannel(ID channelId, Thread t)
{
	pushEvent_({ m::mixer::EventType::KEY_KILL, 0, {0, channelId} }, t);
}


/* -------------------------------------------------------------------------- */


void setChannelVolume(ID channelId, float v, Thread t)
{
	v = std::clamp(v, 0.0f, G_MAX_VOLUME);

	pushEvent_({ m::mixer::EventType::CHANNEL_VOLUME, 0, { 0, channelId, 0, {v} } }, t);

	sampleEditor::onRefresh(t == Thread::MAIN, [v](v::gdSampleEditor& e) { e.volumeTool->update(v); });

	if (t != Thread::MAIN) {
		Fl::lock();
		G_MainWin->keyboard->getChannel(channelId)->vol->value(v);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void setChannelPitch(ID channelId, float v,  Thread t)
{	
	v = std::clamp(v, G_MIN_PITCH, G_MAX_PITCH);

	pushEvent_({ m::mixer::EventType::CHANNEL_PITCH, 0, { 0, channelId, 0, {v} } }, t);
	
	sampleEditor::onRefresh(t == Thread::MAIN, [v](v::gdSampleEditor& e) { e.pitchTool->update(v); });
}


/* -------------------------------------------------------------------------- */


void sendChannelPan(ID channelId, float v)
{
	v = std::clamp(v, 0.0f, G_MAX_PAN);

	/* Pan event is currently triggered only by the main thread. */
	pushEvent_({ m::mixer::EventType::CHANNEL_PAN, 0, { 0, channelId, 0, {v} } }, Thread::MAIN);
	
	sampleEditor::onRefresh(/*gui=*/true, [v](v::gdSampleEditor& e) { e.panTool->update(v); });
}

/* -------------------------------------------------------------------------- */


void toggleMuteChannel(ID channelId, Thread t)
{
	pushEvent_({ m::mixer::EventType::CHANNEL_MUTE, 0, {0, channelId} }, t);
}


void toggleSoloChannel(ID channelId, Thread t)
{
	pushEvent_({ m::mixer::EventType::CHANNEL_SOLO, 0, {0, channelId} }, t);
}


/* -------------------------------------------------------------------------- */


void toggleArmChannel(ID channelId, Thread t)
{
	pushEvent_({ m::mixer::EventType::CHANNEL_TOGGLE_ARM, 0, {0, channelId} }, t);
}


void toggleReadActionsChannel(ID channelId, Thread t)
{
	pushEvent_({ m::mixer::EventType::CHANNEL_TOGGLE_READ_ACTIONS, 0, {0, channelId} }, t);
}


/* -------------------------------------------------------------------------- */


void sendMidiToChannel(ID channelId, m::MidiEvent e, Thread t)
{
	pushEvent_({ m::mixer::EventType::MIDI, 0, {0, channelId, 0, e} }, t);
}


/* -------------------------------------------------------------------------- */


void toggleMetronome()
{
	m::sequencer::toggleMetronome();
}


/* -------------------------------------------------------------------------- */


void setMasterInVolume(float v, Thread t)
{
	pushEvent_({ m::mixer::EventType::CHANNEL_VOLUME, 0, { 0, m::mixer::MASTER_IN_CHANNEL_ID, 0, {v} }}, t);

	if (t != Thread::MAIN) {
		Fl::lock();
		G_MainWin->mainIO->setInVol(v);
		Fl::unlock();
	}
}


void setMasterOutVolume(float v, Thread t)
{
	pushEvent_({ m::mixer::EventType::CHANNEL_VOLUME, 0, { 0, m::mixer::MASTER_OUT_CHANNEL_ID, 0, {v} }}, t);
	
	if (t != Thread::MAIN) {
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


void toggleSequencer()
{ 
	if (m::clock::isRunning())
		m::sequencer::stop();
	else
		m::sequencer::start();
}


void rewindSequencer()
{ 
	m::sequencer::rewind();
}


/* -------------------------------------------------------------------------- */


void toggleActionRecording()
{
	m::recManager::toggleActionRec(m::conf::conf.recTriggerMode);
}


void toggleInputRecording()
{
	if (!m::recManager::toggleInputRec(m::conf::conf.recTriggerMode))
		v::gdAlert("No channels armed/available for audio recording.");
}


/* -------------------------------------------------------------------------- */


void setPluginParameter(ID pluginId, int paramIndex, float value, bool gui)
{
	m::pluginHost::setPluginParameter(pluginId, paramIndex, value);
	c::plugin::updateWindow(pluginId, gui);
}
}}}; // giada::c::events::
