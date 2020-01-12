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


#include <FL/Fl.H>
#include "core/const.h"
#include "core/model/model.h"
#include "utils/gui.h"
#include "updater.h"


namespace giada {
namespace v {
namespace updater
{
void update(void* p)
{
	if (m::model::waves.changed.load()    == true ||
		m::model::actions.changed.load()  == true ||
		m::model::channels.changed.load() == true)
	{
		u::gui::rebuild();
		m::model::waves.changed.store(false);
		m::model::actions.changed.store(false);
		m::model::channels.changed.store(false);
	}
	else
		u::gui::refresh();

	Fl::add_timeout(G_GUI_REFRESH_RATE, update, nullptr);
}
}}} // giada::v::updater
