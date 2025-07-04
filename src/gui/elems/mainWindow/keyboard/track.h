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

#ifndef GE_TRACK_H
#define GE_TRACK_H

#include "src/core/types.h"
#include "src/glue/channel.h"
#include "src/gui/elems/basics/flexResizable.h"
#include <functional>

namespace giada::v
{
class geTextButton;
class geResizerBar;
class geKeyboard;
class geChannel;
class geTrack : public geFlexResizable
{
public:
	geTrack(int x, int y, int w, int h, std::size_t index, geResizerBar* b);

	/* getChannel
	Returns the channel given the ID. */

	geChannel* getChannel(ID channelId);

	/* getChannelAtCursor
	Returns the channel below the cursor. Returns the last channel if y is
	greater than last channel range. */

	geChannel* getChannelAtCursor(Pixel y) const;

	/* get[First|Last]Channel
	Returns first/last channel in the track. Nullptr if the track is empty. */

	geChannel* getFirstChannel() const;
	geChannel* getLastChannel() const;


	/* showMenu
	Displays the menu for adding/removing channels. */

	void showMenu() const;

	/* addChannel
	Adds a new channel in this track. */

	geChannel* addChannel(const c::channel::Data&);

	/* refreshChannels
	Updates channels' graphical statues. Called on each GUI cycle. */

	void refresh();

	void forEachChannel(std::function<void(geChannel& c)> f) const;

	std::size_t index;

	geResizerBar* resizerBar;
};
} // namespace giada::v

#endif
