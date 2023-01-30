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

#include "src/gui/elems/panTool.h"
#include "core/const.h"
#include "glue/channel.h"
#include "glue/events.h"
#include "gui/ui.h"
#include "src/gui/elems/basics/dial.h"
#include "src/gui/elems/basics/input.h"
#include "src/gui/elems/basics/textButton.h"
#include "utils/gui.h"
#include "utils/math.h"
#include "utils/string.h"
#include <FL/Fl.H>

extern giada::v::Ui g_ui;

namespace giada::v
{
gePanTool::gePanTool(ID channelId, float pan, int labelWidth)
: geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN)
, m_channelId(channelId)
{
	m_input = new geInput(g_ui.getI18Text(LangMap::SAMPLEEDITOR_PAN), labelWidth);
	m_dial  = new geDial();
	m_reset = new geTextButton(g_ui.getI18Text(LangMap::COMMON_RESET));
	add(m_input);
	add(m_dial, G_GUI_UNIT);
	add(m_reset, 70);
	end();

	m_dial->range(0.0f, G_MAX_PAN);
	m_dial->onChange = [this](float val) {
		c::events::sendChannelPan(m_channelId, val);
		update(val);
	};

	m_input->setReadonly(true);
	m_input->setCursorColor(FL_WHITE);

	m_reset->onClick = [this]() {
		c::events::sendChannelPan(m_channelId, 0.5f);
		update(0.5f);
	};

	update(pan);
}

/* -------------------------------------------------------------------------- */

void gePanTool::update(float pan)
{
	m_dial->value(pan);

	if (pan < 0.5f)
	{
		std::string tmp = u::string::iToString((int)((-pan * 200.0f) + 100.0f)) + " L";
		m_input->setValue(tmp);
	}
	else if (pan == 0.5)
		m_input->setValue("C");
	else
	{
		std::string tmp = u::string::iToString((int)((pan * 200.0f) - 100.0f)) + " R";
		m_input->setValue(tmp);
	}
}
} // namespace giada::v