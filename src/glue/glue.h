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
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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


/* keyPress / keyRelease
 * handle the key pressure, either via mouse/keyboard or MIDI. If gui
 * is true it means that the event comes from the main window (mouse,
 * keyb or MIDI), otherwise the event comes from the action recorder. */

void glue_keyPress  (class Channel       *ch, bool ctrl=0, bool shift=0);
void glue_keyPress  (class SampleChannel *ch, bool ctrl=0, bool shift=0);
void glue_keyPress  (class MidiChannel   *ch, bool ctrl=0, bool shift=0);
void glue_keyRelease(class Channel       *ch, bool ctrl=0, bool shift=0);
void glue_keyRelease(class SampleChannel *ch, bool ctrl=0, bool shift=0);

void glue_setBpm(const char *v1, const char *v2);
void glue_setBeats(int beats, int bars, bool expand);

/* start, stop, rewind sequencer
 * if gui == true the signal comes from an internal interaction on the
 * GUI, otherwise it's a MIDI/Jack/external signal. */

void glue_startStopSeq(bool gui=true);
void glue_startSeq    (bool gui=true);
void glue_stopSeq     (bool gui=true);
void glue_rewindSeq   ();

/* start/stopActionRec
 * handle the action recording. */

void glue_startStopActionRec();
void glue_startActionRec();
void glue_stopActionRec();

/* start/stopInputRec
 * handle the input recording (take). If gui == true the signal comes
 * from an internal interaction on the GUI, otherwise it's a
 * MIDI/Jack/external signal. Alert displays or not the popup message
 * if there are no available channels. */

void glue_startStopInputRec(bool gui=true, bool alert=true);
int  glue_startInputRec    (bool gui=true);
int  glue_stopInputRec     (bool gui=true);

/* start/stopReadingRecs
 * handle the 'R' button. If gui == true the signal comes from an
 * internal interaction on the GUI, otherwise it's a MIDI/Jack/external
 * signal. */

void glue_startStopReadingRecs(class SampleChannel *ch, bool gui=true);
void glue_startReadingRecs    (class SampleChannel *ch, bool gui=true);
void glue_stopReadingRecs     (class SampleChannel *ch, bool gui=true);

void glue_quantize(int val);

void glue_setChanVol(class Channel *ch, float v, bool gui=true);
void glue_setOutVol (float v, bool gui=true);
void glue_setInVol  (float v, bool gui=true);

void glue_setPanning(class gdEditor *win, class SampleChannel *ch, float val);

void glue_clearAllSamples();
void glue_clearAllRecs();

/* resetToInitState
 * reset Giada to init state. If resetGui also refresh all widgets. If
 * createColumns also build initial empty columns. */

void glue_resetToInitState(bool resetGui=true, bool createColumns=true);

void glue_startStopMetronome(bool gui=true);

/* setBeginEndChannel
 * sets start/end points in the sample editor.
 * Recalc=false: don't recalc internal position
 * check=true: check the points' consistency */

/** FIXME - nobody will call this via MIDI/keyb/mouse! */
void glue_setBeginEndChannel(class gdEditor *win, class SampleChannel *ch, int b, int e,
														 bool recalc=false, bool check=true);

/** FIXME - nobody will call this via MIDI/keyb/mouse! */
void glue_setBoost(class gdEditor *win, class SampleChannel *ch, float val, bool numeric);

void glue_setPitch(class gdEditor *win, class SampleChannel *ch, float val, bool numeric);

/* setVolEditor
 * handles the volume inside the SAMPLE EDITOR (not the main gui). The
 * numeric flag tells if we want to handle the dial or the numeric input
 * field. */

 /** FIXME - nobody will call this via MIDI/keyb/mouse! */
void glue_setVolEditor(class gdEditor *win, class SampleChannel *ch, float val, bool numeric);

/* mute
 * set mute on or off. If gui == true the signal comes from an internal
 * interaction on the GUI, otherwise it's a MIDI/Jack/external signal. */

void glue_setMute(class Channel *ch, bool gui=true);

/* solo on/off
 * set solo on and off. If gui == true the signal comes from an internal
 * interaction on the GUI, otherwise it's a MIDI/Jack/external signal. */

void glue_setSoloOn (class Channel *ch, bool gui=true);
void glue_setSoloOff(class Channel *ch, bool gui=true);

/* beatsDivide/Multiply
 * shrinks or enlarges the number of beats by 2. */

void glue_beatsMultiply();
void glue_beatsDivide();

#endif
