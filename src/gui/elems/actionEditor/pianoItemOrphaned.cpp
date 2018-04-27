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


#include "../../../core/const.h"
#include "../../../core/kernelMidi.h"
#include "../../../core/recorder.h"
#include "../../../core/channel.h"
#include "../../../core/midiChannel.h"
#include "../../../core/mixer.h"
#include "../../dialogs/gd_actionEditor.h"
#include "pianoRoll.h"
#include "noteEditor.h"
#include "pianoItemOrphaned.h"


using namespace giada::m;


gePianoItemOrphaned::gePianoItemOrphaned(int x, int y, int xRel, int yRel,
  recorder::action action, gdActionEditor* pParent)
  : geBasePianoItem(x, y, WIDTH, pParent)
{
  note  = kernelMidi::getB2(action.iValue);
  frame = action.frame;
  event = action.iValue;
  int newX = xRel + (frame / pParent->zoom);
  int newY = yRel + getY(note);
  resize(newX, newY, w(), h());
}


/* -------------------------------------------------------------------------- */


void gePianoItemOrphaned::reposition(int pianoRollX)
{
  int newX = pianoRollX + (frame / pParent->zoom);
  resize(newX, y(), WIDTH, h());
  redraw();
}


/* -------------------------------------------------------------------------- */


int gePianoItemOrphaned::handle(int e)
{
  int ret = geBasePianoItem::handle(e);
  if (e == FL_PUSH) {
    remove();
    ret = 1;
  }
  return ret;
}


/* -------------------------------------------------------------------------- */


void gePianoItemOrphaned::remove()
{
  MidiChannel *ch = static_cast<MidiChannel*>(pParent->chan);
  recorder::deleteAction(ch->index, frame, G_ACTION_MIDI, true, &mixer::mutex, 
    event, 0.0);
  hide();   // for Windows
  Fl::delete_widget(this);
  ch->hasActions = recorder::hasActions(ch->index);
  static_cast<geNoteEditor*>(parent())->redraw();
}


/* -------------------------------------------------------------------------- */


void gePianoItemOrphaned::draw()
{
  fl_rect(x(), y()+2, WIDTH, h()-3, (Fl_Color) selected ? G_COLOR_LIGHT_1 : G_COLOR_LIGHT_1);
}
