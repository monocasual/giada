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

#ifndef GD_CHANNEL_NAME_INPUT_H
#define GD_CHANNEL_NAME_INPUT_H

#include "window.h"

namespace giada::c::channel
{
struct Data;
}

namespace giada::v
{
class geTextButton;
class geInput;
class gdChannelNameInput : public gdWindow
{
public:
	gdChannelNameInput(const c::channel::Data& d);

private:
	const c::channel::Data& m_data;

	geInput*      m_name;
	geTextButton* m_ok;
	geTextButton* m_cancel;
};
} // namespace giada::v

#endif
