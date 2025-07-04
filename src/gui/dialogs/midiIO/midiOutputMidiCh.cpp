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

#include "src/gui/dialogs/midiIO/midiOutputMidiCh.h"
#include "src/glue/io.h"
#include "src/gui/elems/basics/check.h"
#include "src/gui/elems/basics/choice.h"
#include "src/gui/elems/basics/flex.h"
#include "src/gui/elems/basics/textButton.h"
#include "src/gui/elems/midiIO/midiLearner.h"
#include "src/gui/ui.h"
#include "src/utils/gui.h"

extern giada::v::Ui* g_ui;

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
				m_enableOut   = new geCheck(0, 0, 0, 0, g_ui->getI18Text(LangMap::MIDIOUTPUT_CHANNEL_ENABLE));
				m_chanListOut = new geChoice();

				enableOutGroup->addWidget(m_enableOut, 150);
				enableOutGroup->addWidget(new geBox()); // Spacer
				enableOutGroup->addWidget(m_chanListOut);
				enableOutGroup->end();
			}

			m_enableLightning = new geCheck(0, 0, 0, 0, g_ui->getI18Text(LangMap::MIDIOUTPUT_CHANNEL_ENABLE_LIGHTNING));
			m_learners        = new geLightningLearnerPack(0, 0, channelId);

			body->addWidget(enableOutGroup, G_GUI_UNIT);
			body->addWidget(m_enableLightning, G_GUI_UNIT);
			body->addWidget(m_learners);
			body->end();
		}

		geFlex* footer = new geFlex(Direction::HORIZONTAL);
		{
			m_close = new geTextButton(g_ui->getI18Text(LangMap::COMMON_CLOSE));

			footer->addWidget(new geBox()); // Spacer
			footer->addWidget(m_close, 80);
			footer->end();
		}

		container->addWidget(body);
		container->addWidget(footer, G_GUI_UNIT);
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
	m_chanListOut->onChange = [this](ID id)
	{
		c::io::channel_setMidiOutputFilter(m_channelId, id);
	};

	m_enableOut->onChange = [this](bool value)
	{
		c::io::channel_enableMidiOutput(m_channelId, value);
	};

	m_enableLightning->onChange = [this](bool value)
	{
		c::io::channel_enableMidiLightning(m_channelId, value);
	};

	m_close->onClick = [this]()
	{ do_callback(); };

	set_modal();
	rebuild();
	show();
}

/* -------------------------------------------------------------------------- */

void gdMidiOutputMidiCh::rebuild()
{
	c::io::Channel_OutputData data = c::io::channel_getOutputData(m_channelId);

	assert(data.output.has_value());

	m_learners->update(data);
	m_chanListOut->showItem(data.output->filter);
	m_enableOut->value(data.output->enabled);
	m_enableLightning->value(data.lightningEnabled);

	data.output->enabled ? m_chanListOut->activate() : m_chanListOut->deactivate();
}
} // namespace giada::v
