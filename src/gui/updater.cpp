/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "gui/updater.h"
#include "core/const.h"
#include "core/model/model.h"
#include "gui/ui.h"
#include "utils/gui.h"

namespace giada::v
{
Updater::Updater(Ui& ui)
: m_ui(ui)
{
}

/* -------------------------------------------------------------------------- */

void Updater::run()
{
	while (Fl::wait() > 0)
	{
		Event e;
		while (m_eventQueue.try_dequeue(e))
			e();
	}
}

/* -------------------------------------------------------------------------- */

bool Updater::pumpEvent(const Event& e)
{
	return m_eventQueue.try_enqueue(e);
}

/* -------------------------------------------------------------------------- */

void Updater::update(void* p) { static_cast<Updater*>(p)->update(); }

/* -------------------------------------------------------------------------- */

void Updater::update()
{
	m_ui.refresh();
	Fl::add_timeout(G_GUI_REFRESH_RATE, update, this); // Repeat
}

/* -------------------------------------------------------------------------- */

void Updater::start()
{
	Fl::add_timeout(G_GUI_REFRESH_RATE, update, this);
}

void Updater::stop()
{
	Fl::remove_timeout(update);
}
} // namespace giada::v
