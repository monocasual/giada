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


#include "../utils/log.h"
#include "sampleChannelProc.h"
#include "sampleChannelRec.h"
#include "channelManager.h"
#include "const.h"
#include "wave.h"
#include "sampleChannel.h"


using std::string;


namespace giada {
namespace m 
{
SampleChannel::SampleChannel(bool inputMonitor, int bufferSize)
	: Channel          (ChannelType::SAMPLE, ChannelStatus::EMPTY, bufferSize),
	  mode             (ChannelMode::SINGLE_BASIC),
	  wave             (nullptr),
	  tracker          (0),
	  trackerPreview   (0),
	  shift            (0),
	  quantizing       (false),
	  inputMonitor     (inputMonitor),
	  boost            (G_DEFAULT_BOOST),
	  pitch            (G_DEFAULT_PITCH),
	  begin            (0),
	  end              (0),
	  midiInReadActions(0x0),
	  midiInPitch      (0x0),
	  rsmp_state       (nullptr)
{
	rsmp_state = src_new(SRC_LINEAR, G_MAX_IO_CHANS, nullptr);
	if (rsmp_state == nullptr) {
		gu_log("[SampleChannel] unable to alloc memory for SRC_STATE!\n");
		throw std::bad_alloc();
	}
	bufferPreview.alloc(bufferSize, G_MAX_IO_CHANS);
}


/* -------------------------------------------------------------------------- */


SampleChannel::~SampleChannel()
{
	if (rsmp_state != nullptr)
		src_delete(rsmp_state);
}


/* -------------------------------------------------------------------------- */


void SampleChannel::copy(const Channel* src_, pthread_mutex_t* pluginMutex)
{
	Channel::copy(src_, pluginMutex);
	const SampleChannel* src = static_cast<const SampleChannel*>(src_);
	tracker         = src->tracker;
	begin           = src->begin;
	end             = src->end;
	boost           = src->boost;
	mode            = src->mode;
	quantizing      = src->quantizing;
	setPitch(src->pitch);

	if (src->wave)
		pushWave(std::make_unique<Wave>(*src->wave)); // invoke Wave's copy constructor
}


/* -------------------------------------------------------------------------- */


void SampleChannel::parseEvents(mixer::FrameEvents fe)
{
	sampleChannelProc::parseEvents(this, fe);
	sampleChannelRec::parseEvents(this, fe);
}


/* -------------------------------------------------------------------------- */


void SampleChannel::prepareBuffer(bool running)
{
	sampleChannelProc::prepareBuffer(this, running);
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


void SampleChannel::process(AudioBuffer& out, const AudioBuffer& in, 
	bool audible, bool running)
{
	sampleChannelProc::process(this, out, in, audible, running);
}


/* -------------------------------------------------------------------------- */


void SampleChannel::readPatch(const string& basePath, const patch::channel_t& pch)
{
	Channel::readPatch("", pch);
	channelManager::readPatch(this, basePath, pch);
}


/* -------------------------------------------------------------------------- */


void SampleChannel::writePatch(int i, bool isProject)
{
	Channel::writePatch(i, isProject);
	channelManager::writePatch(this, isProject, i);
}


/* -------------------------------------------------------------------------- */


void SampleChannel::setReadActions(bool v, bool recsStopOnChanHalt)
{
	sampleChannelRec::setReadActions(this, v, recsStopOnChanHalt);
}


/* -------------------------------------------------------------------------- */


bool SampleChannel::hasLogicalData() const
{ 
	return wave != nullptr && wave->isLogical();
};


bool SampleChannel::hasEditedData() const
{ 
	return wave != nullptr && wave->isEdited();
};


bool SampleChannel::hasData() const
{
	return wave != nullptr;
}


/* -------------------------------------------------------------------------- */


void SampleChannel::setBegin(int f)
{
	if (f < 0)
		begin = 0;
	else
	if (f > wave->getSize())
		begin = wave->getSize();
	else
	if (f >= end)
		begin = end - 1;
	else
		begin = f;

	tracker = begin;
	trackerPreview = begin;
}


/* -------------------------------------------------------------------------- */


void SampleChannel::setEnd(int f)
{
	if (f >= wave->getSize())
		end = wave->getSize() - 1;
	else
	if (f <= begin)
		end = begin + 1;
	else
		end = f;
}


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
	if (status != ChannelStatus::EMPTY   && 
	    status != ChannelStatus::MISSING && 
	    status != ChannelStatus::OFF)
		return tracker - begin;
	else
		return -1;
}


/* -------------------------------------------------------------------------- */


void SampleChannel::setBoost(float v)
{
	if (v > G_MAX_BOOST_DB)
		boost = G_MAX_BOOST_DB;
	else 
	if (v < 0.0f)
		boost = 0.0f;
	else
		boost = v;
}


float SampleChannel::getBoost() const
{
	return boost;
}


/* -------------------------------------------------------------------------- */


void SampleChannel::empty()
{
	status     = ChannelStatus::EMPTY;
	begin      = 0;
	end        = 0;
	tracker    = 0;
	volume     = G_DEFAULT_VOL;
	boost      = G_DEFAULT_BOOST;
	hasActions = false;
	wave.reset(nullptr);
	sendMidiLstatus();
}


/* -------------------------------------------------------------------------- */


void SampleChannel::pushWave(std::unique_ptr<Wave>&& w)
{
	status = ChannelStatus::OFF;
	wave   = std::move(w);
	begin  = 0;
	end    = wave->getSize() - 1;
	sendMidiLstatus();
}


/* -------------------------------------------------------------------------- */


bool SampleChannel::canInputRec() const
{
	return wave == nullptr && armed;
}


/* -------------------------------------------------------------------------- */


int SampleChannel::fillBuffer(AudioBuffer& dest, int start, int offset)
{
	if (pitch == 1.0) return fillBufferCopy(dest, start, offset);
	else              return fillBufferResampled(dest, start, offset);
}


/* -------------------------------------------------------------------------- */


int SampleChannel::fillBufferResampled(AudioBuffer& dest, int start, int offset)
{
	rsmp_data.data_in       = wave->getFrame(start);        // Source data
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
	int used = dest.countFrames() - offset;
	if (used + start > wave->getSize())
		used = wave->getSize() - start;

	dest.copyData(wave->getFrame(start), used, offset);

	return used;
}


/* -------------------------------------------------------------------------- */


bool SampleChannel::isAnyLoopMode() const
{
	return mode == ChannelMode::LOOP_BASIC || mode == ChannelMode::LOOP_ONCE || 
	       mode == ChannelMode::LOOP_REPEAT || mode == ChannelMode::LOOP_ONCE_BAR;
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
