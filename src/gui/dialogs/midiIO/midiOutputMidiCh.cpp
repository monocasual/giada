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

#include "gui/dialogs/midiIO/midiOutputMidiCh.h"
#include "glue/io.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/choice.h"
#include "gui/elems/basics/flex.h"
#include "gui/elems/basics/textButton.h"
#include "gui/elems/midiIO/midiLearner.h"
#include "gui/ui.h"
#include "utils/gui.h"

extern giada::v::Ui g_ui;

namespace giada::v
{
gdMidiOutputMidiCh::gdMidiOutputMidiCh(ID channelId)
: gdMidiOutputBase(350, 168, channelId)
{
	geFlex* container = new geFlex(getContentBounds().reduced({G_GUI_OUTER_MARGIN}), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		geFlex* body = new geFlex(Direction::VERTICAL, G_GUI_OUTER_MARGIN);
		{
			geFlex* enableOutGroup = new geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN);
			{
				m_enableOut   = new geCheck(0, 0, 0, 0, g_ui.getI18Text(LangMap::MIDIOUTPUT_CHANNEL_ENABLE));
				m_chanListOut = new geChoice();

				enableOutGroup->add(m_enableOut, 150);
				enableOutGroup->add(new geBox()); // Spacer
				enableOutGroup->add(m_chanListOut);
				enableOutGroup->end();
			}

			m_enableLightning = new geCheck(0, 0, 0, 0, g_ui.getI18Text(LangMap::MIDIOUTPUT_CHANNEL_ENABLE_LIGHTNING));
			m_learners        = new geLightningLearnerPack(0, 0, channelId);

			body->add(enableOutGroup, G_GUI_UNIT);
			body->add(m_enableLightning, G_GUI_UNIT);
			body->add(m_learners);
			body->end();
		}

		geFlex* footer = new geFlex(Direction::HORIZONTAL);
		{
			m_close = new geTextButton(g_ui.getI18Text(LangMap::COMMON_CLOSE));

			footer->add(new geBox()); // Spacer
			footer->add(m_close, 80);
			footer->end();
		}

		container->add(body);
		container->add(footer, G_GUI_UNIT);
		container->end();
	}

	add(container);
	resizable(nullptr);

	m_chanListOut->addItem("Channel 1");
	m_chanListOut->addItem("Channel 2");
	m_chanListOut->addItem("Channel 3");
	m_chanListOut->addItem("Channel 4");
	m_chanListOut->addItem("Channel 5");
	m_chanListOut->addItem("Channel 6");
	m_chanListOut->addItem("Channel 7");
	m_chanListOut->addItem("Channel 8");
	m_chanListOut->addItem("Channel 9");
	m_chanListOut->addItem("Channel 10");
	m_chanListOut->addItem("Channel 11");
	m_chanListOut->addItem("Channel 12");
	m_chanListOut->addItem("Channel 13");
	m_chanListOut->addItem("Channel 14");
	m_chanListOut->addItem("Channel 15");
	m_chanListOut->addItem("Channel 16");
	m_chanListOut->showItem(0);
	m_chanListOut->onChange = [this](ID id) {
		c::io::channel_setMidiOutputFilter(m_channelId, id);
	};

	m_enableOut->onChange = [this](bool value) {
		c::io::channel_enableMidiOutput(m_channelId, value);
	};

	m_enableLightning->onChange = [this](bool value) {
		c::io::channel_enableMidiLightning(m_channelId, value);
	};

	m_close->onClick = [this]() { do_callback(); };

	set_modal();
	rebuild();
	show();
}

/* -------------------------------------------------------------------------- */

void gdMidiOutputMidiCh::rebuild()
{
	m_data = c::io::channel_getOutputData(m_channelId);

	assert(m_data.output.has_value());

	m_learners->update(m_data);
	m_chanListOut->showItem(m_data.output->filter);
	m_enableOut->value(m_data.output->enabled);

	m_data.output->enabled ? m_chanListOut->activate() : m_chanListOut->deactivate();
}
} // namespace giada::v
