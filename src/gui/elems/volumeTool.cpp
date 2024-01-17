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

#include "src/gui/elems/volumeTool.h"
#include "core/const.h"
#include "glue/channel.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/basics/textButton.h"
#include "gui/ui.h"
#include "utils/math.h"
#include "utils/string.h"
#include <fmt/core.h>

extern giada::v::Ui* g_ui;

namespace giada::v
{
geVolumeTool::geVolumeTool(ID channelId, float volume, int labelWidth)
: geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN)
, m_channelId(channelId)
{
	m_input = new geInput(g_ui->getI18Text(LangMap::SAMPLEEDITOR_VOLUME), labelWidth);
	m_dial  = new geDial();
	m_reset = new geTextButton(g_ui->getI18Text(LangMap::COMMON_RESET));
	addWidget(m_input);
	addWidget(m_dial, G_GUI_UNIT);
	addWidget(m_reset, 70);
	end();

	m_dial->range(0.0f, 1.0f);
	m_dial->onChange = [this](float val) {
		c::channel::setChannelVolume(m_channelId, val, Thread::MAIN, /*repaintMainUi=*/true);
		update(val, /*fromDial=*/true);
	};

	m_input->setType(FL_FLOAT_INPUT);
	m_input->setWhen(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY); // on focus lost or enter key
	m_input->onChange = [this](const std::string& val) {
		const float valf = c::channel::setChannelVolume(m_channelId, u::math::dBtoLinear(u::string::toFloat(val)),
		    Thread::MAIN, /*repaintMainUi=*/true);
		update(valf, /*fromDial=*/false);
	};

	m_reset->onClick = [this]() {
		c::channel::setChannelVolume(m_channelId, G_DEFAULT_VOL, Thread::MAIN, /*repaintMainUi=*/true);
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