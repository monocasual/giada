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


#include <FL/Fl.H>
#include "../../../core/const.h"
#include "../../../core/conf.h"
#include "../../../core/midiChannel.h"
#include "../../dialogs/actionEditor/midiActionEditor.h"
#include "pianoRoll.h"
#include "noteEditor.h"


namespace giada {
namespace v
{
geNoteEditor::geNoteEditor(Pixel x, Pixel y, gdMidiActionEditor* base)
: geScroll(x, y, 200, 422),
	m_base  (base)
{
	pianoRoll = new gePianoRoll(x, y, m_base->fullWidth, static_cast<m::MidiChannel*>(m_base->ch));
	
	size(m_base->fullWidth, m::conf::pianoRollH);
	
	type(Fl_Scroll::VERTICAL_ALWAYS);
}


/* -------------------------------------------------------------------------- */


geNoteEditor::~geNoteEditor()
{
	m::conf::pianoRollH = h();
	m::conf::pianoRollY = pianoRoll->y();
}


/* -------------------------------------------------------------------------- */


void geNoteEditor::scroll()
{
	Pixel ey = Fl::event_y() - pianoRoll->pick;

	Pixel y1 = y();
	Pixel y2 = (y() + h()) - pianoRoll->h();

	if (ey > y1) ey = y1; else if (ey < y2) ey = y2;

	pianoRoll->position(x(), ey);

	redraw();
}


/* -------------------------------------------------------------------------- */


void geNoteEditor::rebuild()
{
	size(m_base->fullWidth, h());
	pianoRoll->rebuild();
}
}} // giada::v::