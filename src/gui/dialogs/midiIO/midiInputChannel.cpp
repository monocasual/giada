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

#include "gui/dialogs/midiIO/midiInputChannel.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/plugins/plugin.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/choice.h"
#include "gui/elems/basics/group.h"
#include "gui/elems/basics/scrollPack.h"
#include "gui/elems/basics/textButton.h"
#include "gui/elems/midiIO/midiLearner.h"
#include "gui/elems/midiIO/midiLearnerPack.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include "utils/log.h"
#include "utils/string.h"
#include <FL/Fl_Pack.H>
#include <cassert>
#include <cstddef>

extern giada::v::Ui g_ui;

namespace giada::v
{
geChannelLearnerPack::geChannelLearnerPack(int x, int y, const c::io::Channel_InputData& channel)
: geMidiLearnerPack(x, y, g_ui.getI18Text(LangMap::MIDIINPUT_CHANNEL_LEARN_CHANNEL))
{
	setCallbacks(
	    [channelId = channel.channelId](int param) { c::io::channel_startMidiLearn(param, channelId); },
	    [channelId = channel.channelId](int param) { c::io::channel_clearMidiLearn(param, channelId); });
	addMidiLearner(g_ui.getI18Text(LangMap::MIDIINPUT_CHANNEL_LEARN_KEYPRESS), G_MIDI_IN_KEYPRESS);
	addMidiLearner(g_ui.getI18Text(LangMap::MIDIINPUT_CHANNEL_LEARN_KEYREL), G_MIDI_IN_KEYREL);
	addMidiLearner(g_ui.getI18Text(LangMap::MIDIINPUT_CHANNEL_LEARN_KEYKILL), G_MIDI_IN_KILL);
	addMidiLearner(g_ui.getI18Text(LangMap::MIDIINPUT_CHANNEL_LEARN_ARM), G_MIDI_IN_ARM);
	addMidiLearner(g_ui.getI18Text(LangMap::MIDIINPUT_CHANNEL_LEARN_MUTE), G_MIDI_IN_MUTE);
	addMidiLearner(g_ui.getI18Text(LangMap::MIDIINPUT_CHANNEL_LEARN_SOLO), G_MIDI_IN_SOLO);
	addMidiLearner(g_ui.getI18Text(LangMap::MIDIINPUT_CHANNEL_LEARN_VOLUME), G_MIDI_IN_VOLUME);
	addMidiLearner(g_ui.getI18Text(LangMap::MIDIINPUT_CHANNEL_LEARN_PITCH), G_MIDI_IN_PITCH, /*visible=*/channel.channelType == ChannelType::SAMPLE);
	addMidiLearner(g_ui.getI18Text(LangMap::MIDIINPUT_CHANNEL_LEARN_READACTIONS), G_MIDI_IN_READ_ACTIONS, /*visible=*/channel.channelType == ChannelType::SAMPLE);
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
	    [pluginId = plugin.id](int param) { c::io::plugin_startMidiLearn(param, pluginId); },
	    [pluginId = plugin.id](int param) { c::io::plugin_clearMidiLearn(param, pluginId); });

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

gdMidiInputChannel::gdMidiInputChannel(ID channelId, m::Conf::Data& c)
: gdMidiInputBase(g_ui.getI18Text(LangMap::MIDIINPUT_CHANNEL_TITLE), c)
, m_channelId(channelId)
, m_data(c::io::channel_getInputData(channelId))
{
	end();

	/* Header */

	geGroup* groupHeader = new geGroup(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN);
	m_enable             = new geCheck(0, 0, 120, G_GUI_UNIT, g_ui.getI18Text(LangMap::MIDIINPUT_CHANNEL_ENABLE));
	m_channel            = new geChoice(m_enable->x() + m_enable->w() + 44, 0, 120, G_GUI_UNIT);
	m_veloAsVol          = new geCheck(0, m_enable->y() + m_enable->h() + G_GUI_OUTER_MARGIN, w() - 16, G_GUI_UNIT,
        g_ui.getI18Text(LangMap::MIDIINPUT_CHANNEL_VELOCITYDRIVESVOL));
	groupHeader->add(m_enable);
	groupHeader->add(m_channel);
	groupHeader->add(m_veloAsVol);
	groupHeader->resizable(nullptr);

	/* Main scrollable content. */

	m_container = new geScrollPack(G_GUI_OUTER_MARGIN, groupHeader->y() + groupHeader->h() + G_GUI_OUTER_MARGIN,
	    w() - 16, h() - groupHeader->h() - 52);
	m_container->add(new geChannelLearnerPack(0, 0, m_data));

	for (c::io::PluginData& plugin : m_data.plugins)
		m_container->add(new gePluginLearnerPack(0, 0, plugin));

	/* Footer buttons. */

	geGroup* groupButtons = new geGroup(G_GUI_OUTER_MARGIN, m_container->y() + m_container->h() + G_GUI_OUTER_MARGIN);
	geBox*   spacer       = new geBox(0, 0, w() - 80, G_GUI_UNIT); // spacer window border <-> buttons
	m_ok                  = new geTextButton(w() - 96, 0, 80, G_GUI_UNIT, g_ui.getI18Text(LangMap::COMMON_CLOSE));
	groupButtons->add(spacer);
	groupButtons->add(m_ok);
	groupButtons->resizable(spacer);

	m_ok->onClick = [this]() { do_callback(); };
	m_enable->callback(cb_enable, (void*)this);

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
	m_channel->onChange = [this](ID id) {
		c::io::channel_setMidiInputFilter(m_data.channelId, id == 0 ? -1 : id - 1);
	};

	m_veloAsVol->callback(cb_veloAsVol, (void*)this);

	add(groupHeader);
	add(m_container);
	add(groupButtons);
	resizable(m_container);

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

/* -------------------------------------------------------------------------- */

void gdMidiInputChannel::cb_enable(Fl_Widget* /*w*/, void* p) { ((gdMidiInputChannel*)p)->cb_enable(); }
void gdMidiInputChannel::cb_veloAsVol(Fl_Widget* /*w*/, void* p) { ((gdMidiInputChannel*)p)->cb_veloAsVol(); }

/* -------------------------------------------------------------------------- */

void gdMidiInputChannel::cb_enable()
{
	c::io::channel_enableMidiLearn(m_data.channelId, m_enable->value());
}

/* -------------------------------------------------------------------------- */

void gdMidiInputChannel::cb_veloAsVol()
{
	c::io::channel_enableVelocityAsVol(m_data.channelId, m_veloAsVol->value());
}
} // namespace giada::v
