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
#include "core/model/data.h"
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
ID channelId_ = 0;

#ifdef WITH_VST

int readPatchPlugins_(const std::vector<patch::plugin_t>& list/*, pluginHost::StackType t*/)
{
#if 0
	int ret = 1;
	for (const patch::plugin_t& ppl : list) {
		std::unique_ptr<Plugin> p = pluginManager::makePlugin(ppl.path);
		if (p != nullptr) {
			p->setBypass(ppl.bypass);
			for (unsigned j=0; j<ppl.params.size(); j++)
				p->setParameter(j, ppl.params.at(j));
			pluginHost::addPlugin(std::move(p), {t, 0});
			ret &= 1;
		}
		else
			ret &= 0;
	}
	return ret;
#endif
}

#endif


/* -------------------------------------------------------------------------- */


std::unique_ptr<Channel> createChannel_(ChannelType type, size_t column, ID id)
{
	std::unique_ptr<Channel> ch = channelManager::create(type, 
		kernelAudio::getRealBufSize(), conf::inputMonitorDefaultOn, column);
	ch->id = id;
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


bool channelHas_(std::function<bool(std::unique_ptr<Channel>&)> f)
{
	std::vector<std::unique_ptr<Channel>>& c = m::model::getLayout()->channels;
	return std::any_of(c.begin(), c.end(), f);
}


/* -------------------------------------------------------------------------- */

/* pushWave_
Pushes a new wave into Sample Channel 'ch' and into the corresponding Data map.
Use this when modifying a local layout, before swapping it. */

void pushWave_(SampleChannel* ch, std::unique_ptr<Wave>&& w)
{
	model::Data& data = model::getData();
	data.waves[ch->id] = std::move(w);
	ch->pushWave(data.waves.at(ch->id));
}
}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void init()
{
	/* Create two MASTER channels: input and output. */
	
	std::shared_ptr<model::Layout> layout = model::cloneLayout();

	layout->channels.push_back(createChannel_(ChannelType::MASTER, /*column=*/0, 
		mixer::MASTER_OUT_CHANNEL_ID));
	layout->channels.push_back(createChannel_(ChannelType::MASTER, /*column=*/0, 
		mixer::MASTER_IN_CHANNEL_ID));

	/* IDs here start from 2: ID=0 and ID=1 are already taken by master
	channels. */

	channelId_ = layout->channels.size() + 1;

	model::swapLayout(layout);
}


/* -------------------------------------------------------------------------- */


bool uniqueSamplePath(const SampleChannel* skip, const std::string& path)
{
	assert(false);
	/*
	for (const Channel* ch : mixer::channels) {
		if (skip == ch || ch->type != ChannelType::SAMPLE) // skip itself and MIDI channels
			continue;
		const SampleChannel* sch = static_cast<const SampleChannel*>(ch);
		if (sch->wave != nullptr && path == sch->wave->getPath())
			return false;
	}
	*/
	return true;
}


/* -------------------------------------------------------------------------- */


Channel* addChannel(ChannelType type, size_t column)
{
	/* Create new channel and push it back into the local layout. */

	ID channelId = channelId_++;

	std::shared_ptr<model::Layout> layout = model::cloneLayout();
	layout->channels.push_back(createChannel_(type, column, channelId));
	model::swapLayout(layout);

	return layout->channels.back().get();
}


/* -------------------------------------------------------------------------- */


int loadChannel(ID channelId, const std::string& fname)
{
	waveManager::Result res = createWave_(fname); 
	if (res.status != G_RES_OK) 
		return res.status;

	std::shared_ptr<model::Layout> layout = model::cloneLayout();
	
	pushWave_(static_cast<SampleChannel*>(layout->getChannel(channelId)), 
		std::move(res.wave));
	
	model::swapLayout(layout);

	return res.status;
}


void loadChannel(ID channelId, std::unique_ptr<Wave>&& w)
{
	std::shared_ptr<model::Layout> layout = model::cloneLayout();
	
	pushWave_(static_cast<SampleChannel*>(layout->getChannel(channelId)), 
			std::move(w));
	
	model::swapLayout(layout);
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
	ID channelId = channelId_++;

	std::shared_ptr<model::Layout> layout = model::cloneLayout();
	
	std::unique_ptr<Channel> ch = createChannel_(ChannelType::SAMPLE, 
		columnIndex, channelId);
	layout->channels.push_back(std::move(ch));
	pushWave_(static_cast<SampleChannel*>(ch.get()), std::move(w));

	model::swapLayout(layout);
}


/* -------------------------------------------------------------------------- */


