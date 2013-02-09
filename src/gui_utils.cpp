/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gui_utils
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


#include "mixer.h"
#include "patch.h"
#include "gui_utils.h"
#include "graphics.h"
#include "gd_warnings.h"
#include "ge_window.h"
#include "gd_mainWindow.h"
#include "recorder.h"
#include "wave.h"


extern Mixer 	       G_Mixer;
extern unsigned      G_beats;
extern bool 		     G_audio_status;
extern Patch         G_patch;
extern Conf          G_conf;
extern gdMainWindow *mainWin;


void gu_refresh() {
	Fl::lock();

	/* update dynamic elements */

	mainWin->outMeter->mixerPeak = G_Mixer.peakOut;
	mainWin->inMeter->mixerPeak  = G_Mixer.peakIn;
	mainWin->outMeter->redraw();
	mainWin->inMeter->redraw();
	mainWin->beatMeter->redraw();

	/* update channels */

	for (unsigned i=0; i<MAX_NUM_CHAN; i++) {

		if (G_Mixer.chan[i] == NULL)
			continue;

		if (G_Mixer.chanStatus[i] == STATUS_OFF) {
			mainWin->keyboard->sampleButton[i]->bgColor0 = COLOR_BG_0;
			mainWin->keyboard->sampleButton[i]->bdColor  = COLOR_BD_0;
			mainWin->keyboard->sampleButton[i]->txtColor = COLOR_TEXT_0;
		}

		if (G_Mixer.chanStatus[i] & (STATUS_PLAY | STATUS_WAIT | STATUS_ENDING)) {
			mainWin->keyboard->sampleButton[i]->bgColor0 = COLOR_BG_2;
			mainWin->keyboard->sampleButton[i]->bdColor  = COLOR_BD_1;
			mainWin->keyboard->sampleButton[i]->txtColor = COLOR_TEXT_1;
		}

		if (G_Mixer.chanInput == (int) i)
			mainWin->keyboard->sampleButton[i]->bgColor0 = COLOR_BG_3;

		if (recorder::active)
			if (recorder::canRec(i)) {
				mainWin->keyboard->sampleButton[i]->bgColor0 = COLOR_BG_4;
				mainWin->keyboard->sampleButton[i]->txtColor = COLOR_TEXT_0;
			}

		mainWin->keyboard->sampleButton[i]->redraw();
		mainWin->keyboard->status[i]->redraw();
	}

	/* redraw GUI */

	Fl::unlock();
	Fl::awake();
}


/* ------------------------------------------------------------------ */


void gu_trim_label(const char *str, unsigned n, Fl_Widget *w) {

	/*** we should compute the length of the string in pixels, not in
	 * chars */

	if (strlen(str) < n)
		w->copy_label(str);
	else {
		char out[FILENAME_MAX];
		strncpy(out, str, n);
		out[n] = '\0';
		strcat(out, "...");
		w->copy_label(out);
	}
}


/* ------------------------------------------------------------------ */


