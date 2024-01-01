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

#ifndef GE_COLUMN_H
#define GE_COLUMN_H

#include "core/types.h"
#include "glue/channel.h"
#include "gui/elems/basics/flexResizable.h"
#include <functional>

namespace giada::v
{
class geTextButton;
class geResizerBar;
class geKeyboard;
class geChannel;
class geColumn : public geFlexResizable
{
public:
	geColumn(int x, int y, int w, int h, int index, geResizerBar* b);

	/* getChannel
	Returns the channel given the ID. */

	geChannel* getChannel(ID channelId);

	/* getChannelAtCursor
	Returns the channel below the cursor. Returns the last channel if y is 
	greater than last channel range. */

	geChannel* getChannelAtCursor(Pixel y) const;

	/* get[First|Last]Channel
	Returns first/last channel in the column. Nullptr if the column is empty. */

	geChannel* getFirstChannel() const;
	geChannel* getLastChannel() const;

	/* countChannels
	Returns the number of channels contained into this column. */

	int countChannels() const;

	/* showAddChannelMenu
	Displays the menu for adding/removing channels. */

	void showAddChannelMenu() const;

	/* addChannel
	Adds a new channel in this column. */

	geChannel* addChannel(const c::channel::Data&);

	/* refreshChannels
	Updates channels' graphical statues. Called on each GUI cycle. */

	void refresh();

	void forEachChannel(std::function<void(geChannel& c)> f) const;

	int index;

	geResizerBar* resizerBar;
};
} // namespace giada::v

#endif
