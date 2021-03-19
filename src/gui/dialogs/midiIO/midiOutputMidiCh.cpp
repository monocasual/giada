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

#include "midiOutputMidiCh.h"
#include "glue/io.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/choice.h"
#include "gui/elems/midiIO/midiLearner.h"
#include "utils/gui.h"
#include <FL/Fl_Pack.H>

namespace giada
{
namespace v
{
gdMidiOutputMidiCh::gdMidiOutputMidiCh(ID channelId)
: gdMidiOutputBase(300, 168, channelId)
{
	end();
	setTitle(m_channelId + 1);

	m_enableOut   = new geCheck(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN, 150, G_GUI_UNIT, "Enable MIDI output");
	m_chanListOut = new geChoice(w() - 108, G_GUI_OUTER_MARGIN, 100, G_GUI_UNIT);

	m_enableLightning = new geCheck(G_GUI_OUTER_MARGIN, m_chanListOut->y() + m_chanListOut->h() + G_GUI_OUTER_MARGIN,
	    120, G_GUI_UNIT, "Enable MIDI lightning output");

	m_learners = new geLightningLearnerPack(G_GUI_OUTER_MARGIN,
	    m_enableLightning->y() + m_enableLightning->h() + G_GUI_OUTER_MARGIN, channelId);

	m_close = new geButton(w() - 88, m_learners->y() + m_learners->h() + G_GUI_OUTER_MARGIN, 80, G_GUI_UNIT, "Close");

	add(m_enableOut);
	add(m_chanListOut);
	add(m_enableLightning);
	add(m_learners);
	add(m_close);

	m_chanListOut->add("Channel 1");
	m_chanListOut->add("Channel 2");
	m_chanListOut->add("Channel 3");
	m_chanListOut->add("Channel 4");
	m_chanListOut->add("Channel 5");
	m_chanListOut->add("Channel 6");
	m_chanListOut->add("Channel 7");
	m_chanListOut->add("Channel 8");
	m_chanListOut->add("Channel 9");
	m_chanListOut->add("Channel 10");
	m_chanListOut->add("Channel 11");
	m_chanListOut->add("Channel 12");
	m_chanListOut->add("Channel 13");
	m_chanListOut->add("Channel 14");
	m_chanListOut->add("Channel 15");
	m_chanListOut->add("Channel 16");
	m_chanListOut->value(0);

	m_chanListOut->callback(cb_setChannel, (void*)this);
	m_enableOut->callback(cb_enableOut, (void*)this);
	m_enableLightning->callback(cb_enableLightning, (void*)this);
	m_close->callback(cb_close, (void*)this);

	u::gui::setFavicon(this);

	set_modal();
	rebuild();
	show();
}

/* -------------------------------------------------------------------------- */

void gdMidiOutputMidiCh::rebuild()
{
	m_data = c::io::channel_getOutputData(m_channelId);

	assert(m_data.output.has_value());

	m_learners->update(m_data);
	m_chanListOut->value(m_data.output->filter);
	m_enableOut->value(m_data.output->enabled);

	m_data.output->enabled ? m_chanListOut->activate() : m_chanListOut->deactivate();
}

/* -------------------------------------------------------------------------- */

void gdMidiOutputMidiCh::cb_enableOut(Fl_Widget* /*w*/, void* p) { ((gdMidiOutputMidiCh*)p)->cb_enableOut(); }
void gdMidiOutputMidiCh::cb_setChannel(Fl_Widget* /*w*/, void* p) { ((gdMidiOutputMidiCh*)p)->cb_setChannel(); }

/* -------------------------------------------------------------------------- */

void gdMidiOutputMidiCh::cb_enableOut()
{
	c::io::channel_enableMidiOutput(m_channelId, m_enableOut->value());
}

/* -------------------------------------------------------------------------- */

void gdMidiOutputMidiCh::cb_setChannel()
{
	c::io::channel_setMidiOutputFilter(m_channelId, m_chanListOut->value());
}
} // namespace v
} // namespace giada
