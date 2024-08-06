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

#ifndef GE_KEYBOARD_H
#define GE_KEYBOARD_H

#include "core/const.h"
#include "deps/geompp/src/rect.hpp"
#include "gui/elems/basics/scroll.h"
#include "gui/model.h"
#include <FL/Fl_Box.H>
#include <functional>
#include <memory>
#include <vector>

namespace giada::c::channel
{
struct Track;
}

namespace giada::v
{
class geResizerBar;
class geColumn;
class geChannel;
class geKeyboard : public geScroll
{
public:
	friend class ChannelDragger;

	geKeyboard();

	int  handle(int e) override;
	void draw() override;

	/* getChannelColumnIndex
	Given a channel ID, returns the index of the column it belongs to. */

	int getChannelColumnIndex(ID channelId) const;

	/* countColumns
	Returns the number of columns in the viewport. */

	size_t countColumns() const;

	/* rebuild
	Rebuilds this widget from scratch. Used when the model has changed. */

	void rebuild();

	/* refresh
	Refreshes each column's channel, called on each GUI cycle. */

	void refresh();

	/* addColumn
	Adds new column at the end of the stack. */

	void addColumn();

	/* deleteColumn
	Deletes column by index. */

	void deleteColumn(int);

	/* deleteAllColumns
	Deletes all columns from the stack. */

	void deleteAllColumns();

	void setChannelVolume(ID channelId, float v);
	void notifyMidiIn(ID channelId);
	void notifyMidiOut(ID channelId);

	/* init
	Builds the default setup of empty columns. */

	void init();

	void forEachChannel(std::function<void(geChannel& c)> f) const;
	void forEachColumn(std::function<void(const geColumn& c)> f) const;

private:
	static constexpr int COLUMN_GAP = 20;

	class ChannelDragger
	{
	public:
		ChannelDragger(geKeyboard&);

		bool isDragging() const;

		void begin();
		void drag();
		void end();

	private:
		Fl_Box*     m_placeholder;
		geKeyboard& m_keyboard;
		ID          m_channelId;
		int         m_xoffset;

		/* getPositionForCursor
		Given a geColumn and a y-coordinate, returns the channel position for
		that point, taking empty spaces into account. */

		int getPositionForCursor(const geColumn*, Pixel y) const;
	};

	/* getColumnBackround
	Returns a geompp::Rect for the square to be drawn as column background. */

	geompp::Rect<int> getColumnBackround(const geColumn&) const;

	/* getDroppedFilePaths
	Returns a vector of audio file paths after a drag-n-drop from desktop
	event. */

	std::vector<std::string> getDroppedFilePaths() const;

	/* openColumnMenu
	Opens the 'add/remove channel' menu for the column under the cursor. */

	void openColumnMenu() const;

	/* getColumnAtCursor
	Returns the column below the cursor. */

	geColumn* getColumnAtCursor(Pixel x) const;

	/* getChannel
	Given a channel ID returns the UI channel it belongs to. */

	geChannel*       getChannel(ID channelId);
	const geChannel* getChannel(ID channelId) const;

	/* addColumn
	Adds new column at the end of the stack. Pass Column::id != 0 when loading 
	columns from model. */

	geColumn& addColumn(const c::channel::Track&);

	ChannelDragger         m_channelDragger;
	std::vector<geColumn*> m_columns;
};
} // namespace giada::v

#endif
