/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "sampleActionEditor.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/midiEvent.h"
#include "core/model/model.h"
#include "glue/actionEditor.h"
#include "glue/channel.h"
#include "gui/elems/basics/box.h"
#include <string>

namespace giada::v
{
gdSampleActionEditor::gdSampleActionEditor(ID channelId, m::Conf::Data& conf)
: gdBaseActionEditor(channelId, conf)
, m_barPadding(0, 0, w() - 232, G_GUI_UNIT)
, m_sampleActionEditor(0, 0, this)
, m_envelopeEditor(0, 0, "Volume", this)
, m_actionType(0, 0, 80, G_GUI_UNIT)
{
	end();

	m_barTop.add(&m_actionType);
	m_barTop.add(&gridTool);
	m_barTop.add(&m_barPadding);
	m_barTop.add(&zoomInBtn);
	m_barTop.add(&zoomOutBtn);
	m_barTop.resizable(m_barPadding);

	m_actionType.addItem("Key press");
	m_actionType.addItem("Key release");
	m_actionType.addItem("Stop sample");
	m_actionType.showItem(0);
	m_actionType.copy_tooltip("Action type to add");
	if (!canChangeActionType())
		m_actionType.deactivate();

	m_splitScroll.addWidgets(m_sampleActionEditor, m_envelopeEditor, conf.actionEditorSplitH);

	resizable(m_splitScroll); // Make it resizable only once filled with widgets

	prepareWindow();
	rebuild();
}

/* -------------------------------------------------------------------------- */

gdSampleActionEditor::~gdSampleActionEditor()
{
	m_barTop.remove(m_actionType);
	m_barTop.remove(gridTool);
	m_barTop.remove(zoomInBtn);
	m_barTop.remove(zoomOutBtn);
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

	canChangeActionType() ? m_actionType.activate() : m_actionType.deactivate();
	computeWidth(m_data.framesInSeq, m_data.framesInLoop);

	m_sampleActionEditor.rebuild(m_data);
	m_envelopeEditor.rebuild(m_data);
}

/* -------------------------------------------------------------------------- */

int gdSampleActionEditor::getActionType() const
{
	if (m_actionType.getSelectedId() == 0)
		return m::MidiEvent::NOTE_ON;
	else if (m_actionType.getSelectedId() == 1)
		return m::MidiEvent::NOTE_OFF;
	else if (m_actionType.getSelectedId() == 2)
		return m::MidiEvent::NOTE_KILL;

	assert(false);
	return -1;
}
} // namespace giada::v