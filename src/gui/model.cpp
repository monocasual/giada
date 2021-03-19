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

#include "model.h"
#include "core/patch.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "gui/elems/mainWindow/keyboard/column.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "utils/log.h"

extern giada::v::gdMainWindow* G_MainWin;

namespace giada::v::model
{
void store(m::patch::Patch& patch)
{
	G_MainWin->keyboard->forEachColumn([&](const geColumn& c) {
		patch.columns.push_back({c.id, c.w()});
	});
}

/* -------------------------------------------------------------------------- */

void load(const m::patch::Patch& patch)
{
	G_MainWin->keyboard->layout.clear();
	for (const m::patch::Column& col : patch.columns)
		G_MainWin->keyboard->layout.push_back({col.id, col.width});
}
} // namespace giada::v::model
