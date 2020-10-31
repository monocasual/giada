/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#include <cassert>
#include <vector>
#include "glue/plugin.h"
#include "glue/events.h"
#include "utils/log.h"
#include "utils/math.h"
#include "core/model/model.h"
#include "core/conf.h"
#include "core/plugins/pluginHost.h"
#include "core/plugins/plugin.h"
#include "core/types.h"
#include "core/midiController.h"
#include "core/midiDispatcher.h"
#include "core/midiPorts.h"

#include "core/midiMsg.h"

namespace giada {
namespace m {
namespace midiController
{
namespace
{

#ifdef WITH_VST

void processPlugins_(const std::vector<ID>& ids, const MidiEvent& midiEvent)
{
	uint32_t pure = midiEvent.getRawNoVelocity();
	float    vf   = u::math::map(midiEvent.getVelocity(), G_MAX_VELOCITY, 1.0f);

	/* Plugins' parameters layout reflects the structure of the matrix
	Channel::midiInPlugins. It is safe to assume then that Plugin 'p' and 
	k indexes match both the structure of Channel::midiInPlugins and the vector
	of plugins. */

	m::model::PluginsLock l(m::model::plugins);

	for (ID id : ids) {
		const m::Plugin& p = m::model::get(m::model::plugins, id);
		for (unsigned k = 0; k < p.midiInParams.size(); k++) {
			if (pure != p.midiInParams.at(k))
				continue;
			c::events::setPluginParameter(id, k, vf, /*gui=*/false);
			u::log::print("  >>> [plugin %d parameter %d] (pure=0x%X, value=%d, float=%f)\n",
				p.id, k, pure, midiEvent.getVelocity(), vf);
		}
	}
}

#endif


/* -------------------------------------------------------------------------- */


void processChannels_(const MidiEvent& midiEvent)
{
	uint32_t pure = midiEvent.getRawNoVelocity();

	model::ChannelsLock lock(model::channels);

	for (const Channel* c : model::channels) {

		/* Do nothing on this channel if MIDI in is disabled or filtered out for
		the current MIDI channel. */

		if (!c->midiLearner.state->isAllowed(midiEvent.getChannel()))
			continue;

		if (pure == c->midiLearner.state->keyPress.load()) {
			u::log::print("  >>> keyPress, ch=%d (pure=0x%X)\n", c->id, pure);
			c::events::pressChannel(c->id, midiEvent.getVelocity(), Thread::MIDI);
		}
		else if (pure == c->midiLearner.state->keyRelease.load()) {
			u::log::print("  >>> keyRel ch=%d (pure=0x%X)\n", c->id, pure);
			c::events::releaseChannel(c->id, Thread::MIDI);
		}
		else if (pure == c->midiLearner.state->mute.load()) {
			u::log::print("  >>> mute ch=%d (pure=0x%X)\n", c->id, pure);
			c::events::toggleMuteChannel(c->id, Thread::MIDI);
		}		
		else if (pure == c->midiLearner.state->kill.load()) {
			u::log::print("  >>> kill ch=%d (pure=0x%X)\n", c->id, pure);
			c::events::killChannel(c->id, Thread::MIDI);
		}		
		else if (pure == c->midiLearner.state->arm.load()) {
			u::log::print("  >>> arm ch=%d (pure=0x%X)\n", c->id, pure);
			c::events::toggleArmChannel(c->id, Thread::MIDI);
		}
		else if (pure == c->midiLearner.state->solo.load()) {
			u::log::print("  >>> solo ch=%d (pure=0x%X)\n", c->id, pure);
			c::events::toggleSoloChannel(c->id, Thread::MIDI);
		}
		else if (pure == c->midiLearner.state->volume.load()) {
			float vf = u::math::map(midiEvent.getVelocity(), G_MAX_VELOCITY, G_MAX_VOLUME); 
			u::log::print("  >>> volume ch=%d (pure=0x%X, value=%d, float=%f)\n", 
				c->id, pure, midiEvent.getVelocity(), vf);
			c::events::setChannelVolume(c->id, vf, Thread::MIDI);
		}
		else if (pure == c->midiLearner.state->pitch.load()) {
			float vf = u::math::map(midiEvent.getVelocity(), G_MAX_VELOCITY, G_MAX_PITCH); 
			u::log::print("  >>> pitch ch=%d (pure=0x%X, value=%d, float=%f)\n",
				c->id, pure, midiEvent.getVelocity(), vf);
			c::events::setChannelPitch(c->id, vf, Thread::MIDI);
		}
		else if (pure == c->midiLearner.state->readActions.load()) {
			u::log::print("  >>> toggle read actions ch=%d (pure=0x%X)\n", c->id, pure);
			c::events::toggleReadActionsChannel(c->id, Thread::MIDI);
		}

#ifdef WITH_VST
		/* Process learned plugins parameters. */
		processPlugins_(c->pluginIds, midiEvent); 
#endif

		/* Redirect raw MIDI message (pure + velocity) to plug-ins in armed
		channels. */

		if (c->state->armed.load() == true)
			c::events::sendMidiToChannel(c->id, midiEvent, Thread::MIDI);
	}
}

} // {anonymous}

/* -------------------------------------------------------------------------- */

void init(){
	midiDispatcher::registerRule(midiPorts::getInPorts(1), MMF_NOTEONOFFCC,
							"m;midiController");
}

/* -------------------------------------------------------------------------- */

void midiReceive(const MidiMsg& mm)
{
	u::log::print("[MC::midiReceive] Received message from %s.\n",
						mm.getMessageSender().c_str());
	MidiEvent midiEvent(mm.getByte(0), mm.getByte(1), mm.getByte(2));
	processChannels_(midiEvent);
}

}}} // giada::m::midiDispatcher::

