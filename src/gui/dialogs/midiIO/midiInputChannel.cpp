/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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


#include <FL/Fl_Pack.H>
#include "../../../utils/gui.h"
#include "../../../utils/log.h"
#include "../../../core/const.h"
#include "../../../core/conf.h"
#include "../../../core/sampleChannel.h"
#ifdef WITH_VST
	#include "../../../core/pluginHost.h"
	#include "../../../core/plugin.h"
#endif
#include "../../../utils/string.h"
#include "../../elems/midiLearner.h"
#include "../../elems/basics/scroll.h"
#include "../../elems/basics/box.h"
#include "../../elems/basics/button.h"
#include "../../elems/basics/choice.h"
#include "../../elems/basics/check.h"
#include "midiInputChannel.h"


using std::string;
using std::vector;
using namespace giada::m;


gdMidiInputChannel::gdMidiInputChannel(Channel* ch)
	:	gdMidiInputBase(conf::midiInputX, conf::midiInputY, conf::midiInputW,
			conf::midiInputH, "MIDI Input Setup"),
		ch(ch)
{
	string title = "MIDI Input Setup (channel " + gu_iToString(ch->index+1) + ")";
	label(title.c_str());
	size_range(G_DEFAULT_MIDI_INPUT_UI_W, G_DEFAULT_MIDI_INPUT_UI_H);

	Fl_Group* groupHeader = new Fl_Group(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN, w(), 20);
	groupHeader->begin();

		enable = new geCheck(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN, 120, G_GUI_UNIT, 
			"enable MIDI input");
		channel = new geChoice(enable->x()+enable->w()+44, G_GUI_OUTER_MARGIN, 120, G_GUI_UNIT);

	groupHeader->resizable(nullptr);
	groupHeader->end();

	container = new geScroll(G_GUI_OUTER_MARGIN, enable->y()+enable->h()+G_GUI_OUTER_MARGIN, 
		w()-16, h()-76);
	container->begin();

		addChannelLearners();
#ifdef WITH_VST
		addPluginLearners();
#endif

	container->end();

	Fl_Group* groupButtons = new Fl_Group(8, container->y()+container->h()+8, container->w(), 20);
	groupButtons->begin();

		geBox* spacer = new geBox(groupButtons->x(), groupButtons->y(), 100, 20); 	// spacer window border <-> buttons
		ok = new geButton(w()-88, groupButtons->y(), 80, 20, "Close");

	groupButtons->resizable(spacer);
	groupButtons->end();

	ok->callback(cb_close, (void*)this);

	enable->value(ch->midiIn);
	enable->callback(cb_enable, (void*)this);

	channel->add("Channel (any)");
	channel->add("Channel 1");
	channel->add("Channel 2");
	channel->add("Channel 3");
	channel->add("Channel 4");
	channel->add("Channel 5");
	channel->add("Channel 6");
	channel->add("Channel 7");
	channel->add("Channel 8");
	channel->add("Channel 9");
	channel->add("Channel 10");
	channel->add("Channel 11");
	channel->add("Channel 12");
	channel->add("Channel 13");
	channel->add("Channel 14");
	channel->add("Channel 15");
	channel->add("Channel 16");
	channel->value(ch->getMidiInFilter() == -1 ? 0 : ch->getMidiInFilter() + 1);
	channel->callback(cb_setChannel, (void*)this);

	resizable(container);

	end();

	gu_setFavicon(this);
	set_modal();
	show();
}


/* -------------------------------------------------------------------------- */


gdMidiInputChannel::~gdMidiInputChannel()
{
	conf::midiInputX = x();
	conf::midiInputY = y();
	conf::midiInputW = w();
	conf::midiInputH = h();
}


/* -------------------------------------------------------------------------- */


void gdMidiInputChannel::addChannelLearners()
{
	Fl_Pack* pack = new Fl_Pack(container->x(), container->y(), LEARNER_WIDTH, 200);
	pack->spacing(4);
	pack->begin();

		geBox *header = new geBox(0, 0, LEARNER_WIDTH, 20, "channel");
		header->box(FL_BORDER_BOX);
		new geMidiLearner(0, 0, LEARNER_WIDTH, "key press",   cb_learn, &ch->midiInKeyPress, ch);
		new geMidiLearner(0, 0, LEARNER_WIDTH, "key release", cb_learn, &ch->midiInKeyRel, ch);
		new geMidiLearner(0, 0, LEARNER_WIDTH, "key kill",    cb_learn, &ch->midiInKill, ch);
		new geMidiLearner(0, 0, LEARNER_WIDTH, "arm",         cb_learn, &ch->midiInArm, ch);
		new geMidiLearner(0, 0, LEARNER_WIDTH, "mute",        cb_learn, &ch->midiInMute, ch);
		new geMidiLearner(0, 0, LEARNER_WIDTH, "solo",        cb_learn, &ch->midiInSolo, ch);
		new geMidiLearner(0, 0, LEARNER_WIDTH, "volume",      cb_learn, &ch->midiInVolume, ch);
		if (ch->type == CHANNEL_SAMPLE) {
			new geMidiLearner(0, 0, LEARNER_WIDTH, "pitch", cb_learn, 
				&(static_cast<SampleChannel*>(ch))->midiInPitch, ch);
			new geMidiLearner(0, 0, LEARNER_WIDTH, "read actions", cb_learn, 
				&(static_cast<SampleChannel*>(ch))->midiInReadActions, ch);
		}

	pack->end();
}


/* -------------------------------------------------------------------------- */


#ifdef WITH_VST

void gdMidiInputChannel::addPluginLearners()
{
	vector<Plugin*>* plugins = pluginHost::getStack(pluginHost::CHANNEL, ch);
	for (unsigned i=0; i<plugins->size(); i++) {

		Fl_Pack* pack = new Fl_Pack(container->x() + ((i + 1) * (LEARNER_WIDTH + 8)),
			container->y(), LEARNER_WIDTH, 200);
		pack->spacing(4);
		pack->begin();

			Plugin* plugin = plugins->at(i);

			geBox* header = new geBox(0, 0, LEARNER_WIDTH, 20, plugin->getName().c_str());
			header->box(FL_BORDER_BOX);

			for (int k=0; k<plugin->getNumParameters(); k++)
				new geMidiLearner(0, 0, LEARNER_WIDTH, plugin->getParameterName(k).c_str(),
					cb_learn, &plugin->midiInParams.at(k), ch);

		pack->end();
	}
}

#endif


/* -------------------------------------------------------------------------- */


void gdMidiInputChannel::cb_enable(Fl_Widget* w, void* p) { ((gdMidiInputChannel*)p)->cb_enable(); }
void gdMidiInputChannel::cb_setChannel(Fl_Widget* w, void* p) { ((gdMidiInputChannel*)p)->cb_setChannel(); }


/* -------------------------------------------------------------------------- */


void gdMidiInputChannel::cb_enable()
{
	ch->midiIn = enable->value();
	enable->value() ? channel->activate() : channel->deactivate();
}


/* -------------------------------------------------------------------------- */


void gdMidiInputChannel::cb_setChannel()
{
	ch->setMidiInFilter(channel->value() == 0 ? -1 : channel->value() - 1);
	gu_log("[gdMidiInputChannel] Set MIDI channel to %d\n", 
		ch->getMidiInFilter());
}

