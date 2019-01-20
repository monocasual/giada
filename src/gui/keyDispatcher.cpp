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


#include <FL/Fl.H>
#include "../core/mixer.h"
#include "../core/channel.h"
#include "../glue/transport.h"
#include "../glue/io.h"
#include "elems/mainWindow/keyboard/channel.h"
#include "keyDispatcher.h"


namespace giada {
namespace v {
namespace keyDispatcher
{
namespace
{
bool backspace_ = false;
bool end_       = false;
bool enter_     = false;
bool space_     = false;

std::function<void()> signalCb_ = nullptr;


/* -------------------------------------------------------------------------- */


void triggerSignalCb_()
{
	if (signalCb_ == nullptr) 
		return;
	signalCb_();
	signalCb_ = nullptr;
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void dispatch(int event)
{
	if (event == FL_KEYDOWN) {
		if (Fl::event_key() == FL_BackSpace && !backspace_) {
			backspace_ = true;
			c::transport::rewindSeq(/*gui=*/false);
		}
		else if (Fl::event_key() == FL_End && !end_) {
			end_ = true;
			c::io::toggleInputRec(/*gui=*/false);
		}
		else if (Fl::event_key() == FL_Enter && !enter_) {
			enter_ = true;
			c::io::toggleActionRec(/*gui=*/false);
		}
		else if (Fl::event_key() == ' ' && !space_) {
			space_ = true;
			c::transport::startStopSeq(/*gui=*/false);
		}
		else
			triggerSignalCb_();
	}
	else if (event == FL_KEYUP) {
		if (Fl::event_key() == FL_BackSpace)
			backspace_ = false;
		else if (Fl::event_key() == FL_End)
			end_ = false;
		else if (Fl::event_key() == ' ')
			space_ = false;
		else if (Fl::event_key() == FL_Enter)
			enter_ = false;
	}

	/* Walk button arrays, trying to match button's label with the Keyboard 
	event. If found, set that button's value() based on up/down event, and
	invoke that button's callback(). */

	for (m::Channel* ch : m::mixer::channels)
		ch->guiChannel->handleKey(event, ch->key);
}


/* -------------------------------------------------------------------------- */


void setSignalCallback(std::function<void()> f)
{
	signalCb_ = f;
}

}}} // giada::v::keyDispatcher
