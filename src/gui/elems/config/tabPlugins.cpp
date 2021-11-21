/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#ifdef WITH_VST

#include "tabPlugins.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/graphics.h"
#include "glue/layout.h"
#include "glue/plugin.h"
#include "gui/dialogs/window.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/input.h"
#include "utils/gui.h"
#include "utils/string.h"
#include <FL/Fl.H>
#include <functional>

namespace giada::v
{
geTabPlugins::geTabPlugins(int X, int Y, int W, int H)
: Fl_Group(X, Y, W, H, "Plugins")
, m_browse(x() + w() - G_GUI_UNIT, y() + 9, G_GUI_UNIT, G_GUI_UNIT, "", zoomInOff_xpm, zoomInOn_xpm)
, m_folderPath(m_browse.x() - 258, y() + 9, 250, G_GUI_UNIT)
, m_scanButton(x() + w() - 150, m_folderPath.y() + m_folderPath.h() + 8, 150, G_GUI_UNIT)
, m_info(x(), m_scanButton.y() + m_scanButton.h() + 8, w(), 240)
{
	end();

	labelsize(G_GUI_FONT_SIZE_BASE);
	selection_color(G_COLOR_GREY_4);

	m_info.hide();

	m_folderPath.label("Plugins folder");
	m_folderPath.onChange = [this](const std::string& v) {
		m_data.pluginPath = v;
	};

	m_browse.callback(cb_browse, (void*)this);
	m_scanButton.callback(cb_scan, (void*)this);

	rebuild();
}

/* -------------------------------------------------------------------------- */

void geTabPlugins::rebuild()
{
	m_data = c::config::getPluginData();

	const std::string scanLabel = "Scan (" + std::to_string(m_data.numAvailablePlugins) + " found)";
	m_scanButton.copy_label(scanLabel.c_str());

	m_folderPath.value(m_data.pluginPath.c_str());
	m_folderPath.redraw();
}

/* -------------------------------------------------------------------------- */

void geTabPlugins::cb_scan(Fl_Widget* /*w*/, void* p) { ((geTabPlugins*)p)->cb_scan(); }
void geTabPlugins::cb_browse(Fl_Widget* /*w*/, void* p) { ((geTabPlugins*)p)->cb_browse(); }

/* -------------------------------------------------------------------------- */

void geTabPlugins::cb_browse()
{
	c::layout::openBrowserForPlugins(*static_cast<v::gdWindow*>(top_window()));
}

/* -------------------------------------------------------------------------- */

void geTabPlugins::cb_scan()
{
	std::function<void(float)> callback = [this](float progress) {
		std::string l = "Scan in progress (" + std::to_string((int)(progress * 100)) + "%). Please wait...";
		m_info.label(l.c_str());
		Fl::wait();
	};

	m_info.show();
	c::config::scanPlugins(m_folderPath.value(), callback);
	m_info.hide();
	rebuild();
}

/* -------------------------------------------------------------------------- */

void geTabPlugins::save()
{
	c::config::save(m_data);
}
} // namespace giada::v

#endif // WITH_VST