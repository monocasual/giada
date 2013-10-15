/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_midiGrabber
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2013 Giovanni A. Zuliani | Monocasual
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


gdMidiGrabber::gdMidiGrabber()
	: gWindow(300, 154, "MIDI Input Setup")
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
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gdMidiGrabberChannel::gdMidiGrabberChannel(Channel *ch)
	:	gdMidiGrabber(), ch(ch)
{
	set_modal();

	enabled = new gCheck(8, 8, 120, 20, "enable MIDI input");
	new gLearner(8,  30, w()-16, "key press/release", cb_learnKeyPressRel, &ch->midiInKeyPress);
	new gLearner(8,  54, w()-16, "key kill", cb_learnKill, &ch->midiInKill);
	new gLearner(8,  78, w()-16, "mute", cb_learnMute, &ch->midiInMute);
	new gLearner(8, 102, w()-16, "solo", cb_learnSolo, &ch->midiInSolo);
	new gLearner(8, 126, w()-16, "volume", cb_learnVolume, &ch->midiInVolume);

	enabled->value(ch->midiIn);

	gu_setFavicon(this);
	show();
}


/* ------------------------------------------------------------------ */


void gdMidiGrabberChannel::cb_learnKeyPressRel(uint32_t msg, void *d) {
	cbData *data = (cbData*) d;
	gdMidiGrabberChannel *grabber = (gdMidiGrabberChannel*) data->grabber;
	gLearner             *learner = data->learner;
	grabber->__cb_learnKeyPressRel(msg, learner);
	free(data);
}

void gdMidiGrabberChannel::__cb_learnKeyPressRel(uint32_t msg, gLearner *l) {
	ch->midiInKeyPress = msg;
	ch->midiInKeyRel   = msg & 0x8FFFFFFF;  // key release of the same note, same value, same velo
	stopMidiLearn(l);
	printf("[gdMidiGrabberChannel] MIDI learn keypress/rel - done with message=0x%X\n", msg);
}


/* ------------------------------------------------------------------ */


void gdMidiGrabberChannel::cb_learnMute(uint32_t msg, void *d) {
	cbData *data = (cbData*) d;
	gdMidiGrabberChannel *grabber = (gdMidiGrabberChannel*) data->grabber;
	gLearner             *learner = data->learner;
	grabber->__cb_learnMute(msg, learner);
	free(data);
}

void gdMidiGrabberChannel::__cb_learnMute(uint32_t msg, gLearner *l) {
	ch->midiInMute = msg;
	stopMidiLearn(l);
	printf("[gdMidiGrabberChannel] MIDI learn mute - done with message=0x%X\n", msg);
}


/* ------------------------------------------------------------------ */


void gdMidiGrabberChannel::cb_learnKill(uint32_t msg, void *d) {
	cbData *data = (cbData*) d;
	gdMidiGrabberChannel *grabber = (gdMidiGrabberChannel*) data->grabber;
	gLearner             *learner = data->learner;
	grabber->__cb_learnKill(msg, learner);
	free(data);
}

void gdMidiGrabberChannel::__cb_learnKill(uint32_t msg, gLearner *l) {
	ch->midiInKill = msg;
	stopMidiLearn(l);
	printf("[gdMidiGrabberChannel] MIDI learn kill - done with message=0x%X\n", msg);
}


/* ------------------------------------------------------------------ */


void gdMidiGrabberChannel::cb_learnSolo(uint32_t msg, void *d) {
	cbData *data = (cbData*) d;
	gdMidiGrabberChannel *grabber = (gdMidiGrabberChannel*) data->grabber;
	gLearner             *learner = data->learner;
	grabber->__cb_learnSolo(msg, learner);
	free(data);
}

void gdMidiGrabberChannel::__cb_learnSolo(uint32_t msg, gLearner *l) {
	ch->midiInSolo = msg;
	stopMidiLearn(l);
	printf("[gdMidiGrabberChannel] MIDI learn solo - done with message=0x%X\n", msg);
}


/* ------------------------------------------------------------------ */


void gdMidiGrabberChannel::cb_learnVolume(uint32_t msg, void *d) {
	cbData *data = (cbData*) d;
	gdMidiGrabberChannel *grabber = (gdMidiGrabberChannel*) data->grabber;
	gLearner             *learner = data->learner;
	grabber->__cb_learnVolume(msg, learner);
	free(data);
}

void gdMidiGrabberChannel::__cb_learnVolume(uint32_t msg, gLearner *l) {
	ch->midiInVolume = msg;
	stopMidiLearn(l);
	printf("[gdMidiGrabberChannel] MIDI learn volume - done with message=0x%X\n", msg);
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gdMidiGrabberMaster::gdMidiGrabberMaster()
	: gdMidiGrabber()
{
	set_modal();

	//new gLearner(8,  30, w()-16, "master rewind", );
	//new gLearner(8,  54, w()-16, "master play/stop", );

	gu_setFavicon(this);
	show();
}


/* ------------------------------------------------------------------ */


void gdMidiGrabberMaster::cb_learnRewind(uint32_t msg, void *d) {
	cbData *data = (cbData*) d;
	gdMidiGrabberMaster *grabber = (gdMidiGrabberMaster*) data->grabber;
	gLearner            *learner = data->learner;
	grabber->__cb_learnRewind(msg, learner);
	free(data);
}

void gdMidiGrabberMaster::__cb_learnRewind(uint32_t msg, gLearner *l) {
	//ch->midiInKill = msg;
	stopMidiLearn(l);
	printf("[gdMidiGrabberMaster] MIDI learn rewind - done with message=0x%X\n", msg);
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gLearner::gLearner(int X, int Y, int W, const char *l, kernelMidi::cb_midiLearn *cb, uint32_t *midiValue)
	: Fl_Group (X, Y, W, 20),
		midiValue(midiValue),
		callback (cb)
{
	begin();
	text   = new gBox(x(), y(), 156, 20, l);
	value  = new gBox(text->x()+text->w()+4, y(), 80, 20, "(not set)");
	button = new gButton(value->x()+value->w()+4, y(), 40, 20, "learn");
	end();

	text->box(G_BOX);
	text->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

	value->box(G_BOX);
	updateValue();

	button->type(FL_TOGGLE_BUTTON);
	button->callback(cb_button, (void*)this);
}


/* ------------------------------------------------------------------ */


void gLearner::updateValue() {
	if (*midiValue != 0) {
		char buf[9];
		snprintf(buf, 9, "0x%X", *midiValue);
		value->copy_label(buf);
		button->value(0);
	}
}


/* ------------------------------------------------------------------ */


void gLearner::cb_button(Fl_Widget *v, void *p) { ((gLearner*)p)->__cb_button(); }


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

