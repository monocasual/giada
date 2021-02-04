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


#include <string>
#include "core/model/model.h"
#include "core/const.h"
#include "core/midiEvent.h"
#include "core/graphics.h"
#include "glue/actionEditor.h"
#include "glue/channel.h"
#include "gui/elems/basics/pack.h"
#include "gui/elems/basics/scrollPack.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/resizerBar.h"
#include "gui/elems/basics/choice.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/actionEditor/sampleActionEditor.h"
#include "gui/elems/actionEditor/envelopeEditor.h"
#include "gui/elems/actionEditor/gridTool.h"
#include "sampleActionEditor.h"


namespace giada {
namespace v
{
gdSampleActionEditor::gdSampleActionEditor(ID channelId)
: gdBaseActionEditor(channelId)
{
	end();

	computeWidth();

	/* Container with zoom buttons and the action type selector. Scheme of the 
	resizable boxes: |[--b1--][actionType][--b2--][+][-]| */

	gePack* upperArea = new gePack(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN, Direction::HORIZONTAL);
		actionType = new geChoice  (0, 0, 80, 20);
		gridTool   = new geGridTool(0, 0);
		geBox* b1  = new geBox     (0, 0, w() - 232, 20);    // padding actionType - zoomButtons
		zoomInBtn  = new geButton  (0, 0, 20, 20, "", zoomInOff_xpm, zoomInOn_xpm);
		zoomOutBtn = new geButton  (0, 0, 20, 20, "", zoomOutOff_xpm, zoomOutOn_xpm);
	upperArea->add(actionType);
	upperArea->add(gridTool);
	upperArea->add(b1);
	upperArea->add(zoomInBtn);
	upperArea->add(zoomOutBtn);
	upperArea->resizable(b1);

	actionType->add("Key press");
	actionType->add("Key release");
	actionType->add("Kill chan");
	actionType->value(0);
	if (!canChangeActionType())
		actionType->deactivate();

	zoomInBtn->callback(cb_zoomIn, (void*)this);
	zoomOutBtn->callback(cb_zoomOut, (void*)this);

	/* Main viewport: contains all widgets. */

	viewport = new geScrollPack(G_GUI_OUTER_MARGIN, upperArea->y() + upperArea->h() + G_GUI_OUTER_MARGIN, 
		upperArea->w(), h()-44, Fl_Scroll::BOTH, Direction::VERTICAL, /*gutter=*/0);
		m_ae  = new geSampleActionEditor(0, 0, this);
		m_aer = new geResizerBar        (0, 0, viewport->w(), RESIZER_BAR_H, MIN_WIDGET_H, geResizerBar::VERTICAL);	
		m_ee  = new geEnvelopeEditor    (0, 0, "volume", this);
		m_eer = new geResizerBar        (0, 0, viewport->w(), RESIZER_BAR_H, MIN_WIDGET_H, geResizerBar::VERTICAL);
	viewport->add(m_ae);
	viewport->add(m_aer);
	viewport->add(m_ee);
	viewport->add(m_eer);

	add(upperArea);
	add(viewport);
	resizable(upperArea);

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

	canChangeActionType() ? actionType->activate() : actionType->deactivate(); 
	computeWidth();
	
	m_ae->rebuild(m_data);
	m_aer->size(m_ae->w(), m_aer->h());
	m_ee->rebuild(m_data);	
	m_eer->size(m_ee->w(), m_eer->h());
}
}} // giada::v::
