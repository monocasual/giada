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


#include "../gui/keyDispatcher.h"
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
	return true;
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void init(pthread_mutex_t* mixerMutex)
{
	mixerMutex_ = mixerMutex;
}


/* -------------------------------------------------------------------------- */


bool startActionRec(RecTriggerMode mode)
{
	if (mode == RecTriggerMode::NORMAL)
		return startActionRec_();
	if (mode == RecTriggerMode::SIGNAL) {
		m::midiDispatcher::setSignalCallback(startActionRec_);
		v::keyDispatcher::setSignalCallback(startActionRec_);
	}
	return true;
}


/* -------------------------------------------------------------------------- */


void stopActionRec()
{
	recorder::disable();
	recorderHandler::consolidate();

	/* Enable reading actions for Sample Channels that has just been filled
	with action. */

	pthread_mutex_lock(mixerMutex_);

	/* TODO - this is wrong. This enables ALL channels with actions and R disabled. I don't want
	that my channel with disabled action gets started! 
	Instead, get a vector of channels in recorderHandler::consolidate that have been 
	recorded and startReadingAction on them. */

	//for (m::Channel* ch : m::mixer::channels)
		//if (ch->type == ChannelType::SAMPLE && !ch->readActions && ch->hasActions)
		//	ch->startReadingActions(conf::treatRecsAsLoops, conf::recsStopOnChanHalt); 
	pthread_mutex_unlock(mixerMutex_);
}


/* -------------------------------------------------------------------------- */


bool startInputRec(RecTriggerMode mode)
{
	if (mode == RecTriggerMode::NORMAL)
		return startInputRec_();
	if (mode == RecTriggerMode::SIGNAL)
		mixer::setSignalCallback(startInputRec_);
	return true;
}


/* -------------------------------------------------------------------------- */


void stopInputRec()
{
	mh::stopInputRec();
}
}}} // giada::m::recManager