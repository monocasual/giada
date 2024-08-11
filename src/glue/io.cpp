/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "glue/io.h"
#include "core/conf.h"
#include "core/engine.h"
#include "core/kernelAudio.h"
#include "core/midiDispatcher.h"
#include "core/mixer.h"
#include "core/model/model.h"
#include "core/recorder.h"
#include "core/wave.h"
#include "glue/channel.h"
#include "glue/main.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/midiIO/midiInputBase.h"
#include "gui/dialogs/warnings.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "gui/elems/mainWindow/keyboard/channelButton.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "gui/elems/mainWindow/mainTimer.h"
#include "gui/elems/mainWindow/mainTransport.h"
#include "gui/ui.h"
#include "utils/log.h"
#include "utils/math.h"
#include <FL/Fl.H>

extern giada::v::Ui*     g_ui;
extern giada::m::Engine* g_engine;

namespace giada::c::io
{
namespace
{
void rebuildMidiWindows_()
{
	g_ui->rebuildSubWindow(WID_MIDI_INPUT);
	g_ui->rebuildSubWindow(WID_MIDI_OUTPUT);
}

/* -------------------------------------------------------------------------- */

bool isValidKey_(int key)
{
	if (strlen(Fl::event_text()) == 0)
		return false;
	if (key == g_ui->model.keyBindPlay ||
	    key == g_ui->model.keyBindRewind ||
	    key == g_ui->model.keyBindRecordActions ||
	    key == g_ui->model.keyBindRecordInput ||
	    key == g_ui->model.keyBindExit)
		return false;
	return true;
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Channel_InputData::Channel_InputData(const m::Channel& c)
: channelId(c.id)
, channelType(c.type)
, enabled(c.midiInput.enabled)
, velocityAsVol(c.sampleChannel ? c.sampleChannel->velocityAsVol : 0)
, filter(c.midiInput.filter)
, keyPress(c.midiInput.keyPress.getValue())
, keyRelease(c.midiInput.keyRelease.getValue())
, kill(c.midiInput.kill.getValue())
, arm(c.midiInput.arm.getValue())
, volume(c.midiInput.volume.getValue())
, mute(c.midiInput.mute.getValue())
, solo(c.midiInput.solo.getValue())
, pitch(c.midiInput.pitch.getValue())
, readActions(c.midiInput.readActions.getValue())
{
	for (const m::Plugin* p : c.plugins)
	{
		PluginData pd;
		pd.id   = p->id;
		pd.name = p->getName();
		for (int i = 0; i < p->getNumParameters(); i++)
			pd.params.push_back({i, p->getParameterName(i), p->midiInParams.at(i).getValue()});
		plugins.push_back(pd);
	}
}

/* -------------------------------------------------------------------------- */

MidiChannel_OutputData::MidiChannel_OutputData(const m::Channel& c)
: enabled(c.midiChannel->outputEnabled)
, filter(c.midiChannel->outputFilter)
{
}

/* -------------------------------------------------------------------------- */

Channel_OutputData::Channel_OutputData(const m::Channel& c)
: channelId(c.id)
, lightningEnabled(c.midiLightning.enabled)
, lightningPlaying(c.midiLightning.playing.getValue())
, lightningMute(c.midiLightning.mute.getValue())
, lightningSolo(c.midiLightning.solo.getValue())
{
	if (c.type == ChannelType::MIDI)
		output = std::make_optional<MidiChannel_OutputData>(c);
}

/* -------------------------------------------------------------------------- */

Master_InputData::Master_InputData(const m::model::MidiIn& midiIn)
: enabled(midiIn.enabled)
, filter(midiIn.filter)
, rewind(midiIn.rewind)
, startStop(midiIn.startStop)
, actionRec(midiIn.actionRec)
, inputRec(midiIn.inputRec)
, volumeIn(midiIn.volumeIn)
, volumeOut(midiIn.volumeOut)
, beatDouble(midiIn.beatDouble)
, beatHalf(midiIn.beatHalf)
, metronome(midiIn.metronome)
{
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Channel_InputData channel_getInputData(ID channelId)
{
	return Channel_InputData(g_engine->getChannelsApi().get(channelId));
}

/* -------------------------------------------------------------------------- */

Channel_OutputData channel_getOutputData(ID channelId)
{
	return Channel_OutputData(g_engine->getChannelsApi().get(channelId));
}

/* -------------------------------------------------------------------------- */

Master_InputData master_getInputData()
{
	return Master_InputData(g_engine->getIOApi().getModelMidiIn());
}

/* -------------------------------------------------------------------------- */

void channel_enableMidiLearn(ID channelId, bool v)
{
	g_engine->getIOApi().channel_enableMidiInput(channelId, v);
	rebuildMidiWindows_();
}

/* -------------------------------------------------------------------------- */

void channel_enableMidiLightning(ID channelId, bool v)
{
	g_engine->getIOApi().channel_enableMidiLightning(channelId, v);
	rebuildMidiWindows_();
}

/* -------------------------------------------------------------------------- */

void channel_enableMidiOutput(ID channelId, bool v)
{
	g_engine->getIOApi().channel_enableMidiOutput(channelId, v);
	rebuildMidiWindows_();
}

/* -------------------------------------------------------------------------- */

void channel_enableVelocityAsVol(ID channelId, bool v)
{
	g_engine->getIOApi().channel_enableVelocityAsVol(channelId, v);
}

/* -------------------------------------------------------------------------- */

void channel_setMidiInputFilter(ID channelId, int ch)
{
	g_engine->getIOApi().channel_setMidiInputFilter(channelId, ch);
}

void channel_setMidiOutputFilter(ID channelId, int ch)
{
	g_engine->getIOApi().channel_setMidiOutputFilter(channelId, ch);
}

/* -------------------------------------------------------------------------- */

bool channel_setKey(ID channelId, int k)
{
	if (!isValidKey_(k))
		return false;
	g_engine->getIOApi().channel_setKey(channelId, k);
	return true;
}

/* -------------------------------------------------------------------------- */

void channel_startMidiLearn(int param, ID channelId)
{
	g_engine->getIOApi().channel_startMidiLearn(param, channelId, []()
	{ g_ui->pumpEvent([]()
	  { rebuildMidiWindows_(); }); });
}

void master_startMidiLearn(int param)
{
	g_engine->getIOApi().master_startMidiLearn(param, []()
	{ g_ui->pumpEvent([]()
	  { rebuildMidiWindows_(); }); });
}

void plugin_startMidiLearn(int paramIndex, ID pluginId)
{
	g_engine->getIOApi().plugin_startMidiLearn(paramIndex, pluginId, []()
	{ g_ui->pumpEvent([]()
	  { rebuildMidiWindows_(); }); });
}

/* -------------------------------------------------------------------------- */

void stopMidiLearn()
{
	g_engine->getIOApi().stopMidiLearn();
	g_ui->pumpEvent([]()
	{ rebuildMidiWindows_(); });
}

/* -------------------------------------------------------------------------- */

void channel_clearMidiLearn(int param, ID channelId)
{
	g_engine->getIOApi().channel_clearMidiLearn(param, channelId, []()
	{ g_ui->pumpEvent([]()
	  { rebuildMidiWindows_(); }); });
}

void master_clearMidiLearn(int param)
{
	g_engine->getIOApi().master_clearMidiLearn(param, []()
	{ g_ui->pumpEvent([]()
	  { rebuildMidiWindows_(); }); });
}

void plugin_clearMidiLearn(int param, ID pluginId)
{
	g_engine->getIOApi().plugin_clearMidiLearn(param, pluginId, []()
	{ g_ui->pumpEvent([]()
	  { rebuildMidiWindows_(); }); });
}

/* -------------------------------------------------------------------------- */

void master_enableMidiLearn(bool v)
{
	g_engine->getIOApi().master_enableMidiLearn(v);
	rebuildMidiWindows_();
}

/* -------------------------------------------------------------------------- */

void master_setMidiFilter(int c)
{
	g_engine->getIOApi().master_setMidiFilter(c);
}
} // namespace giada::c::io