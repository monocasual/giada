/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef GE_SOUND_METER_H
#define GE_SOUND_METER_H

#include "core/types.h"
#include <FL/Fl_Box.H>

namespace giada::v
{
class geSoundMeter : public Fl_Box
{
public:
	geSoundMeter(int x, int y, int w, int h, const char* l = nullptr);

	void draw() override;

	Peak peak;  // Peak from Mixer
	bool ready; // Kernel state

private:
	class Meter
	{
	public:
		float compute(float peak);

	private:
		float m_dbLevelOld = 0.0f;
	};

	Meter m_left;
	Meter m_right;
};
} // namespace giada::v

#endif
