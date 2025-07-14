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

#include "src/core/plugins/pluginState.h"
#include "src/utils/log.h"
#include <cstddef>

namespace giada::m
{
PluginState::PluginState(juce::MemoryBlock&& data)
: m_data(std::move(data))
{
}

/* -------------------------------------------------------------------------- */

PluginState::PluginState(const std::string& base64)
{
	bool res = m_data.fromBase64Encoding(base64);
	if (!res)
		G_DEBUG("Error while loading plug-in state!", );
}

/* -------------------------------------------------------------------------- */

std::string PluginState::asBase64() const
{
	return m_data.toBase64Encoding().toStdString();
}

/* -------------------------------------------------------------------------- */

const void* PluginState::getData() const
{
	return m_data.getData();
}

size_t PluginState::getSize() const
{
	return m_data.getSize();
}
} // namespace giada::m