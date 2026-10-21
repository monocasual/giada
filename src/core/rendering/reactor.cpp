/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/core/rendering/reactor.h"
#include "src/core/mixer.h"
#include "src/core/model_DEPR_/model.h"
#include "src/core/rendering/midiOutput.h"
#include "src/core/rendering/midiReactions.h"
#include "src/core/rendering/sampleReactions.h"

namespace giada::m::rendering
{
Reactor::Reactor(model::Model& model, model_DEPR_::Model& model_DEPR_, MidiMapper<KernelMidi>& m, ActionManager& a, KernelMidi& km)
: m_model(model)
, m_model_DEPR_(model_DEPR_)
, m_kernelMidi(km)
, m_actionManager(a)
, m_midiMapper(m)
{
}

/* -------------------------------------------------------------------------- */

void Reactor::keyPress(ID channelId, Scene scene, float velocity, bool canRecordActions,
    bool canQuantize, Tick currentTickQuantized)
{
	m_model.writeDocumentAndAssets(model::SwapType::SOFT, [=, this](model::Document& doc, model::Assets& assets)
	{
		model::Channel& ch                = doc.tracks.getChannel(channelId);
		const auto      channelParameters = assets.channelsParameters.find(channelId);
		const auto      channelRendering  = assets.channelsRendering.find(channelId);

		if (ch.type == ChannelType::MIDI)
		{
			// TODO
			// playMidiChannel(ch.shared->playStatus);
		}
		else if (ch.type == ChannelType::SAMPLE && ch.hasWave(scene))
		{
			const bool             isAnyLoopMode = ch.sampleChannel->isAnyLoopMode();
			const bool             velocityAsVol = ch.sampleChannel->velocityAsVol;
			const SamplePlayerMode mode          = ch.sampleChannel->mode;

			// TODO
			// if (canRecordActions && !isAnyLoopMode)
			//	recordSampleKeyPress(channelId, scene, *ch.shared, currentTickQuantized, mode, m_actionManager);

			pressSampleChannel(channelId, *channelParameters, *channelRendering,
			    mode, velocity, canQuantize, isAnyLoopMode, velocityAsVol);
		}
		else if (ch.type == ChannelType::PREVIEW)
		{
			pressSampleChannel(channelId, *channelParameters, *channelRendering,
			    SamplePlayerMode::SINGLE_BASIC_PAUSE, /*velocity=*/0.0f,
			    /*canQuantize=*/false, /*isAnyLoopMode=*/false,
			    /*velocityAsVol=*/false);
		}
		else if (ch.type == ChannelType::GROUP)
		{
			// TODO recursion not allowed
			// for (const model::Channel& child : doc.tracks.getByChannel(ch.id).getChannels().getAll())
			//	if (child.type != ChannelType::GROUP)
			//		keyPress(child.id, scene, velocity, canRecordActions, canQuantize, currentTickQuantized);
		}
	});
}

/* -------------------------------------------------------------------------- */

void Reactor::keyRelease(ID channelId, Scene scene, bool canRecordActions, Tick currentTickQuantized)
{
	m_model.writeDocumentAndAssets(model::SwapType::SOFT, [=](model::Document& doc, model::Assets& assets)
	{
		model::Channel& ch                = doc.tracks.getChannel(channelId);
		const auto      channelParameters = assets.channelsParameters.find(channelId);
		const auto      channelRendering  = assets.channelsRendering.find(channelId);

		if (ch.type == ChannelType::MIDI)
			return;

		if (ch.type == ChannelType::SAMPLE && ch.hasWave(scene))
		{
			const SamplePlayerMode mode = ch.sampleChannel->mode;

			if (canRecordActions && mode == SamplePlayerMode::SINGLE_PRESS)
			{
				/* Record a stop event only if channel is SINGLE_PRESS. For any other
			mode the key release event is meaningless. */

				recordSampleKeyRelease(channelId, scene, currentTickQuantized, m_actionManager);
			}

			releaseSampleChannel(*channelParameters, *channelRendering, mode);
		}
		else if (ch.type == ChannelType::PREVIEW)
		{
			releaseSampleChannel(*channelParameters, *channelRendering, SamplePlayerMode::SINGLE_BASIC_PAUSE);
		}
		else if (ch.type == ChannelType::GROUP)
		{
			// TODO recursion not allowed
			// for (const Channel& child : m_model_DEPR_.get().tracks.getByChannel(ch.id).getChannels().getAll())
			//	if (child.type != ChannelType::GROUP)
			//		keyRelease(child.id, scene, canRecordActions, currentTickQuantized);
		}
	});
}

/* -------------------------------------------------------------------------- */

void Reactor::keyKill(ID channelId, Scene scene, bool canRecordActions, Tick currentTickQuantized)
{
	m_model.writeDocumentAndAssets(model::SwapType::SOFT, [=](model::Document& doc, model::Assets& assets)
	{
		model::Channel& ch                = doc.tracks.getChannel(channelId);
		const auto      channelParameters = assets.channelsParameters.find(channelId);
		const auto      channelRendering  = assets.channelsRendering.find(channelId);

		if (ch.type == ChannelType::MIDI)
		{
			if (channelParameters->isPlaying())
			{
				// TODO
				// stopMidiChannel(ch.shared->playStatus);
				// sendMidiAllNotesOff(ch, m_kernelMidi);
			}
		}
		else if (ch.type == ChannelType::SAMPLE)
		{
			const SamplePlayerMode mode = ch.sampleChannel->mode;

			if (ch.hasWave(scene) && canRecordActions && mode == SamplePlayerMode::SINGLE_PRESS)
			{
				/* Record a stop event only if channel is SINGLE_PRESS. For any other
				mode the key release event is meaningless. */

				recordSampleKeyKill(channelId, scene, currentTickQuantized, m_actionManager);
			}

			killSampleChannel(*channelParameters, *channelRendering, mode);
		}
		else if (ch.type == ChannelType::GROUP)
		{
			// TODO recursion not allowed
			// for (const Channel& child : m_model_DEPR_.get().tracks.getByChannel(ch.id).getChannels().getAll())
			//	if (child.type != ChannelType::GROUP)
			//		keyKill(child.id, scene, canRecordActions, currentTickQuantized);
		}
	});
}

/* -------------------------------------------------------------------------- */

void Reactor::processMidiEvent(ID channelId, Scene scene, const MidiEvent& e,
    bool canRecordActions, Tick currentTickQuantized)
{
	Channel& ch = m_model_DEPR_.get().tracks.getChannel(channelId);

	assert(ch.type == ChannelType::MIDI);

	if (canRecordActions)
	{
		recordMidiAction(channelId, scene, e, currentTickQuantized, m_actionManager);
		m_model_DEPR_.swap(model_DEPR_::SwapType::HARD);
	}
	sendMidiEventToPlugins(ch.shared->midiQueue, e);
	if (ch.canSendMidi())
		sendMidiToOut(channelId, e, ch.midiChannel->outputFilter, m_kernelMidi); // Also send it back to the outside world
}

/* -------------------------------------------------------------------------- */

void Reactor::toggleReadActions(ID channelId, bool seqIsRunning)
{
	Channel& ch = m_model_DEPR_.get().tracks.getChannel(channelId);
	toggleSampleReadActions(*ch.shared, m_model_DEPR_.get().behaviors.treatRecsAsLoops, seqIsRunning);
}

/* -------------------------------------------------------------------------- */

void Reactor::killReadActions(ID channelId)
{
	/* Killing Read Actions, i.e. shift + click on 'R' button is meaningful
	only when the treatRecsAsLoops flag is true. */

	if (!m_model_DEPR_.get().behaviors.treatRecsAsLoops)
		return;
	Channel& ch = m_model_DEPR_.get().tracks.getChannel(channelId);
	killSampleReadActions(*ch.shared);
}

/* -------------------------------------------------------------------------- */

void Reactor::toggleMute(ID channelId)
{
	m_model.writeDocument(model::SwapType::SOFT, [channelId](model::Document& doc)
	{
		model::Channel& ch      = doc.tracks.getChannel(channelId);
		model::Track&   track   = doc.tracks.getByChannel(ch.id);
		const bool      newMute = !ch.isMuted();

		ch.setMute(newMute);

		if (ch.type == ChannelType::GROUP)
		{
			/* Toggling mute on a group will toggle mute on all its children too. */
			for (model::Channel& child : track.getChannels().getAll())
				if (child.type != ChannelType::GROUP)
					child.setMute(newMute);
		}
	});

	// TODO
	// if (ch.midiLightning.enabled)
	//	sendMidiLightningMute(ch.id, ch.midiLightning, newMute, m_midiMapper);
}

/* -------------------------------------------------------------------------- */

void Reactor::toggleSolo(ID channelId)
{
	m_model.writeDocument(model::SwapType::SOFT, [channelId](model::Document& doc)
	{
		model::Channel& ch      = doc.tracks.getChannel(channelId);
		model::Track&   track   = doc.tracks.getByChannel(ch.id);
		const bool      newSolo = !ch.isSoloed();

		ch.setSolo(newSolo);

		if (ch.type == ChannelType::GROUP)
		{
			/* Toggling a solo on a group will toggle solo on all its children too. */
			for (model::Channel& child : track.getChannels().getAll())
				if (child.type != ChannelType::GROUP)
					child.setSolo(newSolo);
		}
		else
		{
			const auto noChildrenSoloed = [](const model::Track& track)
			{
				for (const model::Channel& child : track.getChannels().getAll())
					if (child.type != ChannelType::GROUP && child.isSoloed())
						return false;
				return true;
			};
			/* Turn group solo on if one of the children has been soloed. Turn group solo off
			if no children are soloed instead. */
			model::Channel& group = track.getGroupChannel();
			if (newSolo)
				group.setSolo(true);
			else if (noChildrenSoloed(track))
				group.setSolo(false);
		}
	});

	// TODO
	// if (ch.midiLightning.enabled)
	//	sendMidiLightningSolo(ch.id, ch.midiLightning, newSolo, m_midiMapper);
}

/* -------------------------------------------------------------------------- */

void Reactor::stopAll()
{
	m_model.writeDocumentAndAssets(model::SwapType::SOFT, [](model::Document& doc, model::Assets& assets)
	{
		for (const model::Track& track : doc.tracks.getAll())
		{
			for (const model::Channel& ch : track.getChannels().getAll())
			{
				const auto channelParameters = assets.channelsParameters.find(ch.id);
				const auto channelRendering  = assets.channelsRendering.find(ch.id);

				if (ch.type == ChannelType::MIDI)
				{
					if (!channelParameters->isPlaying())
						continue;

					// TODO
					// stopMidiChannel(ch.shared->playStatus);
					// sendMidiAllNotesOff(ch, m_kernelMidi);
				}
				else if (ch.type == ChannelType::SAMPLE)
				{
					stopSampleChannelBySeq(*channelParameters, *channelRendering,
					    doc.behaviors.chansStopOnSeqHalt, ch.sampleChannel->isAnyLoopMode());
				}
			}
		}
	});
}

/* -------------------------------------------------------------------------- */

void Reactor::rewindAll()
{
	/*
	TODO
	for (const model_DEPR_::Track& track : m_model.get().tracks.getAll())
	    for (const Channel& ch : track.getChannels().getAll())
	        if (ch.type == ChannelType::MIDI)
	            rewindMidiChannel(ch.shared->playStatus);
	m_model.swap(model_DEPR_::SwapType::SOFT);
	*/
}

/* -------------------------------------------------------------------------- */

void Reactor::killEmptySampleChannels(Scene scene)
{
	m_model.writeDocumentAndAssets(model::SwapType::SOFT, [scene](model::Document& doc, model::Assets& assets)
	{
		doc.tracks.forEachChannel([&assets, scene](model::Channel& ch)
		{
			const auto channelParameters = assets.channelsParameters.find(ch.id);
			const auto channelRendering  = assets.channelsRendering.find(ch.id);

			if (!ch.isInternal() && ch.type == ChannelType::SAMPLE && !ch.hasWave(scene) && channelParameters->isPlaying())
				killSampleChannel(*channelParameters, *channelRendering, ch.sampleChannel->mode);

			return true;
		});
	});
}
} // namespace giada::m::rendering
