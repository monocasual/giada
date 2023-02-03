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

#ifndef G_ID_MANAGER_H
#define G_ID_MANAGER_H

#include "core/types.h"

namespace giada::m
{
class IdManager
{
public:
	IdManager();

	/* set
	Stores a new id, only if != 0 (valid) and greater than current id (unique). */

	void set(ID id);

	/* generate
	Generates a new unique id. If 'id' parameter is passed in is valid, it just 
	returns it with no unique id generation. Useful when loading things from the 
	model that already have their own id. */

	ID generate(ID id = 0);

	/* get
	Returns the current id, a.k.a. the last generated one. */

	ID get() const;

	/* getNext
	Returns the upcoming ID, that is the ID that will be generated on the next
	call. */

	ID getNext() const;

private:
	ID m_id;
};
} // namespace giada::m

#endif
