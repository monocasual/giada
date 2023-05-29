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

#include "gui/dialogs/midiIO/midiOutputSampleCh.h"
#include "core/model/model.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/textButton.h"
#include "gui/elems/midiIO/midiLearner.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include <FL/Fl_Pack.H>

extern giada::v::Ui g_ui;

namespace giada::v
{
gdMidiOutputSampleCh::gdMidiOutputSampleCh(ID channelId)
: gdMidiOutputBase(350, 140, channelId)
{
	end();

	m_enableLightning = new geCheck(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN, w(), 20, g_ui.getI18Text(LangMap::MIDIOUTPUT_CHANNEL_ENABLE_LIGHTNING));

	m_learners = new geLightningLearnerPack(G_GUI_OUTER_MARGIN,
	    m_enableLightning->y() + m_enableLightning->h() + 8, channelId);

	m_close = new geTextButton(w() - 88, m_learners->y() + m_learners->h() + 8, 80, 20,
	    g_ui.getI18Text(LangMap::COMMON_CLOSE));

	add(m_enableLightning);
	add(m_learners);
	add(m_close);

	m_close->onClick = [this]() { do_callback(); };

	m_enableLightning->onChange = [this](bool value) {
		c::io::channel_enableMidiLightning(m_channelId, value);
	};

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
} // namespace giada::v
