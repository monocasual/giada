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

#include "tabPlugins.h"
#include "core/conf.h"
#include "core/const.h"
#include "glue/layout.h"
#include "glue/plugin.h"
#include "gui/dialogs/window.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/flex.h"
#include "gui/elems/basics/imageButton.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/basics/textButton.h"
#include "gui/graphics.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include "utils/string.h"
#include <FL/Fl.H>
#include <fmt/core.h>
#include <functional>

extern giada::v::Ui* g_ui;

namespace giada::v
{
geTabPlugins::geTabPlugins(geompp::Rect<int> bounds)
: Fl_Group(bounds.x, bounds.y, bounds.w, bounds.h, g_ui->getI18Text(LangMap::CONFIG_PLUGINS_TITLE))
{
	end();

	geFlex* body = new geFlex(bounds.reduced(G_GUI_OUTER_MARGIN), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		geFlex* line1 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			m_folderPath = new geInput(g_ui->getI18Text(LangMap::CONFIG_PLUGINS_FOLDER));
			m_browse     = new geImageButton(graphics::plusOff, graphics::plusOn);

			line1->addWidget(m_folderPath);
			line1->addWidget(m_browse, 20);
			line1->end();
		}

		m_scanButton = new geTextButton("");
		m_info       = new geBox();

		body->addWidget(line1, 20);
		body->addWidget(m_scanButton, 20);
		body->addWidget(m_info);
		body->end();
	}

	add(body);
	resizable(body);

	m_info->hide();

	m_folderPath->onChange = [this](const std::string& v) {
		m_data.pluginPath = v;
		c::config::save(m_data);
	};

	m_browse->onClick = [this]() {
		c::layout::openBrowserForPlugins(*static_cast<v::gdWindow*>(top_window()));
	};

	m_scanButton->onClick = [this]() {
		std::function<void(float)> callback = [this](float progress) {
			std::string l = fmt::format(fmt::runtime(g_ui->getI18Text(LangMap::CONFIG_PLUGINS_SCANNING)), static_cast<int>(progress * 100));
			m_info->label(l.c_str());
			Fl::wait();
		};

		m_info->show();
		c::config::scanPlugins(m_folderPath->getValue(), callback);
		m_info->hide();
		rebuild();
	};

	rebuild();
}

/* -------------------------------------------------------------------------- */

void geTabPlugins::rebuild()
{
	m_data = c::config::getPluginData();

	const std::string scanLabel = fmt::format(fmt::runtime(g_ui->getI18Text(LangMap::CONFIG_PLUGINS_SCAN)), m_data.numAvailablePlugins);
	m_scanButton->copy_label(scanLabel.c_str());

	m_folderPath->setValue(m_data.pluginPath);
	m_folderPath->redraw();
}
} // namespace giada::v