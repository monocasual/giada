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

#include "volumeTool.h"
#include "core/const.h"
#include "glue/events.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include "utils/math.h"
#include "utils/string.h"
#include <FL/Fl_Pack.H>
#include <cmath>
#include <cstdlib>

extern giada::v::Ui g_ui;

namespace giada::v
{
geVolumeTool::geVolumeTool(const c::sampleEditor::Data& d, int x, int y)
: gePack(x, y, Direction::HORIZONTAL)
, m_data(nullptr)
, m_label(0, 0, 60, G_GUI_UNIT, g_ui.langMapper.get(LangMap::SAMPLEEDITOR_VOLUME), FL_ALIGN_LEFT)
, m_dial(0, 0, G_GUI_UNIT, G_GUI_UNIT)
, m_input(0, 0, 70, G_GUI_UNIT)
{
	add(&m_label);
	add(&m_dial);
	add(&m_input);

	m_dial.range(0.0f, 1.0f);
	m_dial.onChange = [this](float val) {
		c::events::setChannelVolume(m_data->channelId, val, Thread::MAIN);
	};

	m_input.onChange = [this](const std::string& val) {
		c::events::setChannelVolume(m_data->channelId, u::math::dBtoLinear(val == "" ? 0.0 : std::stof(val)), Thread::MAIN);
	};

	rebuild(d);
}

/* -------------------------------------------------------------------------- */

void geVolumeTool::rebuild(const c::sampleEditor::Data& d)
{
	m_data = &d;
	update(m_data->volume, /*isDial=*/false);
}

/* -------------------------------------------------------------------------- */

void geVolumeTool::update(float v, bool isDial)
{
	std::string tmp = "-inf";
	float       dB  = u::math::linearToDB(v);
	if (dB > -INFINITY)
		tmp = u::string::fToString(dB, 2); // 2 digits
	m_input.setValue(tmp);
	if (!isDial)
		m_dial.value(v);
}
} // namespace giada::v