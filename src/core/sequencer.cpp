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
#include "core/const.h"
#include "core/mixer.h"
#include "core/quantizer.h"
#include "core/clock.h"
#include "core/conf.h"
#include "core/recManager.h"
#include "core/kernelAudio.h"
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
		 0.059033f,  0.117240f,  0.173807f,  0.227943f,  0.278890f,  0.325936f,
		 0.368423f,  0.405755f,  0.437413f,  0.462951f,  0.482013f,  0.494333f,
		 0.499738f,  0.498153f,  0.489598f,  0.474195f,  0.452159f,  0.423798f,
		 0.389509f,  0.349771f,  0.289883f,  0.230617f,  0.173194f,  0.118739f,
		 0.068260f,  0.022631f, -0.017423f, -0.051339f,	-0.078721f, -0.099345f,
		-0.113163f, -0.120295f, -0.121028f, -0.115804f, -0.105209f, -0.089954f,
		-0.070862f, -0.048844f
	};

	float bar[CLICK_SIZE] = {
		 0.175860f,  0.341914f,  0.488904f,  0.608633f,  0.694426f,  0.741500f,
		 0.747229f,  0.711293f,  0.635697f,  0.524656f,  0.384362f,  0.222636f,
		 0.048496f, -0.128348f, -0.298035f, -0.451105f, -0.579021f, -0.674653f,
		-0.732667f, -0.749830f, -0.688924f, -0.594091f, -0.474481f, -0.340160f,
	 	-0.201360f, -0.067752f,  0.052194f,  0.151746f,  0.226280f,  0.273493f,
		 0.293425f,  0.288307f,  0.262252f,  0.220811f,  0.170435f,  0.117887f,
		 0.069639f,  0.031320f
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


void rewindQ_(Frame delta)
{
	clock::rewind();
	mixer::pumpEvent({ mixer::EventType::SEQUENCER_REWIND, delta, {} });	
}


/* -------------------------------------------------------------------------- */


void start_()
{
#if defined(G_OS_LINUX) || defined(G_OS_FREEBSD)
	if (kernelAudio::getAPI() == G_SYS_API_JACK)
		kernelAudio::jackStart();
	else
#endif
	start(); 
}


/* -------------------------------------------------------------------------- */


void stop_()
{
#if defined(G_OS_LINUX) || defined(G_OS_FREEBSD)
	if (kernelAudio::getAPI() == G_SYS_API_JACK)
		kernelAudio::jackStop();
	else
#endif
	stop();
}


/* -------------------------------------------------------------------------- */


void rewind_()
{
#if defined(G_OS_LINUX) || defined(G_OS_FREEBSD)
	if (kernelAudio::getAPI() == G_SYS_API_JACK)
		kernelAudio::jackSetPosition(0);
	else
#endif
	rewind();	
}


/* -------------------------------------------------------------------------- */


Quantizer quantizer_;
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void init()
{
	quantizer_.schedule(Q_ACTION_REWIND, rewindQ_);
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
			mixer::pumpEvent({ mixer::EventType::SEQUENCER_FIRST_BEAT, local, { 0, 0, global, {} } });
		else
		if (global % bar == 0)
			mixer::pumpEvent({ mixer::EventType::SEQUENCER_BAR, local, { 0, 0, global, {} } });

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
			start_(); break;
		}
		if (e.type == mixer::EventType::SEQUENCER_STOP) {
			stop_(); break;
		}
		if (e.type == mixer::EventType::SEQUENCER_REWIND_REQ) {
			rewind_(); break;
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
}


/* -------------------------------------------------------------------------- */


void stop()
{
	clock::setStatus(ClockStatus::STOPPED);

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
		rewindQ_(/*delta=*/0);
}


/* -------------------------------------------------------------------------- */


bool isMetronomeOn()      { return metronome_.running; }
void toggleMetronome()    { metronome_.running = !metronome_.running; }
void setMetronome(bool v) { metronome_.running = v; }
}}} // giada::m::sequencer::


