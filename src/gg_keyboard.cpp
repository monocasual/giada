/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gg_keyboard
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2014 Giovanni A. Zuliani | Monocasual
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
 * ------------------------------------------------------------------ */


#include "gg_keyboard.h"
#include "gd_browser.h"
#include "gd_keyGrabber.h"
#include "gd_midiGrabber.h"
#include "gd_midiOutputSetup.h"
#include "gd_actionEditor.h"
#include "gd_warnings.h"
#include "gd_mainWindow.h"
#include "gd_editor.h"
#include "ge_channel.h"
#include "const.h"
#include "mixer.h"
#include "wave.h"
#include "conf.h"
#include "patch.h"
#include "graphics.h"
#include "glue.h"
#include "recorder.h"
#include "pluginHost.h"
#include "channel.h"
#include "sampleChannel.h"
#include "midiChannel.h"
#include "log.h"

#ifdef WITH_VST
#include "gd_pluginList.h"
#endif


extern Mixer 		     G_Mixer;
extern Conf  		     G_Conf;
extern Patch 		     G_Patch;
extern gdMainWindow *mainWin;


gStatus::gStatus(int x, int y, int w, int h, SampleChannel *ch, const char *L)
: Fl_Box(x, y, w, h, L), ch(ch) {}

void gStatus::draw()
{
  fl_rect(x(), y(), w(), h(), COLOR_BD_0);              // reset border
  fl_rectf(x()+1, y()+1, w()-2, h()-2, COLOR_BG_0);     // reset background

  if (ch != NULL) {
    if (ch->status    & (STATUS_WAIT | STATUS_ENDING | REC_ENDING | REC_WAITING) ||
        ch->recStatus & (REC_WAITING | REC_ENDING))
    {
      fl_rect(x(), y(), w(), h(), COLOR_BD_1);
    }
    else
    if (ch->status == STATUS_PLAY)
      fl_rect(x(), y(), w(), h(), COLOR_BD_1);
    else
      fl_rectf(x()+1, y()+1, w()-2, h()-2, COLOR_BG_0);     // status empty


    if (G_Mixer.chanInput == ch)
      fl_rectf(x()+1, y()+1, w()-2, h()-2, COLOR_BG_3);     // take in progress
    else
    if (recorder::active && recorder::canRec(ch))
      fl_rectf(x()+1, y()+1, w()-2, h()-2, COLOR_BG_4);     // action record

    /* equation for the progress bar:
     * ((chanTracker - chanStart) * w()) / (chanEnd - chanStart). */

    int pos = ch->getPosition();
    if (pos == -1)
      pos = 0;
    else
      pos = (pos * (w()-1)) / (ch->end - ch->begin);
    fl_rectf(x()+1, y()+1, pos, h()-2, COLOR_BG_2);
  }
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gModeBox::gModeBox(int x, int y, int w, int h, SampleChannel *ch, const char *L)
  : Fl_Menu_Button(x, y, w, h, L), ch(ch)
{
  box(G_BOX);
  textsize(11);
  textcolor(COLOR_TEXT_0);
  color(COLOR_BG_0);

  add("Loop . basic",      0, cb_change_chanmode, (void *)LOOP_BASIC);
  add("Loop . once",       0, cb_change_chanmode, (void *)LOOP_ONCE);
  add("Loop . once . bar", 0, cb_change_chanmode, (void *)LOOP_ONCE_BAR);
  add("Loop . repeat",     0, cb_change_chanmode, (void *)LOOP_REPEAT);
  add("Oneshot . basic",   0, cb_change_chanmode, (void *)SINGLE_BASIC);
  add("Oneshot . press",   0, cb_change_chanmode, (void *)SINGLE_PRESS);
  add("Oneshot . retrig",  0, cb_change_chanmode, (void *)SINGLE_RETRIG);
  add("Oneshot . endless", 0, cb_change_chanmode, (void *)SINGLE_ENDLESS);
}


/* ------------------------------------------------------------------ */


void gModeBox::draw() {
  fl_rect(x(), y(), w(), h(), COLOR_BD_0);    // border
  switch (ch->mode) {
    case LOOP_BASIC:
      fl_draw_pixmap(loopBasic_xpm, x()+1, y()+1);
      break;
    case LOOP_ONCE:
      fl_draw_pixmap(loopOnce_xpm, x()+1, y()+1);
      break;
    case LOOP_ONCE_BAR:
      fl_draw_pixmap(loopOnceBar_xpm, x()+1, y()+1);
      break;
    case LOOP_REPEAT:
      fl_draw_pixmap(loopRepeat_xpm, x()+1, y()+1);
      break;
    case SINGLE_BASIC:
      fl_draw_pixmap(oneshotBasic_xpm, x()+1, y()+1);
      break;
    case SINGLE_PRESS:
      fl_draw_pixmap(oneshotPress_xpm, x()+1, y()+1);
      break;
    case SINGLE_RETRIG:
      fl_draw_pixmap(oneshotRetrig_xpm, x()+1, y()+1);
      break;
    case SINGLE_ENDLESS:
      fl_draw_pixmap(oneshotEndless_xpm, x()+1, y()+1);
      break;
  }
}


/* ------------------------------------------------------------------ */


void gModeBox::cb_change_chanmode(Fl_Widget *v, void *p) { ((gModeBox*)v)->__cb_change_chanmode((intptr_t)p); }


/* ------------------------------------------------------------------ */


void gModeBox::__cb_change_chanmode(int mode)
{
  ch->mode = mode;

  /* what to do when the channel is playing and you change the mode?
   * Nothing, since v0.5.3. Just refresh the action editor window, in
   * case it's open */

  gu_refreshActionEditor();
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


int gKeyboard::indexColumn = 0;


/* ------------------------------------------------------------------ */


gKeyboard::gKeyboard(int X, int Y, int W, int H)
: Fl_Scroll    (X, Y, W, H),
	bckspcPressed(false),
	endPressed   (false),
	spacePressed (false),
	addColumnBtn (NULL)
{
	color(COLOR_BG_MAIN);
	type(Fl_Scroll::BOTH_ALWAYS);
	scrollbar.color(COLOR_BG_0);
	scrollbar.selection_color(COLOR_BG_1);
	scrollbar.labelcolor(COLOR_BD_1);
	scrollbar.slider(G_BOX);
	hscrollbar.color(COLOR_BG_0);
	hscrollbar.selection_color(COLOR_BG_1);
	hscrollbar.labelcolor(COLOR_BD_1);
	hscrollbar.slider(G_BOX);

	init();
}


/* ------------------------------------------------------------------ */


void gKeyboard::init()
{
	/* add 6 empty columns as init layout */

	__cb_addColumn();
	__cb_addColumn();
	__cb_addColumn();
	__cb_addColumn();
	__cb_addColumn();
	__cb_addColumn();
}


/* ------------------------------------------------------------------ */


void gKeyboard::freeChannel(gChannel *gch)
{
	gch->reset();
}


/* ------------------------------------------------------------------ */


void gKeyboard::deleteChannel(gChannel *gch)
{
	for (unsigned i=0; i<columns.size; i++) {
		int k = columns.at(i)->find(gch);
		if (k != columns.at(i)->children()) {
			columns.at(i)->deleteChannel(gch);
			return;
		}
	}
}


/* ------------------------------------------------------------------ */


void gKeyboard::updateChannel(gChannel *gch)
{
	gch->update();
}


/* ------------------------------------------------------------------ */


void gKeyboard::organizeColumns()
{
	/* if only one column exists don't cleanup: the initial column must
	 * stay here. */

	if (columns.size == 1)
		return;

	/* otherwise delete all empty columns */
	/** FIXME - this for loop might not work correctly! */

	for (unsigned i=columns.size-1; i>=1; i--) {
		if (columns.at(i)->isEmpty()) {
			delete columns.at(i);
			columns.del(i);
		}
	}

	/* compact column, avoid empty spaces */

	for (unsigned i=1; i<columns.size; i++)
		columns.at(i)->position(columns.at(i-1)->x() + columns.at(i-1)->w() + 16, y());

	addColumnBtn->position(columns.last()->x() + columns.last()->w() + 16, y());

	redraw();
}


/* ------------------------------------------------------------------ */


void gKeyboard::cb_addColumn(Fl_Widget *v, void *p) { ((gKeyboard*)p)->__cb_addColumn(); }


/* ------------------------------------------------------------------ */


gChannel *gKeyboard::addChannel(int colIndex, Channel *ch, bool build)
{
	gColumn *col = NULL;

	if (build) {
		__cb_addColumn();
		col = columns.last();
		col->setIndex(colIndex);
	}
	else {
		for (unsigned i=0; i<columns.size; i++)
			if (columns.at(i)->getIndex() == colIndex) {
				col = columns.at(i);
				break;
			}
		if (!col) {
			gLog("[gKeyboard::addChannel] column not found!\n");
			return NULL;
		}
		gLog("[ggKeyboard::addChannel] add to column with index = %d\n", col->getIndex());
	}
  //
  // TODO - resize column if too small, to avoid FLTK issues with invisible
  // widgets
  //
  // col->resize(col->x(), col->y(), 400, col->h());
  //
	return col->addChannel(ch);
}


/* ------------------------------------------------------------------ */


void gKeyboard::refreshColumns()
{
	for (unsigned i=0; i<columns.size; i++)
		columns.at(i)->refreshChannels();
}


/* ------------------------------------------------------------------ */


int gKeyboard::handle(int e)
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
					glue_rewindSeq();
					ret = 1;
					break;
				}
				else if (Fl::event_key() == FL_End && !endPressed) {
					endPressed = true;
					glue_startStopInputRec(false);  // update gui
					ret = 1;
					break;
				}
				else if (Fl::event_key() == FL_Enter && !enterPressed) {
					enterPressed = true;
					glue_startStopActionRec();
					ret = 1;
					break;
				}
				else if (Fl::event_key() == ' ' && !spacePressed) {
					spacePressed = true;
					G_Mixer.running ? glue_stopSeq() : glue_startSeq();
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

			for (unsigned i=0; i<columns.size; i++)
				for (int k=1; k<columns.at(i)->children(); k++)
					ret &= ((gChannel*)columns.at(i)->child(k))->keyPress(e);
			break;
		}
	}
	return ret;
}


