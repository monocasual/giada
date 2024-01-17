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

#ifndef G_IO_API_H
#define G_IO_API_H

#include "core/model/model.h"
#include "core/types.h"
#include <functional>

namespace giada::m
{
class MidiDispatcher;
class IOApi
{
public:
	IOApi(model::Model&, MidiDispatcher&);

	const model::MidiIn& getModelMidiIn() const;

	void channel_enableMidiInput(ID channelId, bool v);
	void channel_enableMidiLightning(ID channelId, bool v);
	void channel_enableMidiOutput(ID channelId, bool v);
	void channel_enableVelocityAsVol(ID channelId, bool v);
	void channel_setMidiInputFilter(ID channelId, int c);
	void channel_setMidiOutputFilter(ID channelId, int c);
	bool channel_setKey(ID channelId, int k);
	void channel_startMidiLearn(int param, ID channelId, std::function<void()> doneCb);
	void channel_clearMidiLearn(int param, ID channelId, std::function<void()> doneCb);

	void master_startMidiLearn(int param, std::function<void()> doneCb);
	void master_clearMidiLearn(int param, std::function<void()> doneCb);
	void master_enableMidiLearn(bool v);
	void master_setMidiFilter(int c);

	void plugin_startMidiLearn(int paramIndex, ID pluginId, std::function<void()> doneCb);
	void plugin_clearMidiLearn(int param, ID pluginId, std::function<void()> doneCb);

	void stopMidiLearn();

private:
	model::Model&   m_model;
	MidiDispatcher& m_midiDispatcher;
};
} // namespace giada::m

#endif
