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
 * will be useful, but WITHOUT ANY WARRANTY without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------- */

#include "src/core/pan.h"
#include "src/utils/math.h"
#include <cassert>

namespace giada
{
Pan::Pan(float v)
: m_data({0, 0})
{
	assert(v >= 0.0f && v <= 1.0f);

	m_data[0] = v <= 0.5f ? 1.0f : u::math::map(v, 0.5f, 1.0f, 1.0f, 0.0f);
	m_data[1] = v >= 0.5f ? 1.0f : u::math::map(v, 0.0f, 0.5f, 0.0f, 1.0f);
}

/* -------------------------------------------------------------------------- */

bool Pan::operator==(const Pan& other) const
{
	return m_data[0] == other.m_data[0] && m_data[1] == other.m_data[1];
}

/* -------------------------------------------------------------------------- */

Pan::Type Pan::get() const
{
	return m_data;
}

/* -------------------------------------------------------------------------- */

float Pan::asFloat() const
{
	return (m_data[0] == 1.0f) ? m_data[1] * 0.5f : 1.0f - m_data[0] * 0.5f;
}
} // namespace giada
