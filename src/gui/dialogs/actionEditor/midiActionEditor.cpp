/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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


#include <string>
#include "../../../core/graphics.h"
#include "../../../core/midiChannel.h"
#include "../../../glue/actionEditor.h"
#include "../../elems/basics/scroll.h"
#include "../../elems/basics/button.h"
#include "../../elems/basics/resizerBar.h"
#include "../../elems/basics/box.h"
#include "../../elems/actionEditor/noteEditor.h"
#include "../../elems/actionEditor/velocityEditor.h"
#include "../../elems/actionEditor/pianoRoll.h"
#include "../../elems/actionEditor/gridTool.h"
#include "midiActionEditor.h"


using std::string;


namespace giada {
namespace v
{
gdMidiActionEditor::gdMidiActionEditor(m::MidiChannel* ch)
: gdBaseActionEditor(ch)
{
	computeWidth();

	Fl_Group* upperArea = new Fl_Group(8, 8, w()-16, 20);

	upperArea->begin();

		gridTool = new geGridTool(8, 8);

		geBox *b1  = new geBox(gridTool->x()+gridTool->w()+4, 8, 300, 20);    // padding actionType - zoomButtons
		zoomInBtn  = new geButton(w()-8-40-4, 8, 20, 20, "", zoomInOff_xpm, zoomInOn_xpm);
		zoomOutBtn = new geButton(w()-8-20,   8, 20, 20, "", zoomOutOff_xpm, zoomOutOn_xpm);
	
	upperArea->end();
	upperArea->resizable(b1);

	zoomInBtn->callback(cb_zoomIn, (void*)this);
	zoomOutBtn->callback(cb_zoomOut, (void*)this);

	/* Main viewport: contains all widgets. */

	viewport = new geScroll(8, 36, w()-16, h()-44);

	m_ne  = new geNoteEditor(viewport->x(), viewport->y(), this);
	m_ner = new geResizerBar(m_ne->x(), m_ne->y()+m_ne->h(), viewport->w(), RESIZER_BAR_H, MIN_WIDGET_H);
	viewport->add(m_ne);
	viewport->add(m_ner);
	
	m_ve  = new geVelocityEditor(viewport->x(), m_ne->y()+m_ne->h()+RESIZER_BAR_H, ch);
	m_ver = new geResizerBar(m_ve->x(), m_ve->y()+m_ve->h(), viewport->w(), RESIZER_BAR_H, MIN_WIDGET_H);
	viewport->add(m_ve);
	viewport->add(m_ver);

	end();
	prepareWindow();
	rebuild();
}


/* -------------------------------------------------------------------------- */


void gdMidiActionEditor::rebuild()
{
	m_actions = c::actionEditor::getActions(ch);
	computeWidth();
	m_ne->rebuild();
	m_ner->size(m_ne->w(), m_ner->h());
	m_ve->rebuild();
	m_ver->size(m_ve->w(), m_ver->h());
}
}} // giada::v::
