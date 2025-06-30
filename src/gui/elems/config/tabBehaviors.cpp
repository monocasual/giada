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

#include "gui/elems/config/tabBehaviors.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/flex.h"
#include "gui/ui.h"
#include <FL/Fl_Pack.H>

extern giada::v::Ui* g_ui;

namespace giada::v
{
geTabBehaviors::geTabBehaviors(geompp::Rect<int> bounds)
: Fl_Group(bounds.x, bounds.y, bounds.w, bounds.h, g_ui->getI18Text(LangMap::CONFIG_BEHAVIORS_TITLE))
, m_data(c::config::getBehaviorsData())
{
	end();

	geFlex* body = new geFlex(bounds.reduced(G_GUI_OUTER_MARGIN), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		m_chansStopOnSeqHalt         = new geCheck(0, 0, 0, 0, g_ui->getI18Text(LangMap::CONFIG_BEHAVIORS_CHANSSTOPONSEQHALT));
		m_treatRecsAsLoops           = new geCheck(0, 0, 0, 0, g_ui->getI18Text(LangMap::CONFIG_BEHAVIORS_TREATRECSASLOOPS));
		m_inputMonitorDefaultOn      = new geCheck(0, 0, 0, 0, g_ui->getI18Text(LangMap::CONFIG_BEHAVIORS_INPUTMONITORDEFAULTON));
		m_overdubProtectionDefaultOn = new geCheck(0, 0, 0, 0, g_ui->getI18Text(LangMap::CONFIG_BEHAVIORS_OVERDUBPROTECTIONDEFAULTON));

		body->addWidget(m_chansStopOnSeqHalt, 20);
		body->addWidget(m_treatRecsAsLoops, 20);
		body->addWidget(m_inputMonitorDefaultOn, 20);
		body->addWidget(m_overdubProtectionDefaultOn, 20);
		body->end();
	};

	add(body);
	resizable(body);

	m_chansStopOnSeqHalt->value(m_data.chansStopOnSeqHalt);
	m_chansStopOnSeqHalt->onChange = [this](bool v)
	{
		m_data.chansStopOnSeqHalt = v;
		c::config::save(m_data);
	};

	m_treatRecsAsLoops->value(m_data.treatRecsAsLoops);
	m_treatRecsAsLoops->onChange = [this](bool v)
	{
		m_data.treatRecsAsLoops = v;
		c::config::save(m_data);
	};

	m_inputMonitorDefaultOn->value(m_data.inputMonitorDefaultOn);
	m_inputMonitorDefaultOn->onChange = [this](bool v)
	{
		m_data.inputMonitorDefaultOn = v;
		c::config::save(m_data);
	};

	m_overdubProtectionDefaultOn->value(m_data.overdubProtectionDefaultOn);
	m_overdubProtectionDefaultOn->onChange = [this](bool v)
	{
		m_data.overdubProtectionDefaultOn = v;
		c::config::save(m_data);
	};
}
} // namespace giada::v