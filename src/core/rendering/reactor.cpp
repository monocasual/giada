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
#include "src/core/model/model.h"
#include "src/core/rendering/midiOutput.h"
#include "src/core/rendering/midiReactions.h"
#include "src/core/rendering/sampleReactions.h"
#include <cassert>

namespace giada::m::rendering
{
namespace
{
void keyPress_(
    model::Document& doc,
    model::Assets&   assets,
    ID               channelId,
    Scene            scene,
    float            velocity,
    bool             canRecordActions,
    bool             canQuantize,
    Tick             currentTickQuantized)
{
	model::Channel&    ch          = doc.tracks.getChannel(channelId);
	model::ChannelView channelView = model::makeChannelView(ch, assets);

	if (ch.type == ChannelType::MIDI)
	{
		playMidiChannel(channelView);
	}
	else if (ch.type == ChannelType::SAMPLE && ch.hasWave(scene))
	{
		const bool             isAnyLoopMode = ch.sampleChannel->isAnyLoopMode();
		const bool             velocityAsVol = ch.sampleChannel->velocityAsVol;
		const SamplePlayerMode mode          = ch.sampleChannel->mode;

		// TODO
		// if (canRecordActions && !isAnyLoopMode)
		//	recordSampleKeyPress(channelId, scene, *ch.shared, currentTickQuantized, mode, m_actionManager);

		pressSampleChannel(channelView, mode, velocity, canQuantize, isAnyLoopMode, velocityAsVol);
	}
	else if (ch.type == ChannelType::PREVIEW)
	{
		pressSampleChannel(channelView,
		    SamplePlayerMode::SINGLE_BASIC_PAUSE, /*velocity=*/0.0f,
		    /*canQuantize=*/false, /*isAnyLoopMode=*/false,
		    /*velocityAsVol=*/false);
	}
	else if (ch.type == ChannelType::GROUP)
	{
		for (const model::Channel& child : doc.tracks.getByChannel(ch.id).getChannels().getAll())
			if (child.type != ChannelType::GROUP)
				keyPress_(doc, assets, child.id, scene, velocity, canRecordActions, canQuantize, currentTickQuantized);
	}
}

/* -------------------------------------------------------------------------- */

void keyRelease_(
    model::Document& doc,
    model::Assets&   assets,
    ID               channelId,
    Scene            scene,
    bool             canRecordActions,
    Tick             currentTickQuantized,
    ActionManager&   actionManager)
{
	model::Channel&    ch          = doc.tracks.getChannel(channelId);
	model::ChannelView channelView = model::makeChannelView(ch, assets);

	if (ch.type == ChannelType::MIDI)
		return;

	if (ch.type == ChannelType::SAMPLE && ch.hasWave(scene))
	{
		const SamplePlayerMode mode = ch.sampleChannel->mode;

		if (canRecordActions && mode == SamplePlayerMode::SINGLE_PRESS)
		{
			/* Record a stop event only if channel is SINGLE_PRESS. For any other
			mode the key release event is meaningless. */

			recordSampleKeyRelease(channelId, scene, currentTickQuantized, actionManager);
		}

		releaseSampleChannel(channelView, mode);
	}
	else if (ch.type == ChannelType::PREVIEW)
	{
		releaseSampleChannel(channelView, SamplePlayerMode::SINGLE_BASIC_PAUSE);
	}
	else if (ch.type == ChannelType::GROUP)
	{
		for (const model::Channel& child : doc.tracks.getByChannel(ch.id).getChannels().getAll())
			if (child.type != ChannelType::GROUP)
				keyRelease_(doc, assets, child.id, scene, canRecordActions, currentTickQuantized, actionManager);
	}
}

/* -------------------------------------------------------------------------- */

void keyKill_(
    model::Document& doc,
    model::Assets&   assets,
    ID               channelId,
    Scene            scene,
    bool             canRecordActions,
    Tick             currentTickQuantized,
    ActionManager&   actionManager,
    KernelMidi&      kernelMidi)
{
	model::Channel&    ch          = doc.tracks.getChannel(channelId);
	model::ChannelView channelView = model::makeChannelView(ch, assets);

	if (ch.type == ChannelType::MIDI)
	{
		if (channelView.getParameters().isPlaying())
		{
			stopMidiChannel(channelView);
			sendMidiAllNotesOff(channelView, kernelMidi);
		}
	}
	else if (ch.type == ChannelType::SAMPLE)
	{
		const SamplePlayerMode mode = ch.sampleChannel->mode;

		if (ch.hasWave(scene) && canRecordActions && mode == SamplePlayerMode::SINGLE_PRESS)
		{
			/* Record a stop event only if channel is SINGLE_PRESS. For any other
			mode the key release event is meaningless. */

			recordSampleKeyKill(channelId, scene, currentTickQuantized, actionManager);
		}

		killSampleChannel(channelView, mode);
	}
	else if (ch.type == ChannelType::GROUP)
	{
		for (const model::Channel& child : doc.tracks.getByChannel(ch.id).getChannels().getAll())
			if (child.type != ChannelType::GROUP)
				keyKill_(doc, assets, child.id, scene, canRecordActions, currentTickQuantized, actionManager, kernelMidi);
	}
}
} // namespace

/* -------------------------------------------------------------------------- */

Reactor::Reactor(model::Model& model, MidiMapper<KernelMidi>& m, ActionManager& a, KernelMidi& km)
: m_model(model)
, m_kernelMidi(km)
, m_actionManager(a)
, m_midiMapper(m)
{
}

/* -------------------------------------------------------------------------- */

void Reactor::keyPress(ID channelId, Scene scene, float velocity, bool canRecordActions,
    bool canQuantize, Tick currentTickQuantized)
{
	m_model.write(model::SwapType::SOFT, [=](model::ModelEdit& edit)
	{
		keyPress_(edit.getDocument(), edit.getAssets(), channelId, scene, velocity, canRecordActions, canQuantize, currentTickQuantized);
	});
}

/* -------------------------------------------------------------------------- */

void Reactor::keyRelease(ID channelId, Scene scene, bool canRecordActions, Tick currentTickQuantized)
{
	m_model.write(model::SwapType::SOFT, [=, this](model::ModelEdit& edit)
	{
		keyRelease_(edit.getDocument(), edit.getAssets(), channelId, scene, canRecordActions, currentTickQuantized, m_actionManager);
	});
}

/* -------------------------------------------------------------------------- */

void Reactor::keyKill(ID channelId, Scene scene, bool canRecordActions, Tick currentTickQuantized)
{
	m_model.write(model::SwapType::SOFT, [=, this](model::ModelEdit& edit)
	{
		keyKill_(edit.getDocument(), edit.getAssets(), channelId, scene, canRecordActions, currentTickQuantized, m_actionManager, m_kernelMidi);
	});
}

/* -------------------------------------------------------------------------- */

void Reactor::processMidiEvent(ID channelId, Scene scene, const MidiEvent& e,
    bool canRecordActions, Tick currentTickQuantized)
{
	const model::ModelView modelView = m_model.read();
	const model::Channel&  ch        = modelView.getDocument().tracks.getChannel(channelId);
	const auto             rendering = modelView.getAssets().channelsRendering.get(channelId);

	assert(ch.type == ChannelType::MIDI);

	if (canRecordActions)
		recordMidiAction(channelId, scene, e, currentTickQuantized, m_actionManager);

	sendMidiEventToPlugins(*rendering, e);
	if (ch.canSendMidi())
		sendMidiToOut(channelId, e, ch.midiChannel->outputFilter, m_kernelMidi); // Also send it back to the outside world
}

/* -------------------------------------------------------------------------- */

void Reactor::toggleReadActions(ID channelId, bool seqIsRunning)
{
	const auto  view             = m_model.read();
	const auto& params           = view.getAssets().channelsParameters.get(channelId);
	const bool  treatRecsAsLoops = view.getDocument().behaviors.treatRecsAsLoops;

	toggleSampleReadActions(*params, treatRecsAsLoops, seqIsRunning);
}

/* -------------------------------------------------------------------------- */

void Reactor::killReadActions(ID channelId)
{
	/* Killing Read Actions, i.e. shift + click on 'R' button is meaningful
	only when the treatRecsAsLoops flag is true. */

	const auto view = m_model.read();
	if (!view.getDocument().behaviors.treatRecsAsLoops)
		return;
	const auto& params = view.getAssets().channelsParameters.get(channelId);
	killSampleReadActions(*params);
}

/* -------------------------------------------------------------------------- */

void Reactor::toggleMute(ID channelId)
{
	m_model.write(model::SwapType::SOFT, [channelId](model::DocumentEdit& edit)
	{
		model::Document& doc     = edit.get();
		model::Channel&  ch      = doc.tracks.getChannel(channelId);
		model::Track&    track   = doc.tracks.getByChannel(ch.id);
		const bool       newMute = !ch.isMuted();

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
	m_model.write(model::SwapType::SOFT, [channelId](model::DocumentEdit& edit)
	{
		model::Document& doc     = edit.get();
		model::Channel&  ch      = doc.tracks.getChannel(channelId);
		model::Track&    track   = doc.tracks.getByChannel(ch.id);
		const bool       newSolo = !ch.isSoloed();

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
	m_model.write(model::SwapType::SOFT, [this](model::ModelEdit& edit)
	{
		model::Document& doc    = edit.getDocument();
		model::Assets&   assets = edit.getAssets();
		doc.tracks.forEachChannel([this, &doc, &assets](model::Channel& ch)
		{
			const model::ChannelView channelView = model::makeChannelView(ch, assets);

			if (ch.type == ChannelType::MIDI)
			{
				if (channelView.getParameters().isPlaying())
				{
					stopMidiChannel(channelView);
					sendMidiAllNotesOff(channelView, m_kernelMidi);
				}
			}
			else if (ch.type == ChannelType::SAMPLE)
			{
				stopSampleChannelBySeq(channelView,
				    doc.behaviors.chansStopOnSeqHalt, ch.sampleChannel->isAnyLoopMode());
			}
			return true;
		});
	});
}

/* -------------------------------------------------------------------------- */

void Reactor::rewindAll()
{
	m_model.write(model::SwapType::SOFT, [](model::ModelEdit& edit)
	{
		model::Document& doc    = edit.getDocument();
		model::Assets&   assets = edit.getAssets();
		for (const model::Track& track : doc.tracks.getAll())
			for (const model::Channel& ch : track.getChannels().getAll())
				if (ch.type == ChannelType::MIDI)
					rewindMidiChannel(model::makeChannelView(ch, assets));
	});
}

/* -------------------------------------------------------------------------- */

void Reactor::killEmptySampleChannels(Scene scene)
{
	m_model.write(model::SwapType::SOFT, [scene](model::ModelEdit& edit)
	{
		model::Document& doc    = edit.getDocument();
		model::Assets&   assets = edit.getAssets();
		doc.tracks.forEachChannel([&assets, scene](model::Channel& ch)
		{
			const model::ChannelView channelView = model::makeChannelView(ch, assets);

			if (!ch.isInternal() && ch.type == ChannelType::SAMPLE && !ch.hasWave(scene) && channelView.getParameters().isPlaying())
				killSampleChannel(channelView, ch.sampleChannel->mode);

			return true;
		});
	});
}
} // namespace giada::m::rendering
