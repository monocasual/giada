/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

class geCheck;
class geButton;
class geInput;

namespace giada::v
{
class geChoice;
class geTabAudio : public Fl_Group
{
  public:
	geTabAudio(int x, int y, int w, int h);

	void save();

	geChoice* soundsys;
	geChoice* buffersize;
	geChoice* samplerate;
	geChoice* sounddevOut;
	geButton* devOutInfo;
	geChoice* channelsOut;
	geCheck*  limitOutput;
	geChoice* sounddevIn;
	geButton* devInInfo;
	geChoice* channelsIn;
	geInput*  recTriggerLevel;
	geChoice* rsmpQuality;

  private:
	static void cb_deactivate_sounddev(Fl_Widget* /*w*/, void* p);
	static void cb_fetchInChans(Fl_Widget* /*w*/, void* p);
	static void cb_fetchOutChans(Fl_Widget* /*w*/, void* p);
	static void cb_showInputInfo(Fl_Widget* /*w*/, void* p);
	static void cb_showOutputInfo(Fl_Widget* /*w*/, void* p);
	void        cb_deactivate_sounddev();
	void        cb_fetchInChans();
	void        cb_fetchOutChans();
	void        cb_showInputInfo();
	void        cb_showOutputInfo();

	void fetchSoundDevs();
	void fetchInChans(int menuItem);
	void fetchOutChans();
	int  findMenuDevice(geChoice* m, int device);

	int soundsysInitValue;
};
} // namespace giada::v

#endif
