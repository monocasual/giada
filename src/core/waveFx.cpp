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


#include <cmath>
#include <algorithm>
#include "../utils/log.h"
#include "wave.h"
#include "waveFx.h"


namespace giada {
namespace m {
namespace wfx
{
namespace
{
void fadeFrame(Wave* w, int i, float val)
{
	float* frame = w->getFrame(i);
	for (int j=0; j<w->getChannels(); j++)
		frame[j] *= val;
}


/* -------------------------------------------------------------------------- */


float getPeak(Wave* w, int a, int b)
{
	float peak = 0.0f;
	float abs  = 0.0f;
	for (int i=a; i<b; i++) {
		float* frame = w->getFrame(i);
		for (int j=0; j<w->getChannels(); j++) // Find highest value in any channel
			abs = fabs(frame[j]);
		if (abs > peak)
			peak = abs;
	}
	return peak;
}
}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


float normalizeSoft(Wave* w)
{
	float peak = getPeak(w, 0, w->getSize());

	/* peak == 0.0f: don't normalize the silence
	 * peak > 1.0f: don't reduce the amplitude, just leave it alone */

	if (peak == 0.0f || peak > 1.0f)
		return 1.0f;

	return 1.0f / peak;
}


/* -------------------------------------------------------------------------- */


void normalizeHard(Wave* w, int a, int b)
{
	float peak = getPeak(w, a, b);
	if (peak == 0.0f || peak > 1.0f)  // as in ::normalizeSoft
		return;

	for (int i=a; i<b; i++) {
		float* frame = w->getFrame(i);
		for (int j=0; j<w->getChannels(); j++)
			frame[j] = frame[j] * (1.0f / peak);
	}
	w->setEdited(true);
}


/* -------------------------------------------------------------------------- */


int monoToStereo(Wave* w)
{
	if (w->getChannels() >= G_DEFAULT_AUDIO_CHANS)
		return G_RES_OK;

	unsigned newSize = w->getSize() * G_DEFAULT_AUDIO_CHANS;
	
	float* newData = new (std::nothrow) float[newSize];
	if (newData == nullptr) {
		gu_log("[wfx::monoToStereo] unable to allocate memory!\n");
		return G_RES_ERR_MEMORY;
	}

	for (int i=0, k=0; i<w->getSize(); i++, k+=G_DEFAULT_AUDIO_CHANS) {
		float* frame = w->getFrame(i);
		for (int j=0; j<G_DEFAULT_AUDIO_CHANS; j++)
			newData[k+j] = frame[j];
	}

	w->free();
	w->setData(newData, newSize);
	w->setChannels(G_DEFAULT_AUDIO_CHANS);

	return G_RES_OK;
}


/* -------------------------------------------------------------------------- */


void silence(Wave* w, int a, int b)
{
	gu_log("[wfx::silence] silencing from %d to %d\n", a, b);

	for (int i=a; i<b; i++) {
		float* frame = w->getFrame(i);
		for (int j=0; j<w->getChannels(); j++)	
			frame[j] = 0.0f;
	}

	w->setEdited(true);
}


/* -------------------------------------------------------------------------- */


int cut(Wave* w, int a, int b)
{
	if (a < 0) a = 0;
	if (b > w->getSize()) b = w->getSize();

	/* Create a new temp wave and copy there the original one, skipping the a-b 
	range. */

	unsigned newSize = (w->getSize() - (b - a)) * w->getChannels();
	float* newData = new (std::nothrow) float[newSize];
	if (newData == nullptr) {
		gu_log("[wfx::cut] unable to allocate memory!\n");
		return G_RES_ERR_MEMORY;
	}

	gu_log("[wfx::cut] cutting from %d to %d\n", a, b);

	for (int i=0, k=0; i<w->getSize(); i++) {
		if (i < a || i >= b) {
			float* frame = w->getFrame(i);
			for (int j=0; j<w->getChannels(); j++)	
				newData[k+j] = frame[j];
			k += w->getChannels();
		}
	}

	w->free();
	w->setData(newData, newSize);
	w->setEdited(true);

	return G_RES_OK;
}


/* -------------------------------------------------------------------------- */


int trim(Wave* w, int a, int b)
{
	if (a < 0) a = 0;
	if (b > w->getSize()) b = w->getSize();

	int newSize = (b - a) * w->getChannels();
	float* newData = new (std::nothrow) float[newSize];
	if (newData == nullptr) {
		gu_log("[wfx::trim] unable to allocate memory!\n");
		return G_RES_ERR_MEMORY;
	}

	gu_log("[wfx::trim] trimming from %d to %d (area = %d)\n", a, b, b-a);

	for (int i=a, k=0; i<b; i++, k+=w->getChannels()) {
		float* frame = w->getFrame(i);
		for (int j=0; j<w->getChannels(); j++)
			newData[k+j] = frame[j];
	}

	w->free();
	w->setData(newData, newSize);
 	w->setEdited(true);

	return G_RES_OK;
}


/* -------------------------------------------------------------------------- */


int paste(Wave* src, Wave* des, int aFrame)
{
	int srcNumSamples = src->getSize() * src->getChannels();
	int desNumSamples = des->getSize() * des->getChannels();
	int aSample = aFrame * des->getChannels();

	int newSize = srcNumSamples + desNumSamples;
	float* newData = new (std::nothrow) float[newSize];
	if (newData == nullptr) {
		gu_log("[wfx::paste] unable to allocate memory!\n");
		return G_RES_ERR_MEMORY;
	}

	/* |---original data---|///paste data///|---original data---|
	          chunk 1            chunk 2          chunk 3
	*/

	float* chunk1a = des->getData();
	float* chunk1b = des->getData() + aSample;

	float* chunk2a = src->getData();
	float* chunk2b = src->getData() + srcNumSamples;	

	float* chunk3a = chunk1b;
	float* chunk3b = des->getData() + desNumSamples;

	std::copy(chunk1a, chunk1b, newData);
	std::copy(chunk2a, chunk2b, newData + aSample);	
	std::copy(chunk3a, chunk3b, newData + aSample + srcNumSamples);

	des->free();
	des->setData(newData, newSize);
 	des->setEdited(true);

	return G_RES_OK;
}


/* -------------------------------------------------------------------------- */


void fade(Wave* w, int a, int b, int type)
{
	gu_log("[wfx::fade] fade from %d to %d (range = %d)\n", a, b, b-a);

	float m = 0.0f;
	float d = 1.0f / (float) (b - a);

	if (type == FADE_IN)
		for (int i=a; i<=b; i++, m+=d)
			fadeFrame(w, i, m);
	else
		for (int i=b; i>=a; i--, m+=d)
			fadeFrame(w, i, m);		

  w->setEdited(true);
}


/* -------------------------------------------------------------------------- */


void smooth(Wave* w, int a, int b)
{
	/* Do nothing if fade edges (both of SMOOTH_SIZE samples) are > than selected 
	portion of wave. SMOOTH_SIZE*2 to count both edges. */

	if (SMOOTH_SIZE*2 > (b-a)) {
		gu_log("[wfx::smooth] selection is too small, nothing to do\n");
		return;
	}

	fade(w, a, a+SMOOTH_SIZE, FADE_IN);
	fade(w, b-SMOOTH_SIZE, b, FADE_OUT);

	w->setEdited(true);
}


/* -------------------------------------------------------------------------- */


void reverse(Wave* w, int a, int b)
{
	/* https://stackoverflow.com/questions/33201528/reversing-an-array-of-structures-in-c */

	float* begin = (w->getData() + (a * w->getChannels()));
	float* end   = (w->getData() + (b * w->getChannels()));

	std::reverse(begin, end);

	w->setEdited(true);
}

}}}; // giada::m::wfx::