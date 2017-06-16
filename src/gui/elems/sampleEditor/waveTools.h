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


#ifndef GE_WAVE_TOOLS_H
#define GE_WAVE_TOOLS_H


#include <FL/Fl_Scroll.H>


class SampleChannel;
class geWaveform;


class geWaveTools : public Fl_Scroll
{
private:

	void openMenu();

public:

	SampleChannel *ch;
	geWaveform *waveform;

	geWaveTools(int X,int Y,int W, int H, SampleChannel *ch, const char *L=0);
	void resize(int x, int y, int w, int h);
	int  handle(int e);

	/* updateWaveform
	Updates the waveform by realloc-ing new data (i.e. when the waveform has
	changed). */

	void updateWaveform();

	/* redrawWaveformAsync
	Redraws the waveform, called by the video thread. This is meant to be called
	repeatedly when you need to update the play head inside the waveform. The
	method is smart enough to skip painting if the channel is stopped. */

	void redrawWaveformAsync();
};

#endif
