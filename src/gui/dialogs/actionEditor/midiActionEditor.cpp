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

#include "midiActionEditor.h"
#include "core/conf.h"
#include "glue/actionEditor.h"
#include "glue/channel.h"
#include "gui/elems/basics/box.h"

namespace giada::v
{
gdMidiActionEditor::gdMidiActionEditor(ID channelId, m::conf::Conf& conf)
: gdBaseActionEditor(channelId, conf)
, m_barPadding(0, 0, w() - 150, G_GUI_UNIT)
, m_pianoRoll(0, 0, this)
, m_velocityEditor(0, 0, this)
{
	end();

	m_barTop.add(&gridTool);
	m_barTop.add(&m_barPadding);
	m_barTop.add(&zoomInBtn);
	m_barTop.add(&zoomOutBtn);
	m_barTop.resizable(m_barPadding);

	m_splitScroll.addWidgets(m_pianoRoll, m_velocityEditor, conf.actionEditorSplitH);

	if (conf.actionEditorPianoRollY != -1)
		m_splitScroll.setScrollY(conf.actionEditorPianoRollY);

	resizable(m_splitScroll); // Make it resizable only once filled with widgets

	prepareWindow();
	rebuild();
}

/* -------------------------------------------------------------------------- */

gdMidiActionEditor::~gdMidiActionEditor()
{
	m_conf.actionEditorPianoRollY = m_splitScroll.getScrollY();
	m_barTop.remove(gridTool);
	m_barTop.remove(zoomInBtn);
	m_barTop.remove(zoomOutBtn);
}

/* -------------------------------------------------------------------------- */

void gdMidiActionEditor::rebuild()
{
	m_data = c::actionEditor::getData(channelId);

	computeWidth();

	m_pianoRoll.rebuild(m_data);
	m_velocityEditor.rebuild(m_data);
}
} // namespace giada::v