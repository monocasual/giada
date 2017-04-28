/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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
#include "../../../dialogs/gd_warnings.h"
#include "../../basics/boxtypes.h"
#include "column.h"
#include "sampleChannel.h"
#include "channelButton.h"
#include "keyboard.h"


int geKeyboard::indexColumn = 0;


/* -------------------------------------------------------------------------- */


geKeyboard::geKeyboard(int X, int Y, int W, int H)
: Fl_Scroll    (X, Y, W, H),
	bckspcPressed(false),
	endPressed   (false),
	spacePressed (false),
	addColumnBtn (nullptr)
{
	color(COLOR_BG_MAIN);
	type(Fl_Scroll::BOTH_ALWAYS);
	scrollbar.color(COLOR_BG_0);
	scrollbar.selection_color(COLOR_BG_1);
	scrollbar.labelcolor(COLOR_BD_1);
	scrollbar.slider(G_CUSTOM_BORDER_BOX);
	hscrollbar.color(COLOR_BG_0);
	hscrollbar.selection_color(COLOR_BG_1);
	hscrollbar.labelcolor(COLOR_BD_1);
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


void geKeyboard::freeChannel(geChannel *gch)
{
	gch->reset();
}


/* -------------------------------------------------------------------------- */


void geKeyboard::deleteChannel(geChannel *gch)
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


void geKeyboard::updateChannel(geChannel *gch)
{
	gch->update();
}


/* -------------------------------------------------------------------------- */


void geKeyboard::organizeColumns()
{
	/* if only one column exists don't cleanup: the initial column must
	 * stay here. */

	if (columns.size() == 1)
		return;

	/* otherwise delete all empty columns */
	/** FIXME - this for loop might not work correctly! */

	for (unsigned i=columns.size()-1; i>=1; i--) {
		if (columns.at(i)->isEmpty()) {
			//Fl::delete_widget(columns.at(i));
			delete columns.at(i);
			columns.erase(columns.begin() + i);
		}
	}

	/* compact column, avoid empty spaces */

	for (unsigned i=1; i<columns.size(); i++)
		columns.at(i)->position(columns.at(i-1)->x() + columns.at(i-1)->w() + 16, y());

	addColumnBtn->position(columns.back()->x() + columns.back()->w() + 16, y());

	/* recompute col indexes */

	refreshColIndexes();

	redraw();
}


/* -------------------------------------------------------------------------- */


void geKeyboard::cb_addColumn(Fl_Widget *v, void *p)
{
	((geKeyboard*)p)->__cb_addColumn(G_DEFAULT_COLUMN_WIDTH);
}


/* -------------------------------------------------------------------------- */


geChannel *geKeyboard::addChannel(int colIndex, Channel *ch, bool build)
{
	geColumn *col = getColumnByIndex(colIndex);

	/* no column with index 'colIndex' found? Just create it and set its index
	to 'colIndex'. */

	if (!col) {
		__cb_addColumn();
		col = columns.back();
		col->setIndex(colIndex);
		gu_log("[geKeyboard::addChannel] created new column with index=%d\n", colIndex);
	}

	gu_log("[geKeyboard::addChannel] add to column with index = %d\n", col->getIndex());
	return col->addChannel(ch);
}


/* -------------------------------------------------------------------------- */


void geKeyboard::refreshColumns()
{
	for (unsigned i=0; i<columns.size(); i++)
		columns.at(i)->refreshChannels();
}


/* -------------------------------------------------------------------------- */


geColumn *geKeyboard::getColumnByIndex(int index)
{
	for (unsigned i=0; i<columns.size(); i++)
		if (columns.at(i)->getIndex() == index)
			return columns.at(i);
	return nullptr;
}


/* -------------------------------------------------------------------------- */

/* TODO - the following event handling for play, stop, rewind, start rec and
so on should be moved to the proper widget: gdMainWindow or (better) geController. */

int geKeyboard::handle(int e)
{
	int ret = Fl_Group::handle(e);  // assume the buttons won't handle the Keyboard events
	switch (e) {
		case FL_FOCUS:
		case FL_UNFOCUS: {
			ret = 1;                	// enables receiving Keyboard events
			break;
		}
		case FL_SHORTCUT:           // in case widget that isn't ours has focus
		case FL_KEYDOWN:            // Keyboard key pushed
		case FL_KEYUP: {            // Keyboard key released

			/* rewind session. Avoid retrigs */

			if (e == FL_KEYDOWN) {
				if (Fl::event_key() == FL_BackSpace && !bckspcPressed) {
					bckspcPressed = true;
					glue_rewindSeq(false);          // not from GUI
					ret = 1;
					break;
				}
				else if (Fl::event_key() == FL_End && !endPressed) {
					endPressed = true;
					glue_startStopInputRec(false);  // not from GUI
					ret = 1;
					break;
				}
				else if (Fl::event_key() == FL_Enter && !enterPressed) {
					enterPressed = true;
					glue_startStopActionRec(false); // not from GUI
					ret = 1;
					break;
				}
				else if (Fl::event_key() == ' ' && !spacePressed) {
					spacePressed = true;
          glue_startStopSeq(false);      // unot from GUI
					ret = 1;
					break;
				}
			}
			else if (e == FL_KEYUP) {
				if (Fl::event_key() == FL_BackSpace)
					bckspcPressed = false;
				else if (Fl::event_key() == FL_End)
					endPressed = false;
				else if (Fl::event_key() == ' ')
					spacePressed = false;
				else if (Fl::event_key() == FL_Enter)
					enterPressed = false;
			}

			/* Walk button arrays, trying to match button's label with the Keyboard event.
			 * If found, set that button's value() based on up/down event,
			 * and invoke that button's callback() */

			for (unsigned i=0; i<columns.size(); i++)
				for (int k=1; k<columns.at(i)->children(); k++)
					ret &= ((geChannel*)columns.at(i)->child(k))->keyPress(e);
			break;
		}
	}
	return ret;
}


/* -------------------------------------------------------------------------- */


void geKeyboard::clear()
{
	for (unsigned i=0; i<columns.size(); i++)
		delete columns.at(i);
	columns.clear();
	indexColumn = 0;     // new columns will start from index=0
	addColumnBtn->position(8, y());
}


/* -------------------------------------------------------------------------- */


void geKeyboard::setChannelWithActions(geSampleChannel *gch)
{
	if (gch->ch->hasActions)
		gch->showActionButton();
	else
		gch->hideActionButton();
}


/* -------------------------------------------------------------------------- */


void geKeyboard::printChannelMessage(int res)
{
	if      (res == SAMPLE_NOT_VALID)
		gdAlert("This is not a valid WAVE file.");
	else if (res == SAMPLE_MULTICHANNEL)
		gdAlert("Multichannel samples not supported.");
	else if (res == SAMPLE_WRONG_BIT)
		gdAlert("This sample has an\nunsupported bit-depth (> 32 bit).");
	else if (res == SAMPLE_WRONG_ENDIAN)
		gdAlert("This sample has a wrong\nbyte order (not little-endian).");
	else if (res == SAMPLE_WRONG_FORMAT)
		gdAlert("This sample is encoded in\nan unsupported audio format.");
	else if (res == SAMPLE_READ_ERROR)
		gdAlert("Unable to read this sample.");
	else if (res == SAMPLE_PATH_TOO_LONG)
		gdAlert("File path too long.");
	else
		gdAlert("Unknown error.");
}


/* -------------------------------------------------------------------------- */


void geKeyboard::__cb_addColumn(int width)
{
	int colx;
	int colxw;
	int gap = 16;
	if (columns.size() == 0) {
		colx  = x() - xposition();  // mind the offset with xposition()
		colxw = colx + width;
	}
	else {
		geColumn *prev = columns.back();
		colx  = prev->x()+prev->w() + gap;
		colxw = colx + width;
	}

	/* add geColumn to geKeyboard and to columns vector */

	geColumn *gc = new geColumn(colx, y(), width, 2000, indexColumn, this);
  add(gc);
	columns.push_back(gc);
	indexColumn++;

	/* move addColumn button */

	addColumnBtn->position(colxw + gap, y());
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


geColumn *geKeyboard::getColumn(int i)
{
  return columns.at(i);
}
