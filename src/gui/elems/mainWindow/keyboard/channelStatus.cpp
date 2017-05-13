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


#include <FL/fl_draw.H>
#include "../../../../core/mixer.h"
#include "../../../../core/clock.h"
#include "../../../../core/sampleChannel.h"
#include "../../../../core/recorder.h"
#include "../../../../core/const.h"
#include "channelStatus.h"


using namespace giada::m;


geChannelStatus::geChannelStatus(int x, int y, int w, int h, SampleChannel *ch,
  const char *L)
  : Fl_Box(x, y, w, h, L), ch(ch) {}


/* -------------------------------------------------------------------------- */


void geChannelStatus::draw()
{
  fl_rect(x(), y(), w(), h(), COLOR_BD_0);              // reset border
  fl_rectf(x()+1, y()+1, w()-2, h()-2, COLOR_BG_0);     // reset background

  if (ch != nullptr) {
    if (ch->status    & (STATUS_WAIT | STATUS_ENDING | REC_ENDING | REC_WAITING) ||
        ch->recStatus & (REC_WAITING | REC_ENDING))
    {
      fl_rect(x(), y(), w(), h(), COLOR_BD_1);
    }
    else
    if (ch->status == STATUS_PLAY)
      fl_rect(x(), y(), w(), h(), COLOR_BD_1);
    else
      fl_rectf(x()+1, y()+1, w()-2, h()-2, COLOR_BG_0);     // status empty


    if (mixer::recording && ch->armed)
      fl_rectf(x()+1, y()+1, w()-2, h()-2, COLOR_BG_3);     // take in progress
    else
    if (recorder::active && recorder::canRec(ch, clock::isRunning(), mixer::recording))
      fl_rectf(x()+1, y()+1, w()-2, h()-2, COLOR_BG_4);     // action record

    /* equation for the progress bar:
     * ((chanTracker - chanStart) * w()) / (chanEnd - chanStart). */

    int pos = ch->getPosition();
    if (pos == -1)
      pos = 0;
    else
      pos = (pos * (w()-1)) / (ch->end - ch->begin);
    fl_rectf(x()+1, y()+1, pos, h()-2, COLOR_BG_2);
  }
}
