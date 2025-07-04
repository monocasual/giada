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

#ifndef GE_LEARNER_PACK_H
#define GE_LEARNER_PACK_H

#include "src/gui/elems/basics/pack.h"
#include "src/gui/elems/midiIO/midiLearner.h"
#include <string>
#include <vector>

namespace giada::v
{
class geMidiLearnerPack : public gePack
{
public:
	static constexpr int LEARNER_WIDTH = 334;

	geMidiLearnerPack(int x, int y, std::string title = "");

	void setCallbacks(std::function<void(uint32_t)>, std::function<void(uint32_t)>);
	void addMidiLearner(std::string label, int param, bool visible = true);
	void setEnabled(bool v);

	std::vector<geMidiLearner*> learners;

private:
	std::function<void(uint32_t)> m_onStartLearn;
	std::function<void(uint32_t)> m_onClearLearn;
};
} // namespace giada::v

#endif
