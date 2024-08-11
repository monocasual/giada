/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_PATCH_FACTORY_H
#define G_PATCH_FACTORY_H

#include "core/patch.h"

namespace giada::m::patchFactory
{
/* serialize
Writes Patch to disk. The 'filePath' parameter refers to the .gptc file. */

bool serialize(const Patch&, const std::string& filePath);

/* deserialize
Reads data from disk into a new Patch object. The 'filePath' parameter refers to
the .gptc file. */

Patch deserialize(const std::string& filePath);
} // namespace giada::m::patchFactory

#endif
