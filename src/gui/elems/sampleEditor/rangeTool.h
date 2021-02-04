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


#ifndef GE_RANGE_TOOL_H
#define GE_RANGE_TOOL_H


#include "gui/elems/basics/pack.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/basics/button.h"


namespace giada {
namespace v 
{
class geRangeTool : public gePack
{
public:

	geRangeTool(const c::sampleEditor::Data& d, int x, int y);

	void rebuild(const c::sampleEditor::Data& d);
	void update(Frame begin, Frame end);

private:

	static void cb_setChanPos   (Fl_Widget* /*w*/, void* p);
	static void cb_resetStartEnd(Fl_Widget* /*w*/, void* p);
	void cb_setChanPos();
	void cb_resetStartEnd();

	const c::sampleEditor::Data* m_data;

	geBox    m_label;
	geInput  m_begin;
	geInput  m_end;
	geButton m_reset;
};
}} // giada::v::


#endif
