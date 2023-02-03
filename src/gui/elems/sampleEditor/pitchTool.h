/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef GE_PITCH_TOOL_H
#define GE_PITCH_TOOL_H

#include "gui/elems/basics/flex.h"

namespace giada::c::sampleEditor
{
struct Data;
}

namespace giada::v
{
class geInput;
class geBox;
class geDial;
class geTextButton;
class geImageButton;
class gePitchTool : public geFlex
{
public:
	gePitchTool(const c::sampleEditor::Data& d);

	void rebuild(const c::sampleEditor::Data& d);
	void update(float v, bool isDial = false);

private:
	const c::sampleEditor::Data* m_data;

	geBox*         m_label;
	geDial*        m_dial;
	geInput*       m_input;
	geTextButton*  m_pitchToBar;
	geTextButton*  m_pitchToSong;
	geImageButton* m_pitchHalf;
	geImageButton* m_pitchDouble;
	geTextButton*  m_pitchReset;
};
} // namespace giada::v

#endif
