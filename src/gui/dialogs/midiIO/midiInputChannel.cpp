/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * midiInputChannel
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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


extern PluginHost G_PluginHost;


using std::string;


gdMidiInputChannel::gdMidiInputChannel(Channel *ch)
	:	gdMidiInputBase(300, 400, "MIDI Input Setup"),
		ch(ch)
{
	label(string("MIDI Input Setup (channel " + gu_itoa(ch->index+1) + ")").c_str());
	set_modal();
  size_range(300, 400);

	enable = new gCheck(8, 8, 120, 20, "enable MIDI input");

  container = new geScroll(8, enable->y()+enable->h()+4, 282, 330);
  container->begin();

    Fl_Pack *pack = new Fl_Pack(container->x(), container->y(), LEARNER_WIDTH, 200);
    pack->spacing(4);
    pack->begin();

      gBox *channelBox = new gBox(0, 0, LEARNER_WIDTH, 20, "channel controls");
      channelBox->box(FL_BORDER_BOX);
    	new geMidiLearner(0, 0, LEARNER_WIDTH, "key press",   cb_learn, &ch->midiInKeyPress);
    	new geMidiLearner(0, 0, LEARNER_WIDTH, "key release", cb_learn, &ch->midiInKeyRel);
    	new geMidiLearner(0, 0, LEARNER_WIDTH, "key kill",    cb_learn, &ch->midiInKill);
    	new geMidiLearner(0, 0, LEARNER_WIDTH, "arm",         cb_learn, &ch->midiInArm);
    	new geMidiLearner(0, 0, LEARNER_WIDTH, "mute",        cb_learn, &ch->midiInMute);
    	new geMidiLearner(0, 0, LEARNER_WIDTH, "solo",        cb_learn, &ch->midiInSolo);
    	new geMidiLearner(0, 0, LEARNER_WIDTH, "volume",      cb_learn, &ch->midiInVolume);
    	if (ch->type == CHANNEL_SAMPLE) {
    		new geMidiLearner(0, 0, w()-16, "pitch", cb_learn, &((SampleChannel*)ch)->midiInPitch);
    		new geMidiLearner(0, 0, w()-16, "read actions", cb_learn, &((SampleChannel*)ch)->midiInReadActions);
    	}

    pack->end();

#ifdef WITH_VST

    addPluginLearners();

#endif

  container->end();

	ok = new gButton(w()-88, container->y()+container->h()+8, 80, 20, "Close");
	ok->callback(cb_close, (void*)this);

	enable->value(ch->midiIn);
	enable->callback(cb_enable, (void*)this);

	gu_setFavicon(this);

  resizable(container);

	show();
}


/* -------------------------------------------------------------------------- */


#ifdef WITH_VST

void gdMidiInputChannel::addPluginLearners()
{
  /* Plugins' parameters layout reflect the structure of the matrix
  Channel::midiInPlugins. It is safe to assume then that i and k indexes match
  both the structure of Channel::midiInPlugins and vector <Plugin *> *plugins. */

  vector <Plugin *> *plugins = G_PluginHost.getStack(PluginHost::CHANNEL, ch);
  for (unsigned i=0; i<plugins->size(); i++) {

    Fl_Pack *pack = new Fl_Pack(container->x() + ((i + 1) * (LEARNER_WIDTH + 8)),
      container->y(), LEARNER_WIDTH, 200);
    pack->spacing(4);
    pack->begin();

      Plugin *plugin = plugins->at(i);

      gBox *pluginBox = new gBox(0, 0, LEARNER_WIDTH, 20, plugin->getName().toRawUTF8());
      pluginBox->box(FL_BORDER_BOX);

      int numParams = plugin->getNumParameters();
      for (int k=0; k<numParams; k++)
        new geMidiLearner(0, 0, LEARNER_WIDTH,
          plugin->getParameterName(k).toRawUTF8(), cb_learn,
            &ch->midiInPlugins.at(i).at(k));

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
