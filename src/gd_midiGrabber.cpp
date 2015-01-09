/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_midiGrabber
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2014 Giovanni A. Zuliani | Monocasual
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
 * ------------------------------------------------------------------ */


#include "gd_midiGrabber.h"
#include "ge_mixed.h"
#include "gui_utils.h"
#include "kernelMidi.h"
#include "conf.h"
#include "sampleChannel.h"
#include "log.h"


extern Conf G_Conf;


gdMidiGrabber::gdMidiGrabber(int w, int h, const char *title)
	: gWindow(w, h, title)
{
}


/* ------------------------------------------------------------------ */


gdMidiGrabber::~gdMidiGrabber() {
	kernelMidi::stopMidiLearn();
}


/* ------------------------------------------------------------------ */


void gdMidiGrabber::stopMidiLearn(gLearner *learner) {
	kernelMidi::stopMidiLearn();
	learner->updateValue();
}


/* ------------------------------------------------------------------ */


void gdMidiGrabber::__cb_learn(uint32_t *param, uint32_t msg, gLearner *l) {
	*param = msg;
	stopMidiLearn(l);
	gLog("[gdMidiGrabber] MIDI learn done - message=0x%X\n", msg);
}


/* ------------------------------------------------------------------ */


void gdMidiGrabber::cb_learn(uint32_t msg, void *d) {
	cbData *data = (cbData*) d;
	gdMidiGrabber *grabber = (gdMidiGrabber*) data->grabber;
	gLearner      *learner = data->learner;
	uint32_t      *param   = learner->param;
	grabber->__cb_learn(param, msg, learner);
	free(data);
}


/* ------------------------------------------------------------------ */


void gdMidiGrabber::cb_close(Fl_Widget *w, void *p)  { ((gdMidiGrabber*)p)->__cb_close(); }


/* ------------------------------------------------------------------ */


void gdMidiGrabber::__cb_close() {
	do_callback();
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gdMidiGrabberChannel::gdMidiGrabberChannel(Channel *ch, bool midichannel)
	:	gdMidiGrabber(300, 206, "MIDI Input Setup"),
		ch(ch)
{
	char title[64];
	sprintf(title, "MIDI Input Setup (channel %d)", ch->index+1);
	label(title);

	set_modal();

	if( midichannel ) {
		enable = new gCheck(8, 2, 120, 20, "enable MIDI input");
		enableTunnel = new gCheck(8, 16, 120, 20, "enable MIDI tunnelling");
	} else {
		enable = new gCheck(8, 8, 120, 20, "enable MIDI input");
		enableTunnel = NULL;
	}
	
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

	ok = new gButton(w()-88, yy, 80, 20, "Ok");
	ok->callback(cb_close, (void*)this);

	enable->value(ch->midiIn);
	enable->callback(cb_enable, (void*)this);
	enableTunnel->value(ch->tunnelIn);
	enableTunnel->callback(cb_enableTunnel, (void*)this);

	gu_setFavicon(this);
	show();
}


/* ------------------------------------------------------------------ */


void gdMidiGrabberChannel::cb_enable(Fl_Widget *w, void *p)  { ((gdMidiGrabberChannel*)p)->__cb_enable(); }

void gdMidiGrabberChannel::cb_enableTunnel(Fl_Widget *w, void *p)  { ((gdMidiGrabberChannel*)p)->__cb_enableTunnel(); }


/* ------------------------------------------------------------------ */


void gdMidiGrabberChannel::__cb_enable() {
	ch->midiIn = enable->value();
}


/* ------------------------------------------------------------------ */


void gdMidiGrabberChannel::__cb_enableTunnel() {
	ch->tunnelIn = enable->value();
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gdMidiGrabberMaster::gdMidiGrabberMaster()
	: gdMidiGrabber(300, 256, "MIDI Input Setup (global)")
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
	ok = new gButton(w()-88, 228, 80, 20, "Ok");

	ok->callback(cb_close, (void*)this);

	gu_setFavicon(this);
	show();
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gLearner::gLearner(int X, int Y, int W, const char *l, kernelMidi::cb_midiLearn *cb, uint32_t *param)
	: Fl_Group(X, Y, W, 20),
		callback(cb),
		param   (param)
{
	begin();
	text   = new gBox(x(), y(), 156, 20, l);
	value  = new gClick(text->x()+text->w()+4, y(), 80, 20, "(not set)");
	button = new gButton(value->x()+value->w()+4, y(), 40, 20, "learn");
	end();

	text->box(G_BOX);
	text->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

	value->box(G_BOX);
	value->callback(cb_value, (void*)this);
	value->when(FL_WHEN_RELEASE);
	updateValue();

	button->type(FL_TOGGLE_BUTTON);
	button->callback(cb_button, (void*)this);
}


/* ------------------------------------------------------------------ */


void gLearner::updateValue() {
	char buf[16];
	if (*param != 0x0)
		snprintf(buf, 9, "0x%X", *param);
	else
		snprintf(buf, 16, "(not set)");
	value->copy_label(buf);
	button->value(0);
}


/* ------------------------------------------------------------------ */


void gLearner::cb_button(Fl_Widget *v, void *p) { ((gLearner*)p)->__cb_button(); }
void gLearner::cb_value(Fl_Widget *v, void *p) { ((gLearner*)p)->__cb_value(); }


/* ------------------------------------------------------------------ */


void gLearner::__cb_value() {
	if (Fl::event_button() == FL_RIGHT_MOUSE) {
		*param = 0x0;
		updateValue();
	}
	/// TODO - elif (LEFT_MOUSE) : insert values by hand
}


/* ------------------------------------------------------------------ */


void gLearner::__cb_button() {
	if (button->value() == 1) {
		cbData *data  = (cbData*) malloc(sizeof(cbData));
		data->grabber = (gdMidiGrabber*) parent();  // parent = gdMidiGrabberChannel
		data->learner = this;
		kernelMidi::startMidiLearn(callback, (void*)data);
	}
	else
		kernelMidi::stopMidiLearn();
}

