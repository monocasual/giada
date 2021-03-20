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

#ifndef GE_MAIN_TRANSPORT_H
#define GE_MAIN_TRANSPORT_H

#include "gui/elems/basics/box.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/pack.h"
#include "gui/elems/basics/statusButton.h"

namespace giada::v
{
class geMainTransport : public gePack
{
public:
	geMainTransport(int x, int y);

	void refresh();

  private:
	geButton       m_rewind;
	geStatusButton m_play;
	geBox          m_spacer1;
	geStatusButton m_recTriggerMode;
	geStatusButton m_recAction;
	geStatusButton m_recInput;
	geBox          m_spacer2;
	geStatusButton m_metronome;
};
} // namespace giada::v

#endif
