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


#include "../gui/dispatcher.h"
#include "types.h"
#include "clock.h"
#include "kernelAudio.h"
#include "conf.h"
#include "channel.h"
#include "mixer.h"
#include "mixerHandler.h"
#include "midiDispatcher.h"
#include "recorder.h"
#include "recorderHandler.h"
#include "recManager.h"


namespace giada {
namespace m {
namespace recManager
{
namespace
{
pthread_mutex_t* mixerMutex_ = nullptr;
bool isWaiting_ = false;
bool isActive_  = false;


/* -------------------------------------------------------------------------- */


bool startActionRec_()
{
	if (!kernelAudio::getStatus())
		return false;
	recorder::enable();
	clock::start();
#ifdef __linux__
	kernelAudio::jackStart();
#endif
	isWaiting_ = false;
	return true;	
}


/* -------------------------------------------------------------------------- */


bool startInputRec_()
{
	if (!kernelAudio::getStatus() || !mh::startInputRec())
		return false;
	if (!clock::isRunning()) {
		clock::start();
#ifdef __linux__
		kernelAudio::jackStart();
#endif
	}
	isWaiting_ = false;
	return true;
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void init(pthread_mutex_t* mixerMutex)
{
	mixerMutex_ = mixerMutex;
	isWaiting_  = false;
	isActive_   = false;
}


/* -------------------------------------------------------------------------- */


bool isWaiting() { return isWaiting_; }
bool isActive()  { return isActive_; }


/* -------------------------------------------------------------------------- */


bool startActionRec(RecTriggerMode mode)
{
	isActive_ = true;
	if (mode == RecTriggerMode::NORMAL)
		return startActionRec_();
	if (mode == RecTriggerMode::SIGNAL) {
		clock::stop();
		clock::rewind();
		m::midiDispatcher::setSignalCallback(startActionRec_);
		v::dispatcher::setSignalCallback(startActionRec_);
		isWaiting_ = true;
	}
	return true;
}


/* -------------------------------------------------------------------------- */


void stopActionRec()
{
	bool wasWaiting = isWaiting_;

	isActive_  = false;
	isWaiting_ = false;

	if (wasWaiting)
		return;

	recorder::disable();
	std::unordered_set<int> channels = recorderHandler::consolidate();

	/* Enable reading actions for Channels that have just been filled with 
	actions. Start reading right away, without checking whether 
	conf::treatRecsAsLoops is enabled or not. */

	pthread_mutex_lock(mixerMutex_);
	for (int index : channels)
		mh::getChannelByIndex(index)->startReadingActions(
			/*treatRecsAsLoops=*/false, /*recsStopOnChanHalt=*/false); 
	pthread_mutex_unlock(mixerMutex_);
}


/* -------------------------------------------------------------------------- */


bool startInputRec(RecTriggerMode mode)
{
	if (mode == RecTriggerMode::NORMAL)
		isActive_ = startInputRec_();
	if (mode == RecTriggerMode::SIGNAL) {
		clock::stop();
		clock::rewind();
		mixer::setSignalCallback(startInputRec_);
		isWaiting_ = true;
		isActive_  = true;
	}
	return isActive_;
}


/* -------------------------------------------------------------------------- */


void stopInputRec()
{
	bool wasWaiting = isWaiting_;

	isActive_  = false;
	isWaiting_ = false;

	if (!wasWaiting)
		mh::stopInputRec();
}
}}} // giada::m::recManager