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

#include "core/rendering/reactor.h"
#include "core/mixer.h"
#include "core/model/model.h"
#include "core/rendering/midiOutput.h"
#include "core/rendering/midiReactions.h"
#include "core/rendering/sampleReactions.h"

namespace giada::m::rendering
{
Reactor::Reactor(model::Model& model, MidiMapper<KernelMidi>& m, ActionRecorder& a, KernelMidi& km)
: m_model(model)
, m_kernelMidi(km)
, m_actionRecorder(a)
, m_midiMapper(m)
{
}

/* -------------------------------------------------------------------------- */

void Reactor::keyPress(ID channelId, float velocity, bool canRecordActions, bool canQuantize, Frame currentFrameQuantized)
{
	Channel& ch = m_model.get().channels.get(channelId);

	if (ch.type == ChannelType::MIDI)
	{
		playMidiChannel(ch.shared->playStatus);
	}
	else if (ch.type == ChannelType::SAMPLE && ch.hasWave())
	{
		const bool             isAnyLoopMode = ch.sampleChannel->isAnyLoopMode();
		const bool             velocityAsVol = ch.sampleChannel->velocityAsVol;
		const SamplePlayerMode mode          = ch.sampleChannel->mode;

		if (canRecordActions && !isAnyLoopMode)
		{
			recordSampleKeyPress(channelId, *ch.shared, currentFrameQuantized, mode, m_actionRecorder);
			ch.hasActions = true;
		}

		pressSampleChannel(channelId, *ch.shared, mode, velocity, canQuantize, isAnyLoopMode, velocityAsVol);
	}
	else if (ch.type == ChannelType::PREVIEW)
	{
		pressSampleChannel(channelId, *ch.shared, SamplePlayerMode::SINGLE_BASIC_PAUSE,
		    /*velocity=*/0.0f, /*canQuantize=*/false, /*isAnyLoopMode=*/false,
		    /*velocityAsVol=*/false);
	}

	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void Reactor::keyRelease(ID channelId, bool canRecordActions, Frame currentFrameQuantized)
{
	Channel& ch = m_model.get().channels.get(channelId);

	if (ch.type == ChannelType::MIDI)
		return;

	if (ch.type == ChannelType::SAMPLE && ch.hasWave())
	{
		const SamplePlayerMode mode = ch.sampleChannel->mode;

		if (canRecordActions && mode == SamplePlayerMode::SINGLE_PRESS)
		{
			/* Record a stop event only if channel is SINGLE_PRESS. For any other 
		mode the key release event is meaningless. */

			recordSampleKeyRelease(channelId, currentFrameQuantized, m_actionRecorder);
			ch.hasActions = true;
		}

		releaseSampleChannel(*ch.shared, mode);
	}
	else if (ch.type == ChannelType::PREVIEW)
	{
		releaseSampleChannel(*ch.shared, SamplePlayerMode::SINGLE_BASIC_PAUSE);
	}

	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void Reactor::keyKill(ID channelId, bool canRecordActions, Frame currentFrameQuantized)
{
	Channel& ch = m_model.get().channels.get(channelId);

	if (ch.type == ChannelType::MIDI)
	{
		if (ch.isPlaying())
		{
			stopMidiChannel(ch.shared->playStatus);
			sendMidiAllNotesOff(ch, m_kernelMidi);
		}
	}
	else if (ch.type == ChannelType::SAMPLE)
	{
		const SamplePlayerMode mode = ch.sampleChannel->mode;

		if (ch.hasWave() && canRecordActions && mode == SamplePlayerMode::SINGLE_PRESS)
		{
			/* Record a stop event only if channel is SINGLE_PRESS. For any other 
			mode the key release event is meaningless. */

			recordSampleKeyKill(channelId, currentFrameQuantized, m_actionRecorder);
			ch.hasActions = true;
		}

		killSampleChannel(*ch.shared, mode);
	}

	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void Reactor::processMidiEvent(ID channelId, const MidiEvent& e, bool canRecordActions, Frame currentFrameQuantized)
{
	Channel& ch = m_model.get().channels.get(channelId);

	assert(ch.type == ChannelType::MIDI);

	if (canRecordActions)
	{
		recordMidiAction(channelId, e, currentFrameQuantized, m_actionRecorder);
		ch.hasActions = true;
		m_model.swap(model::SwapType::HARD);
	}
	sendMidiEventToPlugins(ch.shared->midiQueue, e);
}

/* -------------------------------------------------------------------------- */

void Reactor::toggleReadActions(ID channelId, bool seqIsRunning)
{
	Channel& ch = m_model.get().channels.get(channelId);
	if (!ch.hasActions)
		return;
	toggleSampleReadActions(*ch.shared, m_model.get().behaviors.treatRecsAsLoops, seqIsRunning);
}

/* -------------------------------------------------------------------------- */

void Reactor::killReadActions(ID channelId)
{
	/* Killing Read Actions, i.e. shift + click on 'R' button is meaningful 
	only when the treatRecsAsLoops flag is true. */

	if (!m_model.get().behaviors.treatRecsAsLoops)
		return;
	Channel& ch = m_model.get().channels.get(channelId);
	killSampleReadActions(*ch.shared);
}

/* -------------------------------------------------------------------------- */

void Reactor::toggleMute(ID channelId)
{
	Channel&   ch      = m_model.get().channels.get(channelId);
	const bool newMute = !ch.isMuted();

	ch.setMute(newMute);

	m_model.swap(model::SwapType::SOFT);

	if (ch.midiLightning.enabled)
		sendMidiLightningMute(ch.id, ch.midiLightning, newMute, m_midiMapper);
}

/* -------------------------------------------------------------------------- */

void Reactor::toggleSolo(ID channelId)
{
	Channel&   ch      = m_model.get().channels.get(channelId);
	const bool newSolo = !ch.isSoloed();

	ch.setSolo(newSolo);

	m_model.swap(model::SwapType::SOFT);

	if (ch.midiLightning.enabled)
		sendMidiLightningSolo(ch.id, ch.midiLightning, newSolo, m_midiMapper);
}

/* -------------------------------------------------------------------------- */

void Reactor::stopAll()
{
	for (Channel& ch : m_model.get().channels.getAll())
	{
		if (ch.type == ChannelType::MIDI)
		{
			if (!ch.isPlaying())
				continue;
			stopMidiChannel(ch.shared->playStatus);
			sendMidiAllNotesOff(ch, m_kernelMidi);
		}
		else if (ch.type == ChannelType::SAMPLE)
		{
			stopSampleChannelBySeq(*ch.shared, m_model.get().behaviors.chansStopOnSeqHalt, ch.sampleChannel->isAnyLoopMode());
		}
	}
	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void Reactor::rewindAll()
{
	for (Channel& ch : m_model.get().channels.getAll())
		if (ch.type == ChannelType::MIDI)
			rewindMidiChannel(ch.shared->playStatus);
	m_model.swap(model::SwapType::SOFT);
}
} // namespace giada::m::rendering
