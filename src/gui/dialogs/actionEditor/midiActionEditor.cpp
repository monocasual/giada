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
#include "core/graphics.h"
#include "glue/actionEditor.h"
#include "glue/channel.h"
#include "gui/elems/actionEditor/gridTool.h"
#include "gui/elems/actionEditor/noteEditor.h"
#include "gui/elems/actionEditor/pianoRoll.h"
#include "gui/elems/actionEditor/velocityEditor.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/resizerBar.h"
#include "gui/elems/basics/scrollPack.h"
#include <string>

namespace giada
{
namespace v
{
gdMidiActionEditor::gdMidiActionEditor(ID channelId)
: gdBaseActionEditor(channelId)
{
	end();

	computeWidth();

	gePack* upperArea = new gePack(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN, Direction::HORIZONTAL);
	gridTool          = new geGridTool(0, 0);
	geBox* b1         = new geBox(0, 0, w() - 150, G_GUI_UNIT); // padding actionType - zoomButtons
	zoomInBtn         = new geButton(0, 0, G_GUI_UNIT, G_GUI_UNIT, "", zoomInOff_xpm, zoomInOn_xpm);
	zoomOutBtn        = new geButton(0, 0, G_GUI_UNIT, G_GUI_UNIT, "", zoomOutOff_xpm, zoomOutOn_xpm);
	upperArea->add(gridTool);
	upperArea->add(b1);
	upperArea->add(zoomInBtn);
	upperArea->add(zoomOutBtn);
	upperArea->resizable(b1);

	/* Main viewport: contains all widgets. */

	viewport = new geScrollPack(G_GUI_OUTER_MARGIN, upperArea->y() + upperArea->h() + G_GUI_OUTER_MARGIN,
	    upperArea->w(), h() - 44, Fl_Scroll::BOTH, Direction::VERTICAL, /*gutter=*/0);
	m_ne     = new geNoteEditor(0, 0, this);
	m_ner    = new geResizerBar(0, 0, viewport->w(), RESIZER_BAR_H, MIN_WIDGET_H, geResizerBar::VERTICAL);
	m_ve     = new geVelocityEditor(0, 0, this);
	m_ver    = new geResizerBar(0, 0, viewport->w(), RESIZER_BAR_H, MIN_WIDGET_H, geResizerBar::VERTICAL);
	viewport->add(m_ne);
	viewport->add(m_ner);
	viewport->add(m_ve);
	viewport->add(m_ver);

	zoomInBtn->callback(cb_zoomIn, (void*)this);
	zoomInBtn->copy_tooltip("Zoom in");
	zoomOutBtn->callback(cb_zoomOut, (void*)this);
	zoomOutBtn->copy_tooltip("Zoom out");

	add(upperArea);
	add(viewport);
	resizable(upperArea);

	prepareWindow();
	rebuild();
}

/* -------------------------------------------------------------------------- */

void gdMidiActionEditor::rebuild()
{
	m_data = c::actionEditor::getData(channelId);

	computeWidth();
	m_ne->rebuild(m_data);
	m_ner->size(m_ne->w(), m_ner->h());
	m_ve->rebuild(m_data);
	m_ver->size(m_ve->w(), m_ver->h());
}
} // namespace v
} // namespace giada
