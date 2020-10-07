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

#include <functional>
#include "utils/log.h"
#include "core/midiDispatcher.h"
#include "core/midiMsg.h"
#include "core/midiMsgFilter.h"
#include "core/midiPorts.h"
#include "core/midiSignalCb.h"

// TODO: multiple callback support
// TODO: persistent callback support

namespace giada {
namespace m {
namespace midiSignalCb
{
namespace
{

std::function<void()>		signalCb_ = nullptr;
MidiMsgFilter			signalCbMmf_ = MidiMsgFilter();

} // {anonymous}

/* -------------------------------------------------------------------------- */

void init() {
	// midiSignalCb should always get messages, and filter them on its own
	// Will be easier to support multiple and persistent callbacks
	// However, signals are limited to port inputs only
	midiDispatcher::registerRule(midiPorts::getInPorts(1),
						MMF_ANY, "m;midiSignalCb");
}

/* -------------------------------------------------------------------------- */

void midiReceive(const MidiMsg& mm)
{
	if (signalCb_ == nullptr) 
		return;
	if (!(signalCbMmf_.check(mm)))
		return;
	signalCb_();
	signalCb_ = nullptr;
}

/* -------------------------------------------------------------------------- */

void setSignalCallback(std::function<void()> f, const MidiMsgFilter& mmf)
{
	signalCb_ = f;
	signalCbMmf_ = mmf;
}

}}} // giada::m::midiSignalCb::

