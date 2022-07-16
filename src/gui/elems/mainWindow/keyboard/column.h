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

#ifndef GE_COLUMN_H
#define GE_COLUMN_H

#include "core/types.h"
#include "glue/channel.h"
#include <FL/Fl_Group.H>
#include <functional>
#include <vector>

namespace giada::v
{
class geTextButton;
class geResizerBar;
class geKeyboard;
class geChannel;
class geColumn : public Fl_Group
{
public:
	geColumn(int x, int y, int w, int h, ID id, geResizerBar* b);

	/* getChannel
	Returns the channel given the ID. */

	geChannel* getChannel(ID channelId) const;

	/* getChannelAtCursor
	Returns the channel below the cursor. Returns the last channel if y is 
	greater than last channel range. */

	geChannel* getChannelAtCursor(Pixel y) const;

	/* countChannels
	Returns the number of channels contained into this column. */

	int countChannels() const;

	/* addChannel
	Adds a new channel in this column. */

	geChannel* addChannel(c::channel::Data d);

	/* refreshChannels
	Updates channels' graphical statues. Called on each GUI cycle. */

	void refresh();

	void init();

	void forEachChannel(std::function<void(geChannel& c)> f) const;

	ID id;

	geResizerBar* resizerBar;

private:
	int computeHeight() const;

	void addChannel();

	std::vector<geChannel*> m_channels;

	geTextButton* m_addChannelBtn;
};
} // namespace giada::v

#endif
