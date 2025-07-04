/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/gui/elems/midiIO/midiLearnerPack.h"
#include "src/glue/io.h"
#include "src/gui/elems/basics/box.h"
#include <cassert>

namespace giada::v
{
geMidiLearnerPack::geMidiLearnerPack(int X, int Y, std::string title)
: gePack(X, Y, Direction::VERTICAL)
{
	end();

	if (title != "")
	{
		geBox* header = new geBox(0, 0, LEARNER_WIDTH, G_GUI_UNIT, title.c_str());
		header->box(FL_BORDER_BOX);
		add(header);
	}
}

/* -------------------------------------------------------------------------- */

void geMidiLearnerPack::setCallbacks(std::function<void(uint32_t)> s, std::function<void(uint32_t)> c)
{
	m_onStartLearn = s;
	m_onClearLearn = c;
}

/* -------------------------------------------------------------------------- */

void geMidiLearnerPack::addMidiLearner(std::string label, int param, bool visible)
{
	geMidiLearner* l = new geMidiLearner(0, 0, LEARNER_WIDTH, G_GUI_UNIT, label, param);

	l->onStartLearn = m_onStartLearn;
	l->onClearLearn = m_onClearLearn;
	l->onStopLearn  = []()
	{ c::io::stopMidiLearn(); };

	add(l);
	if (!visible)
		l->hide();
	learners.push_back(l);
}

/* -------------------------------------------------------------------------- */

void geMidiLearnerPack::setEnabled(bool v)
{
	if (v)
		for (auto* l : learners)
			l->activate();
	else
		for (auto* l : learners)
			l->deactivate();
}
} // namespace giada::v