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
#include "conf.h"


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
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gdMidiGrabberChannel::gdMidiGrabberChannel(Channel *ch)
	:	gdMidiGrabber(300, 178, "MIDI Input Setup"),
		ch(ch)
{
	char title[64];
	sprintf(title, "MIDI Input Setup (channel %d)", ch->index);
	label(title);

	set_modal();

	enable = new gCheck(8, 8, 120, 20, "enable MIDI input");
	new gLearner(8,  30, w()-16, "key press", cb_learnKeyPress, &ch->midiInKeyPress);
	new gLearner(8,  54, w()-16, "key release", cb_learnKeyRelease, &ch->midiInKeyRel);
	new gLearner(8,  78, w()-16, "key kill", cb_learnKill, &ch->midiInKill);
	new gLearner(8, 102, w()-16, "mute", cb_learnMute, &ch->midiInMute);
	new gLearner(8, 126, w()-16, "solo", cb_learnSolo, &ch->midiInSolo);
	new gLearner(8, 150, w()-16, "volume", cb_learnVolume, &ch->midiInVolume);

	enable->value(ch->midiIn);
	enable->callback(cb_enable, (void*)this);

	gu_setFavicon(this);
	show();
}


/* ------------------------------------------------------------------ */


void gdMidiGrabberChannel::cb_enable(Fl_Widget *w, void *p)  { ((gdMidiGrabberChannel*)p)->__cb_enable(); }


/* ------------------------------------------------------------------ */


void gdMidiGrabberChannel::__cb_enable() {
	ch->midiIn = enable->value();
}


/* ------------------------------------------------------------------ */


/* all callbacks are ultra-condensed. Real code:
 *
 * cbData *data = (cbData*) d;
 * gdMidiGrabberChannel *grabber = (gdMidiGrabberChannel*) data->grabber;
 * gLearner             *learner = data->learner;
 * grabber->__cb_learn[something](msg, learner);
 * free(data); */

void gdMidiGrabberChannel::cb_learnKeyPress(uint32_t msg, void *d) {
	cbData *data = (cbData*) d;
	((gdMidiGrabberChannel*)data->grabber)->__cb_learnKeyPress(msg, (gLearner *)data->learner);
	free(data);
}

void gdMidiGrabberChannel::__cb_learnKeyPress(uint32_t msg, gLearner *l) {
	ch->midiInKeyPress = msg;
	stopMidiLearn(l);
	printf("[gdMidiGrabberChannel] MIDI learn keypress - done with message=0x%X\n", msg);
}


/* ------------------------------------------------------------------ */


void gdMidiGrabberChannel::cb_learnKeyRelease(uint32_t msg, void *d) {
	cbData *data = (cbData*) d;
	((gdMidiGrabberChannel*)data->grabber)->__cb_learnKeyRelease(msg, (gLearner *)data->learner);
	free(data);
}

void gdMidiGrabberChannel::__cb_learnKeyRelease(uint32_t msg, gLearner *l) {
	ch->midiInKeyRel = msg;
	stopMidiLearn(l);
	printf("[gdMidiGrabberChannel] MIDI learn keyrelease - done with message=0x%X\n", msg);
}


/* ------------------------------------------------------------------ */


