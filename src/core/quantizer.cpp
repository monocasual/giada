/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#include "quantizer.h"
#include <cassert>

namespace giada::m
{
void Quantizer::trigger(int id)
{
	assert(m_callbacks.count(id) > 0); // Make sure id exists

	m_performId.store(id);
}

/* -------------------------------------------------------------------------- */

void Quantizer::schedule(int id, std::function<void(Frame delta)> f)
{
	m_callbacks[id] = f;
}

/* -------------------------------------------------------------------------- */

void Quantizer::advance(Range<Frame> block, Frame quantizerStep)
{
	/* Nothing to do if there's no action to perform. */

	const int pid = m_performId.load();

	if (pid == -1)
		return;

	assert(m_callbacks.count(pid) > 0);

	for (Frame global = block.getBegin(), local = 0; global < block.getEnd(); global++, local++)
	{

		if (global % quantizerStep != 0) // Skip if it's not on a quantization unit.
			continue;

		m_callbacks.at(pid)(local);
		m_performId.store(-1);
		return;
	}
}

/* -------------------------------------------------------------------------- */

void Quantizer::clear()
{
	m_performId.store(-1);
}

/* -------------------------------------------------------------------------- */

bool Quantizer::hasBeenTriggered() const
{
	return m_performId.load() != -1;
}
} // namespace giada::m