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

#include "idManager.h"

namespace giada::m
{
IdManager::IdManager()
: m_id(0)
{
}

/* -------------------------------------------------------------------------- */

void IdManager::set(ID id)
{
	if (id != 0 && id > m_id)
		m_id = id;
}

/* -------------------------------------------------------------------------- */

ID IdManager::generate(ID id)
{
	if (id != 0)
	{
		m_id = id;
		return id;
	}
	return ++m_id;
}

/* -------------------------------------------------------------------------- */

ID IdManager::get() const
{
	return m_id;
}

/* -------------------------------------------------------------------------- */

ID IdManager::getNext() const
{
	return m_id + 1;
}
} // namespace giada::m
