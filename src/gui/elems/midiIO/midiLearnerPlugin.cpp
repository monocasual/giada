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


#include <FL/Fl.H>
#include "core/model/model.h"
#include "glue/io.h"
#include "gui/elems/basics/button.h"
#include "midiLearnerPlugin.h"


namespace giada {
namespace v 
{
geMidiLearnerPlugin::geMidiLearnerPlugin(int x, int y, int w, std::string l, int param, uint32_t value, ID pluginId)
: geMidiLearnerBase(x, y, w, l, param, value),
  m_pluginId       (pluginId)
{
}


/* -------------------------------------------------------------------------- */


void geMidiLearnerPlugin::refresh()
{
	m::model::onGet(m::model::plugins, m_pluginId, [&](const m::Plugin& p)
	{
		assert(static_cast<size_t>(m_param) < p.midiInParams.size());
		update(p.midiInParams[m_param]);
	});
}


/* -------------------------------------------------------------------------- */


void geMidiLearnerPlugin::onLearn()
{
	if (m_button->value() == 1)
		c::io::startPluginMidiLearn(m_param, m_pluginId);
	else
		c::io::stopMidiLearn();
}


void geMidiLearnerPlugin::onReset()
{
	if (Fl::event_button() == FL_RIGHT_MOUSE)
		c::io::clearPluginMidiLearn(m_param, m_pluginId);	
}
}} // giada::v::


#endif