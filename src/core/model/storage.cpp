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


#include <cassert>
#include "core/model/model.h"
#include "core/channels/channelManager.h"
#include "core/kernelAudio.h"
#include "core/patch.h"
#include "core/conf.h"
#include "core/plugins/pluginManager.h"
#include "core/recorderHandler.h"
#include "core/waveManager.h"
#include "core/sequencer.h"
#include "core/model/storage.h"


namespace giada {
namespace m {
namespace model
{
void store(patch::Patch& patch)
{
#ifdef WITH_VST
	PluginsLock  pl (plugins);
#endif
	ActionsLock  al (actions);
	WavesLock    wl (waves);
	ClockLock    cl (clock);
	ChannelsLock chl(channels);

	patch.bars       = clock.get()->bars;
	patch.beats      = clock.get()->beats;
	patch.bpm        = clock.get()->bpm;
	patch.quantize   = clock.get()->quantize;
	patch.metronome  = sequencer::isMetronomeOn();
	patch.samplerate = conf::conf.samplerate;

#ifdef WITH_VST
	for (const Plugin* p : plugins) 
		patch.plugins.push_back(pluginManager::serializePlugin(*p));
#endif

	patch.actions = recorderHandler::serializeActions(actions.get()->map); 

	for (const Wave* w : waves)
		patch.waves.push_back(waveManager::serializeWave(*w));

	for (const Channel* c : channels)
		patch.channels.push_back(channelManager::serializeChannel(*c));
}


/* -------------------------------------------------------------------------- */


void store(conf::Conf& conf)
{
	MidiInLock l(midiIn);

	conf.midiInEnabled    = midiIn.get()->enabled;
	conf.midiInFilter     = midiIn.get()->filter;
	conf.midiInRewind     = midiIn.get()->rewind;
	conf.midiInStartStop  = midiIn.get()->startStop;
	conf.midiInActionRec  = midiIn.get()->actionRec;
	conf.midiInInputRec   = midiIn.get()->inputRec;
	conf.midiInMetronome  = midiIn.get()->metronome;
	conf.midiInVolumeIn   = midiIn.get()->volumeIn;
	conf.midiInVolumeOut  = midiIn.get()->volumeOut;
	conf.midiInBeatDouble = midiIn.get()->beatDouble;
	conf.midiInBeatHalf   = midiIn.get()->beatHalf;
}


/* -------------------------------------------------------------------------- */


void load(const patch::Patch& patch)
{
	onSwap(clock, [&](Clock& c)
	{
	    c.status       = ClockStatus::STOPPED;
	    c.bars         = patch.bars;
	    c.beats        = patch.beats;
	    c.bpm          = patch.bpm;
	    c.quantize     = patch.quantize;
	});

	onSwap(actions, [&](Actions& a)
	{
		a.map = std::move(recorderHandler::deserializeActions(patch.actions));
	});

#ifdef WITH_VST
    for (const patch::Plugin& pplugin : patch.plugins)
        plugins.push(pluginManager::deserializePlugin(pplugin, patch.version));
#endif
    
	for (const patch::Wave& pwave : patch.waves) {
		std::unique_ptr<Wave> w = waveManager::deserializeWave(pwave, conf::conf.samplerate,
			conf::conf.rsmpQuality);
		if (w != nullptr)
			waves.push(std::move(w));	
	}

	channels.clear();
    for (const patch::Channel& pchannel : patch.channels)
		channels.push(channelManager::deserializeChannel(pchannel, kernelAudio::getRealBufSize()));
	
	/* Load Waves into Channels. */

	ChannelsLock cl(channels);
	WavesLock    wl(waves);

	float samplerateRatio = conf::conf.samplerate / static_cast<float>(patch::patch.samplerate);
	
	for (Channel* c : channels) {
		if (!c->samplePlayer)
			continue;
		if (exists(waves, c->samplePlayer->getWaveId()))
			c->samplePlayer->setWave(get(waves, c->samplePlayer->getWaveId()), samplerateRatio);
		else
			c->samplePlayer->setInvalidWave();
	}
}


/* -------------------------------------------------------------------------- */


void load(const conf::Conf& c)
{
	onSwap(midiIn, [&](MidiIn& m)
	{
		m.enabled    = c.midiInEnabled;
		m.filter     = c.midiInFilter;
		m.rewind     = c.midiInRewind;
		m.startStop  = c.midiInStartStop;
		m.actionRec  = c.midiInActionRec;
		m.inputRec   = c.midiInInputRec;
		m.volumeIn   = c.midiInMetronome;
		m.volumeOut  = c.midiInVolumeIn;
		m.beatDouble = c.midiInVolumeOut;
		m.beatHalf   = c.midiInBeatDouble;
		m.metronome  = c.midiInBeatHalf;
	});	
}
}}} // giada::m::model::
