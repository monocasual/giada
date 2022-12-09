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

#include "worker.h"
#include "utils/time.h"

namespace giada
{
Worker::Worker(int sleep)
: m_running(false)
, m_sleep(sleep)
{
}

/* -------------------------------------------------------------------------- */

Worker::~Worker()
{
	stop();
}

/* -------------------------------------------------------------------------- */

void Worker::start(std::function<void()> f) const
{
	m_running.store(true);
	m_thread = std::thread([this, f]() {
		while (m_running.load() == true)
		{
			f();
			u::time::sleep(m_sleep);
		}
	});
}

/* -------------------------------------------------------------------------- */

void Worker::stop() const
{
	m_running.store(false);
	if (m_thread.joinable())
		m_thread.join();
}

/* -------------------------------------------------------------------------- */

void Worker::setSleep(int sleep)
{
	m_sleep = sleep;
}
} // namespace giada
