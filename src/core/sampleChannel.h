/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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


#include <functional>
#include <samplerate.h>
#include "types.h"
#include "channel.h"


class Patch;
class Wave;


class SampleChannel : public Channel
{
public:

	SampleChannel(bool inputMonitor, int bufferSize);
	~SampleChannel();

	void copy(const Channel* src, pthread_mutex_t* pluginMutex) override;
	void prepareBuffer(bool running) override;
	void parseEvents(giada::m::mixer::FrameEvents fe) override;
	void process(giada::m::AudioBuffer& out, const giada::m::AudioBuffer& in,
		bool audible, bool running) override;
	void readPatch(const std::string& basePath, int i) override;
	void writePatch(int i, bool isProject) override;

	void start(int frame, bool doQuantize, int velocity) override;
	void stop() override;
	void kill(int frame) override;
	bool recordStart(bool canQuantize) override;
	bool recordKill() override;
	void recordStop() override;
	void recordMute() override;
	void setMute(bool value, giada::EventType eventType) override;
	void startReadingActions(bool treatRecsAsLoops, bool recsStopOnChanHalt) override;
	void stopReadingActions(bool running, bool treatRecsAsLoops, 
		bool recsStopOnChanHalt) override;
	void empty() override;
	void stopBySeq(bool chansStopOnSeqHalt) override;
	void rewindBySeq() override;
	bool canInputRec() override;
	void stopInputRec(int globalFrame) override;
	bool hasLogicalData() const override;
	bool hasEditedData() const override;
	bool hasData() const override;

	float getBoost() const;	
	int   getBegin() const;
	int   getEnd() const;
	float getPitch() const;
	bool isAnyLoopMode() const;
	bool isAnySingleMode() const;
	bool isOnLastFrame() const;

	/* getPosition
	Returns the position of an active sample. If EMPTY o MISSING returns -1. */

	int getPosition() const;

	/* fillBuffer
	Fills 'dest' buffer at point 'offset' with Wave data taken from 'start'. 
	Returns how many frames have been used from the original Wave data. It also
	resamples data if pitch != 1.0f. */

	int fillBuffer(giada::m::AudioBuffer& dest, int start, int offset);

	/* pushWave
	Adds a new wave to an existing channel. */

	void pushWave(Wave* w);

	void setPitch(float v);
	void setBegin(int f);
	void setEnd(int f);
	void setBoost(float v);

	void setReadActions(bool v, bool recsStopOnChanHalt);

	/* onPreviewEnd
	A callback fired when audio preview ends. */

	std::function<void()> onPreviewEnd;

	/* bufferPreview
	Extra buffer for audio preview. */

	giada::m::AudioBuffer bufferPreview;
	
	giada::ChannelMode mode;
	
	Wave* wave;
	int   tracker;         // chan position
	int   trackerPreview;  // chan position for audio preview
	int   shift;
	bool  qWait;           // quantizer wait
	bool  inputMonitor;  
	float boost;
	float pitch;

	/* begin, end
	Begin/end point to read wave data from/to. */

	int begin;
	int end;

	/* midi stuff */

	bool     midiInVeloAsVol;
	uint32_t midiInReadActions;
	uint32_t midiInPitch;
	
private:

	/* rsmp_state, rsmp_data
	Structs from libsamplerate. */

	SRC_STATE* rsmp_state;
	SRC_DATA   rsmp_data;
};

#endif
