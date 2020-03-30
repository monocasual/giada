/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#ifndef GE_MAIN_IO_H
#define GE_MAIN_IO_H


#include <FL/Fl_Pack.H>


class geSoundMeter;
class geDial;
#ifdef WITH_VST
class geStatusButton;
class geButton;
#endif


namespace giada {
namespace v
{
class geMainIO : public Fl_Pack
{
public:

	geMainIO(int x, int y);

	void refresh();
	void rebuild();

	void setOutVol(float v);
	void setInVol (float v);
#ifdef WITH_VST
	void setMasterFxOutFull(bool v);
	void setMasterFxInFull(bool v);
#endif

private:

	static void cb_outVol     (Fl_Widget* v, void* p);
	static void cb_inVol      (Fl_Widget* v, void* p);
#ifdef WITH_VST
	static void cb_masterFxOut(Fl_Widget* v, void* p);
	static void cb_masterFxIn (Fl_Widget* v, void* p);
	static void cb_inToOut    (Fl_Widget* v, void* p);
#endif
	void cb_outVol();
	void cb_inVol();
#ifdef WITH_VST
	void cb_masterFxOut();
	void cb_masterFxIn();
	void cb_inToOut();
#endif

	geSoundMeter* outMeter;
	geSoundMeter* inMeter;
	geDial*       outVol;
	geDial*       inVol;
	geStatusButton* midiInMeter;
	geStatusButton* midiOutMeter;
#ifdef WITH_VST
  geStatusButton* masterFxOut;
  geStatusButton* masterFxIn;
  geButton*       inToOut;
#endif
};
}} // giada::v::


#endif
