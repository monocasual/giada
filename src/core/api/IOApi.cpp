/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "core/api/IOApi.h"
#include "core/midiDispatcher.h"
#include "core/model/model.h"

namespace giada::m
{
IOApi::IOApi(model::Model& m, MidiDispatcher& md)
: m_model(m)
, m_midiDispatcher(md)
{
}

/* -------------------------------------------------------------------------- */

const model::MidiIn& IOApi::getModelMidiIn() const
{
	return m_model.get().midiIn;
}

/* -------------------------------------------------------------------------- */

void IOApi::channel_enableMidiLearn(ID channelId, bool v)
{
	m_model.get().channels.get(channelId).midiLearner.enabled = v;
	m_model.swap(m::model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

void IOApi::channel_enableMidiLightning(ID channelId, bool v)
{
	m_model.get().channels.get(channelId).midiLighter.enabled = v;
	m_model.swap(m::model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

void IOApi::channel_enableMidiOutput(ID channelId, bool v)
{
	m_model.get().channels.get(channelId).midiSender->enabled = v;
	m_model.swap(m::model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

void IOApi::channel_enableVelocityAsVol(ID channelId, bool v)
{
	m_model.get().channels.get(channelId).samplePlayer->velocityAsVol = v;
	m_model.swap(m::model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

void IOApi::channel_setMidiInputFilter(ID channelId, int ch)
{
	m_model.get().channels.get(channelId).midiLearner.filter = ch;
	m_model.swap(m::model::SwapType::NONE);
}

void IOApi::channel_setMidiOutputFilter(ID channelId, int ch)
{
	m_model.get().channels.get(channelId).midiSender->filter = ch;
	m_model.swap(m::model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

bool IOApi::channel_setKey(ID channelId, int k)
{
	m_model.get().channels.get(channelId).key = k;
	m_model.swap(m::model::SwapType::HARD);
	return true;
}

/* -------------------------------------------------------------------------- */

void IOApi::channel_startMidiLearn(int param, ID channelId, std::function<void()> doneCb)
{
	m_midiDispatcher.startChannelLearn(param, channelId, doneCb);
}

void IOApi::master_startMidiLearn(int param, std::function<void()> doneCb)
{
	m_midiDispatcher.startMasterLearn(param, doneCb);
}

void IOApi::plugin_startMidiLearn(int paramIndex, ID pluginId, std::function<void()> doneCb)
{
	m_midiDispatcher.startPluginLearn(paramIndex, pluginId, doneCb);
}

/* -------------------------------------------------------------------------- */

void IOApi::stopMidiLearn()
{
	m_midiDispatcher.stopLearn();
}

/* -------------------------------------------------------------------------- */

void IOApi::channel_clearMidiLearn(int param, ID channelId, std::function<void()> doneCb)
{
	m_midiDispatcher.clearChannelLearn(param, channelId, doneCb);
}

void IOApi::master_clearMidiLearn(int param, std::function<void()> doneCb)
{
	m_midiDispatcher.clearMasterLearn(param, doneCb);
}

void IOApi::plugin_clearMidiLearn(int param, ID pluginId, std::function<void()> doneCb)
{
	m_midiDispatcher.clearPluginLearn(param, pluginId, doneCb);
}

/* -------------------------------------------------------------------------- */

void IOApi::master_enableMidiLearn(bool v)
{
	m_model.get().midiIn.enabled = v;
	m_model.swap(m::model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

void IOApi::master_setMidiFilter(int c)
{
	m_model.get().midiIn.filter = c;
	m_model.swap(m::model::SwapType::NONE);
}
} // namespace giada::m
