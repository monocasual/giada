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

#ifndef G_QUEUE_H
#define G_QUEUE_H

#include <cstddef>
#include <array>
#include <atomic>

namespace giada::m
{
/* Queue
Single producer, single consumer lock-free queue. */

template <typename T, std::size_t size>
class Queue
{
public:
	Queue()
	: m_head(0)
	, m_tail(0)
	{
	}

	Queue(const Queue&) = delete;
	Queue(Queue&&)      = delete;
	Queue& operator=(const Queue&) = delete;
	Queue& operator=(Queue&&) = delete;

	bool pop(T& item)
	{
		std::size_t curr = m_head.load();
		if (curr == m_tail.load()) // Queue empty, nothing to do
			return false;

		item = m_data[curr];
		m_head.store(increment(curr));
		return true;
	}

	bool push(const T& item)
	{
		std::size_t curr = m_tail.load();
		std::size_t next = increment(curr);

		if (next == m_head.load()) // Queue full, nothing to do
			return false;

		m_data[curr] = item;
		m_tail.store(next);
		return true;
	}

private:
	std::size_t increment(std::size_t i) const
	{
		return (i + 1) % size;
	}

	std::array<T, size>      m_data;
	std::atomic<std::size_t> m_head;
	std::atomic<std::size_t> m_tail;
};
} // namespace giada::m

#endif
