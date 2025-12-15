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

#include "src/core/model/midiIn.h"
#include "src/deps/mcl-utils/src/container.hpp"

namespace utils = mcl::utils;

namespace giada::m::model
{
bool MidiIn::hasScene(uint32_t val) const
{
	return utils::container::has(scenes, val);
}

/* -------------------------------------------------------------------------- */

Scene MidiIn::getScene(uint32_t val) const
{
	assert(hasScene(val));

	return Scene{static_cast<std::size_t>(utils::container::indexOf(scenes, val))};
}

/* -------------------------------------------------------------------------- */

void MidiIn::setScene(Scene scene, uint32_t val)
{
	assert(scene.index < scenes.size());

	scenes[scene.index] = val;
}
} // namespace giada::m::model
