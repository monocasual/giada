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

#include "gui/dialogs/midiIO/midiInputChannel.h"
#include "core/const.h"
#include "core/plugins/plugin.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/choice.h"
#include "gui/elems/basics/flex.h"
#include "gui/elems/basics/group.h"
#include "gui/elems/basics/scrollPack.h"
#include "gui/elems/basics/textButton.h"
#include "gui/elems/midiIO/midiLearner.h"
#include "gui/elems/midiIO/midiLearnerPack.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include "utils/log.h"
#include "utils/string.h"
#include <cassert>
#include <cstddef>

extern giada::v::Ui* g_ui;

namespace giada::v
{
geChannelLearnerPack::geChannelLearnerPack(int x, int y, const c::io::Channel_InputData& channel)
: geMidiLearnerPack(x, y, g_ui->getI18Text(LangMap::MIDIINPUT_CHANNEL_LEARN_CHANNEL))
{
	setCallbacks(
	    [channelId = channel.channelId](int param)
	{ c::io::channel_startMidiLearn(param, channelId); },
	    [channelId = channel.channelId](int param)
	{ c::io::channel_clearMidiLearn(param, channelId); });
	addMidiLearner(g_ui->getI18Text(LangMap::MIDIINPUT_CHANNEL_LEARN_KEYPRESS), G_MIDI_IN_KEYPRESS);
	addMidiLearner(g_ui->getI18Text(LangMap::MIDIINPUT_CHANNEL_LEARN_KEYREL), G_MIDI_IN_KEYREL);
	addMidiLearner(g_ui->getI18Text(LangMap::MIDIINPUT_CHANNEL_LEARN_KEYKILL), G_MIDI_IN_KILL);
	addMidiLearner(g_ui->getI18Text(LangMap::MIDIINPUT_CHANNEL_LEARN_ARM), G_MIDI_IN_ARM);
	addMidiLearner(g_ui->getI18Text(LangMap::MIDIINPUT_CHANNEL_LEARN_MUTE), G_MIDI_IN_MUTE);
	addMidiLearner(g_ui->getI18Text(LangMap::MIDIINPUT_CHANNEL_LEARN_SOLO), G_MIDI_IN_SOLO);
	addMidiLearner(g_ui->getI18Text(LangMap::MIDIINPUT_CHANNEL_LEARN_VOLUME), G_MIDI_IN_VOLUME);
	addMidiLearner(g_ui->getI18Text(LangMap::MIDIINPUT_CHANNEL_LEARN_PITCH), G_MIDI_IN_PITCH, /*visible=*/channel.channelType == ChannelType::SAMPLE);
	addMidiLearner(g_ui->getI18Text(LangMap::MIDIINPUT_CHANNEL_LEARN_READACTIONS), G_MIDI_IN_READ_ACTIONS, /*visible=*/channel.channelType == ChannelType::SAMPLE);
}

/* -------------------------------------------------------------------------- */

void geChannelLearnerPack::update(const c::io::Channel_InputData& d)
{
	learners[0]->update(d.keyPress);
	learners[1]->update(d.keyRelease);
	learners[2]->update(d.kill);
	learners[3]->update(d.arm);
	learners[4]->update(d.mute);
	learners[5]->update(d.solo);
	learners[6]->update(d.volume);
	learners[7]->update(d.pitch);
	learners[8]->update(d.readActions);
	setEnabled(d.enabled);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gePluginLearnerPack::gePluginLearnerPack(int x, int y, const c::io::PluginData& plugin)
: geMidiLearnerPack(x, y, plugin.name)
{
	setCallbacks(
	    [pluginId = plugin.id](int param)
	{ c::io::plugin_startMidiLearn(param, pluginId); },
	    [pluginId = plugin.id](int param)
	{ c::io::plugin_clearMidiLearn(param, pluginId); });

	for (const c::io::PluginParamData& param : plugin.params)
		addMidiLearner(param.name, param.index);
}

/* -------------------------------------------------------------------------- */

void gePluginLearnerPack::update(const c::io::PluginData& d, bool enabled)
{
	std::size_t i = 0;
	for (const c::io::PluginParamData& param : d.params)
		learners[i++]->update(param.value);
	setEnabled(enabled);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gdMidiInputChannel::gdMidiInputChannel(ID channelId, const Model& model)
: gdMidiInputBase(g_ui->getI18Text(LangMap::MIDIINPUT_CHANNEL_TITLE), model)
, m_channelId(channelId)
, m_data(c::io::channel_getInputData(channelId))
{
	geFlex* container = new geFlex(getContentBounds().reduced({G_GUI_OUTER_MARGIN}), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		geFlex* enableGroup = new geFlex(Direction::HORIZONTAL);
		{
			m_enable  = new geCheck(0, 0, 0, 0, g_ui->getI18Text(LangMap::MIDIINPUT_CHANNEL_ENABLE));
			m_channel = new geChoice();

			enableGroup->addWidget(m_enable, geMidiLearnerPack::LEARNER_WIDTH - 120);
			enableGroup->addWidget(m_channel, 120);
			enableGroup->end();
		}

		m_veloAsVol = new geCheck(0, 0, 0, 0, g_ui->getI18Text(LangMap::MIDIINPUT_CHANNEL_VELOCITYDRIVESVOL));

		m_container = new geScrollPack(0, 0, 0, 0);
		m_container->add(new geChannelLearnerPack(0, 0, m_data));
		for (c::io::PluginData& plugin : m_data.plugins)
			m_container->add(new gePluginLearnerPack(0, 0, plugin));

		geFlex* footer = new geFlex(Direction::HORIZONTAL);
		{
			m_ok = new geTextButton(g_ui->getI18Text(LangMap::COMMON_CLOSE));

			footer->addWidget(new geBox()); // Spacer
			footer->addWidget(m_ok, 80);
			footer->end();
		}

		container->addWidget(enableGroup, G_GUI_UNIT);
		container->addWidget(m_veloAsVol, G_GUI_UNIT);
		container->addWidget(m_container);
		container->addWidget(footer, G_GUI_UNIT);
		container->end();
	}

	add(container);
	resizable(container);

	m_ok->onClick = [this]()
	{ do_callback(); };

	m_enable->onChange = [this](bool value)
	{
		c::io::channel_enableMidiLearn(m_data.channelId, value);
	};

	m_channel->addItem("Channel (any)");
	m_channel->addItem("Channel 1");
	m_channel->addItem("Channel 2");
	m_channel->addItem("Channel 3");
	m_channel->addItem("Channel 4");
	m_channel->addItem("Channel 5");
	m_channel->addItem("Channel 6");
	m_channel->addItem("Channel 7");
	m_channel->addItem("Channel 8");
	m_channel->addItem("Channel 9");
	m_channel->addItem("Channel 10");
	m_channel->addItem("Channel 11");
	m_channel->addItem("Channel 12");
	m_channel->addItem("Channel 13");
	m_channel->addItem("Channel 14");
	m_channel->addItem("Channel 15");
	m_channel->addItem("Channel 16");
	m_channel->onChange = [this](ID id)
	{
		c::io::channel_setMidiInputFilter(m_data.channelId, id == 0 ? -1 : id - 1);
	};

	m_veloAsVol->onChange = [this](bool value)
	{
		c::io::channel_enableVelocityAsVol(m_data.channelId, value);
	};

	set_modal();
	rebuild();
	show();
}

/* -------------------------------------------------------------------------- */

void gdMidiInputChannel::rebuild()
{
	m_data = c::io::channel_getInputData(m_channelId);

	m_enable->value(m_data.enabled);

	if (m_data.channelType == ChannelType::SAMPLE)
	{
		m_veloAsVol->activate();
		m_veloAsVol->value(m_data.velocityAsVol);
	}
	else
		m_veloAsVol->deactivate();

	int i = 0;
	static_cast<geChannelLearnerPack*>(m_container->getChild(i++))->update(m_data);

	for (c::io::PluginData& plugin : m_data.plugins)
		static_cast<gePluginLearnerPack*>(m_container->getChild(i++))->update(plugin, m_data.enabled);

	m_channel->showItem(m_data.filter == -1 ? 0 : m_data.filter + 1);

	if (m_data.enabled)
	{
		m_channel->activate();
		if (m_data.channelType == ChannelType::SAMPLE)
			m_veloAsVol->activate();
	}
	else
	{
		m_channel->deactivate();
		m_veloAsVol->deactivate();
	}
}
} // namespace giada::v
