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

#include "math.h"
#include <cmath>

namespace giada
{
namespace u
{
namespace math
{
float linearToDB(float f)
{
	return 20 * std::log10(f);
}

/* -------------------------------------------------------------------------- */

int quantize(int x, int step)
{
	/* Source:
	https://en.wikipedia.org/wiki/Quantization_(signal_processing)#Rounding_example */
	return step * std::floor((x / (float)step) + 0.5f);
}

/* -------------------------------------------------------------------------- */

float dBtoLinear(float f)
{
	return std::pow(10, f / 20.0f);
}

} // namespace math
} // namespace u
} // namespace giada