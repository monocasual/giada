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

#include "src/gui/elems/volumeTool.h"
#include "core/const.h"
#include "glue/channel.h"
#include "glue/events.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/basics/input.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include "utils/math.h"
#include <fmt/core.h>

extern giada::v::Ui g_ui;

namespace giada::v
{
geVolumeTool::geVolumeTool(ID channelId, float volume, int labelWidth)
: geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN)
, m_channelId(channelId)
{
	m_input = new geInput(g_ui.langMapper.get(LangMap::SAMPLEEDITOR_VOLUME), labelWidth);
	m_dial  = new geDial();
	m_reset = new geButton(g_ui.langMapper.get(LangMap::COMMON_RESET));
	add(m_input);
	add(m_dial, G_GUI_UNIT);
	add(m_reset, 70);
	end();

	m_dial->range(0.0f, 1.0f);
	m_dial->onChange = [this](float val) {
		c::events::setChannelVolume(m_channelId, val, Thread::MAIN, /*repaintMainUi=*/true);
		update(val, /*fromDial=*/true);
	};

	m_input->onChange = [this](const std::string& val) {
		const float valf = u::math::dBtoLinear(val == "" ? 0.0 : std::stof(val));
		c::events::setChannelVolume(m_channelId, valf, Thread::MAIN, /*repaintMainUi=*/true);
	};

	m_reset->onClick = [this]() {
		c::events::setChannelVolume(m_channelId, G_DEFAULT_VOL, Thread::MAIN, /*repaintMainUi=*/true);
		update(G_DEFAULT_VOL, /*fromDial=*/false);
	};

	update(volume, /*fromDial=*/false);
}

/* -------------------------------------------------------------------------- */

void geVolumeTool::update(float volume, bool fromDial)
{
	const float dB = u::math::linearToDB(volume);
	m_input->setValue(dB > -INFINITY ? fmt::format("{:.2f}", dB) : "-inf");

	if (!fromDial)
		m_dial->value(volume);
}
} // namespace giada::v