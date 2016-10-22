/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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


#include <FL/fl_draw.H>
#include "../../core/const.h"
#include "../../core/conf.h"
#include "../../utils/log.h"
#include "../dialogs/gd_actionEditor.h"
#include "pianoItem.h"
#include "pianoRoll.h"
#include "noteEditor.h"


extern Conf G_Conf;


geNoteEditor::geNoteEditor(int x, int y, gdActionEditor *pParent)
  : Fl_Scroll(x, y, 200, 422),
    pParent  (pParent)
{
	size(pParent->totalWidth, G_Conf.pianoRollH);
	pianoRoll = new gePianoRoll(x, y, pParent->totalWidth, pParent);
}


/* -------------------------------------------------------------------------- */


geNoteEditor::~geNoteEditor()
{
	clear();
	G_Conf.pianoRollH = h();
	G_Conf.pianoRollY = pianoRoll->y();
}


/* -------------------------------------------------------------------------- */


void geNoteEditor::updateActions()
{
	pianoRoll->updateActions();
}


/* -------------------------------------------------------------------------- */


void geNoteEditor::draw()
{
	pianoRoll->size(this->w(), pianoRoll->h());  /// <--- not optimal

	/* clear background */

	fl_rectf(x(), y(), w(), h(), COLOR_BG_MAIN);

	/* clip pianoRoll to pianoRollContainer size */

	fl_push_clip(x(), y(), w(), h());
	draw_child(*pianoRoll);
	fl_pop_clip();

	fl_color(COLOR_BD_0);
	fl_line_style(0);
	fl_rect(x(), y(), pParent->totalWidth, h());
}
