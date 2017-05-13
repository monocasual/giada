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


#include <cmath>
#include <FL/Fl_Pack.H>
#include "../../../core/sampleChannel.h"
#include "../../../core/const.h"
#include "../../../glue/channel.h"
#include "../../../utils/gui.h"
#include "../../../utils/math.h"
#include "../basics/dial.h"
#include "../basics/input.h"
#include "../basics/box.h"
#include "../mainWindow/keyboard/channel.h"
#include "volumeTool.h"


geVolumeTool::geVolumeTool(int x, int y, SampleChannel *ch)
  : Fl_Group(x, y, 200, 20),
    ch      (ch)
{
  begin();
    label = new geBox  (0, 0, gu_getStringWidth("Volume"), 20, "Volume", FL_ALIGN_RIGHT);
    dial  = new geDial (label->x()+label->w()+4, 0, 20, 20);
    input = new geInput(dial->x()+dial->w()+4, 0, 70, 20);
  end();

  dial->range(0.0f, 1.0f);
  dial->callback(cb_setVolume, (void*)this);

  input->callback(cb_setVolumeNum, (void*)this);

  refresh();
}


/* -------------------------------------------------------------------------- */


void geVolumeTool::refresh()
{
  char buf[16];
  float dB = gu_linearToDB(ch->volume);
  if (dB > -INFINITY) sprintf(buf, "%.2f", dB);
  else                sprintf(buf, "-inf");
  input->value(buf);
  dial->value(ch->guiChannel->vol->value());
}


/* -------------------------------------------------------------------------- */


void geVolumeTool::cb_setVolume   (Fl_Widget *w, void *p) { ((geVolumeTool*)p)->__cb_setVolume(); }
void geVolumeTool::cb_setVolumeNum(Fl_Widget *w, void *p) { ((geVolumeTool*)p)->__cb_setVolumeNum(); }


/* -------------------------------------------------------------------------- */


void geVolumeTool::__cb_setVolume()
{
  glue_setVolume(ch, dial->value(), false, true);
  refresh();
}


/* -------------------------------------------------------------------------- */


void geVolumeTool::__cb_setVolumeNum()
{
  float value = pow(10, (atof(input->value()) / 20)); // linear = 10^(dB/20)
  glue_setVolume(ch, value, false, true);
  dial->value(ch->guiChannel->vol->value());
}
