/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "gui/dialogs/midiIO/midiOutputBase.h"
#include "glue/io.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/midiIO/midiLearner.h"
#include "gui/ui.h"
#include "utils/gui.h"

extern giada::v::Ui* g_ui;

namespace giada::v
{
geLightningLearnerPack::geLightningLearnerPack(int x, int y, ID channelId)
: geMidiLearnerPack(x, y)
{
	setCallbacks(
	    [channelId](int param) { c::io::channel_startMidiLearn(param, channelId); },
	    [channelId](int param) { c::io::channel_clearMidiLearn(param, channelId); });
	addMidiLearner(g_ui->getI18Text(LangMap::MIDIOUTPUT_CHANNEL_LEARN_PLAYING), G_MIDI_OUT_L_PLAYING);
	addMidiLearner(g_ui->getI18Text(LangMap::MIDIOUTPUT_CHANNEL_LEARN_MUTE), G_MIDI_OUT_L_MUTE);
	addMidiLearner(g_ui->getI18Text(LangMap::MIDIOUTPUT_CHANNEL_LEARN_SOLO), G_MIDI_OUT_L_SOLO);
}

/* -------------------------------------------------------------------------- */

void geLightningLearnerPack::update(const c::io::Channel_OutputData& d)
{
	learners[0]->update(d.lightningPlaying);
	learners[1]->update(d.lightningMute);
	learners[2]->update(d.lightningSolo);
	setEnabled(d.lightningEnabled);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gdMidiOutputBase::gdMidiOutputBase(int w, int h, ID channelId)
: gdWindow(u::gui::getCenterWinBounds({-1, -1, w, h}), g_ui->getI18Text(LangMap::MIDIOUTPUT_CHANNEL_TITLE), WID_MIDI_OUTPUT)
, m_channelId(channelId)
{
}

/* -------------------------------------------------------------------------- */

gdMidiOutputBase::~gdMidiOutputBase()
{
	c::io::stopMidiLearn();
}
} // namespace giada::v