void gu_update_controls() {
	for (unsigned i=0; i<MAX_NUM_CHAN; i++) {

		/* update status box and sampleButton */

		gu_resetChannel(i);

		switch (G_Mixer.chanStatus[i]) {
			case STATUS_EMPTY:
				mainWin->keyboard->sampleButton[i]->label("-- no sample --");
				break;
			case STATUS_MISSING:
			case STATUS_WRONG:
				mainWin->keyboard->sampleButton[i]->label("* file not found! *");
				break;
			default:
				gu_trim_label(G_Mixer.chan[i]->name.c_str(), 28, mainWin->keyboard->sampleButton[i]);
				break;
		}

		mainWin->keyboard->sampleButton[i]->redraw();

		/* update volumes+mute */

		mainWin->keyboard->vol[i]->value(G_Mixer.chanVolume[i]);
		mainWin->keyboard->mute[i]->value(G_Mixer.chanMute[i]);

		/* updates modebox */

		mainWin->keyboard->modeBoxes[i]->value(G_Mixer.chanMode[i]);
		mainWin->keyboard->modeBoxes[i]->redraw();

		/* upate channels. If you load a patch with recorded actions, the 'R'
		 * button must be shown. Moreover if the actions are active, the 'R'
		 * button must be activated accordingly. */

		if (recorder::chanEvents[i])
			mainWin->keyboard->addActionButton(i, recorder::chanActive[i]);
		else
			mainWin->keyboard->remActionButton(i);
	}

	mainWin->outVol->value(G_Mixer.outVol);
	mainWin->inVol->value(G_Mixer.inVol);

	/* if you reset to init state while the seq is in play: it's better to
	 * update the button status */

	mainWin->beat_stop->value(G_Mixer.running);

	/* update bpm and beats. If bpm < 99.9 show just 4 digits (+ escape),
	 * otherwise a bad looking 0 remains */

	int size = G_Mixer.bpm < 100.0f ? 5 : 6;
	char buf_bpm[6];
	snprintf(buf_bpm, size, "%f", G_Mixer.bpm);
	mainWin->bpm->copy_label(buf_bpm);

	char buf_batt[8];
	sprintf(buf_batt, "%d/%d", G_Mixer.beats, G_Mixer.bars);
	mainWin->beats->copy_label(buf_batt);

	if 			(G_Mixer.quantize == 6)		mainWin->quantize->value(5);
	else if (G_Mixer.quantize == 8)		mainWin->quantize->value(6);
	else		mainWin->quantize->value(G_Mixer.quantize);

	mainWin->metronome->value(0);
	mainWin->metronome->redraw();
}


/* ------------------------------------------------------------------ */


void gu_update_win_label(const char *c) {
	std::string out = VERSIONE_STR;
	out += " - ";
	out += c;
	mainWin->copy_label(out.c_str());
}


/* ------------------------------------------------------------------ */


void gu_resetChannel(int c) {
	mainWin->keyboard->sampleButton[c]->bgColor0 = COLOR_BG_0;
	mainWin->keyboard->sampleButton[c]->bdColor  = COLOR_BD_0;
	mainWin->keyboard->sampleButton[c]->txtColor = COLOR_TEXT_0;
	mainWin->keyboard->sampleButton[c]->label("-- no sample --");
	mainWin->keyboard->remActionButton(c);
	mainWin->keyboard->sampleButton[c]->redraw();
	mainWin->keyboard->status[c]->redraw();
}


/* ------------------------------------------------------------------ */


void gu_setFavicon(Fl_Window *w) {
#if defined(__linux__)
	fl_open_display();
	Pixmap p, mask;
	XpmCreatePixmapFromData(
		fl_display,
		DefaultRootWindow(fl_display),
		(char **)giada_icon,
		&p,
		&mask,
		NULL);
	w->icon((char *)p);
#elif defined(_WIN32)
	w->icon((char *)LoadIcon(fl_display, MAKEINTRESOURCE(IDI_ICON1)));
#endif
}


/* ------------------------------------------------------------------ */


void gu_openSubWindow(gWindow *parent, gWindow *child, int id) {
	if (parent->hasWindow(id)) {
		printf("[GU] parent has subwindow with id=%d, deleting\n", id);
		parent->delSubWindow(id);
	}
	child->setId(id);
	parent->addSubWindow(child);
}


/* ------------------------------------------------------------------ */


void gu_refreshActionEditor() {

	/** FIXME - why don't we simply call WID_ACTION_EDITOR->redraw()? */

	gdActionEditor *aeditor = (gdActionEditor*) mainWin->getChild(WID_ACTION_EDITOR);
	if (aeditor) {
		int chan = aeditor->chan;
		mainWin->delSubWindow(WID_ACTION_EDITOR);
		gu_openSubWindow(mainWin, new gdActionEditor(chan), WID_ACTION_EDITOR);
	}
}


/* ------------------------------------------------------------------ */


gWindow *gu_getSubwindow(gWindow *parent, int id) {
	if (parent->hasWindow(id))
		return parent->getChild(id);
	else
		return NULL;
}


/* ------------------------------------------------------------------ */


void gu_closeAllSubwindows() {

	/* don't close WID_FILE_BROWSER, because it's the caller of this
	 * function */

	mainWin->delSubWindow(WID_ACTION_EDITOR);
	mainWin->delSubWindow(WID_SAMPLE_EDITOR);
	mainWin->delSubWindow(WID_FX_LIST);
	mainWin->delSubWindow(WID_FX);
}
