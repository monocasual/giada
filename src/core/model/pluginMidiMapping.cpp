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

#include "src/core/model/pluginMidiMapping.h"
#include "src/core/model/plugin.h"

namespace giada::m::model
{
void PluginMidiMapping::init(const Plugin& p)
{
	m_parameters[p.id] = std::vector<uint32_t>(p.getParameters().size(), 0x0);
}

/* -------------------------------------------------------------------------- */

void PluginMidiMapping::remove(ID id) { m_parameters.erase(id); }

/* -------------------------------------------------------------------------- */

void PluginMidiMapping::set(ID id, std::size_t index, uint32_t value)
{
	get(id).at(index) = value;
}

/* -------------------------------------------------------------------------- */

void PluginMidiMapping::clear() { m_parameters.clear(); }

/* -------------------------------------------------------------------------- */

const std::vector<uint32_t>& PluginMidiMapping::get(ID id) const
{
	return m_parameters.at(id);
}

/* -------------------------------------------------------------------------- */

std::vector<uint32_t>& PluginMidiMapping::get(ID id)
{
	return m_parameters.at(id);
}
} // namespace giada::m::model
