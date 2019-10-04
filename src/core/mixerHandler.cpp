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
#include <vector>
#include <algorithm>
#include "utils/fs.h"
#include "utils/string.h"
#include "utils/log.h"
#include "utils/vector.h"
#include "glue/main.h"
#include "glue/channel.h"
#include "core/model/model.h"
#include "core/channels/channel.h"
#include "core/channels/sampleChannel.h"
#include "core/channels/midiChannel.h"
#include "core/channels/channelManager.h"
#include "core/kernelMidi.h"
#include "core/mixer.h"
#include "core/const.h"
#include "core/init.h"
#include "core/pluginHost.h"
#include "core/pluginManager.h"
#include "core/plugin.h"
#include "core/waveFx.h"
#include "core/conf.h"
#include "core/patch.h"
#include "core/recorder.h"
#include "core/recorderHandler.h"
#include "core/recManager.h"
#include "core/clock.h"
#include "core/kernelAudio.h"
#include "core/midiMapConf.h"
#include "core/wave.h"
#include "core/waveManager.h"
#include "core/mixerHandler.h"


namespace giada {
namespace m {
namespace mh
{
namespace
{
std::unique_ptr<Channel> createChannel_(ChannelType type, size_t column, ID id=0)
{
	std::unique_ptr<Channel> ch = channelManager::create(type, 
		kernelAudio::getRealBufSize(), conf::inputMonitorDefaultOn, column);

	if (type == ChannelType::MASTER) {
		assert(id != 0);
		ch->id = id;
	}
	
	return ch;	
}


/* -------------------------------------------------------------------------- */


waveManager::Result createWave_(const std::string& fname)
{
	waveManager::Result res = waveManager::createFromFile(fname); 
	if (res.status != G_RES_OK)
		return res;
	if (res.wave->getRate() != conf::samplerate) {
		gu_log("[mh::loadChannel] input rate (%d) != system rate (%d), conversion needed\n",
			res.wave->getRate(), conf::samplerate);
		res.status = waveManager::resample(*res.wave.get(), conf::rsmpQuality, conf::samplerate); 
		if (res.status != G_RES_OK)
			return res;
	}
	return res;
}


/* -------------------------------------------------------------------------- */


bool channelHas_(std::function<bool(const Channel*)> f)
{
	model::ChannelsLock lock(model::channels);
	return std::any_of(model::channels.begin(), model::channels.end(), f);
}


/* -------------------------------------------------------------------------- */

/* pushWave_
Pushes a new wave into Sample Channel 'ch' and into the corresponding Wave list.
Use this when modifying a local model, before swapping it. */

void pushWave_(SampleChannel& ch, std::unique_ptr<Wave>&& w)
{
	ID    id   = w->id;
	Frame size = w->getSize();

	model::waves.push(std::move(w));
	ch.pushWave(id, size);	
}
}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void init()
{
	mixer::init(clock::getFramesInLoop(), kernelAudio::getRealBufSize());
	
	/* Create two MASTER channels: input and output. */
		
	model::channels.push(createChannel_(ChannelType::MASTER, /*column=*/0, 
		mixer::MASTER_OUT_CHANNEL_ID));
	model::channels.push(createChannel_(ChannelType::MASTER, /*column=*/0, 
		mixer::MASTER_IN_CHANNEL_ID));
}


/* -------------------------------------------------------------------------- */


void close()
{
	model::channels.clear();
	model::waves.clear();
	mixer::close();
}


/* -------------------------------------------------------------------------- */


bool uniqueSamplePath(ID channelToSkip, const std::string& path)
{
	model::ChannelsLock cl(model::channels);
	model::WavesLock    wl(model::waves);

	for (const Channel* c : model::channels) {
		if (c->id == channelToSkip || c->type != ChannelType::SAMPLE)
			continue;
		const SampleChannel* sc = static_cast<const SampleChannel*>(c);
		if (sc->hasWave && model::get(model::waves, sc->waveId).getPath() == path)
			return false;
	}
	return true;
}


/* -------------------------------------------------------------------------- */


void addChannel(ChannelType type, size_t column)
{
	model::channels.push(createChannel_(type, column));
}


/* -------------------------------------------------------------------------- */


int loadChannel(ID channelId, const std::string& fname)
{
	waveManager::Result res = createWave_(fname); 

	if (res.status != G_RES_OK) 
		return res.status;

	model::onSwap(model::channels, channelId, [&](Channel& c)
	{
		pushWave_(static_cast<SampleChannel&>(c), std::move(res.wave));
	});

	return res.status;
}


void loadChannel(ID channelId, std::unique_ptr<Wave>&& w)
{
	/* TODO */
	assert(false);
#if 0
	std::unique_ptr<SampleChannel> mc = model::channels.clone<SampleChannel>(model::getChannelIndex(channelId));

	pushWave_(static_cast<SampleChannel&>(*mc.get()), std::move(w));
	
	model::channels.swap(std::move(mc));
#endif
}


/* -------------------------------------------------------------------------- */


int addAndLoadChannel(size_t columnIndex, const std::string& fname)
{
	waveManager::Result res = createWave_(fname);
	if (res.status == G_RES_OK)
		addAndLoadChannel(columnIndex, std::move(res.wave));
	return res.status;
}


void addAndLoadChannel(size_t columnIndex, std::unique_ptr<Wave>&& w)
{
	size_t channelIndex = model::channels.size();
	
	std::unique_ptr<Channel> ch = createChannel_(ChannelType::SAMPLE, 
		columnIndex);

	/* Add Wave to Wave list first. */
	/* TODO - error: missing ch->waveIndex assignment */

	pushWave_(static_cast<SampleChannel&>(*ch.get()), std::move(w));

	/* Then add new channel to Channel list. */

	model::channels.push(std::move(ch));
}


/* -------------------------------------------------------------------------- */


void cloneChannel(ID channelId)
{
	model::ChannelsLock cl(model::channels);
	model::WavesLock    wl(model::waves);

	const Channel&           oldChannel = model::get(model::channels, channelId);
	std::unique_ptr<Channel> newChannel = channelManager::create(oldChannel);

	/* Set a new ID for the new channel. */

	newChannel->id = model::channels.size() + 1; 

	/* Clone plugins, actions and wave first in their own lists. */
	
	pluginHost::clonePlugins(oldChannel, *newChannel.get());
	recorderHandler::cloneActions(channelId, newChannel->id);
	
	if (newChannel->hasData()) {
		SampleChannel* sch = static_cast<SampleChannel*>(newChannel.get());
		const Wave&    w   = model::get(model::waves, sch->waveId);

		pushWave_(*sch, waveManager::createFromWave(w, 0, w.getSize()));
	}

	/* Then push the new channel in the channels list. */

	model::channels.push(std::move(newChannel));
}


/* -------------------------------------------------------------------------- */


void freeChannel(ID channelId)
{
	bool   hasWave;	
	size_t waveId;
	
	/* Remove Wave reference from Channel. */
	
	model::onSwap(model::channels, channelId, [&](Channel& c)
	{
		SampleChannel& sc = static_cast<SampleChannel&>(c);
		hasWave = sc.hasWave;
		waveId  = sc.waveId;
		sc.empty();
	});

	/* Then remove the actual Wave, if any. */
	
	if (hasWave)
		model::waves.pop(model::getIndex(model::waves, waveId)); 
}


/* -------------------------------------------------------------------------- */


void freeAllChannels()
{
	for (size_t i = 0; i < model::channels.size(); i++) {
		model::onSwap(model::channels, model::getId(model::channels, i), [](Channel& c)
		{
			c.empty();
		});
	}
	model::waves.clear();
}


/* -------------------------------------------------------------------------- */


void deleteChannel(ID channelId)
{
	bool            hasWave = false;
	size_t          waveId;
	std::vector<ID> pluginIds;

	model::onGet(model::channels, channelId, [&](Channel& c)
	{
		pluginIds = c.pluginIds;
		if (c.type != ChannelType::SAMPLE)
			return;
		SampleChannel& sc = static_cast<SampleChannel&>(c);
		hasWave   = sc.hasWave;
		waveId    = sc.waveId;
	});
	
	model::channels.pop(model::getIndex(model::channels, channelId));

	if (hasWave)
		model::waves.pop(model::getIndex(model::waves, waveId)); 

	pluginHost::freePlugins(pluginIds);
}


/* -------------------------------------------------------------------------- */


void renameChannel(ID channelId, const std::string& name)
{
	model::onSwap(model::channels, channelId, [&](Channel& c) { c.name = name; });
}


/* -------------------------------------------------------------------------- */


void startSequencer()
{
	switch (m::clock::getStatus()) {
		case ClockStatus::STOPPED:
			m::clock::setStatus(ClockStatus::RUNNING); 
			break;
		case ClockStatus::WAITING:
			m::clock::setStatus(ClockStatus::RUNNING); 
			m::recManager::stopActionRec();
			break;
		default: 
			break;
	}

#ifdef __linux__
	m::kernelAudio::jackStart();
#endif
}


/* -------------------------------------------------------------------------- */


void stopSequencer()
{
	clock::setStatus(ClockStatus::STOPPED);

	model::ChannelsLock l(model::channels);
	for (Channel* c : model::channels)
		c->stopBySeq(conf::chansStopOnSeqHalt);

#ifdef __linux__
	m::kernelAudio::jackStop();
#endif

	/* If recordings (both input and action) are active deactivate them, but 
	store the takes. RecManager takes care of it. */

	if (m::recManager::isRecordingAction())
		m::recManager::stopActionRec();
	else
	if (m::recManager::isRecordingInput())
		m::recManager::stopInputRec();
}


/* -------------------------------------------------------------------------- */


void toggleSequencer()
{
	m::clock::isRunning() ? stopSequencer() : startSequencer();
}


/* -------------------------------------------------------------------------- */


void updateSoloCount()
{
	m::model::onSwap(m::model::mixer, [&](m::model::Mixer& m)
	{
		m.hasSolos = channelHas_([](const Channel* ch) { return ch->solo; });
		printf("%d\n", m.hasSolos);
	});
}


/* -------------------------------------------------------------------------- */


void setInVol(float v)
{
	/* TODO - too much swap operations. Use direct variable update */
	m::model::onSwap(m::model::mixer, [&](m::model::Mixer& m)
	{
		m.inVol = v;
	});
}


void setOutVol(float v)
{
	/* TODO - too much swap operations. Use direct variable update */
	m::model::onSwap(m::model::mixer, [&](m::model::Mixer& m)
	{
		m.outVol = v;
	});
}


void setInToOut(bool v)
{
	m::model::onSwap(m::model::mixer, [&](m::model::Mixer& m)
	{
		m.inToOut = v;
	});
}


/* -------------------------------------------------------------------------- */


float getInVol()
{
	model::MixerLock lock(model::mixer); return model::mixer.get()->inVol;
}


float getOutVol()
{
	model::MixerLock lock(model::mixer); return model::mixer.get()->outVol;
}


bool getInToOut()
{
	model::MixerLock lock(model::mixer); return model::mixer.get()->inToOut;
}


/* -------------------------------------------------------------------------- */


void readPatch()
{
#if 0
	mixer::ready = false;

	mixer::outVol.store(patch::masterVolOut);
	mixer::inVol.store(patch::masterVolIn);
	clock::setBpm(patch::bpm);
	clock::setBars(patch::bars);
	clock::setBeats(patch::beats);
	clock::setQuantize(patch::quantize);
	clock::updateFrameBars();
	mixer::setMetronome(patch::metronome);

#ifdef WITH_VST

	readPatchPlugins_(patch::masterInPlugins, pluginHost::StackType::MASTER_IN);
	readPatchPlugins_(patch::masterOutPlugins, pluginHost::StackType::MASTER_OUT);

#endif

	/* Rewind and update frames in Mixer. Also alloc new space in the virtual
	input buffer, in case the patch has a sequencer size != default one (which is
	very likely). */

	mixer::rewind();
	mixer::allocVirtualInput(clock::getFramesInLoop());
	mixer::ready = true;
#endif
}


/* -------------------------------------------------------------------------- */


void rewindSequencer()
{
	if (clock::getQuantize() > 0 && clock::isRunning())   // quantize rewind
		mixer::rewindWait = true;
	else
		clock::rewind();

	/* FIXME - potential desync when Quantizer is enabled from this point on.
	Mixer would wait, while the following calls would be made regardless of its
	state. */

#ifdef __linux__
	m::kernelAudio::jackSetPosition(0);
#endif

	if (m::conf::midiSync == MIDI_SYNC_CLOCK_M)
		m::kernelMidi::send(MIDI_POSITION_PTR, 0, 0);
}


/* -------------------------------------------------------------------------- */

/*
bool startInputRec()
{
	if (!hasRecordableSampleChannels())
		return false;
	mixer::startInputRec();
	return true;
}
*/

/* -------------------------------------------------------------------------- */


/* Push a new Wave into each recordable channel. Warning: this algorithm will 
require some changes when we will allow overdubbing (the previous existing Wave
has to be overwritten somehow). */

void finalizeInputRec()
{
	const AudioBuffer& virtualInput = mixer::getVirtualInput();

	/* Can't loop with foreach, as it would require a lock on model::channels
	list which would deadlock during the model::channels::swap() call below. 
	Also skip channels 0 and 1: they are MASTER_IN and MASTER_OUT. */

	for (size_t i = 2; i < model::channels.size(); i++) {

		ID channelId;
		{
			model::ChannelsLock l(model::channels);
			Channel* ch = model::channels.get(i);
			if (!ch->canInputRec())
				continue;
			channelId = ch->id;
		}

		/* Create a new Wave with audio coming from Mixer's virtual input and
		push it in the model::waves list. */

		std::string filename = "TAKE-" + std::to_string(patch::lastTakeId++);

		std::unique_ptr<Wave> wave = waveManager::createEmpty(clock::getFramesInLoop(), 
			G_MAX_IO_CHANS, conf::samplerate, filename + ".wav");

		wave->copyData(virtualInput[0], virtualInput.countFrames());
		
		model::waves.push(std::move(wave));

		/* Update Channel with the new Wave. */

		std::unique_ptr<SampleChannel> ch = model::channels.clone<SampleChannel>(i);
		static_cast<SampleChannel*>(ch.get())->pushWave(wave->id, wave->getSize());
		model::channels.swap(std::move(ch), i);
	}

	mixer::clearVirtualInput();
}


/* -------------------------------------------------------------------------- */


bool hasRecordableSampleChannels()
{
	return channelHas_([](const Channel* ch) { return ch->canInputRec(); });
}


bool hasLogicalSamples()
{
	return channelHas_([](const Channel* ch) { return ch->hasLogicalData(); });
}


bool hasEditedSamples()
{
	return channelHas_([](const Channel* ch) { return ch->hasEditedData(); });
}


bool hasActions()
{
	return channelHas_([](const Channel* ch) { return ch->hasActions; });
}


bool hasAudioData()
{
	return channelHas_([](const Channel* ch) { return ch->hasData(); });
}
}}}; // giada::m::mh::
