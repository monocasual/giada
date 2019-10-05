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
#include "utils/math.h"
#include "core/model/model.h"
#include "core/channels/sampleChannel.h"
#include "core/const.h"
#include "core/pluginHost.h"
#include "core/mixerHandler.h"
#include "sampleChannelProc.h"


namespace giada {
namespace m {
namespace sampleChannelProc
{
namespace
{
void rewind_(SampleChannel* ch, Frame localFrame)
{
	/* Quantization stops on rewind. */

	ch->quantizing = false; 

	if (ch->isPlaying()) { 
		ch->rewinding    = true;
		ch->bufferOffset = localFrame;
	}
	else
		ch->tracker = ch->begin;
}


/* -------------------------------------------------------------------------- */


/* quantize
Starts channel according to quantizer. */

void quantize_(SampleChannel* ch, int localFrame)
{
	switch (ch->playStatus) {
		case ChannelStatus::OFF:
			ch->playStatus   = ChannelStatus::PLAY;
			ch->bufferOffset = localFrame;
			ch->sendMidiLstatus();
			// ch->quantizing = false is set by sampleChannelRec::quantize()
			break;

		default:
			rewind_(ch, localFrame);
			break;
	}
}


/* -------------------------------------------------------------------------- */


/* onBar
Things to do when the sequencer is on a bar. */

void onBar_(SampleChannel* ch, int localFrame)
{
	switch (ch->playStatus) {
		case ChannelStatus::PLAY:
			if (ch->mode == ChannelMode::LOOP_REPEAT)
				rewind_(ch, localFrame);
			break;

		case ChannelStatus::WAIT:
			if (ch->mode == ChannelMode::LOOP_ONCE_BAR) {
				ch->playStatus       = ChannelStatus::PLAY;
				ch->bufferOffset = localFrame;
				ch->sendMidiLstatus();
			}
			break;

		default: break;
	}
}


/* -------------------------------------------------------------------------- */


/* onFirstBeat
Things to do when the sequencer is on the first beat. */

void onFirstBeat_(SampleChannel* ch, Frame localFrame)
{
	switch (ch->playStatus) {
		case ChannelStatus::PLAY: 
			if (ch->isAnyLoopMode())
				rewind_(ch, localFrame);
			break;

		case ChannelStatus::WAIT:
			ch->playStatus       = ChannelStatus::PLAY;
			ch->bufferOffset = localFrame;
			ch->sendMidiLstatus();
			break;

		case ChannelStatus::ENDING: 
			if (ch->isAnyLoopMode())
				kill(ch, localFrame);

		default: break;
	}
}


/* -------------------------------------------------------------------------- */


/* onLastFrame
Things to do when the sample has reached the end (i.e. last frame). Called by
prepareBuffer(). */

void onLastFrame_(SampleChannel* ch, bool running)
{
	switch (ch->playStatus) {
		case ChannelStatus::PLAY:
			/* Stop LOOP_* when the sequencer is off, or SINGLE_* except for
			SINGLE_ENDLESS, which runs forever unless it's in ENDING mode. 
			Other loop once modes are put in wait mode. */
			if ((ch->mode == ChannelMode::SINGLE_BASIC   || 
				 ch->mode == ChannelMode::SINGLE_PRESS   ||
				 ch->mode == ChannelMode::SINGLE_RETRIG) || 
				(ch->isAnyLoopMode() && !running))
				ch->playStatus = ChannelStatus::OFF;
			else
			if (ch->mode == ChannelMode::LOOP_ONCE     ||
			    ch->mode == ChannelMode::LOOP_ONCE_BAR)
				ch->playStatus = ChannelStatus::WAIT;
			ch->sendMidiLstatus();
			break;			

		case ChannelStatus::ENDING:
			/* LOOP_ONCE or LOOP_ONCE_BAR: if ending (i.e. the user requested 
			their termination), stop 'em. Let them wait otherwise. */
			if (ch->mode == ChannelMode::LOOP_ONCE ||
			    ch->mode == ChannelMode::LOOP_ONCE_BAR)
				ch->playStatus = ChannelStatus::WAIT;
			else {
				ch->playStatus = ChannelStatus::OFF;
				ch->sendMidiLstatus();
			}
			break;

		default: break;
	}
}


/* -------------------------------------------------------------------------- */


void processIO_(SampleChannel* ch, m::AudioBuffer& out, const m::AudioBuffer& in, 
	bool running)
{
	assert(out.countSamples() == ch->buffer.countSamples());
	if (in.isAllocd())
		assert(in.countSamples() == ch->buffer.countSamples());

	/* If armed and input buffer is not empty (i.e. input device available) and
	input monitor is on, copy input buffer to channel buffer: this enables the 
	input monitoring. The channel buffer will be overwritten later on by 
	pluginHost::processStack, so that you would record "clean" audio 
	(i.e. not plugin-processed). */

	if (ch->armed && in.isAllocd() && ch->inputMonitor) {
		for (int i=0; i<ch->buffer.countFrames(); i++)
			for (int j=0; j<ch->buffer.countChannels(); j++)
				ch->buffer[i][j] += in[i][j];   // add, don't overwrite
	}

#ifdef WITH_VST
	pluginHost::processStack(ch->buffer, ch->pluginIds);
#endif

	for (int i=0; i<out.countFrames(); i++) {
		if (running)
			ch->calcVolumeEnvelope();
		if (!ch->mute)
			for (int j=0; j<out.countChannels(); j++)
				out[i][j] += ch->buffer[i][j] * ch->volume * ch->volume_i * ch->calcPanning(j);	
	}
}


/* -------------------------------------------------------------------------- */


void processPreview_(SampleChannel* ch, m::AudioBuffer& out)
{
	ch->bufferPreview.clear();

	/* If the tracker exceedes the end point and preview is looped, split the 
	rendering as in SampleChannel::reset(). */

	if (ch->trackerPreview + ch->bufferPreview.countFrames() >= ch->end) {
		int offset = ch->end - ch->trackerPreview;
		ch->trackerPreview += ch->fillBuffer(ch->bufferPreview, ch->trackerPreview, 0);
		ch->trackerPreview = ch->begin;
		if (ch->previewMode == PreviewMode::LOOP)
			ch->trackerPreview += ch->fillBuffer(ch->bufferPreview, ch->begin, offset);
		else
		if (ch->previewMode == PreviewMode::NORMAL)
			ch->previewMode = PreviewMode::NONE;
	}
	else
		ch->trackerPreview += ch->fillBuffer(ch->bufferPreview, ch->trackerPreview, 0);

	for (int i=0; i<out.countFrames(); i++)
		for (int j=0; j<out.countChannels(); j++)
			out[i][j] += ch->bufferPreview[i][j] * ch->volume * ch->calcPanning(j);	
}


/* -------------------------------------------------------------------------- */


void fillBuffer_(SampleChannel* ch, bool running)
{
	ch->buffer.clear();

	if (!ch->hasData() || !ch->isPlaying())
		return;

	if (ch->rewinding) {

		/* Fill the tail. */
		
		if (!ch->isOnLastFrame())
			ch->fillBuffer(ch->buffer, ch->tracker, 0);
		
		/* Reset tracker to begin point. */

		ch->tracker = ch->begin;
		
		/* Then fill the new head. */

		ch->tracker += ch->fillBuffer(ch->buffer, ch->tracker, ch->bufferOffset);
		ch->bufferOffset = 0;
		ch->rewinding    = false;
	}
	else {
		Frame framesUsed = ch->fillBuffer(ch->buffer, ch->tracker, ch->bufferOffset);
		ch->tracker      += framesUsed;
		ch->bufferOffset  = 0;
		if (ch->isOnLastFrame()) {
			onLastFrame_(ch, running);
			ch->tracker = ch->begin;
			if (ch->mode == ChannelMode::LOOP_BASIC  || 
			    ch->mode == ChannelMode::LOOP_REPEAT || 
			    ch->mode == ChannelMode::SINGLE_ENDLESS) {
				/* framesUsed might be imprecise when working with resampled 
				audio, which could cause a buffer overflow if used as offset.
				Let's clamp it to be at most buffer->countFrames(). */
				ch->tracker += ch->fillBuffer(ch->buffer, ch->tracker, 
					u::math::bound(framesUsed, 0, ch->buffer.countFrames() - 1));
			}
		}
	}
}
}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void kill(SampleChannel* ch, int localFrame)
{
	switch (ch->playStatus) {
		case ChannelStatus::WAIT:
		case ChannelStatus::PLAY:
		case ChannelStatus::ENDING:
			/*  Clear data in range [localFrame, (buffer.size)) if the kill event 
			occurs in the middle of the buffer. */
			if (localFrame != 0)
				ch->buffer.clear(localFrame);
			ch->playStatus = ChannelStatus::OFF;
			ch->sendMidiLstatus();
			rewind_(ch, localFrame);
			break;

		default: break;
	}
}


/* -------------------------------------------------------------------------- */


void stop(SampleChannel* ch)
{
	switch (ch->playStatus) {
		case ChannelStatus::PLAY:
			if (ch->mode == ChannelMode::SINGLE_PRESS)
				kill(ch, 0);
			break;

		default:
			/* If quantizing, stop a SINGLE_PRESS immediately. */
			if (ch->mode == ChannelMode::SINGLE_PRESS && ch->quantizing)
				ch->quantizing = false;	
			break;		
	}
}


/* -------------------------------------------------------------------------- */


void stopInputRec(SampleChannel* ch, int globalFrame)
{
	/* Start all sample channels in loop mode that were armed, i.e. that were
	recording stuff and not yet in play. They are also started in force mode, i.e.
	they must start playing right away at the current global frame, not at the 
	next first beat. */
	if (ch->isAnyLoopMode() && ch->playStatus == ChannelStatus::OFF && ch->armed) {
		ch->playStatus  = ChannelStatus::PLAY;
		ch->tracker = globalFrame;
	}
}


/* -------------------------------------------------------------------------- */


void stopBySeq(SampleChannel* ch, bool chansStopOnSeqHalt)
{
	switch (ch->playStatus) {
		case ChannelStatus::WAIT:
			/* Loop-mode channels in wait status get stopped right away. */
			if (ch->isAnyLoopMode())
				ch->playStatus = ChannelStatus::OFF;
			break;

		case ChannelStatus::PLAY:
			/* Kill samples if a) chansStopOnSeqHalt == true (run the sample to end 
			otherwise); b) when a channel is reading (and playing) actions. */
			if (chansStopOnSeqHalt)
				if (ch->isAnyLoopMode() || ch->isReadingActions())
					kill(ch, 0);
			break;

		default: break;
	}
}


/* -------------------------------------------------------------------------- */


void rewindBySeq(SampleChannel* ch)
{
	/* Rewind LOOP_ANY or SINGLE_ANY only if it's in read-record-mode. Rewind by 
	sequencer is a user-generated event, it always occurs on local frame 0. */

	if (ch->hasData()) {
		if ((ch->isAnyLoopMode()) || (ch->recStatus == ChannelStatus::PLAY && (ch->isAnySingleMode())))
			rewind_(ch, 0);
	}	
}


/* -------------------------------------------------------------------------- */


void setMute(SampleChannel* ch, bool value)
{
	ch->mute = value;

	// This is for processing playing_inaudible
	ch->sendMidiLstatus();	

	ch->sendMidiLmute();
}


/* -------------------------------------------------------------------------- */


void setSolo(SampleChannel* ch, bool value)
{
	ch->solo = value;

	// This is for processing playing_inaudible
	model::ChannelsLock l(model::channels);
	for (Channel* c : model::channels)
		c->sendMidiLstatus();
	
	ch->sendMidiLsolo();
}


/* -------------------------------------------------------------------------- */


void start(SampleChannel* ch, int localFrame, bool doQuantize, int velocity)
{
	/* For one-shot modes, velocity drives the internal volume. */
	if (velocity != 0) {
		if (ch->isAnySingleMode() && ch->midiInVeloAsVol)
			ch->volume_i.store(u::math::map<int, float>(velocity, 0, G_MAX_VELOCITY, 0.0, 1.0));		
	}

	switch (ch->playStatus) {
		case ChannelStatus::OFF:
			ch->bufferOffset = localFrame;
			if (ch->isAnyLoopMode()) {
				ch->playStatus = ChannelStatus::WAIT;
				ch->sendMidiLstatus();
			}
			else {
				if (doQuantize)
					ch->quantizing = true;
				else {
					ch->playStatus = ChannelStatus::PLAY;
					ch->sendMidiLstatus();
				}
			}
			break;

		case ChannelStatus::PLAY:
			if (ch->mode == ChannelMode::SINGLE_RETRIG) {
				if (doQuantize)
					ch->quantizing = true;
				else
					rewind_(ch, localFrame);
			}
			else
			if (ch->isAnyLoopMode() || ch->mode == ChannelMode::SINGLE_ENDLESS) {
				ch->playStatus = ChannelStatus::ENDING;
				ch->sendMidiLstatus();
			}
			else
			if (ch->mode == ChannelMode::SINGLE_BASIC) {
				rewind_(ch, localFrame);
				ch->playStatus = ChannelStatus::OFF;
				ch->sendMidiLstatus();
			}
			break;

		case ChannelStatus::WAIT:
			ch->playStatus = ChannelStatus::OFF;
			ch->sendMidiLstatus();
			break;

		case ChannelStatus::ENDING:
			ch->playStatus = ChannelStatus::PLAY;
			ch->sendMidiLstatus();
			break;

		default: break;
	}
}


/* -------------------------------------------------------------------------- */


void render(SampleChannel* ch, AudioBuffer& out, const AudioBuffer& in, 
		AudioBuffer& inToOut, bool audible, bool running)
{
	fillBuffer_(ch, running);

	if (audible)
		processIO_(ch, out, in, running);

	if (ch->isPreview())
		processPreview_(ch, out);
}


/* -------------------------------------------------------------------------- */


void parseEvents(SampleChannel* ch, mixer::FrameEvents fe)
{
	if (!ch->hasData())
		return;

	/* Quantize only if is single mode and in quantizer-wait mode and a
	quantizer step has passed. */

	if (ch->isAnySingleMode() && ch->quantizing && fe.quantoPassed)	
		quantize_(ch, fe.frameLocal);
	if (fe.onBar)
		onBar_(ch, fe.frameLocal);
	if (fe.onFirstBeat)
		onFirstBeat_(ch, fe.frameLocal);
}
}}};
