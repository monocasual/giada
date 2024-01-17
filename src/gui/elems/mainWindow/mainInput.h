/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef GE_MAIN_INPUT_H
#define GE_MAIN_INPUT_H

#include "glue/main.h"
#include "gui/elems/basics/flex.h"

namespace giada::v
{
class geDial;
class geSoundMeter;
class geImageButton;
class geMidiLed;
class geMainInput : public geFlex
{
public:
	geMainInput();

	void refresh();
	void rebuild();

	void setInVol(float v);
	void setMasterFxInFull(bool v);
	void setMidiInActivity();

private:
	c::main::IO m_io;

	geSoundMeter*  m_outMeter;
	geSoundMeter*  m_inMeter;
	geDial*        m_inVol;
	geImageButton* m_inToOut;
	geImageButton* m_masterFxIn;
	geMidiLed*     m_midiIn;
};
} // namespace giada::v

#endif
