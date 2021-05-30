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

#include "updater.h"
#include "core/const.h"
#include "core/model/model.h"
#include "utils/gui.h"
#include <FL/Fl.H>

namespace giada::v::updater
{
void init()
{
	m::model::onSwap([](m::model::SwapType type) {
		if (type == m::model::SwapType::NONE)
			return;

		/* This callback is fired by the updater thread, so it requires
		synchronization with the main one. */

		Fl::lock();
		type == m::model::SwapType::HARD ? u::gui::rebuild() : u::gui::refresh();
		Fl::unlock();
	});
}

/* -------------------------------------------------------------------------- */

void update(void* /*p*/)
{
	u::gui::refresh();
	Fl::add_timeout(G_GUI_REFRESH_RATE, update, nullptr);
}

/* -------------------------------------------------------------------------- */

void close()
{
	Fl::remove_timeout(update);
}
} // namespace giada::v::updater
