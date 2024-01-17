/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_MIDI_LEARN_PARAM_H
#define G_MIDI_LEARN_PARAM_H

#include "core/weakAtomic.h"
#include <atomic>
#include <cstddef>

namespace giada::m
{
class MidiLearnParam
{
public:
	MidiLearnParam();
	MidiLearnParam(uint32_t v, std::size_t index = 0);

	uint32_t    getValue() const;
	std::size_t getIndex() const;
	void        setValue(uint32_t v);

private:
	WeakAtomic<uint32_t> m_param;
	std::size_t          m_index;
};
} // namespace giada::m

#endif
