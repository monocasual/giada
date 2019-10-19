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
#include "utils/log.h"
#include "core/const.h"
#include "core/wave.h"
#include "core/model/model.h"
#include "sampleChannelProc.h"
#include "sampleChannelRec.h"
#include "channelManager.h"
#include "sampleChannel.h"


namespace giada {
namespace m 
{
SampleChannel::SampleChannel(bool inputMonitor, int bufferSize,
	ID columnId, ID id)
: Channel          (ChannelType::SAMPLE, ChannelStatus::EMPTY, bufferSize,
                    columnId, id),
  hasWave          (false),
  waveId           (0),
  shift            (0),
  mode             (ChannelMode::SINGLE_BASIC),
  quantizing       (false),
  inputMonitor     (inputMonitor),
  pitch            (G_DEFAULT_PITCH),
  tracker          (0),
  trackerPreview   (0),
  begin            (0),
  end              (0),
  midiInVeloAsVol  (false),
  midiInReadActions(0x0),
  midiInPitch      (0x0),
  bufferOffset     (0),
  rewinding        (false),
  rsmp_state       (src_new(SRC_LINEAR, G_MAX_IO_CHANS, nullptr))
{
	if (rsmp_state == nullptr) {
		gu_log("[SampleChannel] unable to alloc memory for SRC_STATE!\n");
		throw std::bad_alloc();
	}
	bufferPreview.alloc(bufferSize, G_MAX_IO_CHANS);
}


/* -------------------------------------------------------------------------- */


SampleChannel::SampleChannel(const SampleChannel& o)
: Channel          (o),
  hasWave          (o.hasWave),
  waveId           (o.waveId),
  shift            (o.shift),
  mode             (o.mode),
  quantizing       (o.quantizing),
  inputMonitor     (o.inputMonitor),
  pitch            (o.pitch),
  tracker          (o.tracker.load()),
  trackerPreview   (0),
  begin            (o.begin),
  end              (o.end),
  midiInVeloAsVol  (o.midiInVeloAsVol),
  midiInReadActions(o.midiInReadActions.load()),
  midiInPitch      (o.midiInPitch.load()),
  bufferOffset     (o.bufferOffset),
  rewinding        (o.rewinding),
  rsmp_state       (src_new(SRC_LINEAR, G_MAX_IO_CHANS, nullptr))
{
	if (rsmp_state == nullptr) {
		gu_log("[SampleChannel] unable to alloc memory for SRC_STATE!\n");
		throw std::bad_alloc();
	}
	
	bufferPreview.alloc(o.bufferPreview.countFrames(), G_MAX_IO_CHANS);
}


/* -------------------------------------------------------------------------- */


SampleChannel::SampleChannel(const patch::Channel& p, int bufferSize)
: Channel          (p, bufferSize),
  hasWave          (p.waveId != 0),
  waveId           (p.waveId),
  shift            (0), // TODO
  mode             (p.mode),
  quantizing       (false),
  inputMonitor     (p.inputMonitor),
  pitch            (p.pitch),
  tracker          (0),
  trackerPreview   (0),
  begin            (p.begin),
  end              (p.end),
  midiInVeloAsVol  (p.midiInVeloAsVol),
  midiInReadActions(p.midiInReadActions),
  midiInPitch      (p.midiInPitch),
  bufferOffset     (0),
  rewinding        (0),
  rsmp_state       (src_new(SRC_LINEAR, G_MAX_IO_CHANS, nullptr))
{
	if (rsmp_state == nullptr) {
		gu_log("[SampleChannel] unable to alloc memory for SRC_STATE!\n");
		throw std::bad_alloc();
	}
	
	bufferPreview.alloc(bufferSize, G_MAX_IO_CHANS);
}


/* -------------------------------------------------------------------------- */


SampleChannel* SampleChannel::clone() const
{
	return new SampleChannel(*this);
}


/* -------------------------------------------------------------------------- */


SampleChannel::~SampleChannel()
{
	if (rsmp_state != nullptr)
		src_delete(rsmp_state);
}


/* -------------------------------------------------------------------------- */


void SampleChannel::parseEvents(mixer::FrameEvents fe)
{
	sampleChannelProc::parseEvents(this, fe);
	sampleChannelRec::parseEvents(this, fe);
}


/* -------------------------------------------------------------------------- */


void SampleChannel::render(AudioBuffer& out, const AudioBuffer& in, 
        AudioBuffer& inToOut, bool audible, bool running)
{
	sampleChannelProc::render(this, out, in, inToOut, audible, running);
}



/* -------------------------------------------------------------------------- */


void SampleChannel::rewindBySeq()
{
	sampleChannelProc::rewindBySeq(this);
}


/* -------------------------------------------------------------------------- */


void SampleChannel::start(int localFrame, bool doQuantize, int velocity)
{
	sampleChannelProc::start(this, localFrame, doQuantize, velocity);
}


/* -------------------------------------------------------------------------- */


void SampleChannel::stop()
{
	sampleChannelProc::stop(this);
}


/* -------------------------------------------------------------------------- */


void SampleChannel::stopBySeq(bool chansStopOnSeqHalt)
{
	sampleChannelProc::stopBySeq(this, chansStopOnSeqHalt);
}


/* -------------------------------------------------------------------------- */


void SampleChannel::kill(int localFrame)
{
	sampleChannelProc::kill(this, localFrame);
}


/* -------------------------------------------------------------------------- */


bool SampleChannel::recordStart(bool canQuantize) 
{
	return sampleChannelRec::recordStart(this, canQuantize);
}


bool SampleChannel::recordKill()
{
	return sampleChannelRec::recordKill(this);
}


void SampleChannel::recordStop()
{
	sampleChannelRec::recordStop(this);
}


/* -------------------------------------------------------------------------- */


void SampleChannel::startReadingActions(bool treatRecsAsLoops, bool recsStopOnChanHalt)
{
	sampleChannelRec::startReadingActions(this, treatRecsAsLoops, recsStopOnChanHalt);
}


/* -------------------------------------------------------------------------- */


void SampleChannel::stopReadingActions(bool running, bool treatRecsAsLoops, 
		bool recsStopOnChanHalt)
{
	sampleChannelRec::stopReadingActions(this, running, treatRecsAsLoops, 
		recsStopOnChanHalt);
}


/* -------------------------------------------------------------------------- */


void SampleChannel::stopInputRec(int globalFrame)
{
	sampleChannelProc::stopInputRec(this, globalFrame);
}


/* -------------------------------------------------------------------------- */


void SampleChannel::setMute(bool value)
{
	sampleChannelProc::setMute(this, value);
}


/* -------------------------------------------------------------------------- */


void SampleChannel::setSolo(bool value)
{
	sampleChannelProc::setSolo(this, value);
}


/* -------------------------------------------------------------------------- */


void SampleChannel::setReadActions(bool v, bool recsStopOnChanHalt)
{
	sampleChannelRec::setReadActions(this, v, recsStopOnChanHalt);
}


/* -------------------------------------------------------------------------- */


bool SampleChannel::hasLogicalData() const
{ 
	if (!hasWave)
		return false;

	model::WavesLock wl(model::waves);
	return model::get(model::waves, waveId).isLogical();
};


bool SampleChannel::hasEditedData() const
{ 
	if (!hasWave)
		return false;

	model::WavesLock wl(model::waves);
	return model::get(model::waves, waveId).isEdited();
};


bool SampleChannel::hasData() const
{
	return hasWave;
}


/* -------------------------------------------------------------------------- */


void SampleChannel::setBegin(int f)
{
	model::WavesLock lock(model::waves);
	const Wave& wave = model::get(model::waves, waveId);
	
	if (f < 0)
		f = 0;
	else
	if (f > wave.getSize())
		f = wave.getSize();
	else
	if (f >= end)
		f = end - 1;

	begin          = f;
	tracker        = f;
	trackerPreview = f;
}


/* -------------------------------------------------------------------------- */


void SampleChannel::setEnd(int f)
{
	model::WavesLock lock(model::waves);
	const Wave& wave = model::get(model::waves, waveId);
	
	if (f >= wave.getSize())
		f = wave.getSize() - 1;
	else
	if (f <= begin)
		f = begin + 1;

	end = f;}


/* -------------------------------------------------------------------------- */


int SampleChannel::getBegin() const { return begin; }
int SampleChannel::getEnd() const   { return end; }


/* -------------------------------------------------------------------------- */


void SampleChannel::setPitch(float v)
{
	if (v > G_MAX_PITCH)
		pitch = G_MAX_PITCH;
	else
	if (v < G_MIN_PITCH)
		pitch = G_MIN_PITCH;
	else 
		pitch = v;

// ???? 	/* if status is off don't slide between frequencies */
// ???? 
// ???? 	if (status & (STATUS_OFF | STATUS_WAIT))
// ???? 		src_set_ratio(rsmp_state, 1/pitch);
}


float SampleChannel::getPitch() const { return pitch; }


/* -------------------------------------------------------------------------- */


int SampleChannel::getPosition() const
{
	if (playStatus != ChannelStatus::EMPTY   && 
	    playStatus != ChannelStatus::MISSING && 
	    playStatus != ChannelStatus::OFF)
		return tracker - begin;
	else
		return -1;
}


/* -------------------------------------------------------------------------- */


void SampleChannel::empty()
{
	playStatus = ChannelStatus::EMPTY;
	begin      = 0;
	end        = 0;
	tracker    = 0;
	volume     = G_DEFAULT_VOL;
	hasActions = false;
	hasWave    = false;
	waveId     = 0;
	sendMidiLstatus();
}


/* -------------------------------------------------------------------------- */


void SampleChannel::pushWave(ID wid, Frame size)
{
	playStatus = ChannelStatus::OFF;
	waveId     = wid;
	begin      = 0;
	end        = size;
	tracker    = 0;
	hasWave    = true;
	sendMidiLstatus();
}


void SampleChannel::popWave()
{
	playStatus = ChannelStatus::OFF;
	waveId     = 0;
	begin      = 0;
	end        = 0;
	tracker    = 0;
	hasWave    = false;
	sendMidiLstatus();
}


/* -------------------------------------------------------------------------- */


std::string SampleChannel::getSamplePath() const
{
	if (!hasWave)
		return "";

	model::WavesLock wl(model::waves);
	return model::get(model::waves, waveId).getPath();
}


/* -------------------------------------------------------------------------- */


bool SampleChannel::canInputRec() const
{
	return !hasWave && armed == true;
}


/* -------------------------------------------------------------------------- */


int SampleChannel::fillBuffer(AudioBuffer& dest, int start, int offset)
{
	assert(offset < dest.countFrames());
	
	if (pitch == 1.0) return fillBufferCopy(dest, start, offset);
	else              return fillBufferResampled(dest, start, offset);
}


/* -------------------------------------------------------------------------- */


int SampleChannel::fillBufferResampled(AudioBuffer& dest, int start, int offset)
{
	model::WavesLock lock(model::waves);
	const Wave& wave = model::get(model::waves, waveId);
	
	rsmp_data.data_in       = wave.getFrame(start);        // Source data
	rsmp_data.input_frames  = end - start;                  // How many readable frames
	rsmp_data.data_out      = dest[offset];                 // Destination (processed data)
	rsmp_data.output_frames = dest.countFrames() - offset;  // How many frames to process
	rsmp_data.end_of_input  = false;
	rsmp_data.src_ratio     = 1 / pitch;

	src_process(rsmp_state, &rsmp_data);

	return rsmp_data.input_frames_used; // Returns used frames
}

/* -------------------------------------------------------------------------- */


int SampleChannel::fillBufferCopy(AudioBuffer& dest, int start, int offset)
{
	model::WavesLock lock(model::waves);
	const Wave& wave = model::get(model::waves, waveId);

	int used = dest.countFrames() - offset;
	if (used > wave.getSize() - start)
		used = wave.getSize() - start;

	dest.copyData(wave.getFrame(start), used, offset);

	return used;
}


/* -------------------------------------------------------------------------- */


bool SampleChannel::isAnyLoopMode() const
{
	return mode == ChannelMode::LOOP_BASIC  || 
	       mode == ChannelMode::LOOP_ONCE   || 
	       mode == ChannelMode::LOOP_REPEAT || 
	       mode == ChannelMode::LOOP_ONCE_BAR;
}


bool SampleChannel::isAnySingleMode() const
{
	return !isAnyLoopMode();
}


/* -------------------------------------------------------------------------- */


bool SampleChannel::isOnLastFrame() const
{
	return tracker >= end;
}

}} // giada::m::
