/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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
#include "../../../core/const.h"
#include "../../../core/conf.h"
#include "../../../core/sampleChannel.h"
#ifdef WITH_VST
  #include "../../../core/pluginHost.h"
  #include "../../../core/plugin.h"
#endif
#include "../../../utils/string.h"
#include "../../elems/ge_mixed.h"
#include "../../elems/midiLearner.h"
#include "../../elems/basics/scroll.h"
#include "midiInputChannel.h"


#ifdef WITH_VST
extern PluginHost G_PluginHost;
#endif


using std::string;
using std::vector;
using namespace giada;


gdMidiInputChannel::gdMidiInputChannel(Channel *ch)
	:	gdMidiInputBase(conf::midiInputX, conf::midiInputY, conf::midiInputW,
      conf::midiInputH, "MIDI Input Setup"),
		ch(ch)
{
  string title = "MIDI Input Setup (channel " + gu_itoa(ch->index+1) + ")";
	label(title.c_str());
  size_range(G_DEFAULT_MIDI_INPUT_UI_W, G_DEFAULT_MIDI_INPUT_UI_H);

	enable = new gCheck(8, 8, 120, 20, "enable MIDI input");

  container = new geScroll(8, enable->y()+enable->h()+4, w()-16, h()-68);
  container->begin();

    addChannelLearners();
#ifdef WITH_VST
    addPluginLearners();
#endif

  container->end();

  Fl_Group *groupButtons = new Fl_Group(8, container->y()+container->h()+8, container->w(), 20);
  groupButtons->begin();

    gBox *spacer = new gBox(groupButtons->x(), groupButtons->y(), 100, 20); 	// spacer window border <-> buttons
	  ok = new gButton(w()-88, groupButtons->y(), 80, 20, "Close");

  groupButtons->resizable(spacer);
  groupButtons->end();

  ok->callback(cb_close, (void*)this);

  enable->value(ch->midiIn);
	enable->callback(cb_enable, (void*)this);

  resizable(container);

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
  Fl_Pack *pack = new Fl_Pack(container->x(), container->y(), LEARNER_WIDTH, 200);
  pack->spacing(4);
  pack->begin();

    gBox *header = new gBox(0, 0, LEARNER_WIDTH, 20, "channel");
    header->box(FL_BORDER_BOX);
    new geMidiLearner(0, 0, LEARNER_WIDTH, "key press",   cb_learn, &ch->midiInKeyPress);
    new geMidiLearner(0, 0, LEARNER_WIDTH, "key release", cb_learn, &ch->midiInKeyRel);
    new geMidiLearner(0, 0, LEARNER_WIDTH, "key kill",    cb_learn, &ch->midiInKill);
    new geMidiLearner(0, 0, LEARNER_WIDTH, "arm",         cb_learn, &ch->midiInArm);
    new geMidiLearner(0, 0, LEARNER_WIDTH, "mute",        cb_learn, &ch->midiInMute);
    new geMidiLearner(0, 0, LEARNER_WIDTH, "solo",        cb_learn, &ch->midiInSolo);
    new geMidiLearner(0, 0, LEARNER_WIDTH, "volume",      cb_learn, &ch->midiInVolume);
    if (ch->type == CHANNEL_SAMPLE) {
      new geMidiLearner(0, 0, LEARNER_WIDTH, "pitch", cb_learn, &((SampleChannel*)ch)->midiInPitch);
      new geMidiLearner(0, 0, LEARNER_WIDTH, "read actions", cb_learn, &((SampleChannel*)ch)->midiInReadActions);
    }

  pack->end();
}


/* -------------------------------------------------------------------------- */


#ifdef WITH_VST

void gdMidiInputChannel::addPluginLearners()
{
  vector <Plugin *> *plugins = G_PluginHost.getStack(PluginHost::CHANNEL, ch);
  for (unsigned i=0; i<plugins->size(); i++) {

    Fl_Pack *pack = new Fl_Pack(container->x() + ((i + 1) * (LEARNER_WIDTH + 8)),
      container->y(), LEARNER_WIDTH, 200);
    pack->spacing(4);
    pack->begin();

      Plugin *plugin = plugins->at(i);

      gBox *header = new gBox(0, 0, LEARNER_WIDTH, 20, plugin->getName().c_str());
      header->box(FL_BORDER_BOX);

      for (int k=0; k<plugin->getNumParameters(); k++)
        new geMidiLearner(0, 0, LEARNER_WIDTH, plugin->getParameterName(k).c_str(),
          cb_learn, &plugin->midiInParams.at(k));

    pack->end();
  }
}

#endif


/* -------------------------------------------------------------------------- */


void gdMidiInputChannel::cb_enable(Fl_Widget *w, void *p)  { ((gdMidiInputChannel*)p)->__cb_enable(); }


/* -------------------------------------------------------------------------- */


void gdMidiInputChannel::__cb_enable()
{
	ch->midiIn = enable->value();
}
