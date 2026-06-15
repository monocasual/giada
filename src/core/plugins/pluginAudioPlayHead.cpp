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

#include "src/core/plugins/pluginAudioPlayHead.h"
#include "src/core/model/sequencer.h"

namespace giada::m
{
PluginAudioPlayHead::PluginAudioPlayHead(const model::Sequencer& s, int sampleRate)
: m_sequencer(s)
, m_sampleRate(sampleRate)
{
}

/* -------------------------------------------------------------------------- */

juce::Optional<juce::AudioPlayHead::PositionInfo> PluginAudioPlayHead::getPosition() const
{
	juce::AudioPlayHead::PositionInfo info;

	info.setBpm(m_sequencer.getBpm());
	info.setTimeInSamples(m_sequencer.a_getCurrentFrame());
	info.setTimeInSeconds(m_sequencer.a_getCurrentSecond(m_sampleRate));
	info.setPpqPosition(m_sequencer.a_getCurrentQuarterNotePosition(m_sampleRate));
	info.setIsPlaying(m_sequencer.isRunning());

	return {info};
}

/* -------------------------------------------------------------------------- */

bool PluginAudioPlayHead::canControlTransport()
{
	return false;
}
} // namespace giada::m
