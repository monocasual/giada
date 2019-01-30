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


#include "../../../../core/sampleChannel.h"
#include "../../../../glue/transport.h"
#include "../../../../glue/io.h"
#include "../../../../utils/log.h"
#include "../../../dialogs/warnings.h"
#include "../../../dispatcher.h"
#include "../../basics/boxtypes.h"
#include "column.h"
#include "sampleChannel.h"
#include "channelButton.h"
#include "keyboard.h"


namespace giada {
namespace v
{
int geKeyboard::indexColumn = 0;


/* -------------------------------------------------------------------------- */


geKeyboard::geKeyboard(int X, int Y, int W, int H)
: Fl_Scroll    (X, Y, W, H),
	bckspcPressed(false),
	endPressed   (false),
	spacePressed (false),
	addColumnBtn (nullptr)
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

	addColumnBtn = new geButton(8, y(), 200, 20, "Add new column");
	addColumnBtn->callback(cb_addColumn, (void*) this);
	add(addColumnBtn);

	init();
}


/* -------------------------------------------------------------------------- */


void geKeyboard::init()
{
	/* add 6 empty columns as init layout */

	__cb_addColumn();
	__cb_addColumn();
	__cb_addColumn();
	__cb_addColumn();
	__cb_addColumn();
	__cb_addColumn();
}


/* -------------------------------------------------------------------------- */


void geKeyboard::freeChannel(geChannel* gch)
{
	gch->reset();
}


/* -------------------------------------------------------------------------- */


void geKeyboard::deleteChannel(geChannel* gch)
{
	for (unsigned i=0; i<columns.size(); i++) {
		int k = columns.at(i)->find(gch);
		if (k != columns.at(i)->children()) {
			columns.at(i)->deleteChannel(gch);
			return;
		}
	}
}


/* -------------------------------------------------------------------------- */


void geKeyboard::updateChannel(geChannel* gch)
{
	gch->update();
}


/* -------------------------------------------------------------------------- */


void geKeyboard::organizeColumns()
{
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
}


/* -------------------------------------------------------------------------- */


void geKeyboard::cb_addColumn(Fl_Widget* v, void* p)
{
	((geKeyboard*)p)->__cb_addColumn(G_DEFAULT_COLUMN_WIDTH);
}


/* -------------------------------------------------------------------------- */


geChannel* geKeyboard::addChannel(int colIndex, m::Channel* ch, int size, bool build)
{
	geColumn* col = getColumnByIndex(colIndex);

	/* no column with index 'colIndex' found? Just create it and set its index
	to 'colIndex'. */

	if (!col) {
		__cb_addColumn();
		col = columns.back();
		col->setIndex(colIndex);
		gu_log("[geKeyboard::addChannel] created new column with index=%d\n", colIndex);
	}

	gu_log("[geKeyboard::addChannel] add to column with index=%d, size=%d\n", 
		col->getIndex(), size);
	return col->addChannel(ch, size);
}


/* -------------------------------------------------------------------------- */


void geKeyboard::refreshColumns()
{
	for (unsigned i=0; i<columns.size(); i++)
		columns.at(i)->refreshChannels();
}


/* -------------------------------------------------------------------------- */


geColumn* geKeyboard::getColumnByIndex(int index)
{
	for (unsigned i=0; i<columns.size(); i++)
		if (columns.at(i)->getIndex() == index)
			return columns.at(i);
	return nullptr;
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


void geKeyboard::clear()
{
	for (unsigned i=0; i<columns.size(); i++)
		Fl::delete_widget(columns.at(i));
	columns.clear();
	indexColumn = 0;     // new columns will start from index=0
	addColumnBtn->position(8, y());
}


/* -------------------------------------------------------------------------- */


void geKeyboard::setChannelWithActions(geSampleChannel* gch)
{
	if (gch->ch->hasActions)
		gch->showActionButton();
	else
		gch->hideActionButton();
}


/* -------------------------------------------------------------------------- */


void geKeyboard::printChannelMessage(int res)
{
	if      (res == G_RES_ERR_WRONG_DATA)
		gdAlert("Multichannel samples not supported.");
	else if (res == G_RES_ERR_IO)
		gdAlert("Unable to read this sample.");
	else if (res == G_RES_ERR_PATH_TOO_LONG)
		gdAlert("File path too long.");
	else if (res == G_RES_ERR_NO_DATA)
		gdAlert("No file specified.");
	else
		gdAlert("Unknown error.");
}


/* -------------------------------------------------------------------------- */


void geKeyboard::__cb_addColumn(int width)
{
	int colx;
	int colxw;
	if (columns.size() == 0) {
		colx  = x() - xposition();  // mind the offset with xposition()
		colxw = colx + width;
	}
	else {
		geColumn* prev = columns.back();
		colx  = prev->x()+prev->w() + COLUMN_GAP;
		colxw = colx + width;
	}

	/* add geColumn to geKeyboard and to columns vector */

	geColumn* gc = new geColumn(colx, y(), width, 2000, indexColumn, this);
  add(gc);
	columns.push_back(gc);
	indexColumn++;

	/* move addColumn button */

	addColumnBtn->position(colxw + COLUMN_GAP, y());
	redraw();

	gu_log("[geKeyboard::__cb_addColumn] new column added (index=%d, w=%d), total count=%d, addColumn(x)=%d\n",
		gc->getIndex(), width, columns.size(), addColumnBtn->x());

	/* recompute col indexes */

	refreshColIndexes();
}


/* -------------------------------------------------------------------------- */


void geKeyboard::addColumn(int width)
{
	__cb_addColumn(width);
}


/* -------------------------------------------------------------------------- */


void geKeyboard::refreshColIndexes()
{
	for (unsigned i=0; i<columns.size(); i++)
		columns.at(i)->setIndex(i);
}


/* -------------------------------------------------------------------------- */


int geKeyboard::getColumnWidth(int i)
{
  return getColumnByIndex(i)->w();
}


/* -------------------------------------------------------------------------- */


geColumn* geKeyboard::getColumn(int i)
{
  return columns.at(i);
}

}} // giada::v::