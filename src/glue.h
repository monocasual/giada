/* ---------------------------------------------------------------------
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
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2011 Giovanni A. Zuliani | Monocasual
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


#ifndef GLUE_H
#define GLUE_H

/* addChannel
 * add an empty new channel to the stack. Returns the new channel. */

struct channel *glue_addChannel(int side);

/* loadChannel
 * fill an existing channel with a wave. */

int glue_loadChannel(struct channel *ch, const char *fname, const char *fpath);

void glue_deleteChannel(struct channel *ch);

void glue_freeChannel(struct channel *ch);

/** FIXME - noboy will call these via MIDI/keyb/mouse! */
int glue_loadPatch(const char *fname, const char *fpath, class gProgress *status, bool isProject);
int glue_savePatch(const char *fullpath, const char *name, bool isProject);

/** FIXME - noboy will call this via MIDI/keyb/mouse! */
int glue_saveSample(int ch, const char *fullpath);

/* keyPress / keyRelease
 * handle the key pressure, either via mouse/keyboard or MIDI. If gui
 * is true it means that the event comes from the main window (mouse,
 * keyb or MIDI), otherwise the event comes from the action recorder. */

void glue_keyPress  (struct channel *ch, bool ctrl=0, bool shift=0);
void glue_keyRelease(struct channel *ch, bool ctrl=0, bool shift=0);

void glue_setBpm(const char *v1, const char *v2);
void glue_setBeats(int beats, int bars, bool expand);

void glue_startSeq();
void glue_stopSeq();
void glue_rewindSeq();

/* start/stopRec
 * handle the action recording. */

void glue_startRec();
void glue_stopRec();

/* start/stopInputRec
 * handle the input recording (take). */

int glue_startInputRec();
int glue_stopInputRec();

/* start/stopReadingRecs
 * handle the 'R' button. */

void glue_startReadingRecs(struct channel *ch);
void glue_stopReadingRecs(struct channel *ch);

void glue_quantize(int val);

void glue_setVol(struct channel *ch, float v);
void glue_setVolMainWin(struct channel *ch, float v);
void glue_setOutVol(float v);
void glue_setInVol(float v);

void glue_setPanning(class gdEditor *win, struct channel *ch, float val);

void glue_clearAllSamples();
void glue_clearAllRecs();

/* resetToInitState
 * reset Giada to init state. */

void glue_resetToInitState(bool resetGui=true);

void glue_startStopMetronome();

/* setBeginEndChannel
 * sets start/end points in the sample editor.
 * Recalc=false: don't recalc internal position
 * check=true: check the points' consistency */

/** FIXME - noboy will call this via MIDI/keyb/mouse! */
void glue_setBeginEndChannel(class gdEditor *win, struct channel *ch, int b, int e,
														 bool recalc=false, bool check=true);

/** FIXME - noboy will call this via MIDI/keyb/mouse! */
void glue_setBoost(class gdEditor *win, struct channel *ch, float val, bool numeric);

void glue_setPitch(class gdEditor *win, struct channel *ch, float val, bool numeric);

/* setVolEditor
 * handles the volume inside the SAMPLE EDITOR (not the main gui). The
 * numeric flag tells if we want to handle the dial or the numeric input
 * field. */

 /** FIXME - noboy will call this via MIDI/keyb/mouse! */
void glue_setVolEditor(class gdEditor *win, struct channel *ch, float val, bool numeric);

void glue_setMute(struct channel *ch);

void glue_setSoloOn (struct channel *ch);
void glue_setSoloOff(struct channel *ch);

/** FIXME - noboy will call this via MIDI/keyb/mouse! */
int glue_saveProject(const char *folderPath, const char *projName);


/* beatsDivide/Multiply
 * shrinks or enlarges the number of beats by 2. */

void glue_beatsMultiply();
void glue_beatsDivide();

#endif
