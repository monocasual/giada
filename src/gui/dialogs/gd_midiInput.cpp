/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_midiInput
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


#include "../../utils/gui.h"
#include "../../core/kernelMidi.h"
#include "../../core/conf.h"
#include "../../core/const.h"
#include "../../core/sampleChannel.h"
#include "../../utils/log.h"
#include "../../utils/string.h"
#include "../elems/ge_mixed.h"
#include "../elems/midiLearner.h"
#include "gd_midiInput.h"


extern Conf       G_Conf;
extern KernelMidi G_KernelMidi;


using std::string;


gdMidiInput::gdMidiInput(int w, int h, const char *title)
	: gWindow(w, h, title)
{
}


/* -------------------------------------------------------------------------- */


gdMidiInput::~gdMidiInput()
{
	G_KernelMidi.stopMidiLearn();
}


/* -------------------------------------------------------------------------- */


void gdMidiInput::stopMidiLearn(geMidiLearner *learner)
{
	G_KernelMidi.stopMidiLearn();
	learner->updateValue();
}


/* -------------------------------------------------------------------------- */


void gdMidiInput::__cb_learn(uint32_t *param, uint32_t msg, geMidiLearner *l)
{
	*param = msg;
	stopMidiLearn(l);
	gu_log("[gdMidiGrabber] MIDI learn done - message=0x%X\n", msg);
}


/* -------------------------------------------------------------------------- */


void gdMidiInput::cb_learn(uint32_t msg, void *d)
{
	geMidiLearner::cbData_t *data = (geMidiLearner::cbData_t *) d;
	gdMidiInput   *window  = (gdMidiInput*) data->window;
	geMidiLearner *learner = data->learner;
	uint32_t      *param   = learner->param;
	window->__cb_learn(param, msg, learner);
}


/* -------------------------------------------------------------------------- */


void gdMidiInput::cb_close(Fl_Widget *w, void *p)  { ((gdMidiInput*)p)->__cb_close(); }


/* -------------------------------------------------------------------------- */


void gdMidiInput::__cb_close()
{
	do_callback();
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


gdMidiInputChannel::gdMidiInputChannel(Channel *ch)
	:	gdMidiInput(300, 230, "MIDI Input Setup"),
		ch(ch)
{
  string title = "MIDI Input Setup (channel " + gu_itoa(ch->index+1) + ")";
	label(title.c_str());

	set_modal();

	enable = new gCheck(8, 8, 120, 20, "enable MIDI input");
	new geMidiLearner(8,  30, w()-16, "key press",   cb_learn, &ch->midiInKeyPress);
	new geMidiLearner(8,  54, w()-16, "key release", cb_learn, &ch->midiInKeyRel);
	new geMidiLearner(8,  78, w()-16, "key kill",    cb_learn, &ch->midiInKill);
	new geMidiLearner(8, 102, w()-16, "arm",         cb_learn, &ch->midiInArm);
	new geMidiLearner(8, 126, w()-16, "mute",        cb_learn, &ch->midiInMute);
	new geMidiLearner(8, 150, w()-16, "solo",        cb_learn, &ch->midiInSolo);
	new geMidiLearner(8, 174, w()-16, "volume",      cb_learn, &ch->midiInVolume);
	int yy = 202;

	if (ch->type == CHANNEL_SAMPLE) {
		size(300, 278);
		new geMidiLearner(8, 198, w()-16, "pitch", cb_learn, &((SampleChannel*)ch)->midiInPitch);
		new geMidiLearner(8, 222, w()-16, "read actions", cb_learn, &((SampleChannel*)ch)->midiInReadActions);
		yy = 250;
	}

	ok = new gButton(w()-88, yy, 80, 20, "Close");
	ok->callback(cb_close, (void*)this);

	enable->value(ch->midiIn);
	enable->callback(cb_enable, (void*)this);

	gu_setFavicon(this);
	show();
}


/* -------------------------------------------------------------------------- */


void gdMidiInputChannel::cb_enable(Fl_Widget *w, void *p)  { ((gdMidiInputChannel*)p)->__cb_enable(); }


/* -------------------------------------------------------------------------- */


void gdMidiInputChannel::__cb_enable()
{
	ch->midiIn = enable->value();
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


gdMidiInputMaster::gdMidiInputMaster()
	: gdMidiInput(300, 256, "MIDI Input Setup (global)")
{
	set_modal();

	new geMidiLearner(8,   8, w()-16, "rewind",           &cb_learn, &G_Conf.midiInRewind);
	new geMidiLearner(8,  32, w()-16, "play/stop",        &cb_learn, &G_Conf.midiInStartStop);
	new geMidiLearner(8,  56, w()-16, "action recording", &cb_learn, &G_Conf.midiInActionRec);
	new geMidiLearner(8,  80, w()-16, "input recording",  &cb_learn, &G_Conf.midiInInputRec);
	new geMidiLearner(8, 104, w()-16, "metronome",        &cb_learn, &G_Conf.midiInMetronome);
	new geMidiLearner(8, 128, w()-16, "input volume",     &cb_learn, &G_Conf.midiInVolumeIn);
	new geMidiLearner(8, 152, w()-16, "output volume",    &cb_learn, &G_Conf.midiInVolumeOut);
	new geMidiLearner(8, 176, w()-16, "sequencer ×2",     &cb_learn, &G_Conf.midiInBeatDouble);
	new geMidiLearner(8, 200, w()-16, "sequencer ÷2",     &cb_learn, &G_Conf.midiInBeatHalf);
	ok = new gButton(w()-88, 228, 80, 20, "Close");

	ok->callback(cb_close, (void*)this);

	gu_setFavicon(this);
	show();
}
