/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_midiInput
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2015 Giovanni A. Zuliani | Monocasual
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


#include "gd_midiInput.h"
#include "ge_mixed.h"
#include "ge_midiIoTools.h"
#include "gui_utils.h"
#include "kernelMidi.h"
#include "core/conf.h"
#include "sampleChannel.h"
#include "log.h"


extern Conf G_Conf;


gdMidiInput::gdMidiInput(int w, int h, const char *title)
	: gWindow(w, h, title)
{
}


/* -------------------------------------------------------------------------- */


gdMidiInput::~gdMidiInput() {
	kernelMidi::stopMidiLearn();
}


/* -------------------------------------------------------------------------- */


void gdMidiInput::stopMidiLearn(gLearner *learner) {
	kernelMidi::stopMidiLearn();
	learner->updateValue();
}


/* -------------------------------------------------------------------------- */


void gdMidiInput::__cb_learn(uint32_t *param, uint32_t msg, gLearner *l) {
	*param = msg;
	stopMidiLearn(l);
	gLog("[gdMidiGrabber] MIDI learn done - message=0x%X\n", msg);
}


/* -------------------------------------------------------------------------- */


void gdMidiInput::cb_learn(uint32_t msg, void *d) {
	cbData *data = (cbData*) d;
	gdMidiInput   *window  = (gdMidiInput*) data->window;
	gLearner      *learner = data->learner;
	uint32_t      *param   = learner->param;
	window->__cb_learn(param, msg, learner);
	free(data);
}


/* -------------------------------------------------------------------------- */


void gdMidiInput::cb_close(Fl_Widget *w, void *p)  { ((gdMidiInput*)p)->__cb_close(); }


/* -------------------------------------------------------------------------- */


void gdMidiInput::__cb_close() {
	do_callback();
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


gdMidiInputChannel::gdMidiInputChannel(Channel *ch)
	:	gdMidiInput(300, 206, "MIDI Input Setup"),
		ch(ch)
{
	char title[64];
	sprintf(title, "MIDI Input Setup (channel %d)", ch->index+1);
	label(title);

	set_modal();

	enable = new gCheck(8, 8, 120, 20, "enable MIDI input");
	new gLearner(8,  30, w()-16, "key press",   cb_learn, &ch->midiInKeyPress);
	new gLearner(8,  54, w()-16, "key release", cb_learn, &ch->midiInKeyRel);
	new gLearner(8,  78, w()-16, "key kill",    cb_learn, &ch->midiInKill);
	new gLearner(8, 102, w()-16, "mute",        cb_learn, &ch->midiInMute);
	new gLearner(8, 126, w()-16, "solo",        cb_learn, &ch->midiInSolo);
	new gLearner(8, 150, w()-16, "volume",      cb_learn, &ch->midiInVolume);
	int yy = 178;

	if (ch->type == CHANNEL_SAMPLE) {
		size(300, 254);
		new gLearner(8, 174, w()-16, "pitch", cb_learn, &((SampleChannel*)ch)->midiInPitch);
		new gLearner(8, 198, w()-16, "read actions", cb_learn, &((SampleChannel*)ch)->midiInReadActions);
		yy = 226;
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


void gdMidiInputChannel::__cb_enable() {
	ch->midiIn = enable->value();
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


gdMidiInputMaster::gdMidiInputMaster()
	: gdMidiInput(300, 256, "MIDI Input Setup (global)")
{
	set_modal();

	new gLearner(8,   8, w()-16, "rewind",           &cb_learn, &G_Conf.midiInRewind);
	new gLearner(8,  32, w()-16, "play/stop",        &cb_learn, &G_Conf.midiInStartStop);
	new gLearner(8,  56, w()-16, "action recording", &cb_learn, &G_Conf.midiInActionRec);
	new gLearner(8,  80, w()-16, "input recording",  &cb_learn, &G_Conf.midiInInputRec);
	new gLearner(8, 104, w()-16, "metronome",        &cb_learn, &G_Conf.midiInMetronome);
	new gLearner(8, 128, w()-16, "input volume",     &cb_learn, &G_Conf.midiInVolumeIn);
	new gLearner(8, 152, w()-16, "output volume",    &cb_learn, &G_Conf.midiInVolumeOut);
	new gLearner(8, 176, w()-16, "sequencer ×2",     &cb_learn, &G_Conf.midiInBeatDouble);
	new gLearner(8, 200, w()-16, "sequencer ÷2",     &cb_learn, &G_Conf.midiInBeatHalf);
	ok = new gButton(w()-88, 228, 80, 20, "Close");

	ok->callback(cb_close, (void*)this);

	gu_setFavicon(this);
	show();
}
