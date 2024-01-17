/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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
#include <functional>

namespace giada
{
template <typename T>
class WeakAtomic
{
public:
	WeakAtomic() = default;

	WeakAtomic(T t)
	: m_atomic(t)
	, m_value(t)
	{
	}

	WeakAtomic(const WeakAtomic& o)
	: onChange(o.onChange)
	, m_atomic(o.load())
	, m_value(o.m_value)
	{
	}

	WeakAtomic(WeakAtomic&& o) = delete;

	WeakAtomic& operator=(const WeakAtomic& o)
	{
		if (this == &o)
			return *this;
		onChange = o.onChange;
		store(o.load());
		m_value = o.m_value;
		return *this;
	}

	WeakAtomic& operator=(WeakAtomic&& o) = delete;

	T load() const
	{
		return m_atomic.load(std::memory_order_relaxed);
	}

	void store(T t)
	{
		m_atomic.store(t, std::memory_order_relaxed);
		if (onChange != nullptr && t != m_value)
			onChange(t);
		m_value = t;
	}

	std::function<void(T)> onChange = nullptr;

private:
	std::atomic<T> m_atomic;
	T              m_value;
};
} // namespace giada

#endif