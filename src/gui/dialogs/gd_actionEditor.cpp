/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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


#include <cmath>
#include "../../utils/gui.h"
#include "../../utils/string.h"
#include "../../core/graphics.h"
#include "../../core/conf.h"
#include "../../core/const.h"
#include "../../core/clock.h"
#include "../../core/sampleChannel.h"
#include "../elems/basics/scroll.h"
#include "../elems/basics/button.h"
#include "../elems/basics/resizerBar.h"
#include "../elems/basics/choice.h"
#include "../elems/basics/box.h"
#include "../elems/actionEditor/actionEditor.h"
#include "../elems/actionEditor/envelopeEditor.h"
#include "../elems/actionEditor/muteEditor.h"
#include "../elems/actionEditor/noteEditor.h"
#include "../elems/actionEditor/gridTool.h"
#include "gd_actionEditor.h"


using std::string;
using namespace giada;
using namespace giada::m;


gdActionEditor::gdActionEditor(Channel* chan)
	:	gdWindow(640, 284),
		chan   (chan),
		zoom   (100),
		coverX (0)
{
	if (conf::actionEditorW) {
		resize(conf::actionEditorX, conf::actionEditorY, conf::actionEditorW, conf::actionEditorH);
		zoom = conf::actionEditorZoom;
	}

	totalWidth = (int) std::ceil(clock::getFramesInSeq() / (float) zoom);

	/* container with zoom buttons and the action type selector. Scheme of
	 * the resizable boxes: |[--b1--][actionType][--b2--][+][-]| */

	Fl_Group *upperArea = new Fl_Group(8, 8, w()-16, 20);

	upperArea->begin();

	if (chan->type == ChannelType::SAMPLE) {
	  actionType = new geChoice(8, 8, 80, 20);
	  gridTool   = new geGridTool(actionType->x()+actionType->w()+4, 8, this);
		actionType->add("key press");
		actionType->add("key release");
		actionType->add("kill chan");
		actionType->value(0);

		SampleChannel *ch = static_cast<SampleChannel*>(chan);
		if (ch->mode == ChannelMode::SINGLE_PRESS || ch->isAnyLoopMode())
		actionType->deactivate();
	}
	else {
		gridTool = new geGridTool(8, 8, this);
	}

		geBox *b1   = new geBox(gridTool->x()+gridTool->w()+4, 8, 300, 20);    // padding actionType - zoomButtons
		zoomIn     = new geButton(w()-8-40-4, 8, 20, 20, "", zoomInOff_xpm, zoomInOn_xpm);
		zoomOut    = new geButton(w()-8-20,   8, 20, 20, "", zoomOutOff_xpm, zoomOutOn_xpm);
	upperArea->end();
	upperArea->resizable(b1);

	zoomIn->callback(cb_zoomIn, (void*)this);
	zoomOut->callback(cb_zoomOut, (void*)this);

	/* main scroller: contains all widgets */

	scroller = new geScroll(8, 36, w()-16, h()-44);

	if (chan->type == ChannelType::SAMPLE) {

		SampleChannel *ch = static_cast<SampleChannel*>(chan);

		ac = new geActionEditor  (scroller->x(), upperArea->y()+upperArea->h()+8, this, ch);
		mc = new geMuteEditor    (scroller->x(), ac->y()+ac->h()+8, this);
		vc = new geEnvelopeEditor(scroller->x(), mc->y()+mc->h()+8, this, G_ACTION_VOLUME, G_RANGE_FLOAT, "volume");
		scroller->add(ac);
		//scroller->add(new geResizerBar(ac->x(), ac->y()+ac->h(), scroller->w(), 8));
		scroller->add(mc);
		//scroller->add(new geResizerBar(mc->x(), mc->y()+mc->h(), scroller->w(), 8));
		scroller->add(vc);
		//scroller->add(new geResizerBar(vc->x(), vc->y()+vc->h(), scroller->w(), 8));

		/* fill volume envelope with actions from recorder */

		vc->fill();

		/* if channel is LOOP_ANY, deactivate it: a loop mode channel cannot
		 * hold keypress/keyrelease actions */

		if (ch->isAnyLoopMode())
			ac->deactivate();
	}
	else {
		pr = new geNoteEditor(scroller->x(), upperArea->y()+upperArea->h()+8, this);
		scroller->add(pr);
		/* TODO - avoid magic number 30 for minimum height */
		scroller->add(new geResizerBar(pr->x(), pr->y()+pr->h(), scroller->w(), 30, 8));
	}

	end();

	/* compute values */

	update();
	gridTool->calc();

	gu_setFavicon(this);

	string buf = "Edit Actions in Channel " + gu_iToString(chan->index+1);
	label(buf.c_str());

	set_non_modal();
	size_range(640, 284);
	resizable(scroller);

	show();
}


