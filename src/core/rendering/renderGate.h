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

#ifndef G_RENDER_GATE_H
#define G_RENDER_GATE_H

#include <atomic>
#include <cstdint>
#include <thread>

namespace giada::m::rendering
{
/* RenderGate
Small utility class to allow one realtime render at a time. Provides suspend and
resume methods to disable rendering for e.g. loading a new project or restore
to the default one.  */

class RenderGate final
{
public:
	class Guard final
	{
	public:
		explicit Guard(RenderGate& gate)
		: m_gate(gate)
		{
			std::uint32_t expected = IDLE;
			m_entered              = m_gate.m_state.compare_exchange_strong(expected, RENDERING);
		}

		~Guard()
		{
			if (m_entered)
				m_gate.m_state.fetch_and(~RENDERING);
		}

		Guard(const Guard&)            = delete;
		Guard& operator=(const Guard&) = delete;

		explicit operator bool() const { return m_entered; }

	private:
		RenderGate& m_gate;
		bool        m_entered = false;
	};

	/* tryEnter
	Realtime thread only. A successful guard keeps rendering active until it is
	destroyed; an unsuccessful guard means the caller must not render. */

	[[nodiscard]] Guard tryEnter() { return Guard(*this); }

	/* suspend
	Prevents new renders and waits for an active render to finish. */

	void suspend()
	{
		m_state.fetch_or(SUSPENDED);
		while (m_state.load() & RENDERING)
			std::this_thread::yield();
	}

	/* resume
	Allows rendering again after suspension has completed. */

	void resume() { m_state.store(IDLE); }

private:
	static constexpr std::uint32_t IDLE      = 0;
	static constexpr std::uint32_t RENDERING = std::uint32_t{1} << 0;
	static constexpr std::uint32_t SUSPENDED = std::uint32_t{1} << 1;

	static_assert(std::atomic<std::uint32_t>::is_always_lock_free);
	std::atomic<std::uint32_t> m_state{IDLE};
};
} // namespace giada::m::rendering

#endif
