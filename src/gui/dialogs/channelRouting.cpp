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

#include "src/gui/dialogs/channelRouting.h"
#include "src/glue/channel.h"
#include "src/gui/elems/basics/box.h"
#include "src/gui/elems/basics/check.h"
#include "src/gui/elems/basics/flex.h"
#include "src/gui/elems/basics/textButton.h"
#include "src/gui/elems/panTool.h"
#include "src/gui/elems/volumeTool.h"
#include "src/gui/ui.h"
#include "src/utils/gui.h"

extern giada::v::Ui* g_ui;

namespace giada::v
{
gdChannelRouting::gdChannelRouting(const c::channel::RoutingData& d)
: gdWindow(u::gui::getCenterWinBounds({-1, -1, 260, 120}), g_ui->getI18Text(LangMap::CHANNELROUTING_TITLE), WID_CHANNEL_ROUTING)
{
	constexpr int LABEL_WIDTH = 70;

	geFlex* container = new geFlex(getContentBounds().reduced({G_GUI_OUTER_MARGIN}), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		geFlex* body = new geFlex(Direction::VERTICAL, G_GUI_INNER_MARGIN);
		{
			m_volume       = new geVolumeTool(d.id, d.volume, LABEL_WIDTH);
			m_pan          = new gePanTool(d.id, d.pan, LABEL_WIDTH);
			m_sendToMaster = new geCheck("Send to master");
			body->addWidget(m_volume, G_GUI_UNIT);
			body->addWidget(m_pan, G_GUI_UNIT);
			body->addWidget(new geBox(), G_GUI_INNER_MARGIN);
			body->addWidget(m_sendToMaster, G_GUI_UNIT);
			body->end();
		}

		geFlex* footer = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			m_close = new geTextButton(g_ui->getI18Text(LangMap::COMMON_CLOSE));
			footer->addWidget(new geBox());
			footer->addWidget(m_close, 70);
			footer->end();
		}

		container->addWidget(body);
		container->addWidget(footer, G_GUI_UNIT);
		container->end();
	}

	add(container);

	m_sendToMaster->value(d.sendToMaster);
	m_sendToMaster->onChange = [id = d.id](bool value)
	{
		c::channel::setSendToMaster(id, value);
	};

	m_close->onClick = [this]()
	{ do_callback(); };

	set_modal();
	show();
}
} // namespace giada::v