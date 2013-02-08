/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_editor
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2013 Giovanni A. Zuliani | Monocasual
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


#include "gd_editor.h"
#include "gd_mainWindow.h"


extern Mixer         G_Mixer;
extern gdMainWindow *mainWin;
extern Conf	         G_Conf;

gdEditor::gdEditor(const char *title, int chan)
: gWindow(500, 292, title), chan(chan)
{
	set_non_modal();

	if (G_Conf.sampleEditorX)
		resize(G_Conf.sampleEditorX, G_Conf.sampleEditorY, w(), h());

	wt            = new gWaveTools(8, 8, 484, 179, chan);

	Fl_Group *tools = new Fl_Group(8, wt->y()+wt->h()+5, w()-16, 92);
	tools->begin();
		chanStart     = new gInput(50,  192, 70, 20, "Range");
		chanEnd       = new gInput(124, 192, 70, 20);
		resetStartEnd = new gClick(198, 192, 50, 20, "Reset");
		volume        = new gDial (100,	216, 20, 20, "Volume");
		volumeNum     = new gInput(124,	216, 45, 20, "dB");
		boost         = new gDial (100,	240, 20, 20, "Boost");
		boostNum      = new gInput(124,	240, 45, 20, "dB");
		normalize     = new gClick(198, 240, 50, 20, "Norm.");
		pan 					= new gDial (100, 264, 20, 20, "Pan");
		panNum    		= new gInput(124,	264, 45, 20, "%");
		reload    		= new gClick(252, 192, 50, 20, "Reload");
		pitch					= new gDial (300,	216, 20, 20, "Pitch");
		pitchNum		  = new gInput(324,	216, 45, 20);
		gBox *spacer  = new gBox(pitchNum->x()+pitchNum->w()+4, tools->y(), 80, tools->h());    // padding actionType - zoomButtons
	tools->end();
	tools->resizable(spacer);

	char buf[16];
	sprintf(buf, "%d", G_Mixer.chanStartTrue[chan]/2); // divided by 2 because stereo
	chanStart->value(buf);
	chanStart->type(FL_INT_INPUT);
	chanStart->callback(cb_setChanPos, this);

	/* inputs callback: fire when they lose focus or Enter is pressed. */

	chanStart->when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY);
	chanEnd  ->when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY);

	sprintf(buf, "%d", G_Mixer.chanEndTrue[chan]/2);	// divided by 2 because stereo
	chanEnd->value(buf);
	chanEnd->type(FL_INT_INPUT);
	chanEnd->callback(cb_setChanPos, this);

	resetStartEnd->callback(cb_resetStartEnd, this);

	volume->callback(cb_setVolume, (void*)this);
	volume->value(mainWin->keyboard->vol[chan]->value());

	float dB = 20*log10(G_Mixer.chanVolume[chan]);   // dB = 20*log_10(linear value)
	if (dB > -INFINITY)	sprintf(buf, "%.2f", dB);
	else            		sprintf(buf, "-inf");
	volumeNum->value(buf);
	volumeNum->align(FL_ALIGN_RIGHT);
	volumeNum->callback(cb_setVolumeNum, (void*)this);

	boost->range(1.0f, 10.0f);
	boost->callback(cb_setBoost, (void*)this);
	if (G_Mixer.chanBoost[chan] > 10.f)
		boost->value(10.0f);
	else
		boost->value(G_Mixer.chanBoost[chan]);
	boost->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE);

	float boost = 20*log10(G_Mixer.chanBoost[chan]); // dB = 20*log_10(linear value)
	sprintf(buf, "%.2f", boost);
	boostNum->value(buf);
	boostNum->align(FL_ALIGN_RIGHT);
	boostNum->callback(cb_setBoostNum, (void*)this);

	normalize->callback(cb_normalize, (void*)this);

	pan->range(0.0f, 2.0f);
	pan->callback(cb_panning, (void*)this);

	pitch->range(0.1f, 2.0f);
	pitch->value(G_Mixer.chanPitch[chan]);
	pitch->callback(cb_setPitch, (void*)this);

	sprintf(buf, "%.4f", G_Mixer.chanPitch[chan]); // 4 digits
	pitchNum->value(buf);
	pitchNum->align(FL_ALIGN_RIGHT);
	pitchNum->callback(cb_setPitchNum, (void*)this);
	pitchNum->when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY);

	reload->callback(cb_reload, (void*)this);

	/* logical samples (aka takes) cannot be reloaded. So far. */

	if (G_Mixer.chan[chan]->isLogical)
		reload->deactivate();

	if (G_Mixer.chanPanRight[chan] < 1.0f) {
		char buf[8];
		sprintf(buf, "%d L", abs((G_Mixer.chanPanRight[chan] * 100.0f) - 100));
		pan->value(G_Mixer.chanPanRight[chan]);
		panNum->value(buf);
	}
	else if (G_Mixer.chanPanRight[chan] == 1.0f && G_Mixer.chanPanLeft[chan] == 1.0f) {
	  pan->value(1.0f);
	  panNum->value("C");
	}
	else {
		char buf[8];
		sprintf(buf, "%d R", abs((G_Mixer.chanPanLeft[chan] * 100.0f) - 100));
		pan->value(2.0f-G_Mixer.chanPanLeft[chan]);
		panNum->value(buf);
	}

	panNum->align(FL_ALIGN_RIGHT);
	panNum->readonly(1);
	panNum->cursor_color(FL_WHITE);

	gu_setFavicon(this);
	size_range(500, 292);
	resizable(wt);

	show();
}


