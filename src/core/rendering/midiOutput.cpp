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

#include "src/core/rendering/midiOutput.h"
#include "src/core/actions/ActionManager.h"
#include "src/core/kernelMidi.h"
#include <cassert>
#include <utility>

namespace giada::m::rendering
{
namespace
{
std::function<void(ID)> onSend_ = nullptr;

/* -------------------------------------------------------------------------- */

template <typename KernelMidiI>
void sendMidiLightning_(ID channelId, uint32_t learnt, const MidiMap::Message& msg, MidiMapper<KernelMidiI>& midiMapper)
{
	assert(onSend_ != nullptr);

	midiMapper.sendMidiLightning(learnt, msg);
	onSend_(channelId);
}

/* -------------------------------------------------------------------------- */

void sendMidiToPlugins_(const model::Channel::Rendering& rendering, const MidiEvent& e, Frame localFrame)
{
	MidiEvent eWithDelta(e);
	eWithDelta.setDelta(localFrame);
	rendering.pushMidiEvent(eWithDelta);
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void registerOnSendMidiCb(std::function<void(ID)> f)
{
	onSend_ = f;
}

/* -------------------------------------------------------------------------- */

void sendMidiFromAction(
    const model::ChannelView& channelView,
    Scene                     scene,
    const model::Action&      action,
    Frame                     delta,
    KernelMidi&               kernelMidi)
{
	const model::Channel& ch = channelView.getChannel();

	if (action.channelId != ch.id || action.scene != scene)
		return;
	sendMidiToPlugins_(channelView.getRendering(), action.event, delta);
	if (ch.canSendMidi())
		sendMidiToOut(ch.id, action.event, ch.midiChannel->outputFilter, kernelMidi);
}

/* -------------------------------------------------------------------------- */

void sendMidiAllNotesOff(const model::ChannelView& channelView, KernelMidi& kernelMidi)
{
	const model::Channel& ch = channelView.getChannel();
	const MidiEvent       e  = MidiEvent::makeFromRaw(G_MIDI_ALL_NOTES_OFF, /*numBytes=*/3);

	sendMidiToPlugins_(channelView.getRendering(), e, 0);
	if (ch.canSendMidi())
		sendMidiToOut(ch.id, e, ch.midiChannel->outputFilter, kernelMidi);
}

/* -------------------------------------------------------------------------- */

void sendMidiEventToPlugins(model::Channel::Rendering& rendering, const MidiEvent& e)
{
	/* Now all messages are turned into Channel-0 messages. Giada doesn't care
	about holding MIDI channel information. Moreover, having all internal
	messages on channel 0 is way easier. Then send it to plug-ins. */

	MidiEvent flat(e);
	flat.setChannel(0);
	flat.setDelta(0);
	rendering.pushMidiEvent(std::move(flat));
}

/* -------------------------------------------------------------------------- */

void sendMidiToOut(ID channelId, MidiEvent e, int outputFilter, KernelMidi& kernelMidi)
{
	assert(onSend_ != nullptr);

	e.setChannel(outputFilter);
	kernelMidi.send(e);
	onSend_(channelId);
}

/* -------------------------------------------------------------------------- */

template <typename KernelMidiI>
void sendMidiLightningStatus(ID channelId, const MidiLightning& m, ChannelStatus status, bool audible, MidiMapper<KernelMidiI>& midiMapper)
{
	const MidiMap& midiMap   = midiMapper.currentMap;
	const uint32_t l_playing = m.playing.getValue();

	if (l_playing == 0x0)
		return;

	switch (status)
	{
	case ChannelStatus::OFF:
		sendMidiLightning_(channelId, l_playing, midiMap.stopped, midiMapper);
		break;

	case ChannelStatus::WAIT:
		sendMidiLightning_(channelId, l_playing, midiMap.waiting, midiMapper);
		break;

	case ChannelStatus::ENDING:
		sendMidiLightning_(channelId, l_playing, midiMap.stopping, midiMapper);
		break;

	case ChannelStatus::PLAY:
		sendMidiLightning_(channelId, l_playing, audible ? midiMap.playing : midiMap.playingInaudible, midiMapper);
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

template <typename KernelMidiI>
void sendMidiLightningMute(ID channelId, const MidiLightning& m, bool isMuted, MidiMapper<KernelMidiI>& midiMapper)
{
	const MidiMap& midiMap = midiMapper.currentMap;
	const uint32_t l_mute  = m.mute.getValue();

	if (l_mute != 0x0)
		sendMidiLightning_(channelId, l_mute, isMuted ? midiMap.muteOn : midiMap.muteOff, midiMapper);
}

/* -------------------------------------------------------------------------- */

template <typename KernelMidiI>
void sendMidiLightningSolo(ID channelId, const MidiLightning& m, bool isSoloed, MidiMapper<KernelMidiI>& midiMapper)
{
	const MidiMap& midiMap = midiMapper.currentMap;
	const uint32_t l_solo  = m.solo.getValue();

	if (l_solo != 0x0)
		sendMidiLightning_(channelId, l_solo, isSoloed ? midiMap.soloOn : midiMap.soloOff, midiMapper);
}

/* -------------------------------------------------------------------------- */

template void sendMidiLightningStatus(ID channelId, const MidiLightning&, ChannelStatus, bool audible, MidiMapper<KernelMidi>&);
template void sendMidiLightningMute(ID channelId, const MidiLightning&, bool isMuted, MidiMapper<KernelMidi>&);
template void sendMidiLightningSolo(ID channelId, const MidiLightning&, bool isSoloed, MidiMapper<KernelMidi>&);
#ifdef WITH_TESTS
template void sendMidiLightningStatus(ID channelId, const MidiLightning&, giada::ChannelStatus, bool audible, MidiMapper<KernelMidiMock>&);
template void sendMidiLightningMute(ID channelId, const MidiLightning&, bool isMuted, MidiMapper<KernelMidiMock>&);
template void sendMidiLightningSolo(ID channelId, const MidiLightning&, bool isSoloed, MidiMapper<KernelMidiMock>&);
#endif
} // namespace giada::m::rendering
