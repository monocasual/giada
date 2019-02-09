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
#include "../core/init.h"
#include "../core/const.h"
#include "../core/mixer.h"
#include "../core/channel.h"
#include "../glue/transport.h"
#include "../glue/io.h"
#include "elems/mainWindow/keyboard/channel.h"
#include "dispatcher.h"


namespace giada {
namespace v {
namespace dispatcher
{
namespace
{
bool backspace_ = false;
bool end_       = false;
bool enter_     = false;
bool space_     = false;
bool esc_       = false;

std::function<void()> signalCb_ = nullptr;


/* -------------------------------------------------------------------------- */


void perform_(m::Channel* ch, int event)
{
	if (event == FL_KEYDOWN)
		c::io::keyPress(ch, Fl::event_ctrl(), Fl::event_shift(), G_MAX_VELOCITY);
	else
	if (event == FL_KEYUP)	
		c::io::keyRelease(ch, Fl::event_ctrl(), Fl::event_shift());
}


/* -------------------------------------------------------------------------- */


/* Walk channels array, trying to match button's bound key with the event. If 
found, trigger the key-press function. */

void dispatchChannels_(int event)
{
	for (m::Channel* ch : m::mixer::channels)
		if (ch->guiChannel->handleKey(event))
			perform_(ch, event);
}


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


void dispatchKey(int event)
{
	/* These events come from the keyboard, not from a direct interaction on the 
	UI with the mouse/touch. So the 'gui' parameter is set to false. */

	const bool gui = false;

	if (event == FL_KEYDOWN) {
		if (Fl::event_key() == FL_BackSpace && !backspace_) {
			backspace_ = true;
			c::transport::rewindSeq(gui);
		}
		else if (Fl::event_key() == FL_End && !end_) {
			end_ = true;
			c::io::toggleInputRec(gui);
		}
		else if (Fl::event_key() == FL_Enter && !enter_) {
			enter_ = true;
			c::io::toggleActionRec(gui);
		}
		else if (Fl::event_key() == ' ' && !space_) {
			space_ = true;
			c::transport::startStopSeq(gui);
		}
		else if (Fl::event_key() == FL_Escape && !esc_) {
			esc_ = true;
			m::init::closeMainWindow();
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
		else if (Fl::event_key() == FL_Escape)
			esc_ = false;
	}

	dispatchChannels_(event);
}


/* -------------------------------------------------------------------------- */


void dispatchTouch(m::Channel* ch, bool status)
{
	triggerSignalCb_();
	perform_(ch, status ? FL_KEYDOWN : FL_KEYUP);
}


/* -------------------------------------------------------------------------- */


void setSignalCallback(std::function<void()> f)
{
	signalCb_ = f;
}

}}} // giada::v::dispatcher
