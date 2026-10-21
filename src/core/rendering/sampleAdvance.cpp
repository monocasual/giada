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

#include "src/core/rendering/sampleAdvance.h"
#include "src/core/channels/channel.h"
#include "src/core/channels/channelShared.h"
#include "src/core/rendering/sampleReactions.h"

namespace giada::m::rendering
{
namespace
{
RenderInfo rewind_(Frame localFrame)
{
	return {RenderInfo::Mode::REWIND, localFrame};
}

/* -------------------------------------------------------------------------- */

RenderInfo stop_(Frame localFrame)
{
	return {RenderInfo::Mode::STOP, localFrame};
}

/* -------------------------------------------------------------------------- */

RenderInfo play_(const model::ChannelView& channelView, Frame localFrame)
{
	channelView.getParameters().setPlayStatus(ChannelStatus::PLAY);
	return {RenderInfo::Mode::NORMAL, localFrame};
}

/* -------------------------------------------------------------------------- */

RenderInfo onFirstBeat_(const model::ChannelView& channelView, Frame localFrame, bool isLoop)
{
	const model::Channel::Parameters& params = channelView.getParameters();
	RenderInfo                        info   = {RenderInfo::Mode::NONE};

	switch (params.getPlayStatus())
	{
	case ChannelStatus::PLAY:
		if (isLoop)
			info = rewind_(localFrame);
		break;

	case ChannelStatus::WAIT:
		info = play_(channelView, localFrame);
		break;

	case ChannelStatus::ENDING:
		if (isLoop)
			info = stop_(localFrame);
		break;

	default:
		break;
	}

	switch (params.getRecStatus())
	{
	case ChannelStatus::WAIT:
		params.setRecStatus(ChannelStatus::PLAY);
		params.setReadActions(true);
		break;

	case ChannelStatus::ENDING:
		params.setRecStatus(ChannelStatus::OFF);
		params.setReadActions(false);
		break;

	default:
		break;
	}

	return info;
}

/* -------------------------------------------------------------------------- */

RenderInfo onBar_(const model::ChannelView& channelView, Frame localFrame, SamplePlayerMode mode)
{
	const model::Channel::Parameters& params     = channelView.getParameters();
	const ChannelStatus               playStatus = params.getPlayStatus();

	if (playStatus == ChannelStatus::PLAY &&
	    (mode == SamplePlayerMode::LOOP_REPEAT || mode == SamplePlayerMode::LOOP_ONCE_BAR))
		return rewind_(localFrame);

	if (playStatus == ChannelStatus::WAIT && mode == SamplePlayerMode::LOOP_ONCE_BAR)
		return play_(channelView, localFrame);

	return {RenderInfo::Mode::NONE};
}

/* -------------------------------------------------------------------------- */

RenderInfo onNoteOn_(const model::ChannelView& channelView, Frame localFrame, SamplePlayerMode mode, float velocity)
{
	const model::Channel::Parameters& params = channelView.getParameters();

	params.setVolumeInternal(velocity);

	switch (params.getPlayStatus())
	{
	case ChannelStatus::OFF:
		return play_(channelView, localFrame);

	case ChannelStatus::PLAY:
		return mode == SamplePlayerMode::SINGLE_RETRIG ? rewind_(localFrame) : stop_(localFrame);

	default:
		return {RenderInfo::Mode::NONE};
	}
}

/* -------------------------------------------------------------------------- */

RenderInfo parseAction_(
    const model::ChannelView& channelView,
    ID                        channelId,
    Scene                     scene,
    const Action&             a,
    Frame                     localFrame,
    SamplePlayerMode          mode)
{
	const model::Channel::Parameters& params = channelView.getParameters();

	if (a.channelId != channelId || a.scene != scene)
		return {RenderInfo::Mode::NONE};

	switch (a.event.getStatus())
	{
	case MidiEvent::CHANNEL_NOTE_ON:
		return onNoteOn_(channelView, localFrame, mode, a.event.getVelocityFloat());

	case MidiEvent::CHANNEL_NOTE_OFF:
	case MidiEvent::CHANNEL_NOTE_KILL:
		if (params.getPlayStatus() == ChannelStatus::PLAY)
			return stop_(localFrame);
		return {RenderInfo::Mode::NONE};

	default:
		return {RenderInfo::Mode::NONE};
	}
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

RenderInfo advanceSampleChannel(
    const model::ChannelView& channelView,
    const Sequencer::Event&   e)
{
	const model::Channel&  ch     = channelView.getChannel();
	const SamplePlayerMode mode   = ch.sampleChannel->mode;
	const bool             isLoop = ch.sampleChannel->isAnyLoopMode();

	switch (e.type)
	{
	case Sequencer::EventType::FIRST_BEAT:
		return onFirstBeat_(channelView, e.delta, isLoop);

	case Sequencer::EventType::BAR:
		return onBar_(channelView, e.delta, mode);

	case Sequencer::EventType::REWIND:
		return isLoop ? rewind_(e.delta) : RenderInfo{RenderInfo::Mode::NONE};

	case Sequencer::EventType::ACTIONS:
		if (!isLoop && channelView.getParameters().isReadingActions())
			return parseAction_(channelView, ch.id, e.scene, *e.action, e.delta, mode);
		return {RenderInfo::Mode::NONE};

	default:
		return {RenderInfo::Mode::NONE};
	}
}
} // namespace giada::m::rendering
