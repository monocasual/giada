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

#ifndef GE_KEYBOARD_H
#define GE_KEYBOARD_H

#include "core/const.h"
#include "core/idManager.h"
#include "deps/geompp/src/rect.hpp"
#include "gui/elems/basics/scroll.h"
#include <FL/Fl_Box.H>
#include <functional>
#include <memory>
#include <vector>

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

	/* getChannelColumnId
	Given a channel ID, returns the ID of the column it belongs to. */

	ID getChannelColumnId(ID channelId) const;

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
	Adds new column at the end of the stack. Pass id != 0 when loading column
	from model. */

	void addColumn(int width = G_DEFAULT_COLUMN_WIDTH, ID id = 0);

	/* deleteColumn
	Deletes column by id. */

	void deleteColumn(ID id);

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

	/* getColumn
	Returns the column given the ID. */

	geColumn* getColumn(ID id);

	/* getColumnAtCursor
	Returns the column below the cursor. */

	geColumn* getColumnAtCursor(Pixel x) const;

	/* getChannel
	Given a channel ID returns the UI channel it belongs to. */

	geChannel*       getChannel(ID channelId);
	const geChannel* getChannel(ID channelId) const;

	/* storeLayout
	Stores the current column layout into the layout vector. */

	void storeLayout();

	m::IdManager           m_columnId;
	ChannelDragger         m_channelDragger;
	std::vector<geColumn*> m_columns;
};
} // namespace giada::v

#endif
