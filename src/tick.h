/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_TICK_H
#define G_TICK_H

#include <compare>
#include <cstdint>

namespace giada
{
class Tick final
{
public:
	using Value = std::int64_t;

	constexpr Tick() noexcept = default;

	explicit constexpr Tick(Value value) noexcept
	: m_value(value)
	{
	}

	[[nodiscard]] constexpr Value value() const noexcept
	{
		return m_value;
	}

	constexpr Tick& operator+=(Tick other) noexcept
	{
		m_value += other.m_value;
		return *this;
	}

	constexpr Tick& operator-=(Tick other) noexcept
	{
		m_value -= other.m_value;
		return *this;
	}

	constexpr Tick& operator*=(std::int64_t other) noexcept
	{
		m_value *= other;
		return *this;
	}

	constexpr Tick& operator/=(std::int64_t other) noexcept
	{
		m_value /= other;
		return *this;
	}

	[[nodiscard]] friend constexpr Tick operator+(Tick lhs, Tick rhs) noexcept
	{
		lhs += rhs;
		return lhs;
	}

	[[nodiscard]] friend constexpr Tick operator-(Tick lhs, Tick rhs) noexcept
	{
		lhs -= rhs;
		return lhs;
	}

	[[nodiscard]] friend constexpr Tick operator*(Tick lhs, std::int64_t rhs) noexcept
	{
		lhs *= rhs;
		return lhs;
	}

	[[nodiscard]] friend constexpr Tick operator/(Tick lhs, std::int64_t rhs) noexcept
	{
		lhs /= rhs;
		return lhs;
	}

	[[nodiscard]] friend constexpr auto operator<=>(Tick, Tick) noexcept = default;
	[[nodiscard]] friend constexpr bool operator==(Tick, Tick) noexcept  = default;

private:
	Value m_value = 0;
};
} // namespace giada

#endif
