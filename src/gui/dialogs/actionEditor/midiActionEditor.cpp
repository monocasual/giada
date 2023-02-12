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

#include "midiActionEditor.h"
#include "core/conf.h"
#include "core/engine.h"
#include "glue/actionEditor.h"
#include "glue/channel.h"
#include "gui/elems/actionEditor/gridTool.h"
#include "gui/elems/actionEditor/pianoRoll.h"
#include "gui/elems/actionEditor/splitScroll.h"
#include "gui/elems/actionEditor/velocityEditor.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/flex.h"
#include "gui/elems/basics/imageButton.h"
#include "gui/graphics.h"
#include "gui/ui.h"

extern giada::v::Ui     g_ui;
extern giada::m::Engine g_engine;

namespace giada::v
{
gdMidiActionEditor::gdMidiActionEditor(ID channelId, const m::Conf& conf)
: gdBaseActionEditor(channelId, conf)
{
	geFlex* container = new geFlex(getContentBounds().reduced({G_GUI_OUTER_MARGIN}), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		geFlex* header = new geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN);
		{
			header->add(gridTool, 80);
			header->add(new geBox());
			header->add(m_zoomInBtn, G_GUI_UNIT);
			header->add(m_zoomOutBtn, G_GUI_UNIT);
			header->end();
		}

		container->add(header, G_GUI_UNIT);
		container->add(m_splitScroll);
		container->end();
	}

	add(container);
	resizable(container);

	m_pianoRoll      = new gePianoRoll(0, 0, this);
	m_velocityEditor = new geVelocityEditor(0, 0, this);
	m_splitScroll->addWidgets(*m_pianoRoll, *m_velocityEditor, conf.actionEditorSplitH);

	if (conf.actionEditorPianoRollY != -1)
		m_splitScroll->setScrollY(conf.actionEditorPianoRollY);

	prepareWindow();
	rebuild();
}

/* -------------------------------------------------------------------------- */

gdMidiActionEditor::~gdMidiActionEditor()
{
	m::Conf conf                = g_engine.getConf();
	conf.actionEditorPianoRollY = m_splitScroll->getScrollY();
	g_engine.setConf(conf);
}

/* -------------------------------------------------------------------------- */

void gdMidiActionEditor::rebuild()
{
	m_data = c::actionEditor::getData(channelId);

	computeWidth(m_data.framesInSeq, m_data.framesInLoop);

	m_pianoRoll->rebuild(m_data);
	m_velocityEditor->rebuild(m_data);
}
} // namespace giada::v