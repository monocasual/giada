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
#include <FL/fl_draw.H>
#include "../../core/const.h"
#include "../../core/kernelAudio.h"
#include "soundMeter.h"


using namespace giada::m;


geSoundMeter::geSoundMeter(int x, int y, int w, int h, const char *L)
  : Fl_Box    (x, y, w, h, L),
    clip      (false),
    mixerPeak (0.0f),
    peak      (0.0f),
    dbLevel   (0.0f),
    dbLevelOld(0.0f)
{
}


/* -------------------------------------------------------------------------- */


void geSoundMeter::draw()
{
  fl_rect(x(), y(), w(), h(), G_COLOR_GREY_4);

  /* peak = the highest value inside the frame */

  peak = 0.0f;
  float tmp_peak = 0.0f;

  tmp_peak = fabs(mixerPeak);
  if (tmp_peak > peak)
    peak = tmp_peak;

  clip = peak >= 1.0f ? true : false; // 1.0f is considered clip


  /*  dBFS (full scale) calculation, plus decay of -2dB per frame */

  dbLevel = 20 * log10(peak);
  if (dbLevel < dbLevelOld)
    if (dbLevelOld > -G_DB_MIN_SCALE)
      dbLevel = dbLevelOld - 2.0f;

  dbLevelOld = dbLevel;

  /* graphical part */

  float px_level = 0.0f;
  if (dbLevel < 0.0f)
    px_level = ((w()/G_DB_MIN_SCALE) * dbLevel) + w();
  else
    px_level = w();

  fl_rectf(x()+1, y()+1, w()-2, h()-2, G_COLOR_GREY_2);
  fl_rectf(x()+1, y()+1, (int) px_level, h()-2, clip || !kernelAudio::getStatus() ? G_COLOR_RED_ALERT : G_COLOR_GREY_4);
}
