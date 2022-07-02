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

#include "panTool.h"
#include "core/const.h"
#include "core/model/model.h"
#include "core/waveFx.h"
#include "glue/events.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include "utils/math.h"
#include "utils/string.h"
#include "waveTools.h"
#include <FL/Fl.H>

extern giada::v::Ui g_ui;

namespace giada::v
{
gePanTool::gePanTool(const c::sampleEditor::Data& d, int x, int y)
: gePack(x, y, Direction::HORIZONTAL)
, m_data(nullptr)
, m_label(0, 0, 60, G_GUI_UNIT, g_ui.langMapper.get(LangMap::SAMPLEEDITOR_PAN), FL_ALIGN_LEFT)
, m_dial(0, 0, G_GUI_UNIT, G_GUI_UNIT)
, m_input(0, 0, 70, G_GUI_UNIT)
, m_reset(0, 0, 70, G_GUI_UNIT, g_ui.langMapper.get(LangMap::COMMON_RESET))
{
	add(&m_label);
	add(&m_dial);
	add(&m_input);
	add(&m_reset);

	m_dial.range(0.0f, G_MAX_PAN);
	m_dial.callback(cb_panning, (void*)this);

	m_input.setReadonly(true);
	m_input.setCursorColor(FL_WHITE);

	m_reset.callback(cb_panReset, (void*)this);

	rebuild(d);
}

/* -------------------------------------------------------------------------- */

void gePanTool::rebuild(const c::sampleEditor::Data& d)
{
	m_data = &d;
	update(m_data->pan);
}

/* -------------------------------------------------------------------------- */

void gePanTool::update(float v)
{
	m_dial.value(v);

	if (v < 0.5f)
	{
		std::string tmp = u::string::iToString((int)((-v * 200.0f) + 100.0f)) + " L";
		m_input.setValue(tmp);
	}
	else if (v == 0.5)
		m_input.setValue("C");
	else
	{
		std::string tmp = u::string::iToString((int)((v * 200.0f) - 100.0f)) + " R";
		m_input.setValue(tmp);
	}
}

/* -------------------------------------------------------------------------- */

void gePanTool::cb_panning(Fl_Widget* /*w*/, void* p) { ((gePanTool*)p)->cb_panning(); }
void gePanTool::cb_panReset(Fl_Widget* /*w*/, void* p) { ((gePanTool*)p)->cb_panReset(); }

/* -------------------------------------------------------------------------- */

void gePanTool::cb_panning()
{
	c::events::sendChannelPan(m_data->channelId, m_dial.value());
}

/* -------------------------------------------------------------------------- */

void gePanTool::cb_panReset()
{
	c::events::sendChannelPan(m_data->channelId, 0.5f);
}
} // namespace giada::v