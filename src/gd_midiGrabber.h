/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_midiGrabber
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


#ifndef GD_MIDIGRABBER_H
#define GD_MIDIGRABBER_H


#include "ge_window.h"
#include "kernelMidi.h"
#include "utils.h"
#include "ge_mixed.h"


class gLearner : public Fl_Group {

private:

	/* midiValue
	 * pointer to ch->midiIn[value] */

	uint32_t *midiValue;

	/* callback
	 * cb to pass to kernelMidi. Requires two parameters:
	 * uint32_t msg - MIDI message
	 * void   *data - extra data */

	kernelMidi::cb_midiLearn *callback;

	class gBox    *text;
	class gBox    *value;
	class gButton *button;

	static void cb_button(Fl_Widget *v, void *p);
	inline void __cb_button();

public:

	gLearner(int x, int y, int w, const char *l, kernelMidi::cb_midiLearn *cb, uint32_t *midiValue);

	void updateValue();
};


/* ------------------------------------------------------------------ */


class gdMidiGrabber : public gWindow {

protected:

	void stopMidiLearn(gLearner *l);

public:

	gdMidiGrabber();
	~gdMidiGrabber();
};


/* ------------------------------------------------------------------ */


class gdMidiGrabberChannel : public gdMidiGrabber {

private:

	class Channel *ch;

	gCheck *enabled;

	//gVector <gLearner *> items; for future use, with vst parameters

	/* cb_*
	 * list of callbacks attached to kernelMidi to learn various actions. */

	static void cb_learnKeyPressRel  (uint32_t msg, void *data);
	static void cb_learnKill         (uint32_t msg, void *data);
	static void cb_learnMute         (uint32_t msg, void *data);
	static void cb_learnSolo         (uint32_t msg, void *data);
	static void cb_learnVolume       (uint32_t msg, void *data);
	inline void __cb_learnKeyPressRel(uint32_t msg, gLearner *l);
	inline void __cb_learnKill       (uint32_t msg, gLearner *l);
	inline void __cb_learnMute       (uint32_t msg, gLearner *l);
	inline void __cb_learnSolo       (uint32_t msg, gLearner *l);
	inline void __cb_learnVolume     (uint32_t msg, gLearner *l);

public:

	gdMidiGrabberChannel(class Channel *ch);
};


/* ------------------------------------------------------------------ */


class gdMidiGrabberMaster : public gdMidiGrabber {

private:

	static void cb_learnRewind      (uint32_t msg, void *data);
	static void cb_learnStartStop   (uint32_t msg, void *data);
	static void cb_learnActionRec   (uint32_t msg, void *data);
	static void cb_learnInputRec    (uint32_t msg, void *data);
	inline void __cb_learnRewind    (uint32_t msg, gLearner *l);
	inline void __cb_learnStartStop (uint32_t msg, gLearner *l);
	inline void __cb_learnActionRec (uint32_t msg, gLearner *l);
	inline void __cb_learnInputRec  (uint32_t msg, gLearner *l);

public:

	gdMidiGrabberMaster();
};


/* ------------------------------------------------------------------ */


/* cbData
 * struct we pass to kernelMidi as extra parameter. Local scope made
 * with unnamed namespace. Infos:
 * http://stackoverflow.com/questions/4422507/superiority-of-unnamed-namespace-over-static */

namespace {
	struct cbData {
		gdMidiGrabber *grabber;
		gLearner      *learner;
	};
}


#endif
