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

#ifndef G_PLUGIN_PARAMETER_H
#define G_PLUGIN_PARAMETER_H

#include "src/core/midiLearnParam.h"
#include <string>

namespace juce
{
class AudioProcessorParameter;
}

namespace giada::m
{
class PluginParameter
{
public:
	PluginParameter(juce::AudioProcessorParameter*, std::size_t index);

	std::string getLabel() const;
	std::string getValueAsText() const;
	float       getValue() const;

	void setValue(float);

	std::size_t    index = 0;
	std::string    name;
	MidiLearnParam learnParam;

private:
	juce::AudioProcessorParameter* m_ptr = nullptr;
};
} // namespace giada::m

#endif