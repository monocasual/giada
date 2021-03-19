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

#include "midiOutputSampleCh.h"
#include "core/model/model.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/midiIO/midiLearner.h"
#include "utils/gui.h"
#include <FL/Fl_Pack.H>

namespace giada
{
namespace v
{
gdMidiOutputSampleCh::gdMidiOutputSampleCh(ID channelId)
: gdMidiOutputBase(300, 140, channelId)
{
	end();
	setTitle(m_channelId);

	m_enableLightning = new geCheck(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN, 120, 20, "Enable MIDI lightning output");

	m_learners = new geLightningLearnerPack(G_GUI_OUTER_MARGIN,
	    m_enableLightning->y() + m_enableLightning->h() + 8, channelId);

	m_close = new geButton(w() - 88, m_learners->y() + m_learners->h() + 8, 80, 20, "Close");

	add(m_enableLightning);
	add(m_learners);
	add(m_close);

	m_close->callback(cb_close, (void*)this);
	m_enableLightning->callback(cb_enableLightning, (void*)this);

	u::gui::setFavicon(this);

	set_modal();
	rebuild();
	show();
}

/* -------------------------------------------------------------------------- */

void gdMidiOutputSampleCh::rebuild()
{
	m_data = c::io::channel_getOutputData(m_channelId);

	m_enableLightning->value(m_data.lightningEnabled);
	m_learners->update(m_data);
}
} // namespace v
} // namespace giada
