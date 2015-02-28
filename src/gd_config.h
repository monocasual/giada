/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_config
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2015 Giovanni A. Zuliani | Monocasual
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

#ifndef GD_CONFIG_H
#define GD_CONFIG_H

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tabs.H>
#include "ge_window.h"


class gdConfig : public gWindow 
{
private:
	static void cb_save_config        (Fl_Widget *w, void *p);
	static void cb_cancel             (Fl_Widget *w, void *p);
	inline void __cb_save_config();
	inline void __cb_cancel();

public:
	gdConfig(int w, int h);
	~gdConfig();

	class gTabAudio     *tabAudio;
	class gTabBehaviors *tabBehaviors;
	class gTabMidi      *tabMidi;
	class gTabMisc      *tabMisc;
	class gClick 	      *save;
	class gClick 	      *cancel;
};


/* ------------------------------------------------------------------ */


class gTabMidi : public Fl_Group 
{
private:
	void fetchSystems();
	void fetchOutPorts();
	void fetchInPorts();
	void fetchMidiMaps();

	static void cb_changeSystem  (Fl_Widget *w, void *p);
	inline void __cb_changeSystem();

	static void cb_browseMidiMap (Fl_Widget *w, void *p);
	inline void __cb_browseMidiMap();

	int systemInitValue;

	char  midiMapPath[FILENAME_MAX];
	char  lastFileMap[FILENAME_MAX];

public:
	class gChoice *system;
	class gChoice *portOut;
	class gChoice *portIn;
	class gCheck  *noNoteOff;
	class gChoice *midiMap;
	class gChoice *sync;

	gTabMidi(int x, int y, int w, int h);

	void save();
};


/* ------------------------------------------------------------------ */


class gTabAudio : public Fl_Group 
{
private:
	static void cb_deactivate_sounddev(Fl_Widget *w, void *p);
	static void cb_fetchInChans       (Fl_Widget *w, void *p);
	static void cb_fetchOutChans      (Fl_Widget *w, void *p);
	static void cb_showInputInfo      (Fl_Widget *w, void *p);
	static void cb_showOutputInfo     (Fl_Widget *w, void *p);
	inline void __cb_deactivate_sounddev();
	inline void __cb_fetchInChans();
	inline void __cb_fetchOutChans();
	inline void __cb_showInputInfo();
	inline void __cb_showOutputInfo();

	void fetchSoundDevs();
	void fetchInChans(int menuItem);
	void fetchOutChans(int menuItem);
	int  findMenuDevice(class gChoice *m, int device);

	int soundsysInitValue;

public:
	class gChoice *soundsys;
	class gChoice *samplerate;
	class gChoice *rsmpQuality;
	class gChoice *sounddevIn;
	class gClick  *devInInfo;
	class gChoice *channelsIn;
	class gChoice *sounddevOut;
	class gClick  *devOutInfo;
	class gChoice *channelsOut;
	class gCheck  *limitOutput;
	class gChoice *buffersize;
	class gInput  *delayComp;

	gTabAudio(int x, int y, int w, int h);

	void save();
};


/* ------------------------------------------------------------------ */


class gTabBehaviors : public Fl_Group 
{
private:
	static void cb_radio_mutex  (Fl_Widget *w, void *p);
	inline void __cb_radio_mutex(Fl_Widget *w);

public:
	class gRadio *recsStopOnChanHalt_1;
	class gRadio *recsStopOnChanHalt_0;
	class gRadio *chansStopOnSeqHalt_1;
	class gRadio *chansStopOnSeqHalt_0;
	class gCheck *treatRecsAsLoops;

	gTabBehaviors(int x, int y, int w, int h);

	void save();
};


/* ------------------------------------------------------------------ */


class gTabMisc : public Fl_Group 
{
public:
	class gChoice *debugMsg;	

	gTabMisc(int x, int y, int w, int h);

	void save();
};


#endif
