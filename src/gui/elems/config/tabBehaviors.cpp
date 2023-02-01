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

#include "tabBehaviors.h"
#include "core/conf.h"
#include "core/const.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/flex.h"
#include "gui/ui.h"
#include <FL/Fl_Pack.H>

extern giada::v::Ui g_ui;

namespace giada::v
{
geTabBehaviors::geTabBehaviors(geompp::Rect<int> bounds, m::Conf& c)
: Fl_Group(bounds.x, bounds.y, bounds.w, bounds.h, g_ui.getI18Text(LangMap::CONFIG_BEHAVIORS_TITLE))
, m_conf(c)
{
	end();

	geFlex* body = new geFlex(bounds.reduced(G_GUI_OUTER_MARGIN), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		m_chansStopOnSeqHalt         = new geCheck(0, 0, 0, 0, g_ui.getI18Text(LangMap::CONFIG_BEHAVIORS_CHANSSTOPONSEQHALT));
		m_treatRecsAsLoops           = new geCheck(0, 0, 0, 0, g_ui.getI18Text(LangMap::CONFIG_BEHAVIORS_TREATRECSASLOOPS));
		m_inputMonitorDefaultOn      = new geCheck(0, 0, 0, 0, g_ui.getI18Text(LangMap::CONFIG_BEHAVIORS_INPUTMONITORDEFAULTON));
		m_overdubProtectionDefaultOn = new geCheck(0, 0, 0, 0, g_ui.getI18Text(LangMap::CONFIG_BEHAVIORS_OVERDUBPROTECTIONDEFAULTON));

		body->add(m_chansStopOnSeqHalt, 30);
		body->add(m_treatRecsAsLoops, 20);
		body->add(m_inputMonitorDefaultOn, 20);
		body->add(m_overdubProtectionDefaultOn, 30);
		body->end();
	};

	add(body);
	resizable(body);

	m_chansStopOnSeqHalt->value(m_conf.chansStopOnSeqHalt);
	m_treatRecsAsLoops->value(m_conf.treatRecsAsLoops);
	m_inputMonitorDefaultOn->value(m_conf.inputMonitorDefaultOn);
	m_overdubProtectionDefaultOn->value(m_conf.overdubProtectionDefaultOn);
}

/* -------------------------------------------------------------------------- */

void geTabBehaviors::save()
{
	m_conf.chansStopOnSeqHalt         = m_chansStopOnSeqHalt->value();
	m_conf.treatRecsAsLoops           = m_treatRecsAsLoops->value();
	m_conf.inputMonitorDefaultOn      = m_inputMonitorDefaultOn->value();
	m_conf.overdubProtectionDefaultOn = m_overdubProtectionDefaultOn->value();
}
} // namespace giada::v