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

#ifndef GE_KEYBOARD_H
#define GE_KEYBOARD_H

#include "core/const.h"
#include "core/idManager.h"
#include "gui/elems/basics/scroll.h"
#include <functional>
#include <vector>

namespace giada::v
{
class geButton;
class geResizerBar;
class geColumn;
class geChannel;
class geKeyboard : public geScroll
{
public:
	struct ColumnLayout
	{
		ID  id;
		int width;
	};

	geKeyboard();

	int  handle(int e) override;
	void draw() override;

	/* getChannelColumnId
	Given a channel ID, returns the ID of the column it belongs to. */

	ID getChannelColumnId(ID channelId) const;

	/* rebuild
	Rebuilds this widget from scratch. Used when the model has changed. */

	void rebuild();

	/* refresh
	Refreshes each column's channel, called on each GUI cycle. */

	void refresh();

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

	/* layout
	The column layout. Each element is a column with a specific width. */

	std::vector<ColumnLayout> layout;

private:
	static constexpr int COLUMN_GAP = 20;

	static void cb_addColumn(Fl_Widget* /*w*/, void* p);
	void        cb_addColumn();

	void addColumn(int width = G_DEFAULT_COLUMN_WIDTH, ID id = 0);

	/* getDroppedFilePaths
	Returns a vector of audio file paths after a drag-n-drop from desktop
	event. */

	std::vector<std::string> getDroppedFilePaths() const;

	/* getColumn
	Returns the column given the ID. */

	geColumn* getColumn(ID id);

	/* getColumnAtCursor
	Returns the column below the cursor. */

	geColumn* getColumnAtCursor(Pixel x);

	/* getChannel
	Given a channel ID returns the UI channel it belongs to. */

	geChannel*       getChannel(ID channelId);
	const geChannel* getChannel(ID channelId) const;

	/* storeLayout
	Stores the current column layout into the layout vector. */

	void storeLayout();

	m::IdManager           m_columnId;
	std::vector<geColumn*> m_columns;

	geButton* m_addColumnBtn;
};
} // namespace giada::v

#endif
