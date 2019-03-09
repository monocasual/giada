/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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


#include <cassert>
#include <FL/Fl_Pack.H>
#include "utils/gui.h"
#include "utils/log.h"
#include "core/model/model.h"
#include "core/channels/sampleChannel.h"
#include "core/const.h"
#include "core/conf.h"
#ifdef WITH_VST
#include "core/pluginHost.h"
#include "core/plugin.h"
#endif
#include "utils/string.h"
#include "gui/elems/midiLearner.h"
#include "gui/elems/basics/scroll.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/choice.h"
#include "gui/elems/basics/check.h"
#include "midiInputChannel.h"


namespace giada {
namespace v 
{
gdMidiInputChannel::gdMidiInputChannel(ID channelId)
: gdMidiInputBase(m::conf::midiInputX, m::conf::midiInputY, m::conf::midiInputW, 
	m::conf::midiInputH, "MIDI Input Setup"),
  m_ch           (m::model::getLayout()->getChannel(channelId))
{
	copy_label(std::string("MIDI Input Setup (m_channel " + std::to_string(m_ch->id) + ")").c_str());
	
	int extra = m_ch->type == ChannelType::SAMPLE ? 28 : 0;

	Fl_Group* groupHeader = new Fl_Group(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN, w(), 20 + extra);
	groupHeader->begin();

		m_enable = new geCheck(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN, 120, G_GUI_UNIT, 
			"Enable MIDI input");
		m_channel = new geChoice(m_enable->x()+m_enable->w()+44, G_GUI_OUTER_MARGIN, 120, G_GUI_UNIT);
		m_veloAsVol = new geCheck(G_GUI_OUTER_MARGIN, m_enable->y()+m_enable->h()+G_GUI_OUTER_MARGIN, 120, G_GUI_UNIT, 
			"Velocity drives volume (one-shot only)");

	groupHeader->resizable(nullptr);
	groupHeader->end();

	m_container = new geScroll(G_GUI_OUTER_MARGIN, groupHeader->y()+groupHeader->h()+G_GUI_OUTER_MARGIN, 
		w()-16, h()-72-extra);
	m_container->begin();

		addChannelLearners();
#ifdef WITH_VST
		addPluginLearners();
#endif

	m_container->end();

	Fl_Group* groupButtons = new Fl_Group(8, m_container->y()+m_container->h()+8, m_container->w(), 20);
	groupButtons->begin();

		geBox* spacer = new geBox(groupButtons->x(), groupButtons->y(), 100, 20); 	// spacer window border <-> buttons
		m_ok = new geButton(w()-88, groupButtons->y(), 80, 20, "Close");

	groupButtons->resizable(spacer);
	groupButtons->end();


	m_ok->callback(cb_close, (void*)this);

	m_enable->value(m_ch->midiIn);
	m_enable->callback(cb_enable, (void*)this);

	if (m_ch->type == ChannelType::SAMPLE) {
		m_veloAsVol->value(static_cast<m::SampleChannel*>(m_ch)->midiInVeloAsVol);
		m_veloAsVol->callback(cb_veloAsVol, (void*)this);	
	}
	else
		m_veloAsVol->hide();

	m_channel->add("Channel (any)");
	m_channel->add("Channel 1");
	m_channel->add("Channel 2");
	m_channel->add("Channel 3");
	m_channel->add("Channel 4");
	m_channel->add("Channel 5");
	m_channel->add("Channel 6");
	m_channel->add("Channel 7");
	m_channel->add("Channel 8");
	m_channel->add("Channel 9");
	m_channel->add("Channel 10");
	m_channel->add("Channel 11");
	m_channel->add("Channel 12");
	m_channel->add("Channel 13");
	m_channel->add("Channel 14");
	m_channel->add("Channel 15");
	m_channel->add("Channel 16");
	m_channel->value(m_ch->midiInFilter == -1 ? 0 : m_ch->midiInFilter + 1);
	m_channel->callback(cb_setChannel, (void*)this);

	resizable(m_container);

	end();

	size_range(G_DEFAULT_MIDI_INPUT_UI_W, G_DEFAULT_MIDI_INPUT_UI_H);

	u::gui::setFavicon(this);
	set_modal();
	show();
}


/* -------------------------------------------------------------------------- */


void gdMidiInputChannel::addChannelLearners()
{
	Fl_Pack* pack = new Fl_Pack(m_container->x(), m_container->y(), LEARNER_WIDTH, 200);
	pack->spacing(G_GUI_INNER_MARGIN);
	pack->begin();
		geBox* header = new geBox(0, 0, LEARNER_WIDTH, G_GUI_UNIT, "m_channel");
		header->box(FL_BORDER_BOX);
		m_learners.push_back(new geMidiLearner(0, 0, LEARNER_WIDTH, "key press",   m_ch->midiInKeyPress, m_ch));
		m_learners.push_back(new geMidiLearner(0, 0, LEARNER_WIDTH, "key release", m_ch->midiInKeyRel,   m_ch));
		m_learners.push_back(new geMidiLearner(0, 0, LEARNER_WIDTH, "key kill",    m_ch->midiInKill,     m_ch));
		m_learners.push_back(new geMidiLearner(0, 0, LEARNER_WIDTH, "arm",         m_ch->midiInArm,      m_ch));
		m_learners.push_back(new geMidiLearner(0, 0, LEARNER_WIDTH, "mute",        m_ch->midiInMute,     m_ch));
		m_learners.push_back(new geMidiLearner(0, 0, LEARNER_WIDTH, "solo",        m_ch->midiInSolo,     m_ch));
		m_learners.push_back(new geMidiLearner(0, 0, LEARNER_WIDTH, "volume",      m_ch->midiInVolume,   m_ch));
		if (m_ch->type == ChannelType::SAMPLE) {
			m::SampleChannel* sch = static_cast<m::SampleChannel*>(m_ch);
			m_learners.push_back(new geMidiLearner(0, 0, LEARNER_WIDTH, "pitch",        sch->midiInPitch,       m_ch));
			m_learners.push_back(new geMidiLearner(0, 0, LEARNER_WIDTH, "read actions", sch->midiInReadActions, m_ch));
		}
	pack->end();
}


/* -------------------------------------------------------------------------- */


#ifdef WITH_VST

void gdMidiInputChannel::addPluginLearners()
{
	int i = 1;
	for (const std::shared_ptr<m::Plugin>& p : *m::model::getLayout()->getPlugins(m_ch->id)) {

		Fl_Pack* pack = new Fl_Pack(m_container->x() + (i++ * (LEARNER_WIDTH + G_GUI_OUTER_MARGIN)),
			m_container->y(), LEARNER_WIDTH, 200);
		pack->spacing(G_GUI_INNER_MARGIN);
		pack->begin();

			geBox* header = new geBox(0, 0, LEARNER_WIDTH, G_GUI_UNIT, p->getName().c_str());
			header->box(FL_BORDER_BOX);

			for (int k=0; k<p->getNumParameters(); k++)
				m_learners.push_back(new geMidiLearner(0, 0, LEARNER_WIDTH, 
					p->getParameterName(k).c_str(), p->midiInParams.at(k), m_ch));

		pack->end();
	}
}

#endif


/* -------------------------------------------------------------------------- */


void gdMidiInputChannel::cb_enable(Fl_Widget* w, void* p) { ((gdMidiInputChannel*)p)->cb_enable(); }
void gdMidiInputChannel::cb_setChannel(Fl_Widget* w, void* p) { ((gdMidiInputChannel*)p)->cb_setChannel(); }
void gdMidiInputChannel::cb_veloAsVol(Fl_Widget* w, void* p) { ((gdMidiInputChannel*)p)->cb_veloAsVol(); }


/* -------------------------------------------------------------------------- */


void gdMidiInputChannel::cb_enable()
{
	m_ch->midiIn = m_enable->value();
	m_enable->value() ? m_channel->activate() : m_channel->deactivate();
}


/* -------------------------------------------------------------------------- */


void gdMidiInputChannel::cb_veloAsVol()
{
	static_cast<m::SampleChannel*>(m_ch)->midiInVeloAsVol = m_veloAsVol->value();
}


/* -------------------------------------------------------------------------- */


void gdMidiInputChannel::cb_setChannel()
{
	m_ch->midiInFilter = m_channel->value() == 0 ? -1 : m_channel->value() - 1;
	gu_log("[gdMidiInputChannel] Set MIDI m_channel to %d\n", m_ch->midiInFilter.load());
}

}} // giada::v::