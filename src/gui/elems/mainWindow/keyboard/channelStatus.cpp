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


#include <FL/fl_draw.H>
#include "../../../../core/mixer.h"
#include "../../../../core/clock.h"
#include "../../../../core/sampleChannel.h"
#include "../../../../core/recorder.h"
#include "../../../../core/const.h"
#include "channelStatus.h"


using namespace giada;
using namespace giada::m;


geChannelStatus::geChannelStatus(int x, int y, int w, int h, SampleChannel *ch,
  const char *L)
  : Fl_Box(x, y, w, h, L), ch(ch) {}


/* -------------------------------------------------------------------------- */


void geChannelStatus::draw()
{
  fl_rect(x(), y(), w(), h(), G_COLOR_GREY_4);              // reset border
  fl_rectf(x()+1, y()+1, w()-2, h()-2, G_COLOR_GREY_2);     // reset background

  if (ch == nullptr) 
    return;

  if (ch->status == ChannelStatus::WAIT    || 
      ch->status == ChannelStatus::ENDING  ||
      ch->recStatus == ChannelStatus::WAIT || 
      ch->recStatus == ChannelStatus::ENDING)
  {
    fl_rect(x(), y(), w(), h(), G_COLOR_LIGHT_1);
  }
  else
  if (ch->status == ChannelStatus::PLAY)
    fl_rect(x(), y(), w(), h(), G_COLOR_LIGHT_1);
  else
    fl_rectf(x()+1, y()+1, w()-2, h()-2, G_COLOR_GREY_2);  // status empty


  if (mixer::recording && ch->armed)
    fl_rectf(x()+1, y()+1, w()-2, h()-2, G_COLOR_RED);     // take in progress
  else
  if (recorder::isActive())
    fl_rectf(x()+1, y()+1, w()-2, h()-2, G_COLOR_BLUE);    // action recording

  /* Equation for the progress bar: 
  ((chanTracker - chanStart) * w()) / (chanEnd - chanStart). */

  int pos = ch->getPosition();
  if (pos == -1)
    pos = 0;
  else
    pos = (pos * (w()-1)) / ((ch->getEnd() - ch->getBegin()));
  fl_rectf(x()+1, y()+1, pos, h()-2, G_COLOR_LIGHT_1);

}
