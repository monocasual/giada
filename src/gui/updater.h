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

#ifndef G_V_UPDATER_H
#define G_V_UPDATER_H

#include "deps/concurrentqueue/concurrentqueue.h"
#include <FL/Fl.H>
#include <functional>

namespace giada::v
{
class Ui;
class Updater final
{
public:
	using Event = std::function<void()>;

	Updater(Ui& ui);

	void start();
	void stop();
	void run();
	bool pumpEvent(const Event&);

private:
	static void update(void*);
	void        update();

	Ui& m_ui;

	moodycamel::ConcurrentQueue<Event> m_eventQueue;
};
} // namespace giada::v

#endif