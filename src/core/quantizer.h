/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_QUANTIZER_H
#define G_QUANTIZER_H

#include "src/core/weakAtomic.h"
#include "src/deps/geompp/src/range.hpp"
#include "src/types.h"
#include <functional>
#include <map>

namespace giada::m
{
class Quantizer
{
public:
	/* schedule
	Schedules a function in slot 'id' to be called at the right time. The
	function has a 'delta' parameter for the buffer offset. */

	void schedule(int id, std::function<void(Frame)>);

	/* trigger
	Triggers the function in slot 'id'. Might start right away, or at the end
	of the quantization step. */

	void trigger(int id);

	/* advance
	Computes the internal state. Wants a range of frames [currentFrame,
	currentFrame + bufferSize) and a quantization step. Call this function
	on each block. */

	void advance(SampleRange block, Frame quantizerStep) const;

	/* clear
	Disables quantized operations in progress, if any. */

	void clear();

	/* hasBeenTriggered
	True if a quantizer function has been triggered(). */

	bool hasBeenTriggered() const;

private:
	std::map<int, std::function<void(Frame)>> m_callbacks;
	mutable WeakAtomic<int>                   m_performId = -1;
};
} // namespace giada::m

#endif