void cloneChannel(ID channelId)
{
	std::shared_ptr<model::Layout> layout = model::cloneLayout();
	
	ID newChannelId = channelId_++;

	const Channel*           oldChannel	= layout->getChannel(channelId);
	std::unique_ptr<Channel> newChannel = channelManager::create(*oldChannel);

	newChannel->id = newChannelId;

	/* Clone plugins first: it will add new values to the Data map. */

	pluginHost::clonePlugins(channelId, newChannelId);
	
	/* Also clone Wave if the original channel has one, i.e. create a new Wave
	in the Data map. */

	if (oldChannel->hasData()) {
		SampleChannel* ch = static_cast<SampleChannel*>(newChannel.get());

		pushWave_(ch, waveManager::createFromWave(*ch->wave, 0, 
			ch->wave->getSize()));
	}

	/* Push back the new channel into the local layout. */

	layout->channels.push_back(std::move(newChannel));
	
	/* Remove plugins from the channel in local layout: they still point to
	the old ones. We want new plugins created by pluginHost above, if any. */
	
	layout->getPlugins(newChannelId)->clear();
	for (std::shared_ptr<Plugin>& plugin : model::getData().plugins.at(newChannelId))
		layout->getPlugins(newChannelId)->push_back(plugin);


	model::swapLayout(layout);

	recorderHandler::cloneActions(channelId, newChannelId);
}


/* -------------------------------------------------------------------------- */


void freeChannel(ID channelId)
{
	std::shared_ptr<model::Layout> layout = model::cloneLayout();
	static_cast<SampleChannel*>(layout->getChannel(channelId))->empty();
	model::swapLayout(layout);

	model::getData().waves.at(channelId) = nullptr;
}


/* -------------------------------------------------------------------------- */


void freeAllChannels()
{
	std::shared_ptr<model::Layout> layout = model::cloneLayout();	
	for (std::unique_ptr<m::Channel>& ch : layout->channels)
		ch->empty();
	model::swapLayout(layout);

	model::getData().waves.clear();
}


/* -------------------------------------------------------------------------- */


void deleteChannel(ID channelId)
{
	std::shared_ptr<model::Layout>         layout   = model::cloneLayout();
	std::vector<std::unique_ptr<Channel>>& channels = layout->channels;

	u::vector::removeIf(channels, [=](const std::unique_ptr<Channel>& c)
	{
		return c->id == channelId;
	});

	model::swapLayout(layout);
	model::getData().waves.erase(channelId);
}


/* -------------------------------------------------------------------------- */


void renameChannel(ID channelId, const std::string& name)
{
	std::shared_ptr<model::Layout> layout = model::cloneLayout();
	static_cast<SampleChannel*>(layout->getChannel(channelId))->name = name;
	model::swapLayout(layout);	
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

	for (std::unique_ptr<Channel>& ch : m::model::getLayout()->channels)
		ch->stopBySeq(conf::chansStopOnSeqHalt);

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
	mixer::hasSolos = channelHas_([](std::unique_ptr<Channel>& ch)
	{
		return ch->solo == true;
	});
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


bool startInputRec()
{
	if (!hasRecordableSampleChannels())
		return false;
	mixer::startInputRec();
	return true;
}


/* -------------------------------------------------------------------------- */


void stopInputRec()
{
	mixer::stopInputRec();

	const AudioBuffer& virtualInput = mixer::getVirtualInput();

	std::shared_ptr<model::Layout> layout = model::cloneLayout();
	model::Data&                   data   = model::getData();

	/* Push a new Wave into each recordable channel. Warning: this algorithm
	will require some changes when we will allow overdubbing (the previous 
	existing Wave has to be discarded somehow). */

	for (std::unique_ptr<Channel>& ch : layout->channels) {

		if (!ch->canInputRec())
			continue;

		SampleChannel* sch = static_cast<SampleChannel*>(ch.get());

		/* Create a new Wave with audio coming from Mixer's virtual input. */

		std::unique_ptr<Wave> wave = waveManager::createEmpty(clock::getFramesInLoop(), 
			G_MAX_IO_CHANS, conf::samplerate, sch->name + ".wav");

		wave->copyData(virtualInput[0], virtualInput.countFrames());

		/* Move the new Wave into Data, then push it into the local Layout. 
		Create the key in data.waves if missing ([] operator). */

		data.waves[sch->id] = std::move(wave);
		sch->pushWave(data.waves.at(sch->id));
		sch->name = std::string("TAKE-" + std::to_string(patch::lastTakeId++));
	}

	model::swapLayout(layout);

	mixer::clearVirtualInput();
}


/* -------------------------------------------------------------------------- */


bool hasArmedSampleChannels()
{
	return channelHas_([](std::unique_ptr<Channel>& ch)
	{
		return ch->type == ChannelType::SAMPLE && ch->armed;
	});
}


bool hasRecordableSampleChannels()
{
	return channelHas_([](std::unique_ptr<Channel>& ch)
	{
		return ch->canInputRec();
	});
}


bool hasLogicalSamples()
{
	return channelHas_([](std::unique_ptr<Channel>& ch)
	{
		return ch->hasLogicalData();
	});
}


bool hasEditedSamples()
{
	return channelHas_([](std::unique_ptr<Channel>& ch)
	{
		return ch->hasEditedData();
	});
}


bool hasActions()
{
	return channelHas_([](std::unique_ptr<Channel>& ch)
	{
		return ch->hasActions.load();
	});
}


bool hasAudioData()
{
	return channelHas_([](std::unique_ptr<Channel>& ch)
	{
		return ch->hasData();
	});
}


}}}; // giada::m::mh::
