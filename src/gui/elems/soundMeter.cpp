/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "soundMeter.h"
#include "core/const.h"
#include "gui/const.h"
#include "gui/drawing.h"
#include "utils/math.h"
#include <FL/fl_draw.H>
#include <algorithm>
#include <cmath>

namespace giada::v
{
namespace
{
Pixel dbToPx_(float db, Pixel max)
{
	const float maxf = max;
	return std::clamp(u::math::map(db, -G_MIN_DB_SCALE, 0.0f, 0.0f, maxf), 0.0f, maxf);
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

float geSoundMeter::Meter::compute(float peak)
{
	/*  dBFS (full scale) calculation, plus decay of -2dB per call. */

	float dbLevelCur = u::math::linearToDB(std::fabs(peak));

	if (dbLevelCur < m_dbLevelOld && m_dbLevelOld > -G_MIN_DB_SCALE)
		dbLevelCur = m_dbLevelOld - 2.0f;

	m_dbLevelOld = dbLevelCur;

	return dbLevelCur;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geSoundMeter::geSoundMeter(Direction d)
: Fl_Box(0, 0, 0, 0)
, m_direction(d)
{
}

/* -------------------------------------------------------------------------- */

void geSoundMeter::draw()
{
	const geompp::Rect outline(x(), y(), w(), h());
	const geompp::Rect body(outline.reduced(1));

	drawRect(outline, G_COLOR_GREY_4);

	if (!ready)
	{
		drawRectf(body, G_COLOR_BLUE);
		return;
	}

	drawRectf(body, G_COLOR_GREY_2); // Cleanup

	const float dbL    = m_left.compute(peak.left);
	const float dbR    = m_right.compute(peak.right);
	const int   colorL = std::fabs(peak.left) > 1.0f ? G_COLOR_BLUE : G_COLOR_GREY_4;
	const int   colorR = std::fabs(peak.right) > 1.0f ? G_COLOR_BLUE : G_COLOR_GREY_4;

	if (m_direction == Direction::HORIZONTAL)
	{
		const geompp::Rect bodyL(body.withTrimmedBottom(h() / 2));
		const geompp::Rect bodyR(body.withTrimmedTop(h() / 2));

		drawRectf(bodyL.withW(dbToPx_(dbL, w() - 2)), colorL);
		drawRectf(bodyR.withW(dbToPx_(dbR, w() - 2)), colorR);
	}
	else
	{
		const geompp::Rect bodyL(body.withTrimmedRight(w() / 2));
		const geompp::Rect bodyR(body.withTrimmedLeft(w() / 2));

		const int dbPxL = dbToPx_(dbL, h() - 2);
		const int dbPxR = dbToPx_(dbR, h() - 2);

		drawRectf(bodyL.withH(dbPxL).withShiftedY(bodyL.h - dbPxL), colorL);
		drawRectf(bodyR.withH(dbPxR).withShiftedY(bodyR.h - dbPxR), colorR);
	}
}
} // namespace giada::v