/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_CHANNEL_H
#define G_CHANNEL_H

#include "core/channels/channelShared.h"
#include "core/channels/midiActionRecorder.h"
#include "core/channels/midiChannel.h"
#include "core/channels/midiController.h"
#include "core/channels/midiLearn.h"
#include "core/channels/midiLightning.h"
#include "core/channels/midiReceiver.h"
#include "core/channels/midiSender.h"
#include "core/channels/sampleAdvancer.h"
#include "core/channels/sampleChannel.h"
#include "core/const.h"
#include "core/eventDispatcher.h"
#include "core/midiEvent.h"
#include "core/mixer.h"
#include "core/patch.h"
#include "core/queue.h"
#include "core/resampler.h"
#include "core/sequencer.h"
#include "deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include <optional>

namespace giada::m
{
class Plugin;
class Channel final
{
public:
	Channel(ChannelType t, ID id, ID columnId, int position, ChannelShared&);
	Channel(const Patch::Channel&, ChannelShared&, float samplerateRatio, Wave*, std::vector<Plugin*>);

	bool operator==(const Channel&);

	bool isPlaying() const;
	bool isInternal() const;
	bool isMuted() const;
	bool isSoloed() const;
	bool canInputRec() const;
	bool canActionRec() const;
	bool hasWave() const;

	/* isAudible
	True if this channel is currently audible: not muted or not included in a 
	solo session. */

	bool isAudible(bool mixerHasSolos) const;

	void setMute(bool);
	void setSolo(bool);

	/* loadWave
	Loads Wave and sets it up (name, markers, ...). Also updates Channel's shared
	state accordingly. Resets begin/end points shift if not specified. */

	void loadWave(Wave*, Frame begin = -1, Frame end = -1, Frame shift = -1);

	/* setWave
	Just sets the pointer to a Wave object. Used during de-serialization. The
	ratio is used to adjust begin/end points in case of patch vs. conf sample
	rate mismatch. If nullptr, set the wave to invalid. */

	void setWave(Wave* w, float samplerateRatio);

	/* kickIn
	Starts the player right away at frame 'f'. Used when launching a loop after
	being live recorded. */

	void kickIn(Frame f);

	ChannelShared*       shared;
	ID                   id;
	ChannelType          type;
	ID                   columnId;
	int                  position;
	float                volume;
	float                volume_i; // Internal volume used for velocity-drives-volume mode on Sample Channels
	float                pan;
	bool                 armed;
	int                  key;
	bool                 hasActions;
	std::string          name;
	Pixel                height;
	std::vector<Plugin*> plugins;

	MidiLearn     midiLearn;
	MidiLightning midiLightning;

	std::optional<MidiController>     midiController;
	std::optional<MidiReceiver>       midiReceiver;
	std::optional<MidiSender>         midiSender;
	std::optional<MidiActionRecorder> midiActionRecorder;
	std::optional<SampleChannel>      sampleChannel;
	std::optional<MidiChannel>        midiChannel;

private:
	bool m_mute;
	bool m_solo;
};
} // namespace giada::m

#endif
