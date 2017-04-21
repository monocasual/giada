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


#ifndef GD_CONFIG_H
#define GD_CONFIG_H


#include "window.h"


class gTabAudio;
class gTabBehaviors;
class gTabMidi;
class gTabMisc;
#ifdef WITH_VST
class gTabPlugins;
#endif
class geButton;
class geChoice;
class gCheck;
class geInput;
class gRadio;
class geBox;


class gdConfig : public gdWindow
{
private:

	static void cb_save_config        (Fl_Widget *w, void *p);
	static void cb_cancel             (Fl_Widget *w, void *p);
	inline void __cb_save_config();
	inline void __cb_cancel();

public:

	gdConfig(int w, int h);
	~gdConfig();

	gTabAudio     *tabAudio;
	gTabBehaviors *tabBehaviors;
	gTabMidi      *tabMidi;
	gTabMisc      *tabMisc;
#ifdef WITH_VST
	gTabPlugins   *tabPlugins;
#endif
	geButton 	    *save;
	geButton 	    *cancel;
};


/* -------------------------------------------------------------------------- */


class gTabMidi : public Fl_Group
{
private:

	void fetchSystems();
	void fetchOutPorts();
	void fetchInPorts();
	void fetchMidiMaps();

	static void cb_changeSystem  (Fl_Widget *w, void *p);
	inline void __cb_changeSystem();

	int systemInitValue;

public:

	geChoice *system;
	geChoice *portOut;
	geChoice *portIn;
	gCheck  *noNoteOff;
	geChoice *midiMap;
	geChoice *sync;

	gTabMidi(int x, int y, int w, int h);

	void save();
};


/* -------------------------------------------------------------------------- */


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
	int  findMenuDevice(geChoice *m, int device);

	int soundsysInitValue;

public:

	geChoice *soundsys;
	geChoice *samplerate;
	geChoice *rsmpQuality;
	geChoice *sounddevIn;
	geButton  *devInInfo;
	geChoice *channelsIn;
	geChoice *sounddevOut;
	geButton  *devOutInfo;
	geChoice *channelsOut;
	gCheck  *limitOutput;
	geChoice *buffersize;
	geInput  *delayComp;

	gTabAudio(int x, int y, int w, int h);

	void save();
};


/* -------------------------------------------------------------------------- */


class gTabBehaviors : public Fl_Group
{
private:

	static void cb_radio_mutex  (Fl_Widget *w, void *p);
	inline void __cb_radio_mutex(Fl_Widget *w);

public:

	gRadio *recsStopOnChanHalt_1;
	gRadio *recsStopOnChanHalt_0;
	gRadio *chansStopOnSeqHalt_1;
	gRadio *chansStopOnSeqHalt_0;
	gCheck *treatRecsAsLoops;

	gTabBehaviors(int x, int y, int w, int h);

	void save();
};


/* -------------------------------------------------------------------------- */


class gTabMisc : public Fl_Group
{
public:

	geChoice *debugMsg;

	gTabMisc(int x, int y, int w, int h);

	void save();
};


/* -------------------------------------------------------------------------- */


#ifdef WITH_VST

class gTabPlugins : public Fl_Group
{
private:

	static void cb_scan  (Fl_Widget *w, void *p);
	static void cb_onScan(float progress, void *p);
	inline void __cb_scan(Fl_Widget *w);

	void updateCount();

public:

	geInput   *folderPath;
	geButton *scanButton;
	geBox     *info;

	gTabPlugins(int x, int y, int w, int h);

	void save();
};

#endif

#endif