/* -------------------------------------------------------------------------- */


gdActionEditor::~gdActionEditor()
{
	conf::actionEditorX = x();
	conf::actionEditorY = y();
	conf::actionEditorW = w();
	conf::actionEditorH = h();
	conf::actionEditorZoom = zoom;

	/** CHECKME - missing clear() ? */

}


/* -------------------------------------------------------------------------- */


void gdActionEditor::cb_zoomIn(Fl_Widget *w, void *p)  { ((gdActionEditor*)p)->__cb_zoomIn(); }
void gdActionEditor::cb_zoomOut(Fl_Widget *w, void *p) { ((gdActionEditor*)p)->__cb_zoomOut(); }


/* -------------------------------------------------------------------------- */


void gdActionEditor::__cb_zoomIn()
{
	/* zoom 50: empirical value, to avoid a totalWidth > 16 bit signed
	 * (32767 max), unsupported by FLTK 1.3.x */

	if (zoom <= 50)
		return;

	zoom /= 2;

	update();

	if (chan->type == ChannelType::SAMPLE) {
		ac->size(totalWidth, ac->h());
		mc->size(totalWidth, mc->h());
		vc->size(totalWidth, vc->h());
		ac->updateActions();
		mc->updateActions();
		vc->updateActions();
	}
	else {
		pr->size(totalWidth, pr->h());
		pr->updateActions();
	}

	/* scroll to pointer */

	int shift = Fl::event_x() + scroller->xposition();
	scroller->scroll_to(scroller->xposition() + shift, scroller->yposition());

	/* update all underlying widgets */

	gridTool->calc();
	scroller->redraw();
}


/* -------------------------------------------------------------------------- */


void gdActionEditor::__cb_zoomOut()
{
	zoom *= 2;

	update();

	if (chan->type == ChannelType::SAMPLE) {
		ac->size(totalWidth, ac->h());
		mc->size(totalWidth, mc->h());
		vc->size(totalWidth, vc->h());
		ac->updateActions();
		mc->updateActions();
		vc->updateActions();
	}
	else {
		pr->size(totalWidth, pr->h());
		pr->updateActions();
	}

	/* scroll to pointer */

	int shift = (Fl::event_x() + scroller->xposition()) / -2;
	if (scroller->xposition() + shift < 0)
			shift = 0;
	scroller->scroll_to(scroller->xposition() + shift, scroller->yposition());

	/* update all underlying widgets */

	gridTool->calc();
	scroller->redraw();
}


/* -------------------------------------------------------------------------- */


void gdActionEditor::update()
{
	totalWidth = (int) ceilf(clock::getFramesInSeq() / (float) zoom);
	if (totalWidth < scroller->w()) {
		totalWidth = scroller->w();
		zoom = (int) ceilf(clock::getFramesInSeq() / (float) totalWidth);
		scroller->scroll_to(0, scroller->yposition());
	}
}


/* -------------------------------------------------------------------------- */


int gdActionEditor::handle(int e)
{
	int ret = Fl_Group::handle(e);
	switch (e) {
		case FL_MOUSEWHEEL: {
			Fl::event_dy() == -1 ? __cb_zoomIn() : __cb_zoomOut();
			ret = 1;
			break;
		}
	}
	return ret;
}


/* -------------------------------------------------------------------------- */


int gdActionEditor::getActionType()
{
	if (actionType->value() == 0)
		return G_ACTION_KEYPRESS;
	else
	if (actionType->value() == 1)
		return G_ACTION_KEYREL;
	else
	if (actionType->value() == 2)
		return G_ACTION_KILL;
	else
		return -1;
}
