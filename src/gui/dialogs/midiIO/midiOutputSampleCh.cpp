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

#include "gui/dialogs/midiIO/midiOutputSampleCh.h"
#include "core/model/model.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/flex.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/textButton.h"
#include "gui/elems/midiIO/midiLearner.h"
#include "gui/ui.h"
#include "utils/gui.h"

extern giada::v::Ui* g_ui;

namespace giada::v
{
gdMidiOutputSampleCh::gdMidiOutputSampleCh(ID channelId)
: gdMidiOutputBase(350, 140, channelId)
{
	geFlex* container = new geFlex(getContentBounds().reduced({G_GUI_OUTER_MARGIN}), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		geFlex* body = new geFlex(Direction::VERTICAL, G_GUI_OUTER_MARGIN);
		{
			m_enableLightning = new geCheck(0, 0, 0, 0, g_ui->getI18Text(LangMap::MIDIOUTPUT_CHANNEL_ENABLE_LIGHTNING));
			m_learners        = new geLightningLearnerPack(0, 0, channelId);

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

	m_close->onClick = [this]() { do_callback(); };

	m_enableLightning->onChange = [this](bool value) {
		c::io::channel_enableMidiLightning(m_channelId, value);
	};

	set_modal();
	rebuild();
	show();
}

/* -------------------------------------------------------------------------- */

void gdMidiOutputSampleCh::rebuild()
{
	c::io::Channel_OutputData data = c::io::channel_getOutputData(m_channelId);

	m_enableLightning->value(data.lightningEnabled);
	m_learners->update(data);
}
} // namespace giada::v
