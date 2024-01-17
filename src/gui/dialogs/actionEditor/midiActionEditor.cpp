/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "midiActionEditor.h"
#include "glue/actionEditor.h"
#include "glue/channel.h"
#include "gui/elems/actionEditor/gridTool.h"
#include "gui/elems/actionEditor/legend.h"
#include "gui/elems/actionEditor/pianoRoll.h"
#include "gui/elems/actionEditor/pianoRollLegend.h"
#include "gui/elems/actionEditor/splitScroll.h"
#include "gui/elems/actionEditor/velocityEditor.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/flex.h"
#include "gui/elems/basics/flexResizable.h"
#include "gui/elems/basics/imageButton.h"
#include "gui/graphics.h"
#include "gui/ui.h"

extern giada::v::Ui* g_ui;

namespace giada::v
{
gdMidiActionEditor::gdMidiActionEditor(ID channelId, const Model& model)
: gdBaseActionEditor(channelId, model)
{
	geFlex* container = new geFlex(getContentBounds().reduced({G_GUI_OUTER_MARGIN}), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		geFlex* header = new geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN);
		{
			header->addWidget(gridTool, 80);
			header->addWidget(new geBox());
			header->addWidget(m_zoomInBtn, G_GUI_UNIT);
			header->addWidget(m_zoomOutBtn, G_GUI_UNIT);
			header->end();
		}

		geFlex* body = new geFlex(Direction::HORIZONTAL);
		{
			geFlex* legendBox = new geFlex(Direction::VERTICAL);
			{
				m_legends->addWidget(new gePianoRollLegend());
				m_legends->addWidget(new geLegend(g_ui->getI18Text(LangMap::ACTIONEDITOR_VELOCITY)));

				legendBox->addWidget(m_legends);
				legendBox->addWidget(new geBox(), m_splitScroll->getBottomScrollbarH() + G_GUI_OUTER_MARGIN); // bottom-right dead corner
				legendBox->end();
			}

			body->addWidget(legendBox, LEGEND_WIDTH);
			body->addWidget(m_splitScroll);
			body->end();
		}

		container->addWidget(header, G_GUI_UNIT);
		container->addWidget(body);
		container->end();
	}

	add(container);
	resizable(container);

	m_pianoRoll      = new gePianoRoll(0, 0, this);
	m_velocityEditor = new geVelocityEditor(0, 0, this);

	m_splitScroll->addWidgets(*m_pianoRoll, *m_velocityEditor, model.actionEditorSplitH);

	m_splitScroll->onDragBar = [this]() {
		m_legends->resizeWidget(0, m_splitScroll->getTopContentH());
	};
	m_splitScroll->onScrollV = [this](int y) {
		static_cast<gePianoRollLegend&>(m_legends->getWidget(0)).scroll_to(0, y);
	};

	m_legends->resizeWidget(0, m_splitScroll->getTopContentH());

	m_legends->onDragBar = [this](const Fl_Widget& widget) {
		m_splitScroll->resizeWidget(0, widget.h());
	};

	if (model.actionEditorPianoRollY != -1)
	{
		m_splitScroll->setScrollY(model.actionEditorPianoRollY);
		static_cast<gePianoRollLegend&>(m_legends->getWidget(0)).scroll_to(0, model.actionEditorPianoRollY);
	}

	prepareWindow();
	rebuild();
}

/* -------------------------------------------------------------------------- */

gdMidiActionEditor::~gdMidiActionEditor()
{
	g_ui->model.actionEditorPianoRollY = m_splitScroll->getScrollY();
}

/* -------------------------------------------------------------------------- */

void gdMidiActionEditor::rebuild()
{
	m_data = c::actionEditor::getData(channelId);

	computeWidth(m_data.framesInSeq, m_data.framesInLoop);

	m_pianoRoll->rebuild(m_data);
	m_velocityEditor->rebuild(m_data);
	m_splitScroll->initScrollbar();
}
} // namespace giada::v