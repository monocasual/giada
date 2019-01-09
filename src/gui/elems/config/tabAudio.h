/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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


#ifndef GE_TAB_AUDIO_H
#define GE_TAB_AUDIO_H


#include <FL/Fl_Group.H>


class geChoice;
class geCheck;
class geButton;
class geInput;


class geTabAudio : public Fl_Group
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
	geCheck  *limitOutput;
	geChoice *buffersize;
	geInput  *delayComp;

	geTabAudio(int x, int y, int w, int h);

	void save();
};


#endif
