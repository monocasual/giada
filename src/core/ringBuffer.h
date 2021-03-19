/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#ifndef G_RING_BUFFER_H
#define G_RING_BUFFER_H

#include <array>

namespace giada
{
/* RingBuffer
A non-thread-safe, fixed-size ring buffer implementation. It grows from 0 to S, 
then items are overwritten starting from position 0. */

template <typename T, std::size_t S>
class RingBuffer
{
  public:
	using iterator       = typename std::array<T, S>::iterator;
	using const_iterator = typename std::array<T, S>::const_iterator;

	iterator       begin() { return m_data.begin(); }
	iterator       end() { return m_data.begin() + m_end; }
	const_iterator begin() const { return m_data.begin(); }
	const_iterator end() const { return m_data.begin() + m_end; }
	const_iterator cbegin() const { return m_data.begin(); }
	const_iterator cend() const { return m_data.begin() + m_end; }

	void clear()
	{
		m_data.fill({});
		m_index = 0;
		m_end   = 0;
	}

	void push_back(T t)
	{
		m_data[m_index] = t;
		m_index         = (m_index + 1) % m_data.size(); // Wraps around at m_data.size()
		m_end           = std::max(m_index, m_end);      // Points to the greater index
	}

	std::size_t size() const noexcept
	{
		return m_end;
	}

  private:
	std::array<T, S> m_data;
	std::size_t      m_index = 0;
	std::size_t      m_end   = 0;
};
} // namespace giada

#endif
