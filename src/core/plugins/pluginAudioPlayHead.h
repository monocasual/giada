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

#ifndef G_PLUGIN_AUDIO_PLAY_HEAD_H
#define G_PLUGIN_AUDIO_PLAY_HEAD_H

#include <juce_audio_basics/juce_audio_basics.h>

namespace giada::m::model
{
class Sequencer;
}

namespace giada::m
{
class PluginAudioPlayHead final : public juce::AudioPlayHead
{
public:
	PluginAudioPlayHead(const model::Sequencer&, int sampleRate);

	juce::Optional<juce::AudioPlayHead::PositionInfo> getPosition() const override;
	bool                                              canControlTransport() override;

private:
	const model::Sequencer& m_sequencer;
	int                     m_sampleRate;
};
} // namespace giada::m

#endif