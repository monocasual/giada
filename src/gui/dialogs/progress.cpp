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

#include "gui/dialogs/progress.h"
#include "core/const.h"
#include "deps/geompp/src/rect.hpp"
#include "utils/gui.h"
#include <FL/Fl.H>

namespace giada::v
{
gdProgress::gdProgress()
: gdWindow(300, 58)
, m_text(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN, w() - (G_GUI_OUTER_MARGIN * 2), 30, "", FL_ALIGN_CENTER)
, m_progress(G_GUI_OUTER_MARGIN, 40, w() - (G_GUI_OUTER_MARGIN * 2), 10)
{
	end();
	add(m_text);
	add(m_progress);

	m_progress.minimum(0.0f);
	m_progress.maximum(1.0f);
	m_progress.value(0.0f);

	hide();
	border(0);
	set_modal();
}

/* -------------------------------------------------------------------------- */

void gdProgress::setProgress(float p)
{
	m_progress.value(p);
	redraw();
	Fl::flush();
}

/* -------------------------------------------------------------------------- */

void gdProgress::popup(const char* s)
{
	m_text.copy_label(s);

	const int px = u::gui::centerWindowX(w());
	const int py = u::gui::centerWindowY(h());

	position(px, py);
	show();
	wait_for_expose(); // No async bullshit, show it right away
	Fl::flush();       // Make sure everything is displayed
}
} // namespace giada::v
