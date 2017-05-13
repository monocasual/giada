/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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
#include "../../../core/sampleChannel.h"
#include "../../../core/const.h"
#include "../../../core/graphics.h"  
#include "../../../core/clock.h"
#include "../../../glue/channel.h"
#include "../../../utils/gui.h"
#include "../../dialogs/sampleEditor.h"
#include "../basics/dial.h"
#include "../basics/input.h"
#include "../basics/box.h"
#include "../basics/button.h"
#include "pitchTool.h"


using namespace giada::m;


gePitchTool::gePitchTool(int x, int y, SampleChannel *ch)
  : Fl_Group(x, y, 600, 20),
    ch      (ch)
{
  begin();
    label       = new geBox(0, 0, gu_getStringWidth("Pitch"), 20, "Pitch", FL_ALIGN_RIGHT);
    dial        = new geDial(label->x()+label->w()+4, 0, 20, 20);
    input       = new geInput(dial->x()+dial->w()+4, 0, 70, 20);
    pitchToBar  = new geButton(input->x()+input->w()+4, 0, 70, 20, "To bar");
    pitchToSong = new geButton(pitchToBar->x()+pitchToBar->w()+4, 0, 70, 20, "To song");
    pitchHalf   = new geButton(pitchToSong->x()+pitchToSong->w()+4, 0, 20, 20, "", divideOff_xpm, divideOn_xpm);
    pitchDouble = new geButton(pitchHalf->x()+pitchHalf->w()+4, 0, 20, 20, "", multiplyOff_xpm, multiplyOn_xpm);
    pitchReset  = new geButton(pitchDouble->x()+pitchDouble->w()+4, 0, 70, 20, "Reset");
  end();

  dial->range(0.01f, 4.0f);
  dial->callback(cb_setPitch, (void*)this);
  dial->when(FL_WHEN_RELEASE);

  input->align(FL_ALIGN_RIGHT);
  input->callback(cb_setPitchNum, (void*)this);
  input->when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY);

  pitchToBar->callback(cb_setPitchToBar, (void*)this);
  pitchToSong->callback(cb_setPitchToSong, (void*)this);
  pitchHalf->callback(cb_setPitchHalf, (void*)this);
  pitchDouble->callback(cb_setPitchDouble, (void*)this);
  pitchReset->callback(cb_resetPitch, (void*)this);

  refresh();
}


/* -------------------------------------------------------------------------- */


void gePitchTool::refresh()
{
  dial->value(ch->getPitch());
  char buf[16];
  sprintf(buf, "%.4f", ch->getPitch()); // 4 digits
  input->value(buf);
}


/* -------------------------------------------------------------------------- */


void gePitchTool::cb_setPitch      (Fl_Widget *w, void *p) { ((gePitchTool*)p)->__cb_setPitch(); }
void gePitchTool::cb_setPitchToBar (Fl_Widget *w, void *p) { ((gePitchTool*)p)->__cb_setPitchToBar(); }
void gePitchTool::cb_setPitchToSong(Fl_Widget *w, void *p) { ((gePitchTool*)p)->__cb_setPitchToSong(); }
void gePitchTool::cb_setPitchHalf  (Fl_Widget *w, void *p) { ((gePitchTool*)p)->__cb_setPitchHalf(); }
void gePitchTool::cb_setPitchDouble(Fl_Widget *w, void *p) { ((gePitchTool*)p)->__cb_setPitchDouble(); }
void gePitchTool::cb_resetPitch    (Fl_Widget *w, void *p) { ((gePitchTool*)p)->__cb_resetPitch(); }
void gePitchTool::cb_setPitchNum   (Fl_Widget *w, void *p) { ((gePitchTool*)p)->__cb_setPitchNum(); }


/* -------------------------------------------------------------------------- */


void gePitchTool::__cb_setPitch()
{
  glue_setPitch(ch, dial->value());
}


/* -------------------------------------------------------------------------- */


void gePitchTool::__cb_setPitchNum()
{
  glue_setPitch(ch, atof(input->value()));
}


/* -------------------------------------------------------------------------- */


void gePitchTool::__cb_setPitchHalf()
{
  glue_setPitch(ch, dial->value()/2);
}


/* -------------------------------------------------------------------------- */


void gePitchTool::__cb_setPitchDouble()
{
  glue_setPitch(ch, dial->value()*2);
}


/* -------------------------------------------------------------------------- */


void gePitchTool::__cb_setPitchToBar()
{
  glue_setPitch(ch, ch->end / (float) clock::getFramesPerBar());
}


/* -------------------------------------------------------------------------- */


void gePitchTool::__cb_setPitchToSong()
{
  glue_setPitch(ch, ch->end / (float) clock::getTotalFrames());
}


/* -------------------------------------------------------------------------- */


void gePitchTool::__cb_resetPitch()
{
  glue_setPitch(ch, G_DEFAULT_PITCH);
}
