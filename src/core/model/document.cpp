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

#include "core/model/document.h"
#include "core/actions/actionFactory.h"
#include "core/channels/channelFactory.h"
#include "core/model/shared.h"

namespace giada::m::model
{
void Document::load(const Patch& patch, Shared& shared, float sampleRateRatio)
{
	channels = {};
	for (const Patch::Channel& pchannel : patch.channels)
	{
		Wave*                wave          = shared.findWave(pchannel.waveId);
		std::vector<Plugin*> plugins       = shared.findPlugins(pchannel.pluginIds);
		ChannelShared*       channelShared = shared.findChannel(pchannel.id);
		assert(channelShared != nullptr);

		Channel channel = channelFactory::deserializeChannel(pchannel, *channelShared, sampleRateRatio, wave, plugins);
		channels.add(channel);
	}

	actions.set(actionFactory::deserializeActions(patch.actions));

	sequencer.status    = SeqStatus::STOPPED;
	sequencer.bars      = patch.bars;
	sequencer.beats     = patch.beats;
	sequencer.bpm       = patch.bpm;
	sequencer.quantize  = patch.quantize;
	sequencer.metronome = patch.metronome;
}

/* -------------------------------------------------------------------------- */

void Document::store(Patch& patch) const
{
	patch.bars      = sequencer.bars;
	patch.beats     = sequencer.beats;
	patch.bpm       = sequencer.bpm;
	patch.quantize  = sequencer.quantize;
	patch.metronome = sequencer.metronome;
	patch.actions   = actionFactory::serializeActions(actions.getAll());
	for (const Channel& c : channels.getAll())
		patch.channels.push_back(channelFactory::serializeChannel(c));
}

/* -------------------------------------------------------------------------- */

#ifdef G_DEBUG_MODE
void Document::debug() const
{
	mixer.debug();
	channels.debug();
	actions.debug();
}
#endif
} // namespace giada::m::model
