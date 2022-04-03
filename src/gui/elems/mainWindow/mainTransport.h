/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "gui/elems/basics/flex.h"

namespace giada::v
{
class geButton;
class geStatusButton;
class geMainTransport : public geFlex
{
public:
	geMainTransport();

	void refresh();

private:
	geButton*       m_rewind;
	geStatusButton* m_play;
	geStatusButton* m_recTriggerMode;
	geStatusButton* m_recAction;
	geStatusButton* m_recInput;
	geStatusButton* m_inputRecMode;
	geStatusButton* m_metronome;
};
} // namespace giada::v

#endif
