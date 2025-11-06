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

#ifndef GE_PLAY_BUTTON_H
#define GE_PLAY_BUTTON_H

#include "src/gui/elems/basics/textButton.h"
#include <string>

namespace giada::v
{
/* gePlayButton
A text button with three states: play, ending and default (idle). Used for
buttons that display transitions between a 'play' and an 'ending' state such
as the Scene or the Channel buttons. */

class gePlayButton : public geTextButton
{
public:
	gePlayButton(const std::string& label = "");

	void setPlayState();
	void setEndingState();
	void setDefaultState();
};
} // namespace giada::v

#endif
