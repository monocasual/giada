/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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

#ifndef G_WEAK_ATOMIC_H
#define G_WEAK_ATOMIC_H

#include <atomic>

namespace giada
{
template <typename T>
class WeakAtomic
{
public:
	WeakAtomic() = default;

	WeakAtomic(T t)
	: m_value(t)
	{
	}

	WeakAtomic(const WeakAtomic& o)
	: m_value(o.load())
	{
	}

	WeakAtomic(WeakAtomic&& o) = delete;

	WeakAtomic& operator=(const WeakAtomic& o)
	{
		if (this == &o)
			return *this;
		store(o.load());
		return *this;
	}

	WeakAtomic& operator=(WeakAtomic&& o) = delete;

	T load() const
	{
		return m_value.load(std::memory_order_relaxed);
	}

	void store(T t)
	{
		return m_value.store(t, std::memory_order_relaxed);
	}

  private:
	std::atomic<T> m_value;
};
} // namespace giada

#endif