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


#include <cmath>
#include <FL/fl_draw.H>
#include "../../core/const.h"
#include "../../core/kernelAudio.h"
#include "../../utils/math.h"
#include "soundMeter.h"


using namespace giada;


geSoundMeter::geSoundMeter(int x, int y, int w, int h, const char* l)
: Fl_Box      (x, y, w, h, l),
  mixerPeak   (0.0f),
  m_dbLevelCur(0.0f),
  m_dbLevelOld(0.0f)
{
}


/* -------------------------------------------------------------------------- */


void geSoundMeter::draw()
{
	fl_rect(x(), y(), w(), h(), G_COLOR_GREY_4);

	/* Compute peak level on 0.0 -> 1.0 scale. 1.0 is considered clip. */

	bool clip = std::fabs(mixerPeak) >= 1.0f ? true : false;

	/*  dBFS (full scale) calculation, plus decay of -2dB per frame. */

	m_dbLevelCur = u::math::linearToDB(std::fabs(mixerPeak));

	if (m_dbLevelCur < m_dbLevelOld && m_dbLevelOld > -G_MIN_DB_SCALE)
		m_dbLevelCur = m_dbLevelOld - 2.0f;

	m_dbLevelOld = m_dbLevelCur;

	/* Paint the meter on screen. */

	float pxLevel = ((w()/G_MIN_DB_SCALE) * m_dbLevelCur) + w();

	fl_rectf(x()+1, y()+1, w()-2, h()-2, G_COLOR_GREY_2);
	fl_rectf(x()+1, y()+1, (int) pxLevel, h()-2, clip || !m::kernelAudio::getStatus() ? G_COLOR_RED_ALERT : G_COLOR_GREY_4);
}
