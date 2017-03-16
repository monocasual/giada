/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_actionEditor
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


#include <math.h>
#include "../../utils/gui.h"
#include "../../core/graphics.h"
#include "../../core/mixer.h"
#include "../../core/recorder.h"
#include "../../core/conf.h"
#include "../../core/clock.h"
#include "../../core/channel.h"
#include "../../core/sampleChannel.h"
#include "../elems/ge_mixed.h"
#include "../elems/basics/scroll.h"
#include "../elems/actionEditor/actionEditor.h"
#include "../elems/actionEditor/envelopeEditor.h"
#include "../elems/actionEditor/muteEditor.h"
#include "../elems/actionEditor/noteEditor.h"
#include "gd_actionEditor.h"


extern Mixer G_Mixer;
extern Clock G_Clock;
extern Conf	 G_Conf;


gdActionEditor::gdActionEditor(Channel *chan)
	:	gWindow(640, 284),
		chan   (chan),
		zoom   (100),
		coverX (0)
{
	if (G_Conf.actionEditorW) {
		resize(G_Conf.actionEditorX, G_Conf.actionEditorY, G_Conf.actionEditorW, G_Conf.actionEditorH);
		zoom = G_Conf.actionEditorZoom;
	}

	totalWidth = (int) ceilf(G_Clock.getFramesInSequencer() / (float) zoom);

	/* container with zoom buttons and the action type selector. Scheme of
	 * the resizable boxes: |[--b1--][actionType][--b2--][+][-]| */

	Fl_Group *upperArea = new Fl_Group(8, 8, w()-16, 20);

	upperArea->begin();

	if (chan->type == CHANNEL_SAMPLE) {
	  actionType = new gChoice(8, 8, 80, 20);
	  gridTool   = new gGridTool(actionType->x()+actionType->w()+4, 8, this);
		actionType->add("key press");
		actionType->add("key release");
		actionType->add("kill chan");
		actionType->value(0);

		SampleChannel *ch = (SampleChannel*) chan;
		if (ch->mode == SINGLE_PRESS || ch->mode & LOOP_ANY)
		actionType->deactivate();
	}
	else {
		gridTool = new gGridTool(8, 8, this);
	}

		gBox *b1   = new gBox(gridTool->x()+gridTool->w()+4, 8, 300, 20);    // padding actionType - zoomButtons
		zoomIn     = new gClick(w()-8-40-4, 8, 20, 20, "", zoomInOff_xpm, zoomInOn_xpm);
		zoomOut    = new gClick(w()-8-20,   8, 20, 20, "", zoomOutOff_xpm, zoomOutOn_xpm);
	upperArea->end();
	upperArea->resizable(b1);

	zoomIn->callback(cb_zoomIn, (void*)this);
	zoomOut->callback(cb_zoomOut, (void*)this);

	/* main scroller: contains all widgets */

	scroller = new geScroll(8, 36, w()-16, h()-44);

	if (chan->type == CHANNEL_SAMPLE) {

		SampleChannel *ch = (SampleChannel*) chan;

		ac = new geActionEditor  (scroller->x(), upperArea->y()+upperArea->h()+8, this, ch);
		mc = new geMuteEditor    (scroller->x(), ac->y()+ac->h()+8, this);
		vc = new geEnvelopeEditor(scroller->x(), mc->y()+mc->h()+8, this, ACTION_VOLUME, RANGE_FLOAT, "volume");
		scroller->add(ac);
		//scroller->add(new gResizerBar(ac->x(), ac->y()+ac->h(), scroller->w(), 8));
		scroller->add(mc);
		//scroller->add(new gResizerBar(mc->x(), mc->y()+mc->h(), scroller->w(), 8));
		scroller->add(vc);
		//scroller->add(new gResizerBar(vc->x(), vc->y()+vc->h(), scroller->w(), 8));

		/* fill volume envelope with actions from recorder */

		vc->fill();

		/* if channel is LOOP_ANY, deactivate it: a loop mode channel cannot
		 * hold keypress/keyrelease actions */

		if (ch->mode & LOOP_ANY)
			ac->deactivate();
	}
	else {
		pr = new geNoteEditor(scroller->x(), upperArea->y()+upperArea->h()+8, this);
		scroller->add(pr);
		scroller->add(new gResizerBar(pr->x(), pr->y()+pr->h(), scroller->w(), 8));
	}

	end();

	/* compute values */

	update();
	gridTool->calc();

	gu_setFavicon(this);

	char buf[256];
	sprintf(buf, "Edit Actions in Channel %d", chan->index+1);
	label(buf);

	set_non_modal();
	size_range(640, 284);
	resizable(scroller);

	show();
}


/* -------------------------------------------------------------------------- */


