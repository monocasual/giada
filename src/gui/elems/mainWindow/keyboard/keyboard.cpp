/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#include <cassert>
#include <FL/fl_draw.H>
#include "core/model/model.h"
#include "core/channels/sampleChannel.h"
#include "glue/io.h"
#include "glue/channel.h"
#include "utils/fs.h"
#include "utils/log.h"
#include "utils/vector.h"
#include "utils/string.h"
#include "gui/dispatcher.h"
#include "gui/dialogs/warnings.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/resizerBar.h"
#include "column.h"
#include "sampleChannel.h"
#include "channelButton.h"
#include "keyboard.h"


namespace giada {
namespace v
{
geKeyboard::geKeyboard(int X, int Y, int W, int H)
: Fl_Scroll     (X, Y, W, H),
  m_addColumnBtn(nullptr)
{
	end();

	color(G_COLOR_GREY_1);
	type(Fl_Scroll::BOTH_ALWAYS);
	scrollbar.color(G_COLOR_GREY_2);
	scrollbar.selection_color(G_COLOR_GREY_4);
	scrollbar.labelcolor(G_COLOR_LIGHT_1);
	scrollbar.slider(G_CUSTOM_BORDER_BOX);
	hscrollbar.color(G_COLOR_GREY_2);
	hscrollbar.selection_color(G_COLOR_GREY_4);
	hscrollbar.labelcolor(G_COLOR_LIGHT_1);
	hscrollbar.slider(G_CUSTOM_BORDER_BOX);

	init();
}


/* -------------------------------------------------------------------------- */


void geKeyboard::init()
{
	m_columnId = m::IdManager();

	deleteAllColumns();

	/* Add 6 empty columns as initial layout. */

	layout.clear();
	layout.push_back({1, G_DEFAULT_COLUMN_WIDTH});
	layout.push_back({2, G_DEFAULT_COLUMN_WIDTH});
	layout.push_back({3, G_DEFAULT_COLUMN_WIDTH});
	layout.push_back({4, G_DEFAULT_COLUMN_WIDTH});
	layout.push_back({5, G_DEFAULT_COLUMN_WIDTH});
	layout.push_back({6, G_DEFAULT_COLUMN_WIDTH});
}


/* -------------------------------------------------------------------------- */


void geKeyboard::rebuild()
{
	/* Wipe out all columns and add them according to the current layout. */

	deleteAllColumns();
	
	for (ColumnLayout c : layout)
		addColumn(c.width, c.id);

	/* Parse the model and assign each channel to its column. */

	m::model::ChannelsLock lock(m::model::channels);

	for (const m::Channel* ch : m::model::channels)
		if (!ch->isInternal())
			getColumn(ch->columnId)->addChannel(ch->id, ch->type, ch->height);
	
	redraw();
}


/* -------------------------------------------------------------------------- */


void geKeyboard::deleteColumn(ID id)
{
	u::vector::removeIf(layout, [=](const ColumnLayout& c) { return c.id == id; });
	rebuild();
}


/* -------------------------------------------------------------------------- */


void geKeyboard::deleteAllColumns(bool clearLayout)
{
	Fl_Scroll::clear();
	m_columns.clear();

	m_addColumnBtn = new geButton(8, y(), 200, 20, "Add new column");
	m_addColumnBtn->callback(cb_addColumn, (void*) this);
	add(m_addColumnBtn);
}


/* -------------------------------------------------------------------------- */


void geKeyboard::cb_addColumn(Fl_Widget* v, void* p)
{
	((geKeyboard*)p)->cb_addColumn();
}


/* -------------------------------------------------------------------------- */


void geKeyboard::refresh()
{
	for (geColumn* c : m_columns)
		c->refresh();
}


/* -------------------------------------------------------------------------- */


int geKeyboard::handle(int e)
{
	switch (e) {
		case FL_FOCUS:
		case FL_UNFOCUS: {
			return 1;               // Enables receiving Keyboard events
		}
		case FL_SHORTCUT:           // In case widget that isn't ours has focus
		case FL_KEYDOWN:            // Keyboard key pushed
		case FL_KEYUP: {            // Keyboard key released
			dispatcher::dispatchKey(e);
			return 1;
		}
		case FL_DND_ENTER:          // return(1) for these events to 'accept' dnd
		case FL_DND_DRAG:
		case FL_DND_RELEASE: {
			return 1;
		}
		case FL_PASTE: {            // handle actual drop (paste) operation
			const geColumn* c = getColumnAtCursor(Fl::event_x());
			if (c != nullptr)
				c::channel::addAndLoadChannels(c->id, getDroppedFilePaths());
			return 1;
		}
	}
	return Fl_Group::handle(e);     // Assume the buttons won't handle the Keyboard events
}


/* -------------------------------------------------------------------------- */


void geKeyboard::draw()
{
	Fl_Scroll::draw();

	/* Paint columns background. Use a clip to draw only what's visible. */

	fl_color(G_COLOR_GREY_1_5);

	fl_push_clip(
		x(), 
		y(), 
		w() - scrollbar_size() - (G_GUI_OUTER_MARGIN * 2), 
		h() - scrollbar_size() - (G_GUI_OUTER_MARGIN * 2));

	for (const geColumn* c : m_columns) {
		fl_rectf(c->x(), c->y() + c->h(), c->w(), h() + yposition());
		// TODO - c->resizerBar->size(c->resizerBar->w(), c->h() < h() ? h() : c->h());
	}

	fl_pop_clip();
}


/* -------------------------------------------------------------------------- */


void geKeyboard::cb_addColumn()
{
	addColumn();
	storeLayout();
}


/* -------------------------------------------------------------------------- */


void geKeyboard::addColumn(int width, ID id)
{
	int colx = x() - xposition();  // Mind the x-scroll offset with xposition()

	/* If this is not the first column... */

	if (m_columns.size() > 0)
		colx = m_columns.back()->x() + m_columns.back()->w() + COLUMN_GAP;

	/* Generate new index. If not passed in. */

	m_columnId.set(id);

	/* Add a new column + a new resizer bar. */

	geResizerBar* bar    = new geResizerBar(colx + width, y(), COLUMN_GAP, h(), G_MIN_COLUMN_WIDTH, geResizerBar::HORIZONTAL);
	geColumn*     column = new geColumn(colx, y(), width, 20, m_columnId.get(id), bar);

	/* Store the column width in layout when the resizer bar is released. */

	bar->onRelease = [=](const Fl_Widget* w)
	{
		storeLayout();
	};

	add(column);
	add(bar);
	m_columns.push_back(column);

	/* And then shift the "add column" button on the rightmost edge. */

	m_addColumnBtn->position(colx + width + COLUMN_GAP, y());

	redraw();
}


/* -------------------------------------------------------------------------- */


void geKeyboard::forEachChannel(std::function<void(geChannel& c)> f) const
{
	for (geColumn* column : m_columns) 
		column->forEachChannel(f);
}


void geKeyboard::forEachColumn(std::function<void(const geColumn& c)> f) const
{
	for (geColumn* column : m_columns) 
		f(*column);
}


/* -------------------------------------------------------------------------- */


geColumn* geKeyboard::getColumn(ID id)
{
	for (geColumn* c : m_columns) 
		if (c->id == id)
			return c;
	assert(false);
	return nullptr;
}


geColumn* geKeyboard::getColumnAtCursor(Pixel px)
{
	px += xposition();
	for (geColumn* c : m_columns)
		if (px > c->x() && px <= c->x() + c->w())
			return c;
	return nullptr;
}


/* -------------------------------------------------------------------------- */


geChannel* geKeyboard::getChannel(ID channelId)
{
	for (geColumn* column : m_columns) {
		geChannel* c = column->getChannel(channelId);
		if (c != nullptr) 
			return c;
	}
	assert(false);
	return nullptr;
}


/* -------------------------------------------------------------------------- */


std::vector<std::string> geKeyboard::getDroppedFilePaths() const
{			
	std::vector<std::string> paths = u::string::split(Fl::event_text(), "\n");
		for (std::string& p : paths)
			p = u::fs::stripFileUrl(p);
	return paths;
}


/* -------------------------------------------------------------------------- */


void geKeyboard::storeLayout()
{
	layout.clear();
	for (const geColumn* c : m_columns)
		layout.push_back({ c->id, c->w() });
}

}} // giada::v::
