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
#include "core/channels/midiChannel.h"
#include "core/model/model.h"
#include "core/pluginHost.h"
#include "core/kernelMidi.h"
#include "core/const.h"
#include "core/action.h"
#include "core/mixerHandler.h"
#include "midiChannelProc.h"


namespace giada {
namespace m {
namespace midiChannelProc
{
namespace
{
void onFirstBeat_(MidiChannel* ch)
{
	if (ch->playStatus == ChannelStatus::ENDING) {
		ch->playStatus = ChannelStatus::OFF;
		ch->sendMidiLstatus();
	}
	else
	if (ch->playStatus == ChannelStatus::WAIT) {
		ch->playStatus = ChannelStatus::PLAY;
		ch->sendMidiLstatus();
	}
}


/* -------------------------------------------------------------------------- */


void sendAllNotesOff_(MidiChannel* ch)
{
	MidiEvent e(MIDI_ALL_NOTES_OFF);
	ch->sendMidi(e, /*localFrame=*/0);

}
}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void parseEvents(MidiChannel* ch, mixer::FrameEvents fe)
{
	if (fe.onFirstBeat)
		onFirstBeat_(ch);
	if (fe.actions != nullptr)
		for (const Action& action : *fe.actions)
			if (action.channelId == ch->id && ch->isPlaying() && !ch->mute)
				ch->sendMidi(action.event, fe.frameLocal);
}


/* -------------------------------------------------------------------------- */


void process(MidiChannel* ch, AudioBuffer& out, const AudioBuffer& in, bool audible)
{
#ifdef WITH_VST

	ch->midiBuffer.clear();
	
	/* Fill the MIDI buffer vector with messages coming from the MIDI queue
	filled by the MIDI thread. This is for live events, e.g. piano keyboards,
	controllers, ... */

	MidiEvent e;
	while (ch->midiQueue.pop(e)) {
		juce::MidiMessage message = juce::MidiMessage(
			e.getStatus(), 
			e.getNote(), 
			e.getVelocity());
		ch->midiBuffer.addEvent(message, e.getDelta());
	}
	pluginHost::processStack(ch->buffer, ch->pluginIds, &ch->midiBuffer);
	
	/* Process the plugin stack first, then quit if the channel is muted/soloed. 
	This way there's no risk of cutting midi event pairs such as note-on and 
	note-off while triggering a mute/solo. */

	if (!audible)
		return;

	for (int i=0; i<out.countFrames(); i++)
		for (int j=0; j<out.countChannels(); j++)
			out[i][j] += ch->buffer[i][j] * ch->volume;	

#endif
}


/* -------------------------------------------------------------------------- */


void start(MidiChannel* ch)
{
	switch (ch->playStatus) {
		case ChannelStatus::PLAY:
			ch->playStatus = ChannelStatus::ENDING;
			ch->sendMidiLstatus();
			break;

		case ChannelStatus::ENDING:
		case ChannelStatus::WAIT:
			ch->playStatus = ChannelStatus::OFF;
			ch->sendMidiLstatus();
			break;

		case ChannelStatus::OFF:
			ch->playStatus = ChannelStatus::WAIT;
			ch->sendMidiLstatus();
			break;

		default: break;
	}	
}


/* -------------------------------------------------------------------------- */


void kill(MidiChannel* ch, int localFrame)
{
	if (ch->isPlaying())
		sendAllNotesOff_(ch);

	ch->playStatus = ChannelStatus::OFF;
	ch->sendMidiLstatus();
}


/* -------------------------------------------------------------------------- */


void rewindBySeq(MidiChannel* ch)
{
	sendAllNotesOff_(ch);
}


/* -------------------------------------------------------------------------- */


void setMute(MidiChannel* ch, bool v)
{
	ch->mute = v;
	if (v)
		sendAllNotesOff_(ch);

	// This is for processing playing_inaudible
	ch->sendMidiLstatus();	

	ch->sendMidiLmute();
}


/* -------------------------------------------------------------------------- */


void setSolo(MidiChannel* ch, bool v)
{
	ch->solo = v;
	mh::updateSoloCount();

	// This is for processing playing_inaudible
	// TODO
	//for (std::unique_ptr<Channel>& c : model::getLayout()->channels)
	//	c->sendMidiLstatus();

	ch->sendMidiLsolo();
}


/* -------------------------------------------------------------------------- */


void stopBySeq(MidiChannel* ch)
{	
	sendAllNotesOff_(ch);
	kill(ch, 0);
}
}}};