gdActionEditor::~gdActionEditor()
{
	G_Conf.actionEditorX = x();
	G_Conf.actionEditorY = y();
	G_Conf.actionEditorW = w();
	G_Conf.actionEditorH = h();
	G_Conf.actionEditorZoom = zoom;

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

	if (chan->type == CHANNEL_SAMPLE) {
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

	if (chan->type == CHANNEL_SAMPLE) {
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
	totalWidth = (int) ceilf(G_Clock.getFramesInSequencer() / (float) zoom);
	if (totalWidth < scroller->w()) {
		totalWidth = scroller->w();
		zoom = (int) ceilf(G_Clock.getFramesInSequencer() / (float) totalWidth);
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
		return ACTION_KEYPRESS;
	else
	if (actionType->value() == 1)
		return ACTION_KEYREL;
	else
	if (actionType->value() == 2)
		return ACTION_KILLCHAN;
	else
		return -1;
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


gGridTool::gGridTool(int x, int y, gdActionEditor *parent)
	:	Fl_Group(x, y, 80, 20), parent(parent)
{
	gridType = new gChoice(x, y, 40, 20);
	gridType->add("1");
	gridType->add("2");
	gridType->add("3");
	gridType->add("4");
	gridType->add("6");
	gridType->add("8");
	gridType->add("16");
	gridType->add("32");
	gridType->value(0);
	gridType->callback(cb_changeType, (void*)this);

	active = new gCheck (x+44, y+4, 12, 12);

	gridType->value(G_Conf.actionEditorGridVal);
	active->value(G_Conf.actionEditorGridOn);

	end();
}


/* -------------------------------------------------------------------------- */


gGridTool::~gGridTool()
{
	G_Conf.actionEditorGridVal = gridType->value();
	G_Conf.actionEditorGridOn  = active->value();
}


/* -------------------------------------------------------------------------- */


void gGridTool::cb_changeType(Fl_Widget *w, void *p)  { ((gGridTool*)p)->__cb_changeType(); }


/* -------------------------------------------------------------------------- */


void gGridTool::__cb_changeType()
{
	calc();
	parent->redraw();
}


/* -------------------------------------------------------------------------- */


bool gGridTool::isOn()
{
	return active->value();
}


/* -------------------------------------------------------------------------- */


int gGridTool::getValue()
{
	switch (gridType->value()) {
		case 0:	return 1;
		case 1: return 2;
		case 2: return 3;
		case 3: return 4;
		case 4: return 6;
		case 5: return 8;
		case 6: return 16;
		case 7: return 32;
	}
	return 0;
}


/* -------------------------------------------------------------------------- */


void gGridTool::calc()
{
	points.clear();
	frames.clear();
	bars.clear();
	beats.clear();

	/* find beats, bars and grid. The method is the same of the waveform in sample
	 * editor. Take totalwidth (the width in pixel of the area to draw), knowing
	 * that totalWidth = totalFrames / zoom. Then, for each pixel of totalwidth,
	 * put a concentrate of each block (which is totalFrames / zoom) */

	int  j   = 0;
	int fpgc = floor(G_Clock.getFramesPerBeat() / getValue());  // frames per grid cell

	for (int i=1; i<parent->totalWidth; i++) {   // if i=0, step=0 -> useless cycle
		int step = parent->zoom*i;
		while (j < step && j < G_Clock.getTotalFrames()) {
			if (j % fpgc == 0) {
				points.push_back(i);
				frames.push_back(j);
			}
			if (j % G_Clock.getFramesPerBeat() == 0)
				beats.push_back(i);
			if (j % G_Clock.getFramesPerBar() == 0 && i != 1)
				bars.push_back(i);
			if (j == G_Clock.getTotalFrames() - 1)
				parent->coverX = i;
			j++;
		}
		j = step;
	}

	/* fix coverX if == 0, which means G_Mixer.beats == G_MAX_BEATS */

	if (G_Clock.getBeats() == G_MAX_BEATS)
		parent->coverX = parent->totalWidth;
}


/* -------------------------------------------------------------------------- */


int gGridTool::getSnapPoint(int v)
{
	if (v == 0) return 0;

	for (int i=0; i<(int)points.size(); i++) {

		if (i == (int) points.size()-1)
			return points.at(i);

		int gp  = points.at(i);
		int gpn = points.at(i+1);

		if (v >= gp && v < gpn)
			return gp;
	}
	return v;  // default value
}


/* -------------------------------------------------------------------------- */


int gGridTool::getSnapFrame(int v)
{
	v *= parent->zoom;  // transformation pixel -> frame

	for (int i=0; i<(int)frames.size(); i++) {

		if (i == (int) frames.size()-1)
			return frames.at(i);

		int gf  = frames.at(i);     // grid frame
		int gfn = frames.at(i+1);   // grid frame next

		if (v >= gf && v < gfn) {

			/* which one is the closest? gf < v < gfn */

			if ((gfn - v) < (v - gf))
				return gfn;
			else
				return gf;
		}
	}
	return v;  // default value
}


/* -------------------------------------------------------------------------- */


int gGridTool::getCellSize()
{
	return (parent->coverX - parent->ac->x()) / G_Clock.getBeats() / getValue();
}
