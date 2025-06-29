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

#ifndef G_VERSION_H
#define G_VERSION_H

#include <string>

namespace giada
{
class Version
{
public:
	constexpr Version(int major, int minor, int patch)
	: m_major(major)
	, m_minor(minor)
	, m_patch(patch)
	{
	}

	constexpr bool operator==(const Version& o) const
	{
		return m_major == o.m_major && m_minor == o.m_minor && m_patch == o.m_patch;
	}

	constexpr bool operator<(const Version& o) const
	{
		if (m_major < o.m_major)
			return true;
		else if (o.m_major < m_major)
			return false;
		if (m_minor < o.m_minor)
			return true;
		else if (o.m_minor < m_minor)
			return false;
		if (m_patch < o.m_patch)
			return true;
		else if (o.m_patch < m_patch)
			return false;
		return false;
	}

	constexpr std::string toString() const
	{
		return std::to_string(m_major) + "." + std::to_string(m_minor) + "." + std::to_string(m_patch);
	}

private:
	int m_major;
	int m_minor;
	int m_patch;
};

} // namespace giada

#endif
