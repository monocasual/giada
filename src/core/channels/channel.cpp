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

#include "channel.h"
#include "core/mixerHandler.h"
#include "core/plugins/pluginHost.h"
#include "core/plugins/pluginManager.h"
#include <cassert>

namespace giada::m::channel
{
namespace
{
AudioBuffer::Pan calcPanning_(float pan)
{
	/* TODO - precompute the AudioBuffer::Pan when pan value changes instead of
	building it on the fly. */

	/* Center pan (0.5f)? Pass-through. */

	if (pan == 0.5f)
		return {1.0f, 1.0f};
	return {1.0f - pan, pan};
}

/* -------------------------------------------------------------------------- */

void react_(Data& d, const eventDispatcher::Event& e)
{
	switch (e.type)
	{

	case eventDispatcher::EventType::CHANNEL_VOLUME:
		d.volume = std::get<float>(e.data);
		break;

	case eventDispatcher::EventType::CHANNEL_PAN:
		d.pan = std::get<float>(e.data);
		break;

	case eventDispatcher::EventType::CHANNEL_MUTE:
		d.mute = !d.mute;
		break;

	case eventDispatcher::EventType::CHANNEL_TOGGLE_ARM:
		d.armed = !d.armed;
		break;

	case eventDispatcher::EventType::CHANNEL_SOLO:
		d.solo = !d.solo;
		m::mh::updateSoloCount();
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

void renderMasterOut_(const Data& d, AudioBuffer& out)
{
	d.buffer->audio.copyData(out);
#ifdef WITH_VST
	if (d.plugins.size() > 0)
		pluginHost::processStack(d.buffer->audio, d.plugins, nullptr);
#endif
	out.copyData(d.buffer->audio, d.volume);
}

/* -------------------------------------------------------------------------- */

void renderMasterIn_(const Data& d, AudioBuffer& in)
{
#ifdef WITH_VST
	if (d.plugins.size() > 0)
		pluginHost::processStack(in, d.plugins, nullptr);
#endif
}

/* -------------------------------------------------------------------------- */

void renderChannel_(const Data& d, AudioBuffer& out, AudioBuffer& in, bool audible)
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
		midiReceiver::render(d);
	else if (d.plugins.size() > 0)
		pluginHost::processStack(d.buffer->audio, d.plugins, nullptr);
#endif

	if (audible)
		out.addData(d.buffer->audio, d.volume * d.volume_i, calcPanning_(d.pan));
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

Data::Data(ChannelType type, ID id, ID columnId, State& state, Buffer& buffer)
: state(&state)
, buffer(&buffer)
, id(id)
, type(type)
, columnId(columnId)
, volume(G_DEFAULT_VOL)
, volume_i(G_DEFAULT_VOL)
, pan(G_DEFAULT_PAN)
, mute(false)
, solo(false)
, armed(false)
, key(0)
, readActions(true)
, hasActions(false)
, height(G_GUI_UNIT)
{
	switch (type)
	{

	case ChannelType::SAMPLE:
		samplePlayer.emplace();
		sampleReactor.emplace(id);
		audioReceiver.emplace();
		sampleActionRecorder.emplace();
		break;

	case ChannelType::PREVIEW:
		samplePlayer.emplace();
		sampleReactor.emplace(id);
		break;

	case ChannelType::MIDI:
		midiController.emplace();
#ifdef WITH_VST
		midiReceiver.emplace();
#endif
		midiSender.emplace();
		midiActionRecorder.emplace();
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

Data::Data(const patch::Channel& p, State& state, Buffer& buffer, float samplerateRatio)
: state(&state)
, buffer(&buffer)
, id(p.id)
, type(p.type)
, columnId(p.columnId)
, volume(p.volume)
, volume_i(G_DEFAULT_VOL)
, pan(p.pan)
, mute(p.mute)
, solo(p.solo)
, armed(p.armed)
, key(p.key)
, readActions(p.readActions)
, hasActions(p.hasActions)
, name(p.name)
, height(p.height)
#ifdef WITH_VST
, plugins(pluginManager::hydratePlugins(p.pluginIds))
#endif
, midiLearner(p)
{
	switch (type)
	{

	case ChannelType::SAMPLE:
		samplePlayer.emplace(p, samplerateRatio);
		sampleReactor.emplace(id);
		audioReceiver.emplace(p);
		sampleActionRecorder.emplace();
		break;

	case ChannelType::PREVIEW:
		samplePlayer.emplace(p, samplerateRatio);
		sampleReactor.emplace(id);
		break;

	case ChannelType::MIDI:
		midiController.emplace();
#ifdef WITH_VST
		midiReceiver.emplace();
#endif
		midiSender.emplace(p);
		midiActionRecorder.emplace();
		break;

	default:
		break;
	}
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
	return !isInternal() && mute;
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

/* -------------------------------------------------------------------------- */

void advance(const Data& d, const sequencer::EventBuffer& events)
{
	for (const sequencer::Event& e : events)
	{

		if (d.midiController)
			midiController::advance(d, e);
		if (d.samplePlayer)
			samplePlayer::advance(d, e);
		if (d.midiSender)
			midiSender::advance(d, e);
#ifdef WITH_VST
		if (d.midiReceiver)
			midiReceiver::advance(d, e);
#endif
	}
}

/* -------------------------------------------------------------------------- */

void react(Data& d, const eventDispatcher::EventBuffer& events, bool audible)
{
	for (const eventDispatcher::Event& e : events)
	{

		if (e.channelId > 0 && e.channelId != d.id)
			continue;

		react_(d, e);
		midiLighter::react(d, e, audible);

		if (d.midiController)
			midiController::react(d, e);
#ifdef WITH_VST
		if (d.midiReceiver)
			midiReceiver::react(d, e);
#endif
		if (d.midiSender)
			midiSender::react(d, e);
		if (d.samplePlayer)
			samplePlayer::react(d, e);
		if (d.midiActionRecorder)
			midiActionRecorder::react(d, e);
		if (d.sampleActionRecorder)
			sampleActionRecorder::react(d, e);
		if (d.sampleReactor)
			sampleReactor::react(d, e);
	}
}

/* -------------------------------------------------------------------------- */

void render(const Data& d, AudioBuffer* out, AudioBuffer* in, bool audible)
{
	if (d.id == mixer::MASTER_OUT_CHANNEL_ID)
		renderMasterOut_(d, *out);
	else if (d.id == mixer::MASTER_IN_CHANNEL_ID)
		renderMasterIn_(d, *in);
	else
		renderChannel_(d, *out, *in, audible);
}
} // namespace giada::m::channel