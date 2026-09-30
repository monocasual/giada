/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_MODEL_PLUGIN_MIDI_MAPPING_H
#define G_MODEL_PLUGIN_MIDI_MAPPING_H

#include "src/aliases.h"
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace giada::m::model
{
class Plugin;

class PluginMidiMapping
{
public:
	/* init
	Creates an empty mapping (0x0) for the plug-in, one entry per parameter. */

	void init(const Plugin&);

	/* remove
	Drops the mapping of the plug-in. */

	void remove(ID);

	/* set
	Sets plug-in parameter value at index 'index'. */

	void set(ID, std::size_t index, uint32_t value);

	/* clear
	Removes everything, leaving the map empty. */

	void clear();

	const std::vector<uint32_t>& get(ID) const;
	std::vector<uint32_t>&       get(ID);

private:
	std::unordered_map<ID, std::vector<uint32_t>> m_parameters;
};
} // namespace giada::m::model

#endif
