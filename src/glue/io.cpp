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


#include <FL/Fl.H>
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/midiIO/midiInputBase.h"
#include "gui/dialogs/warnings.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/mainWindow/mainTransport.h"
#include "gui/elems/mainWindow/mainTimer.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "gui/elems/mainWindow/keyboard/channelButton.h"
#include "gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "utils/gui.h"
#include "utils/log.h"
#include "utils/math.h"
#include "core/model/model.h"
#include "core/recorder.h"
#include "core/conf.h"
#include "core/recManager.h"
#include "core/kernelAudio.h"
#include "core/mixer.h"
#include "core/mixerHandler.h"
#include "core/wave.h"
#include "core/midiLearner.h"
#include "core/clock.h"
#include "core/recorderHandler.h"
#include "main.h"
#include "channel.h"
#include "io.h"


extern giada::v::gdMainWindow* G_MainWin;


namespace giada {
namespace c {
namespace io 
{
namespace
{
void rebuildMidiWindows_()
{
	u::gui::rebuildSubWindow(WID_MIDI_INPUT);
	u::gui::rebuildSubWindow(WID_MIDI_OUTPUT);
}
} // {anonymous}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


Channel_InputData::Channel_InputData(const m::Channel& c)
: channelId    (c.id)
, channelType  (c.getType())
, enabled      (c.midiLearner.state->enabled.load())
, velocityAsVol(c.samplePlayer ? c.samplePlayer->state->velocityAsVol.load() : 0)
, filter       (c.midiLearner.state->filter.load())
, keyPress     (c.midiLearner.state->keyPress.load())
, keyRelease   (c.midiLearner.state->keyRelease.load())
, kill         (c.midiLearner.state->kill.load())
, arm          (c.midiLearner.state->arm.load())
, volume       (c.midiLearner.state->volume.load())
, mute         (c.midiLearner.state->mute.load())
, solo         (c.midiLearner.state->solo.load())
, pitch        (c.midiLearner.state->pitch.load())
, readActions  (c.midiLearner.state->readActions.load()) 
{
#ifdef WITH_VST
	for (ID id : c.pluginIds) {
		m::Plugin& p = m::model::get(m::model::plugins, id);
		
		PluginData pd;
		pd.id = p.id;
		pd.name = p.getName();
		for (int i = 0; i < p.getNumParameters(); i++)
			pd.params.push_back({ i, p.getParameterName(i), p.midiInParams.at(i) });

		plugins.push_back(pd);
	}
#endif
}


/* -------------------------------------------------------------------------- */


MidiChannel_OutputData::MidiChannel_OutputData(const m::MidiSender& s)
: enabled(s.state->enabled.load())
, filter (s.state->filter.load())
{
}


/* -------------------------------------------------------------------------- */


Channel_OutputData::Channel_OutputData(const m::Channel& c)
: channelId       (c.id)
, lightningEnabled(c.midiLighter.state->enabled.load())
, lightningPlaying(c.midiLighter.state->playing.load())
, lightningMute   (c.midiLighter.state->mute.load())
, lightningSolo   (c.midiLighter.state->solo.load())
{	
	if (c.getType() == ChannelType::MIDI)
		output = std::make_optional<MidiChannel_OutputData>(*c.midiSender);
}


/* -------------------------------------------------------------------------- */


Master_InputData::Master_InputData(const m::model::MidiIn& midiIn)
: enabled   (midiIn.enabled)
, filter    (midiIn.filter)
, rewind    (midiIn.rewind)
, startStop (midiIn.startStop)
, actionRec (midiIn.actionRec)
, inputRec  (midiIn.inputRec)
, volumeIn  (midiIn.volumeIn)
, volumeOut (midiIn.volumeOut)
, beatDouble(midiIn.beatDouble)
, beatHalf  (midiIn.beatHalf)
, metronome (midiIn.metronome)	
{
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


Channel_InputData channel_getInputData(ID channelId)
{
	namespace mm = m::model;

	mm::ChannelsLock cl(mm::channels);
#ifdef WITH_VST
	mm::PluginsLock  ml(mm::plugins);
#endif

	return Channel_InputData(mm::get(mm::channels, channelId));	
}


/* -------------------------------------------------------------------------- */


Channel_OutputData channel_getOutputData(ID channelId)
{
	namespace mm = m::model;

	mm::ChannelsLock cl(mm::channels);
	return Channel_OutputData(mm::get(mm::channels, channelId));		
}


/* -------------------------------------------------------------------------- */


Master_InputData master_getInputData()
{
	namespace mm = m::model;

	mm::MidiInLock l(mm::midiIn);
	return Master_InputData(*mm::midiIn.get());
}


/* -------------------------------------------------------------------------- */


void channel_enableMidiLearn(ID channelId, bool v)
{
	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c)
	{
		c.midiLearner.state->enabled.store(v);
	});
	rebuildMidiWindows_();
}


/* -------------------------------------------------------------------------- */


void channel_enableMidiLightning(ID channelId, bool v)
{
	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c)
	{
		c.midiLighter.state->enabled.store(v);
	});
	rebuildMidiWindows_();
}


