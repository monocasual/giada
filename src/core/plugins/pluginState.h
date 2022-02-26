/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifdef WITH_VST

#ifndef G_PLUGIN_STATE_H
#define G_PLUGIN_STATE_H

#include "deps/juce-config.h"
#include <cstddef>
#include <string>

namespace giada::m
{
class PluginState
{
public:
	PluginState() = default; // Invalid state
	PluginState(juce::MemoryBlock&& data);
	PluginState(const std::string& base64);

	std::string asBase64() const;
	const void* getData() const;
	size_t      getSize() const;

private:
	juce::MemoryBlock m_data;
};
} // namespace giada::m

#endif

#endif // #ifdef WITH_VST
