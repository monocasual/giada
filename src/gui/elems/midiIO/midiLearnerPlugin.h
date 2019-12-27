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


#ifdef WITH_VST


#ifndef GE_MIDI_LEARNER_PLUGIN_H
#define GE_MIDI_LEARNER_PLUGIN_H


#include "core/types.h"
#include "midiLearnerBase.h"


namespace giada {
namespace v 
{
class geMidiLearnerPlugin : public geMidiLearnerBase
{
public:

	geMidiLearnerPlugin(int x, int y, int w, std::string l, int param, uint32_t value, ID pluginId);
	
	void refresh() override;
	void onLearn() override;
	void onReset() override;

private:

	ID m_pluginId;
};
}} // giada::v::


#endif
#endif
