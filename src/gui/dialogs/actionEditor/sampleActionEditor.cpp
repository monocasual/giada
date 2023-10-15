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

#include "gui/dialogs/actionEditor/sampleActionEditor.h"
#include "core/conf.h"
#include "core/const.h"
#include "glue/actionEditor.h"
#include "glue/channel.h"
#include "gui/elems/actionEditor/envelopeEditor.h"
#include "gui/elems/actionEditor/gridTool.h"
#include "gui/elems/actionEditor/legend.h"
#include "gui/elems/actionEditor/sampleActionEditor.h"
#include "gui/elems/actionEditor/splitScroll.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/choice.h"
#include "gui/elems/basics/flex.h"
#include "gui/elems/basics/flexResizable.h"
#include "gui/elems/basics/imageButton.h"
#include "gui/graphics.h"
#include "gui/ui.h"
#include <string>

extern giada::v::Ui* g_ui;

namespace giada::v
{
gdSampleActionEditor::gdSampleActionEditor(ID channelId, const Model& model)
: gdBaseActionEditor(channelId, model)
{
	geFlex* container = new geFlex(getContentBounds().reduced({G_GUI_OUTER_MARGIN}), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		geFlex* header = new geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN);
		{
			m_actionType = new geChoice();
			header->addWidget(m_actionType, 120);
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
				m_legends->addWidget(new geLegend(g_ui->getI18Text(LangMap::ACTIONEDITOR_STARTSTOP)));
				m_legends->addWidget(new geLegend(g_ui->getI18Text(LangMap::ACTIONEDITOR_VOLUME)));

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

	m_actionType->addItem(g_ui->getI18Text(LangMap::ACTIONEDITOR_KEYPRESS));
	m_actionType->addItem(g_ui->getI18Text(LangMap::ACTIONEDITOR_KEYRELEASE));
	m_actionType->addItem(g_ui->getI18Text(LangMap::ACTIONEDITOR_STOPSAMPLE));
	m_actionType->showItem(0);
	m_actionType->copy_tooltip(g_ui->getI18Text(LangMap::ACTIONEDITOR_LABEL_ACTIONTYPE));

	m_sampleActionEditor = new geSampleActionEditor(0, 0, this);
	m_envelopeEditor     = new geEnvelopeEditor(0, 0, this);
	m_splitScroll->addWidgets(*m_sampleActionEditor, *m_envelopeEditor, model.actionEditorSplitH);

	m_splitScroll->onDragBar = [this]() {
		m_legends->resizeWidget(0, m_splitScroll->getTopContentH());
	};

	m_legends->resizeWidget(0, m_splitScroll->getTopContentH());

	m_legends->onDragBar = [this](const Fl_Widget& widget) {
		m_splitScroll->resizeWidget(0, widget.h());
	};

	prepareWindow();
	rebuild();
}

/* -------------------------------------------------------------------------- */

bool gdSampleActionEditor::canChangeActionType()
{
	return m_data.sample->channelMode != SamplePlayerMode::SINGLE_PRESS &&
	       m_data.sample->isLoopMode == false;
}

/* -------------------------------------------------------------------------- */

void gdSampleActionEditor::rebuild()
{
	m_data = c::actionEditor::getData(channelId);

	canChangeActionType() ? m_actionType->activate() : m_actionType->deactivate();
	computeWidth(m_data.framesInSeq, m_data.framesInLoop);

	m_sampleActionEditor->rebuild(m_data);
	m_envelopeEditor->rebuild(m_data);
	m_splitScroll->initScrollbar();
}

/* -------------------------------------------------------------------------- */

int gdSampleActionEditor::getActionType() const
{
	if (m_actionType->getSelectedId() == 0)
		return m::MidiEvent::CHANNEL_NOTE_ON;
	else if (m_actionType->getSelectedId() == 1)
		return m::MidiEvent::CHANNEL_NOTE_OFF;
	else if (m_actionType->getSelectedId() == 2)
		return m::MidiEvent::CHANNEL_NOTE_KILL;

	assert(false);
	return -1;
}
} // namespace giada::v