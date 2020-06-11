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
#include "core/channels/state.h"
#include "core/mixerHandler.h"
#include "core/pluginHost.h"
#include "channel.h"


namespace giada {
namespace m 
{
Channel::Channel(ChannelType type, ID id, ID columnId, Frame bufferSize)
: id            (id)
, state         (std::make_unique<ChannelState>(id, bufferSize))
, midiLighter   (state.get())
, m_type        (type)
, m_columnId    (columnId)
{
	switch (m_type) {

		case ChannelType::SAMPLE:
			samplePlayer.emplace(state.get());
			audioReceiver.emplace(state.get());
			sampleActionRecorder.emplace(state.get(), samplePlayer->state.get());	
			break;
		
		case ChannelType::PREVIEW:
			samplePlayer.emplace(state.get());
			break;
		
		case ChannelType::MIDI:
			midiController.emplace(state.get());
#ifdef WITH_VST
			midiReceiver.emplace(state.get());
#endif
			midiSender.emplace(state.get());
			midiActionRecorder.emplace(state.get());		
			break;	
		
		default: break;
	}
}


/* -------------------------------------------------------------------------- */


Channel::Channel(const Channel& o)
: id            (o.id)
#ifdef WITH_VST
, pluginIds     (o.pluginIds)
#endif
, state         (std::make_unique<ChannelState>(*o.state))
, midiLearner   (o.midiLearner)
, midiLighter   (o.midiLighter, state.get())
, m_type        (o.m_type)
, m_columnId    (o.m_columnId)
{
	switch (m_type) {

		case ChannelType::SAMPLE:
			samplePlayer.emplace(o.samplePlayer.value(), state.get());
			audioReceiver.emplace(o.audioReceiver.value(), state.get());
			sampleActionRecorder.emplace(o.sampleActionRecorder.value(), state.get(), samplePlayer->state.get());
			break;
		
		case ChannelType::PREVIEW:
			samplePlayer.emplace(o.samplePlayer.value(), state.get());
			break;
		
		case ChannelType::MIDI:
			midiController.emplace(o.midiController.value(), state.get());
#ifdef WITH_VST
			midiReceiver.emplace(o.midiReceiver.value(), state.get());
#endif
			midiSender.emplace(o.midiSender.value(), state.get());
			midiActionRecorder.emplace(o.midiActionRecorder.value(), state.get());
			break;

		default: break;
	}
}


/* -------------------------------------------------------------------------- */


Channel::Channel(const patch::Channel& p, Frame bufferSize)
: id            (p.id)
#ifdef WITH_VST
, pluginIds     (p.pluginIds)
#endif
, state         (std::make_unique<ChannelState>(p, bufferSize))
, midiLearner   (p)
, midiLighter   (p, state.get())
, m_type        (p.type)
, m_columnId    (p.columnId)
{
	switch (m_type) {

		case ChannelType::SAMPLE:
			samplePlayer.emplace(p, state.get());
			audioReceiver.emplace(p, state.get());
			sampleActionRecorder.emplace(state.get(), samplePlayer->state.get());
			break;
		
		case ChannelType::PREVIEW:
			samplePlayer.emplace(p, state.get());
			break;
		
		case ChannelType::MIDI:
			midiController.emplace(state.get());
#ifdef WITH_VST
			midiReceiver.emplace(p, state.get());
#endif
			midiSender.emplace(state.get());
			midiActionRecorder.emplace(state.get());	
			break;	
		
		default: break;
	}
}


/* -------------------------------------------------------------------------- */


void Channel::parse(const mixer::EventBuffer& events, bool audible) const
{
	for (const mixer::Event& e : events) {

		if (e.action.channelId > 0 && e.action.channelId != id)
			continue;

		parse(e);
		midiLighter.parse(e, audible);

		if (midiController)       midiController->parse(e);
#ifdef WITH_VST
		if (midiReceiver)         midiReceiver->parse(e);
#endif
		if (midiSender)           midiSender->parse(e);
		if (samplePlayer)         samplePlayer->parse(e);
		if (midiActionRecorder)   midiActionRecorder->parse(e);
  		if (sampleActionRecorder && samplePlayer && samplePlayer->hasWave()) 
			sampleActionRecorder->parse(e);
	}
}


/* -------------------------------------------------------------------------- */


void Channel::advance(Frame bufferSize) const
{
	/* TODO - this is used only to advance samplePlayer for its quantizer. Use
	this to render actions in the future. */

	if (samplePlayer) samplePlayer->advance(bufferSize);
}


/* -------------------------------------------------------------------------- */


void Channel::render(AudioBuffer* out, AudioBuffer* in, bool audible) const
{
	if (id == mixer::MASTER_OUT_CHANNEL_ID)
		renderMasterOut(*out);
	else
	if (id == mixer::MASTER_IN_CHANNEL_ID)
		renderMasterIn(*in);
	else 
		renderChannel(*out, *in, audible);
}


/* -------------------------------------------------------------------------- */


void Channel::parse(const mixer::Event& e) const
{
	switch (e.type) {

		case mixer::EventType::CHANNEL_VOLUME:
			state->volume.store(e.action.event.getVelocityFloat()); break;

		case mixer::EventType::CHANNEL_PAN:
			state->pan.store(e.action.event.getVelocityFloat()); break;

		case mixer::EventType::CHANNEL_MUTE:
			state->mute.store(!state->mute.load()); break;

		case mixer::EventType::CHANNEL_TOGGLE_ARM:
			state->armed.store(!state->armed.load()); break;
			
		case mixer::EventType::CHANNEL_SOLO:
			state->solo.store(!state->solo.load()); 
			m::mh::updateSoloCount(); 
			break;

		default: break;
	}
}


/* -------------------------------------------------------------------------- */


void Channel::renderMasterOut(AudioBuffer& out) const
{
	state->buffer.copyData(out);
#ifdef WITH_VST
	if (pluginIds.size() > 0)
		pluginHost::processStack(state->buffer, pluginIds, nullptr);
#endif
	out.copyData(state->buffer, state->volume.load());
}


/* -------------------------------------------------------------------------- */


void Channel::renderMasterIn(AudioBuffer& in) const
{
#ifdef WITH_VST
	if (pluginIds.size() > 0)
		pluginHost::processStack(in, pluginIds, nullptr);
#endif
}


/* -------------------------------------------------------------------------- */


void Channel::renderChannel(AudioBuffer& out, AudioBuffer& in, bool audible) const
{
	state->buffer.clear();

	if (samplePlayer)  samplePlayer->render(out);
	if (audioReceiver) audioReceiver->render(in);

	/* If MidiReceiver exists, let it process the plug-in stack, as it can 
	contain plug-ins that take MIDI events (i.e. synths). Otherwise process the
	plug-in stack internally with no MIDI events. */

#ifdef WITH_VST
	if (midiReceiver)  
		midiReceiver->render(pluginIds); 
	else 
	if (pluginIds.size() > 0)
		pluginHost::processStack(state->buffer, pluginIds, nullptr);
#endif

	if (audible)
	    out.addData(state->buffer, state->volume.load() * state->volume_i, calcPanning());
}


/* -------------------------------------------------------------------------- */


AudioBuffer::Pan Channel::calcPanning() const
{
	/* TODO - precompute the AudioBuffer::Pan when pan value changes instead of
	building it on the fly. */
	
	float pan = state->pan.load();

	/* Center pan (0.5f)? Pass-through. */

	if (pan == 0.5f) return { 1.0f, 1.0f };
	return { 1.0f - pan, pan };
}


/* -------------------------------------------------------------------------- */


ID Channel::getColumnId() const { return m_columnId; };
ChannelType Channel::getType() const { return m_type; };


/* -------------------------------------------------------------------------- */


bool Channel::isInternal() const
{
	return m_type == ChannelType::MASTER || m_type == ChannelType::PREVIEW;
}


bool Channel::isMuted() const
{
	/* Internals can't be muted. */
	return !isInternal() && state->mute.load() == true;
}


bool Channel::canInputRec() const
{
	return samplePlayer && !samplePlayer->hasWave() && state->armed.load() == true;
}
}} // giada::m::
