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

#include "gui/dialogs/progress.h"
#include "core/const.h"
#include "deps/geompp/src/rect.hpp"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/flex.h"
#include "gui/elems/basics/progress.h"
#include "gui/elems/basics/textButton.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include <FL/Fl.H>

extern giada::v::Ui* g_ui;

namespace giada::v
{
gdProgress::gdProgress()
: gdWindow(u::gui::getCenterWinBounds({-1, -1, 388, 58}))
, onCancel(nullptr)
{
	geFlex* container = new geFlex(getContentBounds().reduced({G_GUI_OUTER_MARGIN}), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		m_text = new geBox();

		geFlex* progressArea = new geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN);
		{
			m_progress = new geProgress();
			m_progress->minimum(0.0f);
			m_progress->maximum(1.0f);
			m_progress->value(0.0f);

			m_cancelBtn = new geTextButton(g_ui->getI18Text(LangMap::COMMON_CANCEL));

			progressArea->addWidget(m_progress);
			progressArea->addWidget(m_cancelBtn, 80);
			progressArea->end();
		}

		container->addWidget(m_text);
		container->addWidget(progressArea, G_GUI_UNIT);
		container->end();
	}

	add(container);

	m_cancelBtn->onClick = [this]() {
		if (onCancel)
			onCancel();
	};

	hide();
	border(0);
	set_modal();
}

/* -------------------------------------------------------------------------- */

void gdProgress::setProgress(float p)
{
	m_progress->value(p);
	redraw();
	Fl::flush();
}

/* -------------------------------------------------------------------------- */

void gdProgress::popup(const char* s, bool cancellable)
{
	m_text->copy_label(s);

	if (cancellable)
		m_cancelBtn->show();
	else
		m_cancelBtn->hide();

	const int px = u::gui::centerWindowX(w());
	const int py = u::gui::centerWindowY(h());

	position(px, py);
	show();
	wait_for_expose(); // No async bullshit, show it right away
	Fl::flush();       // Make sure everything is displayed
}
} // namespace giada::v
