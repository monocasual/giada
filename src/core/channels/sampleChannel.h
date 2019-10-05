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


#ifndef G_SAMPLE_CHANNEL_H
#define G_SAMPLE_CHANNEL_H


#include <memory>
#include <functional>
#include <samplerate.h>
#include "core/types.h"
#include "core/channels/channel.h"


namespace giada {
namespace m 
{
class Wave;

class SampleChannel : public Channel
{
public:

	SampleChannel(bool inputMonitor, int bufferSize, ID columnId, ID id);
	SampleChannel(const SampleChannel& o);
	SampleChannel(const patch::Channel& p, int bufferSize);
	~SampleChannel();

	SampleChannel* clone() const override;
	void parseEvents(mixer::FrameEvents fe) override;
	void render(AudioBuffer& out, const AudioBuffer& in, AudioBuffer& inToOut, 
		bool audible, bool running) override;

	void start(int frame, bool doQuantize, int velocity) override;
	void stop() override;
	void kill(int frame) override;
	bool recordStart(bool canQuantize) override;
	bool recordKill() override;
	void recordStop() override;
	void setMute(bool value) override;
	void setSolo(bool value) override;
	void startReadingActions(bool treatRecsAsLoops, bool recsStopOnChanHalt) override;
	void stopReadingActions(bool running, bool treatRecsAsLoops, 
		bool recsStopOnChanHalt) override;
	void empty() override;
	void stopBySeq(bool chansStopOnSeqHalt) override;
	void rewindBySeq() override;
	void stopInputRec(int globalFrame) override;
	bool canInputRec() const override;
	bool hasLogicalData() const override;
	bool hasEditedData() const override;
	bool hasData() const override;

	int   getBegin() const;
	int   getEnd() const;
	float getPitch() const;
	bool isAnyLoopMode() const;
	bool isAnySingleMode() const;
	bool isOnLastFrame() const;
	std::string getSamplePath() const;

	/* getPosition
	Returns the position of an active sample. If EMPTY o MISSING returns -1. */

	int getPosition() const;

	/* fillBuffer
	Fills 'dest' buffer at point 'offset' with Wave data taken from 'start'. 
	Returns how many frames have been used from the original Wave data. It also
	resamples data if pitch != 1.0f. */

	int fillBuffer(AudioBuffer& dest, int start, int offset);

	/* pushWave
	Adds a new wave to this channel. */

	void pushWave(ID waveId, Frame waveSize);
	void popWave();

	void setPitch(float v);
	void setBegin(int f);
	void setEnd(int f);

	void setReadActions(bool v, bool recsStopOnChanHalt);

	/* bufferPreview
	Extra buffer for audio preview. */

	AudioBuffer bufferPreview;
	
	/* hasWave
	Tells if a wave is linked to this channel. */
	/* TODO - useless: check if waveId != 0 */

	bool hasWave;

	/* waveId
	ID of a Wave object. Might be useless if hasWave == false. */

	ID waveId;

	int shift;
	ChannelMode mode;
	bool quantizing;                    // quantization in progress
	bool inputMonitor;  
	float pitch;
	
	std::atomic<Frame> tracker;         // chan position
	std::atomic<Frame> trackerPreview;  // chan position for audio preview

	/* begin, end
	Begin/end point to read wave data from/to. */

	Frame begin;  
	Frame end;    

	/* midiIn*
	MIDI input parameters. */

	bool                  midiInVeloAsVol;
	std::atomic<uint32_t> midiInReadActions;
	std::atomic<uint32_t> midiInPitch;

	/* bufferOffset
	Offset used while filling the internal buffer with audio data. Value is 
	greater than zero on start sample. */
	
	Frame bufferOffset;

	/* rewinding
	Tells whether a rewind event is taking place. Used to fill the audio
	buffer twice. */

	bool rewinding;	

private:

	/* rsmp_state, rsmp_data
	Structs from libsamplerate. */

	SRC_STATE* rsmp_state;
	SRC_DATA   rsmp_data;

	int fillBufferResampled(AudioBuffer& dest, int start, int offset);
	int fillBufferCopy     (AudioBuffer& dest, int start, int offset);
};

}} // giada::m::


#endif
