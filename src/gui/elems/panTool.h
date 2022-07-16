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

#ifndef GE_PAN_TOOL_H
#define GE_PAN_TOOL_H

#include "core/types.h"
#include "gui/elems/basics/flex.h"

namespace giada::v
{
class geInput;
class geDial;
class geTextButton;
class gePanTool : public geFlex
{
public:
	gePanTool(ID channelId, float pan, int labelWidth = 0);

private:
	void update(float pan);

	ID m_channelId;

	geInput*      m_input;
	geDial*       m_dial;
	geTextButton* m_reset;
};
} // namespace giada::v

#endif