/* ------------------------------------------------------------------ */


void gKeyboard::clear()
{
	Fl_Scroll::clear();
	columns.clear();
	indexColumn = 0;     // new columns will start from index=0
	init();
}


/* ------------------------------------------------------------------ */


void gKeyboard::setChannelWithActions(gSampleChannel *gch)
{
	if (gch->ch->hasActions)
		gch->addActionButton();
	else
		gch->delActionButton();
}


/* ------------------------------------------------------------------ */


void gKeyboard::printChannelMessage(int res)
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

/* ------------------------------------------------------------------ */


void gKeyboard::__cb_addColumn()
{
	int colx;
	int colxw;
	int colw = 380;
	if (columns.size == 0) {
		colx  = x() - xposition();  // mind the offset with xposition()
		colxw = colx + colw;
		addColumnBtn = new gClick(colxw + 16, y(), 200, 20, "Add new column");
		addColumnBtn->callback(cb_addColumn, (void*) this);
		add(addColumnBtn);
	}
	else {
		gColumn *prev = columns.last();
		colx  = prev->x()+prev->w() + 16;
		colxw = colx + colw;
		addColumnBtn->position(colxw-4, y());
	}
	gColumn *gc = new gColumn(colx, y(), colw-20, 2000, indexColumn, this);
  add(gc);
  indexColumn++;
	columns.add(gc);
	redraw();

	gLog("[gKeyboard] new column added (index = %d), total count=%d, addColumn=%d\n", gc->getIndex(), columns.size, addColumnBtn->x());
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gColumn::gColumn(int X, int Y, int W, int H, int index, gKeyboard *parent)
	: Fl_Group(X, Y, W, H), index(index)
{
	begin();
	addChannelBtn = new gClick(x(), y(), w(), 20, "Add new channel");
	end();

  resizer = new gResizerBar(x()+w(), y(), 16, h(), false);
  resizer->setMinSize(116);
  parent->add(resizer);

  /* parent() can be NULL: at this point gColumn is still detached from any
   * parent. We use a custom gKeyboard *parent instead. */

  box(FL_BORDER_BOX);

	addChannelBtn->callback(cb_addChannel, (void*)this);

}


/* ------------------------------------------------------------------ */


gColumn::~gColumn()
{
  //delete resizer;   //FIXME - segfault on quit!!!
  //resizer = NULL;   //FIXME - segfault on quit!!!
}


/* ------------------------------------------------------------------ */


int gColumn::handle(int e)
{
	int ret = Fl_Group::handle(e);  // assume the buttons won't handle the Keyboard events
	switch (e) {
		case FL_DND_ENTER:           	// return(1) for these events to 'accept' dnd
		case FL_DND_DRAG:
		case FL_DND_RELEASE: {
			ret = 1;
			break;
		}
		case FL_PASTE: {              // handle actual drop (paste) operation
			SampleChannel *c = (SampleChannel*) glue_addChannel(index, CHANNEL_SAMPLE);
			int result = glue_loadChannel(c, gTrim(gStripFileUrl(Fl::event_text())).c_str());
			if (result != SAMPLE_LOADED_OK) {
				deleteChannel(c->guiChannel);
				((gKeyboard *)parent())->printChannelMessage(result);
			}
			ret = 1;
			break;
		}
	}
	return ret;
}


/* ------------------------------------------------------------------ */


void gColumn::resize(int X, int Y, int W, int H)
{
  /* resize all children */

  int ch = children();
  for (int i=0; i<ch; i++) {
    Fl_Widget *c = child(i);
    c->resize(X, Y + (i * (c->h() + 4)), W, c->h());
  }

  /* resize group itself */

  x(X); y(Y); w(W); h(H);

  /* resize resizerBar */

  resizer->size(16, H);
}


/* ------------------------------------------------------------------ */


void gColumn::refreshChannels()
{
	for (int i=1; i<children(); i++)
		((gChannel*) child(i))->refresh();
}


/* ------------------------------------------------------------------ */


void gColumn::draw()
{
	fl_color(fl_rgb_color(27, 27, 27));
	fl_rectf(x(), y(), w(), h());
	Fl_Group::draw();
}


/* ------------------------------------------------------------------ */


void gColumn::cb_addChannel(Fl_Widget *v, void *p) { ((gColumn*)p)->__cb_addChannel(); }


/* ------------------------------------------------------------------ */


gChannel *gColumn::addChannel(class Channel *ch)
{
	gChannel *gch = NULL;

	if (ch->type == CHANNEL_SAMPLE)
		gch = (gSampleChannel*) new gSampleChannel(
				x(),
				y() + children() * 24,
				w(),
				20,
				(SampleChannel*) ch);
	else
		gch = (gMidiChannel*) new gMidiChannel(
				x(),
				y() + children() * 24,
				w(),
				20,
				(MidiChannel*) ch);

  gch->box(FL_BORDER_BOX);
	add(gch);
  resize(x(), y(), w(), (children() * 24) + 66); // evil space for drag n drop
  redraw();
	parent()->redraw();               // redraw Keyboard

	//~ ch->column = index;
	return gch;
}


/* ------------------------------------------------------------------ */


void gColumn::deleteChannel(gChannel *gch)
{
	gch->hide();
	remove(gch);
	delete gch;

	/* reposition all other channels and resize this group */
	/** TODO
	 * reposition is useless when called by gColumn::clear(). Add a new
	 * parameter to skip the operation */

	for (int i=0; i<children(); i++) {
		gch = (gChannel*) child(i);
		gch->position(gch->x(), y()+(i*24));
	}
	size(w(), children() * 24 + 66);  // evil space for drag n drop
	redraw();
}


/* ------------------------------------------------------------------ */


void gColumn::__cb_addChannel()
{
	gLog("[gColumn::__cb_addChannel] index = %d\n", index);
	int type = openTypeMenu();
	if (type)
		glue_addChannel(index, type);
}


/* ------------------------------------------------------------------ */


int gColumn::openTypeMenu()
{
	Fl_Menu_Item rclick_menu[] = {
		{"Sample channel"},
		{"MIDI channel"},
		{0}
	};

	Fl_Menu_Button *b = new Fl_Menu_Button(0, 0, 100, 50);
	b->box(G_BOX);
	b->textsize(11);
	b->textcolor(COLOR_TEXT_0);
	b->color(COLOR_BG_0);

	const Fl_Menu_Item *m = rclick_menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, b);
	if (!m) return 0;

	if (strcmp(m->label(), "Sample channel") == 0)
		return CHANNEL_SAMPLE;
	if (strcmp(m->label(), "MIDI channel") == 0)
		return CHANNEL_MIDI;
	return 0;
}


/* ------------------------------------------------------------------ */


void gColumn::clear(bool full)
{
	if (full)
		Fl_Group::clear();
	else {
		while (children() >= 2) {  // skip "add new channel" btn
			int i = children()-1;
			deleteChannel((gChannel*)child(i));
		}
	}
}
