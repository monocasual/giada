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

#ifndef G_SCENE_H
#define G_SCENE_H

#include "src/const.h"

namespace giada
{
class Scene
{
public:
	constexpr Scene() noexcept = default; // Invalid scene

	explicit constexpr Scene(std::size_t i) noexcept
	: m_index(i)
	{
		assert(isValid());
	}

	constexpr bool operator==(const Scene&) const noexcept = default;

	constexpr bool isValid() const { return m_index < G_INVALID_SCENE; }

	constexpr std::size_t getIndex() const { return m_index; };

private:
	std::size_t m_index = G_INVALID_SCENE;
};
} // namespace giada

#endif
