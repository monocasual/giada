/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#include "core/model/model.h"
#include "core/mixer.h"
#include "core/quantizer.h"
#include "core/clock.h"
#include "core/conf.h"
#include "core/recManager.h"
#include "core/kernelAudio.h"
#include "core/kernelMidi.h"
#include "sequencer.h"


namespace giada {
namespace m {
namespace sequencer
{
namespace
{
constexpr int Q_ACTION_REWIND = 0;


/* -------------------------------------------------------------------------- */


struct Metronome
{
	static constexpr Frame CLICK_SIZE = 38;

	float beat[CLICK_SIZE] = {
		 0.059033,  0.117240,  0.173807,  0.227943,  0.278890,  0.325936,
		 0.368423,  0.405755,  0.437413,  0.462951,  0.482013,  0.494333,
		 0.499738,  0.498153,  0.489598,  0.474195,  0.452159,  0.423798,
		 0.389509,  0.349771,  0.289883,  0.230617,  0.173194,  0.118739,
		 0.068260,  0.022631, -0.017423, -0.051339,	-0.078721, -0.099345,
		-0.113163, -0.120295, -0.121028, -0.115804, -0.105209, -0.089954,
		-0.070862, -0.048844
	};

	float bar[CLICK_SIZE] = {
		 0.175860,  0.341914,  0.488904,  0.608633,  0.694426,  0.741500,
		 0.747229,  0.711293,  0.635697,  0.524656,  0.384362,  0.222636,
		 0.048496, -0.128348, -0.298035, -0.451105, -0.579021, -0.674653,
		-0.732667, -0.749830, -0.688924, -0.594091, -0.474481, -0.340160,
	 	-0.201360, -0.067752,  0.052194,  0.151746,  0.226280,  0.273493,
		 0.293425,  0.288307,  0.262252,  0.220811,  0.170435,  0.117887,
		 0.069639,  0.031320
	};

	Frame tracker  = 0;
	bool  running  = false;
	bool  playBar  = false;
	bool  playBeat = false;

	void render(AudioBuffer& outBuf, bool& process, float* data, Frame f)
	{
		process = true;
		for (int i=0; i<outBuf.countChannels(); i++)
			outBuf[f][i] += data[tracker];
		if (++tracker > Metronome::CLICK_SIZE) {
			process = false;
			tracker = 0;
		}	
	}
} metronome_;


/* -------------------------------------------------------------------------- */


void renderMetronome_(AudioBuffer& outBuf, Frame f)
{
	if (!metronome_.running)
		return;

	if (clock::isOnBar() || metronome_.playBar)
		metronome_.render(outBuf, metronome_.playBar, metronome_.bar, f);
	else
	if (clock::isOnBeat() || metronome_.playBeat)
		metronome_.render(outBuf, metronome_.playBeat, metronome_.beat, f);
}


/* -------------------------------------------------------------------------- */


void rewind_(Frame delta)
{
	clock::rewind();

#ifdef __linux__
	kernelAudio::jackSetPosition(0);
#endif

	if (conf::conf.midiSync == MIDI_SYNC_CLOCK_M)
		kernelMidi::send(MIDI_POSITION_PTR, 0, 0);

	mixer::pumpEvent({ mixer::EventType::SEQUENCER_REWIND, delta });
}


/* -------------------------------------------------------------------------- */


Quantizer quantizer_;
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void init()
{
	quantizer_.schedule(Q_ACTION_REWIND, rewind_);
	clock::rewind();
}


/* -------------------------------------------------------------------------- */


void run(Frame bufferSize)
{
	Frame start = clock::getCurrentFrame();
	Frame end   = start + bufferSize;
	Frame total = clock::getFramesInLoop();
	Frame bar   = clock::getFramesInBar();

	for (Frame i = start, local = 0; i < end; i++, local++) {

		Frame global = i % total; // wraps around 'total'

		if (global == 0)
			mixer::pumpEvent({ mixer::EventType::SEQUENCER_FIRST_BEAT, local, { 0, 0, global } });
		else
		if (global % bar == 0)
			mixer::pumpEvent({ mixer::EventType::SEQUENCER_BAR, local, { 0, 0, global } });

		const std::vector<Action>* as = recorder::getActionsOnFrame(global);
		if (as != nullptr)
			for (const Action& a : *as)
				mixer::pumpEvent({ mixer::EventType::ACTION, local, a });
	}

	quantizer_.advance(Range<Frame>(start, end), clock::getQuantizerStep());
}


/* -------------------------------------------------------------------------- */


void parse(const mixer::EventBuffer& events)
{
	for (const mixer::Event& e : events) {
		if (e.type == mixer::EventType::SEQUENCER_START) {
			start(); break;
		}
		if (e.type == mixer::EventType::SEQUENCER_STOP) {
			stop(); break;
		}
		if (e.type == mixer::EventType::SEQUENCER_REWIND_REQ) {
			rewind(); break;
		}
	}
}


/* -------------------------------------------------------------------------- */


void advance(AudioBuffer& outBuf)
{
	for (Frame i = 0; i < outBuf.countFrames(); i++) {
		clock::sendMIDIsync(); 
		clock::incrCurrentFrame();
		renderMetronome_(outBuf, i);
	}
}


/* -------------------------------------------------------------------------- */


void start()
{
	switch (clock::getStatus()) {
		case ClockStatus::STOPPED:
			clock::setStatus(ClockStatus::RUNNING); 
			break;
		case ClockStatus::WAITING:
			clock::setStatus(ClockStatus::RUNNING); 
			recManager::stopActionRec();
			break;
		default: 
			break;
	}

#ifdef __linux__
	kernelAudio::jackStart();
#endif
}


/* -------------------------------------------------------------------------- */


void stop()
{
	clock::setStatus(ClockStatus::STOPPED);

#ifdef __linux__
	kernelAudio::jackStop();
#endif

	/* If recordings (both input and action) are active deactivate them, but 
	store the takes. RecManager takes care of it. */

	if (recManager::isRecordingAction())
		recManager::stopActionRec();
	else
	if (recManager::isRecordingInput())
		recManager::stopInputRec();
}


/* -------------------------------------------------------------------------- */


void rewind()
{
	if (clock::canQuantize())
		quantizer_.trigger(Q_ACTION_REWIND);
	else
		rewind_(/*delta=*/0);
}


/* -------------------------------------------------------------------------- */


bool isMetronomeOn()      { return metronome_.running; }
void toggleMetronome()    { metronome_.running = !metronome_.running; }
void setMetronome(bool v) { metronome_.running = v; }
}}}; // giada::m::sequencer::


