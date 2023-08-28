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

#include "tabMisc.h"
#include "core/const.h"
#include "glue/config.h"
#include "gui/elems/basics/choice.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/config/stringMenu.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include <fmt/core.h>

constexpr int LABEL_WIDTH = 120;

extern giada::v::Ui* g_ui;

namespace giada::v
{
geTabMisc::geTabMisc(geompp::Rect<int> bounds)
: Fl_Group(bounds.x, bounds.y, bounds.w, bounds.h, g_ui->getI18Text(LangMap::CONFIG_MISC_TITLE))
, m_data(c::config::getMiscData())
{
	end();

	geFlex* body = new geFlex(bounds.reduced(G_GUI_OUTER_MARGIN), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		m_debugMsg  = new geChoice(g_ui->getI18Text(LangMap::CONFIG_MISC_DEBUGMESSAGES), LABEL_WIDTH);
		m_tooltips  = new geChoice(g_ui->getI18Text(LangMap::CONFIG_MISC_TOOLTIPS), LABEL_WIDTH);
		m_langMap   = new geStringMenu(g_ui->getI18Text(LangMap::CONFIG_MISC_LANGUAGE),
            g_ui->getI18Text(LangMap::CONFIG_MISC_NOLANGUAGESFOUND), LABEL_WIDTH);
		m_uiScaling = new geChoice(g_ui->getI18Text(LangMap::CONFIG_MISC_UISCALING), LABEL_WIDTH);

		body->add(m_debugMsg, G_GUI_UNIT);
		body->add(m_tooltips, G_GUI_UNIT);
		body->add(m_langMap, G_GUI_UNIT);
		body->add(m_uiScaling, G_GUI_UNIT);
		body->add(new geBox(g_ui->getI18Text(LangMap::CONFIG_RESTARTGIADA)));
		body->end();
	}

	add(body);
	resizable(body);

	m_debugMsg->addItem(g_ui->getI18Text(LangMap::CONFIG_MISC_DEBUGMESSAGES_DISABLED));
	m_debugMsg->addItem(g_ui->getI18Text(LangMap::CONFIG_MISC_DEBUGMESSAGES_TOSTDOUT));
	m_debugMsg->addItem(g_ui->getI18Text(LangMap::CONFIG_MISC_DEBUGMESSAGES_TOFILE));
	m_debugMsg->showItem(m_data.logMode);
	m_debugMsg->onChange = [this](ID id) {
		m_data.logMode = id;
		c::config::save(m_data);
	};

	m_tooltips->addItem(g_ui->getI18Text(LangMap::CONFIG_MISC_TOOLTIPS_DISABLED));
	m_tooltips->addItem(g_ui->getI18Text(LangMap::CONFIG_MISC_TOOLTIPS_ENABLED));
	m_tooltips->showItem(m_data.showTooltips);
	m_tooltips->onChange = [this](ID id) {
		m_data.showTooltips = id;
		c::config::save(m_data);
	};

	m_langMap->rebuild(m_data.langMaps);
	m_langMap->addItem("English (default)");
	if (m_data.langMap == "")
		m_langMap->showItem(0);
	else
		m_langMap->showItem(m_data.langMap);
	m_langMap->onChange = [this](ID /*id*/) {
		m_data.langMap = m_langMap->getSelectedLabel();
		c::config::save(m_data);
	};

	m_uiScaling->addItem("Auto", 0);
	m_uiScaling->addItem("100%", 100);
	m_uiScaling->addItem("150%", 150);
	m_uiScaling->addItem("200%", 200);
	m_uiScaling->addItem("250%", 250);
	m_uiScaling->addItem("300%", 300);
	m_uiScaling->showItem(static_cast<int>(m_data.uiScaling * 100));
	m_uiScaling->onChange = [this](ID id) {
		m_data.uiScaling = id / 100.0f;
		c::config::save(m_data);
	};
}
} // namespace giada::v