/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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
#include "../../../utils/string.h"
#include "../../../utils/math.h"
#include "../../dialogs/sampleEditor.h"
#include "../basics/dial.h"
#include "../basics/input.h"
#include "../basics/box.h"
#include "../basics/button.h"
#include "waveTools.h"
#include "boostTool.h"


using namespace giada;


geBoostTool::geBoostTool(int X, int Y, m::SampleChannel* ch)
  : Fl_Group(X, Y, 220, 20),
    ch      (ch)
{
  begin();
    label     = new geBox(x(), y(), gu_getStringWidth("Boost"), 20, "Boost", FL_ALIGN_RIGHT);
    dial      = new geDial(label->x()+label->w()+4, y(), 20, 20);
    input     = new geInput(dial->x()+dial->w()+4, y(), 70, 20);
    normalize = new geButton(input->x()+input->w()+4, y(), 70, 20, "Normalize");
  end();

  dial->range(1.0f, 10.0f);
  dial->callback(cb_setBoost, (void*)this);
  dial->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE);

  input->callback(cb_setBoostNum, (void*)this);

  normalize->callback(cb_normalize, (void*)this);

  refresh();
}


/* -------------------------------------------------------------------------- */


void geBoostTool::refresh()
{
  using namespace giada::u;

  input->value(gu_fToString(math::linearToDB(ch->getBoost()), 2).c_str());  // 2 digits
  // A dial greater than it's max value goes crazy
  dial->value(ch->getBoost() <= 10.0f ? ch->getBoost() : 10.0f);
}


/* -------------------------------------------------------------------------- */


void geBoostTool::cb_setBoost   (Fl_Widget* w, void* p) { ((geBoostTool*)p)->cb_setBoost(); }
void geBoostTool::cb_setBoostNum(Fl_Widget* w, void* p) { ((geBoostTool*)p)->cb_setBoostNum(); }
void geBoostTool::cb_normalize  (Fl_Widget* w, void* p) { ((geBoostTool*)p)->cb_normalize(); }


/* -------------------------------------------------------------------------- */


void geBoostTool::cb_setBoost()
{
  using namespace giada::c;

  if (Fl::event() == FL_DRAG)
    channel::setBoost(ch, dial->value());
  else 
  if (Fl::event() == FL_RELEASE) {
    channel::setBoost(ch, dial->value());
    static_cast<gdSampleEditor*>(window())->waveTools->updateWaveform();
  }
}


/* -------------------------------------------------------------------------- */


void geBoostTool::cb_setBoostNum()
{
  using namespace giada;

  c::channel::setBoost(ch, u::math::dBtoLinear(atof(input->value())));
  static_cast<gdSampleEditor*>(window())->waveTools->updateWaveform();
}


/* -------------------------------------------------------------------------- */


void geBoostTool::cb_normalize()
{
  using namespace giada;

  float val = m::wfx::normalizeSoft(*ch->wave);
  c::channel::setBoost(ch, val); // it's like a fake user moving the dial 
  static_cast<gdSampleEditor*>(window())->waveTools->updateWaveform();
}