/* ------------------------------------------------------------------ */


gdEditor::~gdEditor() {
	G_Conf.sampleEditorX = x();
	G_Conf.sampleEditorY = y();
	G_Conf.sampleEditorW = w();
	G_Conf.sampleEditorH = h();
}


/* ------------------------------------------------------------------ */


void gdEditor::cb_setChanPos   (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_setChanPos(); }
void gdEditor::cb_resetStartEnd(Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_resetStartEnd(); }
void gdEditor::cb_setVolume    (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_setVolume(); }
void gdEditor::cb_setVolumeNum (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_setVolumeNum(); }
void gdEditor::cb_setBoost     (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_setBoost(); }
void gdEditor::cb_setBoostNum  (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_setBoostNum(); }
void gdEditor::cb_normalize    (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_normalize(); }
void gdEditor::cb_panning      (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_panning(); }
void gdEditor::cb_reload       (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_reload(); }
void gdEditor::cb_setPitch     (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_setPitch(); }
void gdEditor::cb_setPitchNum  (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_setPitchNum(); }


/* ------------------------------------------------------------------ */


void gdEditor::__cb_setChanPos() {
	glue_setBeginEndChannel(
		this,
		chan,
		atoi(chanStart->value())*2,  // glue halves printed values
		atoi(chanEnd->value())*2,
		true
	);
}


/* ------------------------------------------------------------------ */


void gdEditor::__cb_resetStartEnd() {
	glue_setBeginEndChannel(this, chan, 0, G_Mixer.chan[chan]->size, true);
}


/* ------------------------------------------------------------------ */


void gdEditor::__cb_setVolume() {
	glue_setVolEditor(this, chan, volume->value(), false);
}


/* ------------------------------------------------------------------ */


void gdEditor::__cb_setVolumeNum() {
	glue_setVolEditor(this, chan, atof(volumeNum->value()), true);
}


/* ------------------------------------------------------------------ */


void gdEditor::__cb_setBoost() {
	if (Fl::event() == FL_DRAG)
		glue_setBoost(this, chan, boost->value(), false);
	else if (Fl::event() == FL_RELEASE) {
		glue_setBoost(this, chan, boost->value(), false);
		wt->wave->alloc();
		wt->wave->redraw();
	}
}


/* ------------------------------------------------------------------ */


void gdEditor::__cb_setBoostNum() {
	glue_setBoost(this, chan, atof(boostNum->value()), true);
	wt->wave->alloc();
	wt->wave->redraw();
}


/* ------------------------------------------------------------------ */


void gdEditor::__cb_normalize() {
	float val = wfx_normalizeSoft(G_Mixer.chan[chan]);
	glue_setBoost(this, chan, val, false); // we pretend that a fake user turns the dial (numeric=false)
	if (val < 0.0f)
		boost->value(0.0f);
	else
	if (val > 20.0f) // a dial > than it's max value goes crazy
		boost->value(10.0f);
	else
		boost->value(val);
	wt->wave->alloc();
	wt->wave->redraw();
}


/* ------------------------------------------------------------------ */


void gdEditor::__cb_panning() {
	glue_setPanning(this, chan, pan->value());
}


/* ------------------------------------------------------------------ */


void gdEditor::__cb_reload() {

	if (!gdConfirmWin("Warning", "Reload sample: are you sure?"))
		return;

	/* no need for glue_loadChan, there's no gui to refresh */

	mh_loadChan(G_Mixer.chan[chan]->pathfile.c_str(), chan);

	glue_setBoost(this, chan, DEFAULT_BOOST, true);
	glue_setPitch(this, chan, gDEFAULT_PITCH, true);
	glue_setPanning(this, chan, 1.0f);
	pan->value(1.0f);  // glue_setPanning doesn't do it
	pan->redraw();     // glue_setPanning doesn't do it

	wt->wave->calcZoom();
	wt->wave->alloc();
	wt->wave->redraw();

	glue_setBeginEndChannel(this, chan, 0, G_Mixer.chan[chan]->size, true);

	redraw();
}


/* ------------------------------------------------------------------ */


void gdEditor::__cb_setPitch() {
	glue_setPitch(this, chan, pitch->value(), false);
}


/* ------------------------------------------------------------------ */


void gdEditor::__cb_setPitchNum() {
	glue_setPitch(this, chan, atof(pitchNum->value()), true);
}
