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

#include "core/channels/channel.h"
#include "core/actions/actionRecorder.h"
#include "core/channels/sampleAdvancer.h"
#include "core/conf.h"
#include "core/engine.h"
#include "core/midiMapper.h"
#include "core/mixerHandler.h"
#include "core/model/model.h"
#include "core/plugins/pluginHost.h"
#include "core/plugins/pluginManager.h"
#include "core/recorder.h"
#include <cassert>

extern giada::m::Engine g_engine;

namespace giada::m::channel
{
namespace
{
mcl::AudioBuffer::Pan calcPanning_(float pan)
{
	/* TODO - precompute the AudioBuffer::Pan when pan value changes instead of
	building it on the fly. */

	/* Center pan (0.5f)? Pass-through. */

	if (pan == 0.5f)
		return {1.0f, 1.0f};
	return {1.0f - pan, pan};
}

/* -------------------------------------------------------------------------- */

void react_(Data& d, const EventDispatcher::Event& e)
{
	switch (e.type)
	{
	case EventDispatcher::EventType::CHANNEL_VOLUME:
		d.volume = std::get<float>(e.data);
		break;

	case EventDispatcher::EventType::CHANNEL_PAN:
		d.pan = std::get<float>(e.data);
		break;

	case EventDispatcher::EventType::CHANNEL_MUTE:
		d.setMute(!d.isMuted());
		break;

	case EventDispatcher::EventType::CHANNEL_TOGGLE_ARM:
		d.armed = !d.armed;
		break;

	case EventDispatcher::EventType::CHANNEL_SOLO:
		d.setSolo(!d.isSoloed());
		g_engine.mixerHandler.updateSoloCount();
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

void renderMasterOut_(const Data& d, mcl::AudioBuffer& out)
{
	d.buffer->audio.set(out, /*gain=*/1.0f);
#ifdef WITH_VST
	if (d.plugins.size() > 0)
		g_engine.pluginHost.processStack(d.buffer->audio, d.plugins, nullptr);
#endif
	out.set(d.buffer->audio, d.volume);
}

/* -------------------------------------------------------------------------- */

#ifdef WITH_VST

void renderMasterIn_(const Data& d, mcl::AudioBuffer& in)
{
	if (d.plugins.size() > 0)
		g_engine.pluginHost.processStack(in, d.plugins, nullptr);
}

#endif

/* -------------------------------------------------------------------------- */

void renderChannel_(const Data& d, mcl::AudioBuffer& out, mcl::AudioBuffer& in, bool audible)
{
	d.buffer->audio.clear();

	if (d.samplePlayer)
		samplePlayer::render(d);
	if (d.audioReceiver)
		audioReceiver::render(d, in);

		/* If MidiReceiver exists, let it process the plug-in stack, as it can 
	contain plug-ins that take MIDI events (i.e. synths). Otherwise process the
	plug-in stack internally with no MIDI events. */

#ifdef WITH_VST
	if (d.midiReceiver)
		midiReceiver::render(d, g_engine.pluginHost);
	else if (d.plugins.size() > 0)
		g_engine.pluginHost.processStack(d.buffer->audio, d.plugins, nullptr);
#endif

	if (audible)
		out.sum(d.buffer->audio, d.volume * d.volume_i, calcPanning_(d.pan));
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Buffer::Buffer(Frame bufferSize)
: audio(bufferSize, G_MAX_IO_CHANS)
{
}

/* -------------------------------------------------------------------------- */

Data::Data(ChannelType type, ID id, ID columnId, State& s, Buffer& b)
: state(&s)
, buffer(&b)
, id(id)
, type(type)
, columnId(columnId)
, volume(G_DEFAULT_VOL)
, volume_i(G_DEFAULT_VOL)
, pan(G_DEFAULT_PAN)
, armed(false)
, key(0)
, hasActions(false)
, height(G_GUI_UNIT)
, midiLighter(g_engine.midiMapper)
, m_mute(false)
, m_solo(false)
{
	switch (type)
	{
	case ChannelType::SAMPLE:
		sampleReactor.emplace(id, g_engine.sequencer, g_engine.model);
		audioReceiver.emplace();
		sampleActionRecorder.emplace(g_engine.actionRecorder, g_engine.sequencer);
		samplePlayer.emplace(&(state->resampler.value()));
		break;

	case ChannelType::PREVIEW:
		samplePlayer.emplace(&(state->resampler.value()));
		sampleReactor.emplace(id, g_engine.sequencer, g_engine.model);
		break;

	case ChannelType::MIDI:
		midiController.emplace();
		midiSender.emplace(g_engine.kernelMidi);
		midiActionRecorder.emplace(g_engine.actionRecorder, g_engine.sequencer);
#ifdef WITH_VST
		midiReceiver.emplace();
#endif
		break;

	default:
		break;
	}

	initCallbacks();
}

/* -------------------------------------------------------------------------- */

Data::Data(const Patch::Channel& p, State& s, Buffer& b, float samplerateRatio, Wave* wave)
: state(&s)
, buffer(&b)
, id(p.id)
, type(p.type)
, columnId(p.columnId)
, volume(p.volume)
, volume_i(G_DEFAULT_VOL)
, pan(p.pan)
, armed(p.armed)
, key(p.key)
, hasActions(p.hasActions)
, name(p.name)
, height(p.height)
#ifdef WITH_VST
, plugins(g_engine.pluginManager.hydratePlugins(p.pluginIds, g_engine.model)) // TODO move outside, as constructor parameter
#endif
, midiLearner(p)
, midiLighter(g_engine.midiMapper, p)
, m_mute(p.mute)
, m_solo(p.solo)
{
	state->readActions.store(p.readActions);
	state->recStatus.store(p.readActions ? ChannelStatus::PLAY : ChannelStatus::OFF);

	switch (type)
	{
	case ChannelType::SAMPLE:
		sampleReactor.emplace(id, g_engine.sequencer, g_engine.model);
		audioReceiver.emplace(p);
		sampleActionRecorder.emplace(g_engine.actionRecorder, g_engine.sequencer);
		samplePlayer.emplace(p, samplerateRatio, &(state->resampler.value()), wave);
		break;

	case ChannelType::PREVIEW:
		samplePlayer.emplace(p, samplerateRatio, &(state->resampler.value()), nullptr);
		sampleReactor.emplace(id, g_engine.sequencer, g_engine.model);
		break;

	case ChannelType::MIDI:
		midiController.emplace();
		midiSender.emplace(p, g_engine.kernelMidi);
		midiActionRecorder.emplace(g_engine.actionRecorder, g_engine.sequencer);
#ifdef WITH_VST
		midiReceiver.emplace();
#endif
		break;

	default:
		break;
	}

	initCallbacks();
}

/* -------------------------------------------------------------------------- */

Data::Data(const Data& other)
: midiLighter(g_engine.midiMapper)
{
	*this = other;
}

/* -------------------------------------------------------------------------- */

Data& Data::operator=(const Data& other)
{
	if (this == &other)
		return *this;

	state      = other.state;
	buffer     = other.buffer;
	id         = other.id;
	type       = other.type;
	columnId   = other.columnId;
	volume     = other.volume;
	volume_i   = other.volume_i;
	pan        = other.pan;
	m_mute     = other.m_mute;
	m_solo     = other.m_solo;
	armed      = other.armed;
	key        = other.key;
	hasActions = other.hasActions;
	name       = other.name;
	height     = other.height;
#ifdef WITH_VST
	plugins = other.plugins;
#endif

	midiLearner    = other.midiLearner;
	midiLighter    = other.midiLighter;
	samplePlayer   = other.samplePlayer;
	sampleReactor  = other.sampleReactor;
	audioReceiver  = other.audioReceiver;
	midiController = other.midiController;
#ifdef WITH_VST
	midiReceiver = other.midiReceiver;
#endif
	midiSender           = other.midiSender;
	sampleActionRecorder = other.sampleActionRecorder;
	midiActionRecorder   = other.midiActionRecorder;

	initCallbacks();

	return *this;
}

/* -------------------------------------------------------------------------- */

bool Data::operator==(const Data& other)
{
	return id == other.id;
}

/* -------------------------------------------------------------------------- */

bool Data::isInternal() const
{
	return type == ChannelType::MASTER || type == ChannelType::PREVIEW;
}

bool Data::isMuted() const
{
	/* Internals can't be muted. */
	return !isInternal() && m_mute;
}

bool Data::isSoloed() const
{
	return m_solo;
}

bool Data::canInputRec() const
{
	if (type != ChannelType::SAMPLE)
		return false;

	bool hasWave     = samplePlayer->hasWave();
	bool isProtected = audioReceiver->overdubProtection;
	bool canOverdub  = !hasWave || (hasWave && !isProtected);

	return armed && canOverdub;
}

bool Data::canActionRec() const
{
	return hasWave() && !samplePlayer->isAnyLoopMode();
}

bool Data::hasWave() const
{
	return samplePlayer && samplePlayer->hasWave();
}

bool Data::isPlaying() const
{
	ChannelStatus s = state->playStatus.load();
	return s == ChannelStatus::PLAY || s == ChannelStatus::ENDING;
}

bool Data::isReadingActions() const
{
	ChannelStatus s = state->recStatus.load();
	return s == ChannelStatus::PLAY || s == ChannelStatus::ENDING;
}

/* -------------------------------------------------------------------------- */

void Data::setMute(bool v)
{
	if (m_mute != v)
		midiLighter.sendMute(v);
	m_mute = v;
}

void Data::setSolo(bool v)
{
	if (m_solo != v)
		midiLighter.sendSolo(v);
	m_solo = v;
}

/* -------------------------------------------------------------------------- */

void Data::initCallbacks()
{
	state->playStatus.onChange = [this](ChannelStatus status) {
		midiLighter.sendStatus(status, g_engine.mixer.isChannelAudible(*this));
	};

	if (samplePlayer)
	{
		samplePlayer->onLastFrame = [this]() {
			sampleAdvancer::onLastFrame(*this, g_engine.sequencer.isRunning());
		};
	}
}

/* -------------------------------------------------------------------------- */

void advance(const Data& d, const Sequencer::EventBuffer& events)
{
	for (const Sequencer::Event& e : events)
	{
		if (d.midiController)
			midiController::advance(d, e);
		if (d.samplePlayer)
			sampleAdvancer::advance(d, e);
		if (d.midiSender)
			midiSender::advance(d, e);
#ifdef WITH_VST
		if (d.midiReceiver)
			midiReceiver::advance(d, e);
#endif
	}
}

/* -------------------------------------------------------------------------- */

void react(Data& d, const EventDispatcher::EventBuffer& events)
{
	for (const EventDispatcher::Event& e : events)
	{
		if (e.channelId > 0 && e.channelId != d.id)
			continue;

		react_(d, e);
		if (d.midiController)
			midiController::react(d, e);
		if (d.midiSender)
			midiSender::react(d, e);
		if (d.samplePlayer)
			samplePlayer::react(d, e);
		if (d.midiActionRecorder)
			midiActionRecorder::react(d, e, g_engine.recorder.canRecordActions());
		if (d.sampleActionRecorder)
			sampleActionRecorder::react(d, e, g_engine.conf.data.treatRecsAsLoops,
			    g_engine.sequencer.isRunning(), g_engine.recorder.canRecordActions());
		if (d.sampleReactor)
			sampleReactor::react(d, e, g_engine.sequencer, g_engine.conf.data);
#ifdef WITH_VST
		if (d.midiReceiver)
			midiReceiver::react(d, e);
#endif
	}
}

/* -------------------------------------------------------------------------- */

void render(const Data& d, mcl::AudioBuffer* out, mcl::AudioBuffer* in, bool audible)
{
	if (d.id == Mixer::MASTER_OUT_CHANNEL_ID)
		renderMasterOut_(d, *out);
#ifdef WITH_VST
	else if (d.id == Mixer::MASTER_IN_CHANNEL_ID)
		renderMasterIn_(d, *in);
#endif
	else
		renderChannel_(d, *out, *in, audible);
}
} // namespace giada::m::channel