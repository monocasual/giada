/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_midiOutput
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


#include "gd_midiOutput.h"
#include "ge_mixed.h"
#include "ge_channel.h"
#include "ge_learner.h"
#include "gg_keyboard.h"
#include "channel.h"
#include "sampleChannel.h"
#include "conf.h"
#include "midiChannel.h"
#include "gui_utils.h"


extern Conf	G_Conf;


gdMidiOutput::gdMidiOutput()
	: gWindow(300, 64, "Midi Output Setup")
{
}


/* -------------------------------------------------------------------------- */


void gdMidiOutput::stopMidiLearn(gLearner *learner) {
	kernelMidi::stopMidiLearn();
	learner->updateValue();
}


/* -------------------------------------------------------------------------- */


void gdMidiOutput::__cb_learn(uint32_t *param, uint32_t msg, gLearner *l) {
	*param = msg;
	stopMidiLearn(l);
	gLog("[gdMidiGrabber] MIDI learn done - message=0x%X\n", msg);
}


/* -------------------------------------------------------------------------- */


void gdMidiOutput::cb_learn(uint32_t msg, void *d) {
	cbData *data = (cbData*) d;
	gdMidiOutput  *grabber = (gdMidiOutput*) data->grabber;
	gLearner      *learner = data->learner;
	uint32_t      *param   = learner->param;
	grabber->__cb_learn(param, msg, learner);
	free(data);
}


/* -------------------------------------------------------------------------- */


void gdMidiOutput::cb_close(Fl_Widget *w, void *p)  { ((gdMidiOutput*)p)->__cb_close(); }


/* -------------------------------------------------------------------------- */


void gdMidiOutput::__cb_close() {
	do_callback();
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


gdMidiOutputMidiCh::gdMidiOutputMidiCh(MidiChannel *ch)
	: gdMidiOutput(), ch(ch)
{
	begin();
	enableOut   = new gCheck(x()+8, y()+8, 150, 20, "Enable MIDI output");
	chanListOut = new gChoice(w()-108, y()+8, 100, 20);

	save   = new gButton(w()-88, chanListOut->y()+chanListOut->h()+8, 80, 20, "Save");
	cancel = new gButton(w()-88-save->w()-8, save->y(), 80, 20, "Cancel");
	end();

	fillChanMenu(chanListOut);

	if (ch->midiOut)
		enableOut->value(1);
	else
		chanListOut->deactivate();

	chanListOut->value(ch->midiOutChan);

	enableOut->callback(cb_enableChanList, (void*)this);
	save->callback(cb_save, (void*)this);
	cancel->callback(cb_cancel, (void*)this);

	set_modal();
	gu_setFavicon(this);
	show();
}


/* -------------------------------------------------------------------------- */


void gdMidiOutputMidiCh::cb_save          (Fl_Widget *w, void *p) { ((gdMidiOutputMidiCh*)p)->__cb_save(); }
void gdMidiOutputMidiCh::cb_cancel        (Fl_Widget *w, void *p) { ((gdMidiOutputMidiCh*)p)->__cb_cancel(); }
void gdMidiOutputMidiCh::cb_enableChanList(Fl_Widget *w, void *p) { ((gdMidiOutputMidiCh*)p)->__cb_enableChanList(); }


/* -------------------------------------------------------------------------- */


void gdMidiOutputMidiCh::__cb_enableChanList() {
	enableOut->value() ? chanListOut->activate() : chanListOut->deactivate();
}


/* -------------------------------------------------------------------------- */


void gdMidiOutputMidiCh::__cb_save() {
	ch->midiOut     = enableOut->value();
	ch->midiOutChan = chanListOut->value();
	ch->guiChannel->update();
	do_callback();
}


/* -------------------------------------------------------------------------- */


void gdMidiOutputMidiCh::__cb_cancel() { do_callback(); }


/* -------------------------------------------------------------------------- */


void gdMidiOutputMidiCh::fillChanMenu(gChoice *m) {
	m->add("Channel 1");
	m->add("Channel 2");
	m->add("Channel 3");
	m->add("Channel 4");
	m->add("Channel 5");
	m->add("Channel 6");
	m->add("Channel 7");
	m->add("Channel 8");
	m->add("Channel 9");
	m->add("Channel 10");
	m->add("Channel 11");
	m->add("Channel 12");
	m->add("Channel 13");
	m->add("Channel 14");
	m->add("Channel 15");
	m->add("Channel 16");
	m->value(0);
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


gdMidiOutputSampleCh::gdMidiOutputSampleCh(SampleChannel *ch)
	: gdMidiOutput(), ch(ch)
{
	size(300, 134);

	//char title[64];
	//sprintf(title, "MIDI Output Setup (channel %d)", ch->index+1);
	//label(title);

	set_modal();

	enableLightning = new gCheck(8, 8, 120, 20, "enable MIDI lightning output");
	new gLearner(8,  30, w()-16, "playing", cb_learn, &ch->midiOutPlaying);
	new gLearner(8,  54, w()-16, "mute",    cb_learn, &ch->midiOutMute);
	new gLearner(8,  78, w()-16, "solo",    cb_learn, &ch->midiOutSolo);
	int yy = 102;


	ok = new gButton(w()-88, yy, 80, 20, "Ok");
	ok->callback(cb_close, (void*)this);

	enableLightning->value(ch->midiOut);
	enableLightning->callback(cb_enableLightning, (void*)this);

	set_modal();
	gu_setFavicon(this);
	show();
}

/* -------------------------------------------------------------------------- */


void gdMidiOutputSampleCh::cb_enableLightning(Fl_Widget *w, void *p)  {
	((gdMidiOutputSampleCh*)p)->__cb_enableLightning();
}


/* -------------------------------------------------------------------------- */


void gdMidiOutputSampleCh::__cb_enableLightning() {
	// TODO
	//ch->midiOut = enableLightning->value();
}
