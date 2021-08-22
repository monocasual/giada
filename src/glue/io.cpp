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

#include "io.h"
#include "channel.h"
#include "core/clock.h"
#include "core/conf.h"
#include "core/kernelAudio.h"
#include "core/midiDispatcher.h"
#include "core/mixer.h"
#include "core/mixerHandler.h"
#include "core/model/model.h"
#include "core/recManager.h"
#include "core/recorder.h"
#include "core/recorderHandler.h"
#include "core/wave.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/midiIO/midiInputBase.h"
#include "gui/dialogs/warnings.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "gui/elems/mainWindow/keyboard/channelButton.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "gui/elems/mainWindow/mainTimer.h"
#include "gui/elems/mainWindow/mainTransport.h"
#include "main.h"
#include "utils/gui.h"
#include "utils/log.h"
#include "utils/math.h"
#include <FL/Fl.H>

extern giada::v::gdMainWindow* G_MainWin;

namespace giada::c::io
{
namespace
{
void rebuildMidiWindows_()
{
	u::gui::rebuildSubWindow(WID_MIDI_INPUT);
	u::gui::rebuildSubWindow(WID_MIDI_OUTPUT);
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Channel_InputData::Channel_InputData(const m::channel::Data& c)
: channelId(c.id)
, channelType(c.type)
, enabled(c.midiLearner.enabled)
, velocityAsVol(c.samplePlayer ? c.samplePlayer->velocityAsVol : 0)
, filter(c.midiLearner.filter)
, keyPress(c.midiLearner.keyPress.getValue())
, keyRelease(c.midiLearner.keyRelease.getValue())
, kill(c.midiLearner.kill.getValue())
, arm(c.midiLearner.arm.getValue())
, volume(c.midiLearner.volume.getValue())
, mute(c.midiLearner.mute.getValue())
, solo(c.midiLearner.solo.getValue())
, pitch(c.midiLearner.pitch.getValue())
, readActions(c.midiLearner.readActions.getValue())
{
#ifdef WITH_VST
	for (const m::Plugin* p : c.plugins)
	{
		PluginData pd;
		pd.id   = p->id;
		pd.name = p->getName();
		for (int i = 0; i < p->getNumParameters(); i++)
			pd.params.push_back({i, p->getParameterName(i), p->midiInParams.at(i).getValue()});
		plugins.push_back(pd);
	}
#endif
}

/* -------------------------------------------------------------------------- */

MidiChannel_OutputData::MidiChannel_OutputData(const m::midiSender::Data& s)
: enabled(s.enabled)
, filter(s.filter)
{
}

/* -------------------------------------------------------------------------- */

Channel_OutputData::Channel_OutputData(const m::channel::Data& c)
: channelId(c.id)
, lightningEnabled(c.midiLighter.enabled)
, lightningPlaying(c.midiLighter.playing.getValue())
, lightningMute(c.midiLighter.mute.getValue())
, lightningSolo(c.midiLighter.solo.getValue())
{
	if (c.type == ChannelType::MIDI)
		output = std::make_optional<MidiChannel_OutputData>(*c.midiSender);
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
	return Channel_InputData(m::model::get().getChannel(channelId));
}

/* -------------------------------------------------------------------------- */

Channel_OutputData channel_getOutputData(ID channelId)
{
	return Channel_OutputData(m::model::get().getChannel(channelId));
}

/* -------------------------------------------------------------------------- */

Master_InputData master_getInputData()
{
	return Master_InputData(m::model::get().midiIn);
}

/* -------------------------------------------------------------------------- */

void channel_enableMidiLearn(ID channelId, bool v)
{
	m::model::get().getChannel(channelId).midiLearner.enabled = v;
	m::model::swap(m::model::SwapType::NONE);
	rebuildMidiWindows_();
}

/* -------------------------------------------------------------------------- */

void channel_enableMidiLightning(ID channelId, bool v)
{
	m::model::get().getChannel(channelId).midiLighter.enabled = v;
	m::model::swap(m::model::SwapType::NONE);
	rebuildMidiWindows_();
}

/* -------------------------------------------------------------------------- */

void channel_enableMidiOutput(ID channelId, bool v)
{
	m::model::get().getChannel(channelId).midiSender->enabled = v;
	m::model::swap(m::model::SwapType::NONE);
	rebuildMidiWindows_();
}

/* -------------------------------------------------------------------------- */

void channel_enableVelocityAsVol(ID channelId, bool v)
{
	m::model::get().getChannel(channelId).samplePlayer->velocityAsVol = v;
	m::model::swap(m::model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

void channel_setMidiInputFilter(ID channelId, int ch)
{
	m::model::get().getChannel(channelId).midiLearner.filter = ch;
	m::model::swap(m::model::SwapType::NONE);
}

void channel_setMidiOutputFilter(ID channelId, int ch)
{
	m::model::get().getChannel(channelId).midiSender->filter = ch;
	m::model::swap(m::model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */
void assign_thru_monitor(int ch)
{
	m::midiDispatcher::set_thruMonitor(ch);
}
/* -------------------------------------------------------------------------- */

void channel_setKey(ID channelId, int k)
{
	m::model::get().getChannel(channelId).key = k;
	m::model::swap(m::model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void channel_startMidiLearn(int param, ID channelId)
{
	m::midiDispatcher::startChannelLearn(param, channelId, rebuildMidiWindows_);
}

void master_startMidiLearn(int param)
{
	m::midiDispatcher::startMasterLearn(param, rebuildMidiWindows_);
}

#ifdef WITH_VST

void plugin_startMidiLearn(int paramIndex, ID pluginId)
{
	m::midiDispatcher::startPluginLearn(paramIndex, pluginId, rebuildMidiWindows_);
}

#endif

/* -------------------------------------------------------------------------- */

void stopMidiLearn()
{
	m::midiDispatcher::stopLearn();
	rebuildMidiWindows_();
}

/* -------------------------------------------------------------------------- */

void channel_clearMidiLearn(int param, ID channelId)
{
	m::midiDispatcher::clearChannelLearn(param, channelId, rebuildMidiWindows_);
}

void master_clearMidiLearn(int param)
{
	m::midiDispatcher::clearMasterLearn(param, rebuildMidiWindows_);
}

#ifdef WITH_VST

void plugin_clearMidiLearn(int param, ID pluginId)
{
	m::midiDispatcher::clearPluginLearn(param, pluginId, rebuildMidiWindows_);
}

#endif

/* -------------------------------------------------------------------------- */

void master_enableMidiLearn(bool v)
{
	m::model::get().midiIn.enabled = v;
	m::model::swap(m::model::SwapType::NONE);
	rebuildMidiWindows_();
}

/* -------------------------------------------------------------------------- */

void master_setMidiFilter(int c)
{
	m::model::get().midiIn.filter = c;
	m::model::swap(m::model::SwapType::NONE);
}
} // namespace giada::c::io
