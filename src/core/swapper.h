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

#ifndef G_SWAPPER_H
#define G_SWAPPER_H

#include <atomic>
#include <functional>

namespace giada
{
/* Swapper
A template class that performs atomic double buffering on type T. */

template <typename T>
class Swapper
{
public:
	class RtLock
	{
		friend Swapper;

	public:
		RtLock(Swapper& s)
		: m_swapper(s)
		{
			m_swapper.rt_lock();
		}

		~RtLock()
		{
			m_swapper.rt_unlock();
		}

		const T& get() const
		{
			return m_swapper.rt_get();
		}

	private:
		Swapper& m_swapper;
	};

	Swapper()
	{
		static_assert(std::is_assignable_v<T, T>);
	}

	/* get
	Returns local data for non-realtime thread. */

	T& get()
	{
		return m_data[(m_bits.load() & BIT_INDEX) ^ 1];
	}

	void swap()
	{
		int bits = m_bits.load();

		/* Wait for the audio thread to finish, i.e. until the BUSY bit becomes
        zero. Only then, swap indexes. This will let the audio thread to pick
        the updated data on its next cycle. */
		int desired;
		do
		{
			bits    = bits & ~BIT_BUSY;               // Expected: current value without busy bit set
			desired = (bits ^ BIT_INDEX) & BIT_INDEX; // Desired: flipped (xor) index
		} while (!m_bits.compare_exchange_weak(bits, desired));

		bits = desired;

		/* After the swap above, m_data[(bits & BIT_INDEX) ^ 1] has become the 
		non-realtime slot and it points to the data previously read by the
		realtime thread. That data is old, so update it: overwrite it with the 
		realtime data in the realtime slot (m_data[bits & BIT_INDEX]) that is 
		currently being read by the realtime thread. */
		m_data[(bits & BIT_INDEX) ^ 1] = m_data[bits & BIT_INDEX];
	}

	bool isLocked()
	{
		return m_bits.load() & BIT_BUSY;
	}

private:
	static constexpr int BIT_INDEX = (1 << 0); // 0001
	static constexpr int BIT_BUSY  = (1 << 1); // 0010

	/* [realtime] lock. */

	void rt_lock()
	{
		/* Set the busy bit and also get the current index. */
		m_index = m_bits.fetch_or(BIT_BUSY) & BIT_INDEX;
	}

	/* [realtime] unlock. */

	void rt_unlock()
	{
		m_bits.store(m_index & BIT_INDEX);
	}

	/* [realtime] Get data currently being ready by the rt thread. */

	const T& rt_get() const
	{
		return m_data[m_bits.load() & BIT_INDEX];
	}

	std::array<T, 2> m_data;
	std::atomic<int> m_bits{0};
	int              m_index{0};
};
} // namespace giada

#endif