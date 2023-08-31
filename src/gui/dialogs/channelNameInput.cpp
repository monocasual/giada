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

#include "gui/dialogs/channelNameInput.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/model/model.h"
#include "glue/channel.h"
#include "gui/elems/basics/flex.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/basics/textButton.h"
#include "gui/ui.h"
#include "utils/gui.h"

extern giada::v::Ui* g_ui;

namespace giada::v
{
gdChannelNameInput::gdChannelNameInput(const c::channel::Data& d)
: gdWindow(u::gui::getCenterWinBounds({-1, -1, 400, 64}), g_ui->getI18Text(LangMap::CHANNELNAME_TITLE), WID_SAMPLE_NAME)
, m_data(d)
{
	geFlex* container = new geFlex(getContentBounds().reduced({G_GUI_OUTER_MARGIN}), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		m_name = new geInput(0, 0, 0, 0);

		geFlex* footer = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			m_ok     = new geTextButton(g_ui->getI18Text(LangMap::COMMON_OK));
			m_cancel = new geTextButton(g_ui->getI18Text(LangMap::COMMON_CANCEL));
			footer->addWidget(new geBox());
			footer->addWidget(m_cancel, 70);
			footer->addWidget(m_ok, 70);
			footer->end();
		}
		container->addWidget(m_name, G_GUI_UNIT);
		container->addWidget(footer, G_GUI_UNIT);
		container->end();
	}

	add(container);

	m_name->setValue(m_data.name);

	m_ok->shortcut(FL_Enter);
	m_ok->onClick = [this]() {
		c::channel::setName(m_data.id, m_name->getValue());
		do_callback();
	};

	m_cancel->onClick = [this]() {
		do_callback();
	};

	set_modal();
	show();
}
} // namespace giada::v