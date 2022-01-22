/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "gui/elems/basics/button.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/basics/pack.h"
#include "gui/elems/soundMeter.h"
#ifdef WITH_VST
#include "gui/elems/basics/statusButton.h"
#endif
#include "glue/main.h"

namespace giada::v
{
class geMainIO : public gePack
{
public:
	geMainIO(int x, int y);

	void refresh();
	void rebuild();

	void setOutVol(float v);
	void setInVol(float v);
#ifdef WITH_VST
	void setMasterFxOutFull(bool v);
	void setMasterFxInFull(bool v);
#endif

private:
	static void cb_outVol(Fl_Widget* /*w*/, void* p);
	static void cb_inVol(Fl_Widget* /*w*/, void* p);
	static void cb_inToOut(Fl_Widget* /*w*/, void* p);
	void        cb_outVol();
	void        cb_inVol();
	void        cb_inToOut();
#ifdef WITH_VST
	static void cb_masterFxOut(Fl_Widget* /*w*/, void* p);
	static void cb_masterFxIn(Fl_Widget* /*w*/, void* p);
	void        cb_masterFxOut();
	void        cb_masterFxIn();
#endif

	c::main::IO m_io;

	geSoundMeter outMeter;
	geSoundMeter inMeter;
	geDial       outVol;
	geDial       inVol;
	geButton     inToOut;
#ifdef WITH_VST
	geStatusButton masterFxOut;
	geStatusButton masterFxIn;
#endif
};
} // namespace giada::v

#endif
