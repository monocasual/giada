/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * waveFx
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


#include <math.h>
#include "waveFx.h"
#include "channel.h"
#include "mixer.h"
#include "wave.h"
#include "log.h"


extern Mixer G_Mixer;


float wfx_normalizeSoft(Wave *w) {
	float peak = 0.0f;
	float abs  = 0.0f;
	for (int i=0; i<w->size; i++) { // i++: both L and R samples
		abs = fabs(w->data[i]);
		if (abs > peak)
			peak = abs;
	}

	/* peak == 0.0f: don't normalize the silence
	 * peak > 1.0f: don't reduce the amplitude, just leave it alone */

	if (peak == 0.0f || peak > 1.0f)
		return 1.0f;

	return 1.0f / peak;
}


/* ------------------------------------------------------------------ */


bool wfx_monoToStereo(Wave *w) {

	unsigned newSize = w->size * 2;
	float *dataNew = (float *) malloc(newSize * sizeof(float));
	if (dataNew == NULL) {
		gLog("[wfx] unable to allocate memory for mono>stereo conversion\n");
		return 0;
	}

	for (int i=0, j=0; i<w->size; i++) {
		dataNew[j]   = w->data[i];
		dataNew[j+1] = w->data[i];
		j+=2;
	}

	free(w->data);
	w->data = dataNew;
	w->size = newSize;
	w->frames(w->frames()*2);
	w->channels(2);

	return 1;
}


/* ------------------------------------------------------------------ */


void wfx_silence(Wave *w, int a, int b) {

	/* stereo values */
	a = a * 2;
	b = b * 2;

	gLog("[wfx] silencing from %d to %d\n", a, b);

	for (int i=a; i<b; i+=2) {
		w->data[i]   = 0.0f;
		w->data[i+1] = 0.0f;
	}

	w->isEdited = true;

	return;
}


/* ------------------------------------------------------------------ */


int wfx_cut(Wave *w, int a, int b) {
	a = a * 2;
	b = b * 2;

	if (a < 0) a = 0;
	if (b > w->size) b = w->size;

	/* create a new temp wave and copy there the original one, skipping
	 * the a-b range */

	unsigned newSize = w->size-(b-a);
	float *temp = (float *) malloc(newSize * sizeof(float));
	if (temp == NULL) {
		gLog("[wfx] unable to allocate memory for cutting\n");
		return 0;
	}

	gLog("[wfx] cutting from %d to %d, new size=%d (video=%d)\n", a, b, newSize, newSize/2);

	for (int i=0, k=0; i<w->size; i++) {
		if (i < a || i >= b) {		               // left margin always included, in order to keep
			temp[k] = w->data[i];   // the stereo pair
			k++;
		}
	}

	free(w->data);
	w->data = temp;
	w->size = newSize;
	//w->inHeader.frames -= b-a;
	w->frames(w->frames() - b - a);
	w->isEdited = true;

	gLog("[wfx] cutting done\n");

	return 1;
}


/* ------------------------------------------------------------------ */


int wfx_trim(Wave *w, int a, int b) {
	a = a * 2;
	b = b * 2;

	if (a < 0) a = 0;
	if (b > w->size) b = w->size;

	int newSize = b - a;
	float *temp = (float *) malloc(newSize * sizeof(float));
	if (temp == NULL) {
		gLog("[wfx] unable to allocate memory for trimming\n");
		return 0;
	}

	gLog("[wfx] trimming from %d to %d (area = %d)\n", a, b, b-a);

	for (int i=a, k=0; i<b; i++, k++)
		temp[k] = w->data[i];

	free(w->data);
	w->data = temp;
	w->size = newSize;
	//w->inHeader.frames = b-a;
	w->frames(b - a);
 	w->isEdited = true;

	return 1;
}


/* ------------------------------------------------------------------ */


void wfx_fade(Wave *w, int a, int b, int type) {

	float m = type == 0 ? 0.0f : 1.0f;
	float d = 1.0f/(float)(b-a);
	if (type == 1)
		d = -d;

	a *= 2;
	b *= 2;

	for (int i=a; i<b; i+=2) {
		w->data[i]   *= m;
		w->data[i+1] *= m;
		m += d;
	}
}


/* ------------------------------------------------------------------ */


void wfx_smooth(Wave *w, int a, int b) {

	int d = 32;  // 64 if stereo data

	/* do nothing if fade edges (both of 32 samples) are > than selected
	 * portion of wave. d*2 => count both edges, (b-a)*2 => stereo
	 * values. */

	if (d*2 > (b-a)*2) {
		gLog("[WFX] selection is too small, nothing to do\n");
		return;
	}

	wfx_fade(w, a, a+d, 0);
	wfx_fade(w, b-d, b, 1);
}
