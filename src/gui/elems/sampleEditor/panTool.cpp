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
#include "../../../core/sampleChannel.h"
#include "../../../core/const.h"
#include "../../../core/waveFx.h"  
#include "../../../glue/channel.h"
#include "../../../utils/gui.h"
#include "../../../utils/math.h"
#include "../../../utils/string.h"
#include "../../dialogs/sampleEditor.h"
#include "../basics/dial.h"
#include "../basics/input.h"
#include "../basics/box.h"
#include "../basics/button.h"
#include "waveTools.h"
#include "panTool.h"


using std::string;
using namespace giada;


gePanTool::gePanTool(int x, int y, m::SampleChannel* ch)
  : Fl_Group(x, y, 200, 20),
    ch      (ch)
{
  begin();
    label = new geBox(x, y, gu_getStringWidth("Pan"), 20, "Pan", FL_ALIGN_RIGHT);
    dial  = new geDial(label->x()+label->w()+4, y, 20, 20);
    input = new geInput(dial->x()+dial->w()+4, y, 70, 20);
    reset = new geButton(input->x()+input->w()+4, y, 70, 20, "Reset");
  end();

  dial->range(0.0f, 1.0f);
  dial->callback(cb_panning, (void*)this);

  input->align(FL_ALIGN_RIGHT);
  input->readonly(1);
  input->cursor_color(FL_WHITE);

  reset->callback(cb_panReset, (void*)this);

  refresh();
}


/* -------------------------------------------------------------------------- */


void gePanTool::refresh()
{
  dial->value(ch->getPan());

  if (ch->getPan() < 0.5f) {
    string tmp = gu_iToString((int) ((-ch->getPan() * 200.0f) + 100.0f)) + " L";
    input->value(tmp.c_str());
  }
  else 
  if (ch->getPan() == 0.5)
    input->value("C");
  else {
    string tmp = gu_iToString((int) ((ch->getPan() * 200.0f) - 100.0f)) + " R";
    input->value(tmp.c_str());
  }
}


/* -------------------------------------------------------------------------- */


void gePanTool::cb_panning (Fl_Widget *w, void *p) { ((gePanTool*)p)->cb_panning(); }
void gePanTool::cb_panReset(Fl_Widget *w, void *p) { ((gePanTool*)p)->cb_panReset(); }



/* -------------------------------------------------------------------------- */


void gePanTool::cb_panning()
{
  c::channel::setPanning(ch, dial->value());
}


/* -------------------------------------------------------------------------- */


void gePanTool::cb_panReset()
{
  c::channel::setPanning(ch, 0.5f);
}