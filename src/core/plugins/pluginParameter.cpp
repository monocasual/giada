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

#include "src/core/plugins/pluginParameter.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace giada::m
{
constexpr int MAX_PARAMETER_NAME_LENGTH = 64;

/* -------------------------------------------------------------------------- */

PluginParameter::PluginParameter(juce::AudioProcessorParameter* ptr, std::size_t index)
: index(index)
, name(ptr->getName(MAX_PARAMETER_NAME_LENGTH).toStdString())
, learnParam(0x0, index)
, m_ptr(ptr)
{
	/* MidiInParam is set empty (0x0) initially: it will be filled during MIDI
	learning process. */
}

/* -------------------------------------------------------------------------- */

std::string PluginParameter::getLabel() const { return m_ptr->getLabel().toStdString(); }
std::string PluginParameter::getValueAsText() const { return m_ptr->getCurrentValueAsText().toStdString(); }
void        PluginParameter::setValue(float f) { m_ptr->setValue(f); }
float       PluginParameter::getValue() const { return m_ptr->getValue(); }
} // namespace giada::m
