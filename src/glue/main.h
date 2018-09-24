/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * glue
 * Intermediate layer GUI <-> CORE.
 *
 * How to know if you need another glue_ function? Ask yourself if the
 * new action will ever be called via MIDI or keyboard/mouse. If yes,
 * put it here.
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


#ifndef G_GLUE_MAIN_H
#define G_GLUE_MAIN_H


/* glue_setBpm (1)
Sets bpm value from string to float. */

void glue_setBpm(const char* v1, const char* v2);

/* glue_setBpm (2)
Sets bpm value. Usually called from the Jack callback or non-UI components. */

void glue_setBpm(float v);

void glue_setBeats(int beats, int bars);
void glue_quantize(int val);
void glue_setOutVol(float v, bool gui=true);
void glue_setInVol(float v, bool gui=true);
void glue_clearAllSamples();
void glue_clearAllActions();

/* resetToInitState
Resets Giada to init state. If resetGui also refresh all widgets. If 
createColumns also build initial empty columns. */

void glue_resetToInitState(bool resetGui=true, bool createColumns=true);

/* beatsDivide/Multiply
Shrinks or enlarges the number of beats by 2. */

void glue_beatsMultiply();
void glue_beatsDivide();

#endif