/* -------------------------------------------------------------------------- */


void channel_enableMidiOutput(ID channelId, bool v)
{
	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c)
	{
		c.midiSender->state->enabled.store(v);
	});	
	rebuildMidiWindows_();
}


/* -------------------------------------------------------------------------- */


void channel_enableVelocityAsVol(ID channelId, bool v)
{
	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c)
	{
		c.samplePlayer->state->velocityAsVol.store(v);
	});
}


/* -------------------------------------------------------------------------- */


void channel_setMidiInputFilter(ID channelId, int ch)
{
	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c)
	{
		c.midiLearner.state->filter.store(ch);
	});
}


void channel_setMidiOutputFilter(ID channelId, int ch)
{
	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c)
	{
		c.midiSender->state->filter.store(ch);
	});	
}


/* -------------------------------------------------------------------------- */


void channel_setKey(ID channelId, int k)
{
	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c)
	{
		c.state->key.store(k);
	}, /*rebuild=*/true);
}


/* -------------------------------------------------------------------------- */


void channel_startMidiLearn(int param, ID channelId)
{
	m::midiLearner::startChannelLearn(param, channelId, rebuildMidiWindows_);
}


void master_startMidiLearn(int param)
{
	m::midiLearner::startMasterLearn(param, rebuildMidiWindows_);
}


#ifdef WITH_VST

void plugin_startMidiLearn(int paramIndex, ID pluginId)
{
	m::midiLearner::startPluginLearn(paramIndex, pluginId, rebuildMidiWindows_);
}

#endif


/* -------------------------------------------------------------------------- */


void stopMidiLearn()
{
	m::midiLearner::stopLearn();
	rebuildMidiWindows_();
}


/* -------------------------------------------------------------------------- */


void channel_clearMidiLearn(int param, ID channelId)
{
	m::midiLearner::clearChannelLearn(param, channelId, rebuildMidiWindows_);
}


void master_clearMidiLearn (int param)
{
	m::midiLearner::clearMasterLearn(param, rebuildMidiWindows_);
}


#ifdef WITH_VST

void plugin_clearMidiLearn (int param, ID pluginId)
{
	m::midiLearner::clearPluginLearn(param, pluginId, rebuildMidiWindows_);
}

#endif


/* -------------------------------------------------------------------------- */


void master_enableMidiLearn(bool v)
{
	m::model::onSwap(m::model::midiIn, [&](m::model::MidiIn& m)
	{
		m.enabled = v;
	});	
	rebuildMidiWindows_();
}


/* -------------------------------------------------------------------------- */


void master_setMidiFilter(int c)
{
	m::model::onSwap(m::model::midiIn, [&](m::model::MidiIn& m)
	{
		m.filter = c;
	});
}
}}} // giada::c::io::
