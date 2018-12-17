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


#include <cmath>
#include <cstdlib>
#include <FL/Fl_Pack.H>
#include "../../../core/sampleChannel.h"
#include "../../../core/const.h"
#include "../../../glue/channel.h"
#include "../../../utils/gui.h"
#include "../../../utils/math.h"
#include "../../../utils/string.h"
#include "../basics/dial.h"
#include "../basics/input.h"
#include "../basics/box.h"
#include "../mainWindow/keyboard/channel.h"
#include "volumeTool.h"


using std::string;
using namespace giada;


geVolumeTool::geVolumeTool(int X, int Y, m::SampleChannel* ch)
  : Fl_Group(X, Y, 150, 20),
    ch      (ch)
{
  begin();
    label = new geBox  (x(), y(), gu_getStringWidth("Volume"), 20, "Volume", FL_ALIGN_RIGHT);
    dial  = new geDial (label->x()+label->w()+4, y(), 20, 20);
    input = new geInput(dial->x()+dial->w()+4, y(), 70, 20);
  end();

  dial->range(0.0f, 1.0f);
  dial->callback(cb_setVolume, (void*)this);

  input->callback(cb_setVolumeNum, (void*)this);

  refresh();
}


/* -------------------------------------------------------------------------- */


void geVolumeTool::refresh()
{
  using namespace giada::u;

  string tmp;
  float dB = math::linearToDB(ch->volume);
  if (dB > -INFINITY) tmp = gu_fToString(dB, 2);  // 2 digits
  else                tmp = "-inf";
  input->value(tmp.c_str());
  dial->value(ch->guiChannel->vol->value());
}


/* -------------------------------------------------------------------------- */


void geVolumeTool::cb_setVolume   (Fl_Widget* w, void* p) { ((geVolumeTool*)p)->cb_setVolume(); }
void geVolumeTool::cb_setVolumeNum(Fl_Widget* w, void* p) { ((geVolumeTool*)p)->cb_setVolumeNum(); }


/* -------------------------------------------------------------------------- */


void geVolumeTool::cb_setVolume()
{
  using namespace giada;

  c::channel::setVolume(ch, dial->value(), false, true);
  refresh();
}


/* -------------------------------------------------------------------------- */


void geVolumeTool::cb_setVolumeNum()
{
  using namespace giada;

  float value = pow(10, (atof(input->value()) / 20)); // linear = 10^(dB/20)
  c::channel::setVolume(ch, value, false, true);
  dial->value(ch->guiChannel->vol->value());
}
