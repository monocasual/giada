/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "core/patch.h"

namespace giada::m
{
bool Patch::Version::operator==(const Version& o) const
{
	return major == o.major && minor == o.minor && patch == o.patch;
}

bool Patch::Version::operator<(const Version& o) const
{
	if (major < o.major)
		return true;
	else if (o.major < major)
		return false;
	if (minor < o.minor)
		return true;
	else if (o.minor < minor)
		return false;
	if (patch < o.patch)
		return true;
	else if (o.patch < patch)
		return false;
	return false;
}
} // namespace giada::m
