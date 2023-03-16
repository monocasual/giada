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

#include "gui/dialogs/midiIO/midiOutputMidiCh.h"
#include "glue/io.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/choice.h"
#include "gui/elems/basics/textButton.h"
#include "gui/elems/midiIO/midiLearner.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include <FL/Fl_Pack.H>

extern giada::v::Ui g_ui;

namespace giada::v
{
gdMidiOutputMidiCh::gdMidiOutputMidiCh(ID channelId)
: gdMidiOutputBase(350, 168, channelId)
{
	end();

	m_enableOut   = new geCheck(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN, 150, G_GUI_UNIT, g_ui.getI18Text(LangMap::MIDIOUTPUT_CHANNEL_ENABLE));
	m_chanListOut = new geChoice(w() - 108, G_GUI_OUTER_MARGIN, 100, G_GUI_UNIT);

	m_enableLightning = new geCheck(G_GUI_OUTER_MARGIN, m_chanListOut->y() + m_chanListOut->h() + G_GUI_OUTER_MARGIN,
	    120, G_GUI_UNIT, g_ui.getI18Text(LangMap::MIDIOUTPUT_CHANNEL_ENABLE_LIGHTNING));

	m_learners = new geLightningLearnerPack(G_GUI_OUTER_MARGIN,
	    m_enableLightning->y() + m_enableLightning->h() + G_GUI_OUTER_MARGIN, channelId);

	m_close = new geTextButton(w() - 88, m_learners->y() + m_learners->h() + G_GUI_OUTER_MARGIN, 80, G_GUI_UNIT,
	    g_ui.getI18Text(LangMap::COMMON_CLOSE));

	add(m_enableOut);
	add(m_chanListOut);
	add(m_enableLightning);
	add(m_learners);
	add(m_close);

	m_chanListOut->addItem("Channel 1");
	m_chanListOut->addItem("Channel 2");
	m_chanListOut->addItem("Channel 3");
	m_chanListOut->addItem("Channel 4");
	m_chanListOut->addItem("Channel 5");
	m_chanListOut->addItem("Channel 6");
	m_chanListOut->addItem("Channel 7");
	m_chanListOut->addItem("Channel 8");
	m_chanListOut->addItem("Channel 9");
	m_chanListOut->addItem("Channel 10");
	m_chanListOut->addItem("Channel 11");
	m_chanListOut->addItem("Channel 12");
	m_chanListOut->addItem("Channel 13");
	m_chanListOut->addItem("Channel 14");
	m_chanListOut->addItem("Channel 15");
	m_chanListOut->addItem("Channel 16");
	m_chanListOut->showItem(0);
	m_chanListOut->onChange = [this](ID id) {
		c::io::channel_setMidiOutputFilter(m_channelId, id);
	};

	m_enableOut->callback(cb_enableOut, (void*)this);

	m_enableLightning->callback(cb_enableLightning, (void*)this);

	m_close->onClick = [this]() { do_callback(); };

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
	m_chanListOut->showItem(m_data.output->filter);
	m_enableOut->value(m_data.output->enabled);

	m_data.output->enabled ? m_chanListOut->activate() : m_chanListOut->deactivate();
}

/* -------------------------------------------------------------------------- */

void gdMidiOutputMidiCh::cb_enableOut(Fl_Widget* /*w*/, void* p) { ((gdMidiOutputMidiCh*)p)->cb_enableOut(); }

/* -------------------------------------------------------------------------- */

void gdMidiOutputMidiCh::cb_enableOut()
{
	c::io::channel_enableMidiOutput(m_channelId, m_enableOut->value());
}
} // namespace giada::v
