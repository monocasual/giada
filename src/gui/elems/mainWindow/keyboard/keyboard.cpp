/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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
#include "core/model/model.h"
#include "core/channels/sampleChannel.h"
#include "glue/io.h"
#include "utils/log.h"
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
int geKeyboard::indexGen = 0;


/* -------------------------------------------------------------------------- */


geKeyboard::geKeyboard(int X, int Y, int W, int H)
: Fl_Scroll     (X, Y, W, H),
  m_addColumnBtn(nullptr)
{
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

	/* Add 6 empty columns as initial layout. */

	//cb_addColumn();
	//cb_addColumn();
	//cb_addColumn();
	//cb_addColumn();
	//cb_addColumn();
	//cb_addColumn();
}


/* -------------------------------------------------------------------------- */


void geKeyboard::init()
{
	clear();
	m_columns.clear();
	m_addColumnBtn = new geButton(8, y(), 200, 20, "Add new column");
	m_addColumnBtn->callback(cb_addColumn, (void*) this);
	add(m_addColumnBtn);
}


/* -------------------------------------------------------------------------- */


void geKeyboard::rebuild()
{
	init();

	const std::vector<std::unique_ptr<m::Channel>>& channels = m::model::getLayout()->channels;

	for (const std::unique_ptr<m::Channel>& ch : channels) {
		if (ch->id == m::mixer::MASTER_OUT_CHANNEL_ID ||
			ch->id == m::mixer::MASTER_IN_CHANNEL_ID)
			continue;
		geColumn* column;
		if (ch->columnIndex < m_columns.size())
			column = m_columns[ch->columnIndex];
		else
			column = cb_addColumn(G_DEFAULT_COLUMN_WIDTH, ch->columnIndex);
		column->addChannel(ch.get(), G_GUI_CHANNEL_H_1);
	}
}


/* -------------------------------------------------------------------------- */


void geKeyboard::organizeColumns()
{
#if 0
	if (columns.size() == 0)
		return;

	/* Otherwise delete all empty columns. */

	for (size_t i=columns.size(); i-- > 0;) {
		if (columns.at(i)->isEmpty()) {
			Fl::delete_widget(columns.at(i));
			columns.erase(columns.begin() + i);
		}
	}

	/* Zero columns? Just add the "add column" button. Compact column and avoid 
	empty spaces otherwise. */

	if (columns.size() == 0)
		addColumnBtn->position(x() - xposition(), y());
	else {
		for (size_t i=0; i<columns.size(); i++) {
			int pos = i == 0 ? x() - xposition() : columns.at(i-1)->x() + columns.at(i-1)->w() + COLUMN_GAP;
			columns.at(i)->position(pos, y());
		}
		addColumnBtn->position(columns.back()->x() + columns.back()->w() + COLUMN_GAP, y());
	}

	refreshColIndexes();
	redraw();
#endif
}


/* -------------------------------------------------------------------------- */


void geKeyboard::cb_addColumn(Fl_Widget* v, void* p)
{
	((geKeyboard*)p)->cb_addColumn(G_DEFAULT_COLUMN_WIDTH);
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
	}
	return Fl_Group::handle(e);     // Assume the buttons won't handle the Keyboard events
}


/* -------------------------------------------------------------------------- */


geColumn* geKeyboard::cb_addColumn(int width, int index)
{
	int colx = x() - xposition();  // Mind the x-scroll offset with xposition()

	/* If this is not the first column... */

	if (m_columns.size() > 0)
		colx = m_columns.back()->x() + m_columns.back()->w() + COLUMN_GAP;

	/* Generate new index. If not passed in. */

	int newIndex = index != -1 ? index : m_columns.size(); 

	/* Add a new column + a new resizer bar. */

	geColumn*     column = new geColumn(colx, y(), width, h(), newIndex);
	geResizerBar* bar    = new geResizerBar(colx + width, y(), COLUMN_GAP, h(), G_MIN_COLUMN_WIDTH, geResizerBar::HORIZONTAL);
	add(column);
	add(bar);
	m_columns.push_back(column);

	/* And then shift the "add column" button on the rightmost edge. */

	m_addColumnBtn->position(colx + width + COLUMN_GAP, y());

	redraw();

	return column;
}


/* -------------------------------------------------------------------------- */


void geKeyboard::addColumn(int width)
{
	cb_addColumn(width);
}


/* -------------------------------------------------------------------------- */


void geKeyboard::forEachChannel(std::function<void(geChannel* c)> f) const
{
	for (geColumn* column : m_columns) 
		column->forEachChannel(f);
}


/* -------------------------------------------------------------------------- */


geChannel* geKeyboard::getChannel(ID chanID)
{
	for (geColumn* column : m_columns) {
		geChannel* c = column->getChannel(chanID);
		if (c != nullptr) 
			return c;
	}
	assert(false);
	return nullptr;
}

}} // giada::v::