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


#include "../../../utils/gui.h"
#include "../../../core/const.h"
#include "../../../core/sampleChannel.h"
#include "../../../core/pluginHost.h"
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
  string title = "MIDI Input Setup (channel " + gu_itoa(ch->index+1) + ")";
	label(title.c_str());

	set_modal();

  size_range(300, 400);

	enable = new gCheck(8, 8, 120, 20, "enable MIDI input");

  container = new geScroll(8, enable->y()+enable->h()+4, 282, 230);
  container->begin();
  container->box(FL_BORDER_BOX);

  	new geMidiLearner(8,  30, LEARNER_WIDTH, "key press",   cb_learn, &ch->midiInKeyPress);
  	new geMidiLearner(8,  54, LEARNER_WIDTH, "key release", cb_learn, &ch->midiInKeyRel);
  	new geMidiLearner(8,  78, LEARNER_WIDTH, "key kill",    cb_learn, &ch->midiInKill);
  	new geMidiLearner(8, 102, LEARNER_WIDTH, "arm",         cb_learn, &ch->midiInArm);
  	new geMidiLearner(8, 126, LEARNER_WIDTH, "mute",        cb_learn, &ch->midiInMute);
  	new geMidiLearner(8, 150, LEARNER_WIDTH, "solo",        cb_learn, &ch->midiInSolo);
  	new geMidiLearner(8, 174, LEARNER_WIDTH, "volume",      cb_learn, &ch->midiInVolume);

  	if (ch->type == CHANNEL_SAMPLE) {
  		new geMidiLearner(8, 198, w()-16, "pitch", cb_learn, &((SampleChannel*)ch)->midiInPitch);
  		new geMidiLearner(8, 222, w()-16, "read actions", cb_learn, &((SampleChannel*)ch)->midiInReadActions);
  	}

    /* PLUGINS */
#if 0
    vector <Plugin *> *plugins = G_PluginHost.getStack(PluginHost::CHANNEL, ch);
    for (unsigned i=0; i<plugins->size(); i++) {
      int lx = container->x() + ((i + 1) * LEARNER_WIDTH);
      new geMidiLearner(lx, 30, LEARNER_WIDTH, "test", cb_learn, &ch->midiInVolume);
    }
#endif
    /* PLUGINS */

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


void gdMidiInputChannel::cb_enable(Fl_Widget *w, void *p)  { ((gdMidiInputChannel*)p)->__cb_enable(); }


/* -------------------------------------------------------------------------- */


void gdMidiInputChannel::__cb_enable()
{
	ch->midiIn = enable->value();
}
