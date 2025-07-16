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
#include "src/gui/elems/basics/box.h"
#include "src/gui/elems/basics/check.h"
#include "src/gui/elems/basics/choice.h"
#include "src/gui/elems/basics/flex.h"
#include "src/gui/elems/basics/imageButton.h"
#include "src/gui/elems/basics/liquidScroll.h"
#include "src/gui/elems/basics/textButton.h"
#include "src/gui/elems/panTool.h"
#include "src/gui/elems/volumeTool.h"
#include "src/gui/graphics.h"
#include "src/gui/ui.h"
#include "src/utils/gui.h"
#include <fmt/core.h>
#include <ranges>

extern giada::v::Ui* g_ui;

namespace giada::v
{
namespace
{
class geOutput : public geFlex
{
public:
	geOutput(const std::string& l, ID channelId, std::size_t index)
	: geFlex(0, 0, 0, G_GUI_UNIT, Direction::HORIZONTAL, G_GUI_INNER_MARGIN)
	{
		m_labelBox  = new geBox(l.c_str());
		m_removeBtn = new geImageButton(graphics::removeOff, graphics::removeOn);
		addWidget(m_labelBox);
		addWidget(m_removeBtn, G_GUI_UNIT);
		end();

		m_labelBox->box(FL_BORDER_BOX);

		m_removeBtn->onClick = [channelId, index]()
		{
			c::channel::removeExtraOutput(channelId, index);
		};
	}

private:
	geBox*         m_labelBox;
	geImageButton* m_removeBtn;
};
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gdChannelRouting::gdChannelRouting(ID channelId)
: gdWindow(u::gui::getCenterWinBounds({-1, -1, 260, 176}), g_ui->getI18Text(LangMap::CHANNELROUTING_TITLE), WID_CHANNEL_ROUTING)
, m_channelId(channelId)
, m_data(c::channel::getRoutingData(channelId))
{
	constexpr int LABEL_WIDTH = 70;

	geFlex* container = new geFlex(getContentBounds().reduced({G_GUI_OUTER_MARGIN}), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		geFlex* body = new geFlex(Direction::VERTICAL, G_GUI_OUTER_MARGIN);
		{
			m_volume       = new geVolumeTool(m_data.id, m_data.volume, LABEL_WIDTH);
			m_pan          = new gePanTool(m_data.id, m_data.pan, LABEL_WIDTH);
			m_sendToMaster = new geCheck(g_ui->getI18Text(LangMap::CHANNELROUTING_SENDTOMASTEROUT));
			m_addNewOutput = new geChoice(g_ui->getI18Text(LangMap::CHANNELROUTING_ADDNEWOUTPUT));
			m_outputs      = new geLiquidScroll(Direction::VERTICAL, Fl_Scroll::VERTICAL);
			body->addWidget(m_volume, G_GUI_UNIT);
			body->addWidget(m_pan, G_GUI_UNIT);
			body->addWidget(m_sendToMaster, G_GUI_UNIT);
			body->addWidget(m_addNewOutput, G_GUI_UNIT);
			body->addWidget(m_outputs);
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
	resizable(container);

	m_sendToMaster->value(m_data.sendToMaster);
	m_sendToMaster->onChange = [id = m_data.id](bool value)
	{
		c::channel::setSendToMaster(id, value);
	};

	m_addNewOutput->onChange = [channelId = m_data.id](ID id)
	{
		c::channel::addExtraOutput(channelId, id - 1);
	};

	m_close->onClick = [this]()
	{ do_callback(); };

	rebuild();

	set_modal();
	show();
}

/* -------------------------------------------------------------------------- */

void gdChannelRouting::rebuild()
{
	m_data = c::channel::getRoutingData(m_channelId);

	const auto makeOutputName = [](const std::string& deviceName, int offset, int maxNumChannels)
	{
		const std::string unreachable = offset >= maxNumChannels ? g_ui->getI18Text(LangMap::CHANNELROUTING_OUTPUT_UNREACHABLE) : "";
		return fmt::format("{} - {},{} {}", deviceName, offset + 1, offset + 2, unreachable);
	};

	m_addNewOutput->clear();
	m_addNewOutput->addItem(g_ui->getI18Text(LangMap::CHANNELROUTING_OUTPUT_CHOOSE), 0);
	for (int offset = 0; offset < m_data.outputMaxNumChannels; offset += 2)
		m_addNewOutput->addItem(makeOutputName(m_data.outputDeviceName, offset, m_data.outputMaxNumChannels), offset + 1);
	m_addNewOutput->showFirstItem();

	m_outputs->removeAllChildren();
	std::size_t i = 0;
	for (const int offset : m_data.extraOutputs)
		m_outputs->addWidget(new geOutput(makeOutputName(m_data.outputDeviceName, offset, m_data.outputMaxNumChannels), m_data.id, i++));
}
} // namespace giada::v