void gdMidiGrabberChannel::cb_learnMute(uint32_t msg, void *d) {
	cbData *data = (cbData*) d;
	((gdMidiGrabberChannel*)data->grabber)->__cb_learnMute(msg, (gLearner *)data->learner);
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
	((gdMidiGrabberChannel*)data->grabber)->__cb_learnKill(msg, (gLearner *)data->learner);
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
	((gdMidiGrabberChannel*)data->grabber)->__cb_learnSolo(msg, (gLearner *)data->learner);
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
	((gdMidiGrabberChannel*)data->grabber)->__cb_learnVolume(msg, (gLearner *)data->learner);
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
	: gdMidiGrabber(300, 200, "MIDI Input Setup (global)")
{
	set_modal();

	new gLearner(8,  30, w()-16, "rewind",    &cb_learnRewind,    &G_Conf.midiInRewind);
	new gLearner(8,  54, w()-16, "play/stop", &cb_learnStartStop, &G_Conf.midiInStartStop);
	new gLearner(8,  78, w()-16, "action recording", &cb_learnActionRec, &G_Conf.midiInActionRec);
	new gLearner(8, 102, w()-16, "input recording",  &cb_learnInputRec, &G_Conf.midiInInputRec);
	new gLearner(8, 126, w()-16, "metronome",  &cb_learnMetronome, &G_Conf.midiInMetronome);
	new gLearner(8, 150, w()-16, "input volume",  &cb_learnVolumeIn, &G_Conf.midiInVolumeIn);
	new gLearner(8, 174, w()-16, "output volume",  &cb_learnVolumeOut, &G_Conf.midiInVolumeOut);

	gu_setFavicon(this);
	show();
}


/* ------------------------------------------------------------------ */


/* all callbacks are ultra-condensed. Real code:
 *
 * cbData *data = (cbData*) d;
 * gdMidiGrabberMaster *grabber = (gdMidiGrabberMaster*) data->grabber;
 * gLearner            *learner = data->learner;
 * grabber->__cb_learn(param, msg, learner);
 * free(data); */


void gdMidiGrabberMaster::cb_learnRewind(uint32_t msg, void *d) {
	cbData *data = (cbData*) d;
	((gdMidiGrabberMaster*)data->grabber)->__cb_learn(&G_Conf.midiInRewind, msg, (gLearner*)data->learner);
	free(data);
}


void gdMidiGrabberMaster::cb_learnStartStop(uint32_t msg, void *d) {
	cbData *data = (cbData*) d;
	((gdMidiGrabberMaster*)data->grabber)->__cb_learn(&G_Conf.midiInStartStop, msg, (gLearner*)data->learner);
	free(data);
}


void gdMidiGrabberMaster::cb_learnActionRec(uint32_t msg, void *d) {
	cbData *data = (cbData*) d;
	((gdMidiGrabberMaster*)data->grabber)->__cb_learn(&G_Conf.midiInActionRec, msg, (gLearner*)data->learner);
	free(data);
}


void gdMidiGrabberMaster::cb_learnInputRec(uint32_t msg, void *d) {
	cbData *data = (cbData*) d;
	((gdMidiGrabberMaster*)data->grabber)->__cb_learn(&G_Conf.midiInInputRec, msg, (gLearner*)data->learner);
	free(data);
}


void gdMidiGrabberMaster::cb_learnMetronome(uint32_t msg, void *d) {
	cbData *data = (cbData*) d;
	((gdMidiGrabberMaster*)data->grabber)->__cb_learn(&G_Conf.midiInMetronome, msg, (gLearner*)data->learner);
	free(data);
}


void gdMidiGrabberMaster::cb_learnVolumeIn(uint32_t msg, void *d) {
	cbData *data = (cbData*) d;
	((gdMidiGrabberMaster*)data->grabber)->__cb_learn(&G_Conf.midiInVolumeIn, msg, (gLearner*)data->learner);
	free(data);
}


void gdMidiGrabberMaster::cb_learnVolumeOut(uint32_t msg, void *d) {
	cbData *data = (cbData*) d;
	((gdMidiGrabberMaster*)data->grabber)->__cb_learn(&G_Conf.midiInVolumeOut, msg, (gLearner*)data->learner);
	free(data);
}


/* ------------------------------------------------------------------ */


void gdMidiGrabberMaster::__cb_learn(uint32_t *param, uint32_t msg, gLearner *l) {
	*param = msg;
	stopMidiLearn(l);
	printf("[gdMidiGrabberMaster] MIDI learn done - message=0x%X\n", msg);
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
	value->callback(cb_value, (void*)this);
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
void gLearner::cb_value(Fl_Widget *v, void *p) { ((gLearner*)p)->__cb_value(); }


/* ------------------------------------------------------------------ */


void gLearner::__cb_value() {
	printf("click\n");
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

