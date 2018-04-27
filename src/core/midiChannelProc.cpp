#include "midiChannel.h"
#include "pluginHost.h"
#include "kernelMidi.h"
#include "const.h"
#include "midiChannelProc.h"


namespace giada {
namespace m {
namespace midiChannelProc
{
namespace
{
void onFirstBeat_(MidiChannel* ch)
{
	if (ch->status == ChannelStatus::ENDING) {
		ch->status = ChannelStatus::OFF;
		ch->sendMidiLstatus();
	}
	else
	if (ch->status == ChannelStatus::WAIT) {
		ch->status = ChannelStatus::PLAY;
		ch->sendMidiLstatus();
	}
}


/* -------------------------------------------------------------------------- */


void parseAction_(MidiChannel* ch, const recorder::action* a, int localFrame)
{
	if (ch->isPlaying() && !ch->mute) {
		if (ch->midiOut)
			kernelMidi::send(a->iValue | MIDI_CHANS[ch->midiOutChan]);
#ifdef WITH_VST
		ch->addVstMidiEvent(a->iValue, localFrame);
#endif
	}
}

}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void parseEvents(MidiChannel* ch, mixer::FrameEvents fe)
{
	if (fe.onFirstBeat)
		onFirstBeat_(ch);
	for (const recorder::action* action : fe.actions)
		if (action->chan == ch->index && action->type == G_ACTION_MIDI)
			parseAction_(ch, action, fe.frameLocal);
}


/* -------------------------------------------------------------------------- */


void process(MidiChannel* ch, giada::m::AudioBuffer& out, const giada::m::AudioBuffer& in)
{
	#ifdef WITH_VST
		pluginHost::processStack(ch->buffer, pluginHost::CHANNEL, ch);
	#endif

		/* TODO - isn't this useful only if WITH_VST ? */
		for (int i=0; i<out.countFrames(); i++)
			for (int j=0; j<out.countChannels(); j++)
				out[i][j] += ch->buffer[i][j] * ch->volume;	
}


/* -------------------------------------------------------------------------- */


void start(MidiChannel* ch)
{
	switch (ch->status) {
		case ChannelStatus::PLAY:
			ch->status = ChannelStatus::ENDING;
			ch->sendMidiLstatus();
			break;

		case ChannelStatus::ENDING:
		case ChannelStatus::WAIT:
			ch->status = ChannelStatus::OFF;
			ch->sendMidiLstatus();
			break;

		case ChannelStatus::OFF:
			ch->status = ChannelStatus::WAIT;
			ch->sendMidiLstatus();
			break;

		default: break;
	}	
}


/* -------------------------------------------------------------------------- */


void kill(MidiChannel* ch, int localFrame)
{
	if (ch->isPlaying()) {
		if (ch->midiOut)
			kernelMidi::send(MIDI_ALL_NOTES_OFF);
#ifdef WITH_VST
		ch->addVstMidiEvent(MIDI_ALL_NOTES_OFF, 0);
#endif
	}
	ch->status = ChannelStatus::OFF;
	ch->sendMidiLstatus();
}


/* -------------------------------------------------------------------------- */


void rewindBySeq(MidiChannel* ch)
{
	if (ch->midiOut)
		kernelMidi::send(MIDI_ALL_NOTES_OFF);
#ifdef WITH_VST
		ch->addVstMidiEvent(MIDI_ALL_NOTES_OFF, 0);
#endif	
}


/* -------------------------------------------------------------------------- */


void setMute(MidiChannel* ch, bool v)
{
	ch->mute = v;
	if (ch->mute) {
		if (ch->midiOut)
			kernelMidi::send(MIDI_ALL_NOTES_OFF);
	#ifdef WITH_VST
			ch->addVstMidiEvent(MIDI_ALL_NOTES_OFF, 0);
	#endif		
		}
	ch->sendMidiLmute();
}


/* -------------------------------------------------------------------------- */


void stopBySeq(MidiChannel* ch)
{
	kill(ch, 0);
}
}}};