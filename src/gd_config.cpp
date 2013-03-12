/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_config
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


#include "gd_config.h"
#include "gd_keyGrabber.h"
#include "gd_devInfo.h"
#include "ge_mixed.h"
#include "conf.h"
#include "gui_utils.h"
#include "patch.h"
#include "kernelAudio.h"


extern Patch G_Patch;
extern Conf	 G_Conf;
extern bool  G_audio_status;


gdConfig::gdConfig(int w, int h)
: gWindow(w, h, "Configuration") {
	set_modal();

	if (G_Conf.configX)
		resize(G_Conf.configX, G_Conf.configY, this->w(), this->h());

	Fl_Tabs *tabs = new Fl_Tabs(8, 8, w-16, h-44);

		Fl_Group *grpSound = new Fl_Group(tabs->x()+10, tabs->y()+20, tabs->w()-20, tabs->h()-40, "Sound System");

			soundsys    = new gChoice(grpSound->x()+92,  grpSound->y()+9,  253, 20, "System");
			buffersize  = new gInput (grpSound->x()+92,  grpSound->y()+37, 55,  20, "Buffer size");
			samplerate  = new gChoice(grpSound->x()+290, grpSound->y()+37, 55,  20, "Sample rate");
			sounddevOut = new gChoice(grpSound->x()+92,  grpSound->y()+65, 225, 20, "Output device");
			devOutInfo  = new gClick (grpSound->x()+325, grpSound->y()+65, 20,  20, "?");
			channelsOut = new gChoice(grpSound->x()+92,  grpSound->y()+93, 55,  20, "Output channels");
			limitOutput = new gCheck (grpSound->x()+155, grpSound->y()+97, 55,  20, "Limit output");

			sounddevIn  = new gChoice(grpSound->x()+92,  grpSound->y()+121, 225, 20, "Input device");
			devInInfo   = new gClick (grpSound->x()+325, grpSound->y()+121, 20,  20, "?");
			channelsIn  = new gChoice(grpSound->x()+92,  grpSound->y()+149, 55,  20, "Input channels");
		  delayComp   = new gInput (grpSound->x()+290, grpSound->y()+149, 55,  20, "Rec delay comp.");

		  rsmpQuality = new gChoice(grpSound->x()+92, grpSound->y()+177, 253, 20, "Resampling");

			new gBox(grpSound->x(), grpSound->y()+220, grpSound->w(), 50, "Restart Giada for the changes to take effect.");

		grpSound->end();
		grpSound->labelsize(11);

		/*
		Fl_Group *grpKeys = new Fl_Group(tabs->x()+10, tabs->y()+20, tabs->w()-20, tabs->h()-40, "Controls");
			listChans	= new gChoice(grpKeys->x()+92,  grpKeys->y()+9, 253, 20, "Key for");
			            new gBox   (grpKeys->x()+22,  grpKeys->y()+37, 70, 20, "Actual key", FL_ALIGN_RIGHT);
			actualKey	= new gBox	 (grpKeys->x()+92,  grpKeys->y()+37, 25, 20);
			setKey	  = new gClick (grpKeys->x()+270, grpKeys->y()+37, 75, 20, "Set new");
			            new gBox(grpKeys->x()+10, setKey->y()+120, grpKeys->w()-10, 50, "Restart Giada for the changes to take effect.");
		grpKeys->end();
		grpKeys->labelsize(11);
		*/

		Fl_Group *grpBehvs = new Fl_Group(tabs->x()+10, tabs->y()+20, tabs->w()-20, tabs->h()-40, "Behaviors");
			Fl_Group *radioGrp_1 = new Fl_Group(grpBehvs->x(), grpBehvs->y()+10, grpBehvs->w(), 70); // radio group for the mutex
				                       new gBox(grpBehvs->x(), grpBehvs->y()+10, 70, 25, "When a channel with recorded actions is halted:", FL_ALIGN_LEFT);
				recsStopOnChanHalt_1 = new gRadio(grpBehvs->x()+25, grpBehvs->y()+35, 280, 20, "stop it immediately");
				recsStopOnChanHalt_0 = new gRadio(grpBehvs->x()+25, grpBehvs->y()+55, 280, 20, "play it until finished");
			radioGrp_1->end();

			Fl_Group *radioGrp_2 = new Fl_Group(grpBehvs->x(), grpBehvs->y()+70, grpBehvs->w(), 70); // radio group for the mutex
			                         new gBox(grpBehvs->x(), grpBehvs->y()+80, 70, 25, "When the sequencer is halted:", FL_ALIGN_LEFT);
				chansStopOnSeqHalt_1 = new gRadio(grpBehvs->x()+25, grpBehvs->y()+105, 280, 20, "stop immediately all dynamic channels");
				chansStopOnSeqHalt_0 = new gRadio(grpBehvs->x()+25, grpBehvs->y()+125, 280, 20, "play all dynamic channels until finished");
			radioGrp_2->end();

			treatRecsAsLoops  = new gCheck(tabs->x()+12, grpBehvs->y()+155, 280, 20, "Treat recorded channels as loops");
			fullChanVolOnLoad = new gCheck(tabs->x()+12, grpBehvs->y()+185, 280, 20, "Bring channels to full volume on sample load");

		grpBehvs->end();
		grpBehvs->labelsize(11);

	tabs->end();
	tabs->box(G_BOX);
	tabs->labelcolor(COLOR_TEXT_0);

	save 	 = new gClick (w-88, h-28, 80, 20, "Save");
	cancel = new gClick (w-176, h-28, 80, 20, "Cancel");

	end();

#if defined(__linux__)
	soundsys->add("ALSA");
	soundsys->add("JACK");
	soundsys->add("PULSE");

	switch (G_Conf.soundSystem) {
		case SYS_API_ALSA:
			soundsys->value(0);
			break;
		case SYS_API_JACK:
			soundsys->value(1);
			buffersize->deactivate();
			samplerate->deactivate();
			break;
		case SYS_API_PULSE:
			soundsys->value(2);
			break;
	}
	soundsysInitValue = soundsys->value();
#elif defined(_WIN32)
	soundsys->add("DirectSound");
	soundsys->add("ASIO");
	soundsys->value(G_Conf.soundSystem == SYS_API_DS ? 0 : 1);
	soundsysInitValue = soundsys->value();
#elif defined (__APPLE__)
	soundsys->add("CoreAudio");
	soundsys->value(0);
	soundsysInitValue = soundsys->value();
#endif

	sounddevIn->callback(cb_fetchInChans, this);
	sounddevOut->callback(cb_fetchOutChans, this);

	devOutInfo->callback(cb_showOutputInfo, this);
	devInInfo->callback(cb_showInputInfo, this);

	fetchSoundDevs();

	fetchOutChans(sounddevOut->value());
	fetchInChans(sounddevIn->value());

	char buf[8];
	sprintf(buf, "%d", G_Conf.buffersize);
	buffersize->value(buf);
	buffersize->type(FL_INT_INPUT);
	buffersize->maximum_size(5);

	/* fill frequency dropdown menu */

	int nfreq = kernelAudio::getTotalFreqs(sounddevOut->value());
	for (int i=0; i<nfreq; i++) {
		char buf[16];
		int  freq = kernelAudio::getFreq(sounddevOut->value(), i);
		sprintf(buf, "%d", freq);
		samplerate->add(buf);
		if (freq == G_Conf.samplerate)
			samplerate->value(i);
	}

	rsmpQuality->add("Sinc best quality (very slow)");
	rsmpQuality->add("Sinc medium quality (slow)");
	rsmpQuality->add("Sinc basic quality (medium)");
	rsmpQuality->add("Zero Order Hold (fast)");
	rsmpQuality->add("Linear (very fast)");
	rsmpQuality->value(G_Conf.rsmpQuality);

	buf[0] = '\0';
	sprintf(buf, "%d", G_Conf.delayComp);
	delayComp->value(buf);
	delayComp->type(FL_INT_INPUT);
	delayComp->maximum_size(5);

	limitOutput->value(G_Conf.limitOutput);

	/**
	sprintf(buf, "%c", G_Conf.keys[0]);
	actualKey->copy_label(buf);
	actualKey->box(G_BOX);
	actualKey->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	*/

	G_Conf.recsStopOnChanHalt == 1 ? recsStopOnChanHalt_1->value(1) : recsStopOnChanHalt_0->value(1);
	G_Conf.chansStopOnSeqHalt == 1 ? chansStopOnSeqHalt_1->value(1) : chansStopOnSeqHalt_0->value(1);
	G_Conf.treatRecsAsLoops   == 1 ? treatRecsAsLoops->value(1)  : treatRecsAsLoops->value(0);
	G_Conf.fullChanVolOnLoad  == 1 ? fullChanVolOnLoad->value(1) : fullChanVolOnLoad->value(0);

	/**
	for (int i=0; i<MAX_NUM_CHAN; i++) {
		char x[11];
		sprintf(x, "channel %d", i+1);
		listChans->add(x, 0, cb_get_key_chan, (void*)this);
	}
	listChans->value(0); // starts with element number 0

	setKey->callback(cb_open_grab_win, (void*)this);
	*/
	save->callback(cb_save_config, (void*)this);
	cancel->callback(cb_cancel, (void*)this);
	soundsys->callback(cb_deactivate_sounddev, (void*)this);
	recsStopOnChanHalt_1->callback(cb_radio_mutex, (void*)this);
	recsStopOnChanHalt_0->callback(cb_radio_mutex, (void*)this);
	chansStopOnSeqHalt_1->callback(cb_radio_mutex, (void*)this);
	chansStopOnSeqHalt_0->callback(cb_radio_mutex, (void*)this);

	gu_setFavicon(this);
	setId(WID_CONFIG);
	show();
}


/* ------------------------------------------------------------------ */


gdConfig::~gdConfig() {
	G_Conf.configX = x();
	G_Conf.configY = y();
}


/* ------------------------------------------------------------------ */


void gdConfig::cb_get_key_chan(Fl_Widget *w, void *p)        { ((gdConfig*)p)->__cb_get_key_chan();  }
void gdConfig::cb_open_grab_win(Fl_Widget *w, void *p)       { ((gdConfig*)p)->__cb_open_grab_win(); }
void gdConfig::cb_save_config(Fl_Widget *w, void *p)         { ((gdConfig*)p)->__cb_save_config();   }
void gdConfig::cb_deactivate_sounddev(Fl_Widget *w, void *p) { ((gdConfig*)p)->__cb_deactivate_sounddev(); }
void gdConfig::cb_fetchInChans(Fl_Widget *w, void *p)        { ((gdConfig*)p)->__cb_fetchInChans(); }
void gdConfig::cb_fetchOutChans(Fl_Widget *w, void *p)       { ((gdConfig*)p)->__cb_fetchOutChans(); }
void gdConfig::cb_radio_mutex(Fl_Widget *w, void *p)         { ((gdConfig*)p)->__cb_radio_mutex(w); }
void gdConfig::cb_showInputInfo(Fl_Widget *w, void *p)       { ((gdConfig*)p)->__cb_showInputInfo(); }
void gdConfig::cb_showOutputInfo(Fl_Widget *w, void *p)      { ((gdConfig*)p)->__cb_showOutputInfo(); }
void gdConfig::cb_cancel        (Fl_Widget *w, void *p)      { ((gdConfig*)p)->__cb_cancel(); }


/* ------------------------------------------------------------------ */


void gdConfig::__cb_get_key_chan() {
	/**
	char key[2];
	sprintf(key, "%c", G_Conf.keys[listChans->value()]);
	actualKey->copy_label(key);
	**/
}


/* ------------------------------------------------------------------ */


void gdConfig::__cb_open_grab_win() {
	//new gdKeyGrabber(listChans->value(), this);
}


/* ------------------------------------------------------------------ */


void gdConfig::__cb_showInputInfo() {
	unsigned dev = kernelAudio::getDeviceByName(sounddevIn->text(sounddevIn->value()));
	new gdDevInfo(dev);
}


/* ------------------------------------------------------------------ */


void gdConfig::__cb_showOutputInfo() {
	unsigned dev = kernelAudio::getDeviceByName(sounddevOut->text(sounddevOut->value()));
	new gdDevInfo(dev);
}


/* ------------------------------------------------------------------ */


void gdConfig::__cb_save_config() {

	/* no saved values here, they are stored in G_Conf and saved
	 * when closing Giada. */

#ifdef __linux__
	if      (soundsys->value() == 0)	G_Conf.soundSystem = SYS_API_ALSA;
	else if (soundsys->value() == 1)	G_Conf.soundSystem = SYS_API_JACK;
	else if (soundsys->value() == 2)	G_Conf.soundSystem = SYS_API_PULSE;
#else
#ifdef _WIN32
	if 			(soundsys->value() == 0)	G_Conf.soundSystem = SYS_API_DS;
	else if (soundsys->value() == 1)  G_Conf.soundSystem = SYS_API_ASIO;
#endif
#endif

	/* use the device name to search into the drop down menu's */

	G_Conf.soundDeviceOut = kernelAudio::getDeviceByName(sounddevOut->text(sounddevOut->value()));
	G_Conf.soundDeviceIn  = kernelAudio::getDeviceByName(sounddevIn->text(sounddevIn->value()));
	G_Conf.channelsOut    = channelsOut->value();
	G_Conf.channelsIn     = channelsIn->value();
	G_Conf.limitOutput    = limitOutput->value();
	G_Conf.rsmpQuality    = rsmpQuality->value();

	int bufsize = atoi(buffersize->value());
	if (bufsize % 2 != 0) bufsize++;
	if (bufsize < 8)		  bufsize = 8;
	if (bufsize > 8192)		bufsize = 8192;
	G_Conf.buffersize = bufsize;

	const Fl_Menu_Item *i = NULL;
	i = samplerate->mvalue(); // mvalue() returns a pointer to the last menu item that was picked
	if (i)
		G_Conf.samplerate = atoi(i->label());

	int _delayComp = atoi(delayComp->value());
	if (_delayComp < 0) _delayComp = 0;
	G_Conf.delayComp = _delayComp;

	G_Conf.recsStopOnChanHalt = recsStopOnChanHalt_1->value() == 1 ? 1 : 0;
	G_Conf.chansStopOnSeqHalt = chansStopOnSeqHalt_1->value() == 1 ? 1 : 0;
	G_Conf.treatRecsAsLoops   = treatRecsAsLoops->value() == 1 ? 1 : 0;
	G_Conf.fullChanVolOnLoad  = fullChanVolOnLoad->value() == 1 ? 1 : 0;

	do_callback();
}


/* ------------------------------------------------------------------ */


void gdConfig::__cb_deactivate_sounddev() {

	/* if the user changes sound system (eg ALSA->JACK) device menu deactivates.
	 * If it returns to the original sound system, we re-fill the list by
	 * querying kernelAudio. */

	if (soundsysInitValue == soundsys->value()) {
		sounddevOut->clear();
		sounddevIn->clear();

		fetchSoundDevs();

		/* the '?' button is added by fetchSoundDevs */

		fetchOutChans(sounddevOut->value());
		sounddevOut->activate();
		channelsOut->activate();

		/* chan menus and '?' button are activated by fetchInChans(...) */

		fetchInChans(sounddevIn->value());
		sounddevIn->activate();
	}
	else {
		sounddevOut->deactivate();
		sounddevOut->clear();
		sounddevOut->add("-- restart to fetch device(s) --");
		sounddevOut->value(0);
		channelsOut->deactivate();
		devOutInfo->deactivate();

		sounddevIn->deactivate();
		sounddevIn->clear();
		sounddevIn->add("-- restart to fetch device(s) --");
		sounddevIn->value(0);
		channelsIn->deactivate();
		devInInfo->deactivate();
	}
}


/* ------------------------------------------------------------------ */


void gdConfig::__cb_radio_mutex(Fl_Widget *w) {
	((Fl_Button *)w)->type(FL_RADIO_BUTTON);
}


/* ------------------------------------------------------------------ */


void gdConfig::__cb_fetchInChans() {
	fetchInChans(sounddevIn->value());
	channelsIn->value(0);
}


/* ------------------------------------------------------------------ */


void gdConfig::__cb_fetchOutChans() {
	fetchOutChans(sounddevOut->value());
	channelsOut->value(0);
}


/* ------------------------------------------------------------------ */


void gdConfig::fetchSoundDevs() {

	if (kernelAudio::numDevs == 0) {
		sounddevOut->add("-- no devices found --");
		sounddevOut->value(0);
		sounddevIn->add("-- no devices found --");
		sounddevIn->value(0);
		devInInfo ->deactivate();
		devOutInfo->deactivate();
	}
	else {

		devInInfo ->activate();
		devOutInfo->activate();

		/* input device may be disabled: now device number -1 is the disabled
		 * one. KernelAudio knows how to handle it. */

		sounddevIn->add("(disabled)");

		for (unsigned i=0; i<kernelAudio::numDevs; i++) {


			/* escaping '/', very dangerous in FLTK (it creates a submenu) */
			/** FIXME: escape also & and _*/

			std::string tmp = kernelAudio::getDeviceName(i);
			for (unsigned k=0; k<tmp.size(); k++)
				if (tmp[k] == '/')
					tmp[k] = '-';

			/* add to list devices with at least 1 channel available. In this
			 * way we can filter devices only for input or output, e.g. an input
			 * devices has 0 output channels. */

			if (kernelAudio::getMaxOutChans(i) > 0)
				sounddevOut->add(tmp.c_str());

			if (kernelAudio::getMaxInChans(i) > 0)
				sounddevIn->add(tmp.c_str());
		}

		/* we show the device saved in the configuration file. */

		if (sounddevOut->size() == 0) {
			sounddevOut->add("-- no devices found --");
			sounddevOut->value(0);
			devOutInfo->deactivate();
		}
		else {
			int outMenuValue = findMenuDevice(sounddevOut, G_Conf.soundDeviceOut);
			sounddevOut->value(outMenuValue);
		}

		if (sounddevIn->size() == 0) {
			sounddevIn->add("-- no devices found --");
			sounddevIn->value(0);
			devInInfo->deactivate();
		}
		else {
			int inMenuValue = findMenuDevice(sounddevIn, G_Conf.soundDeviceIn);
			sounddevIn->value(inMenuValue);
		}
	}
}


/* ------------------------------------------------------------------ */


void gdConfig::fetchInChans(int menuItem) {

	/* if menuItem==0 device in input is disabled. */

	if (menuItem == 0) {
		devInInfo ->deactivate();
		channelsIn->deactivate();
		delayComp ->deactivate();
		return;
	}

	devInInfo ->activate();
	channelsIn->activate();
	delayComp ->activate();

	channelsIn->clear();

	unsigned dev = kernelAudio::getDeviceByName(sounddevIn->text(sounddevIn->value()));
	unsigned chs = kernelAudio::getMaxInChans(dev);

	if (chs == 0) {
		channelsIn->add("none");
		channelsIn->value(0);
		return;
	}
	for (unsigned i=0; i<chs; i+=2) {
		char str[16];
		sprintf(str, "%d-%d", (i+1), (i+2));
		channelsIn->add(str);
	}
	channelsIn->value(G_Conf.channelsIn);
}


/* ------------------------------------------------------------------ */


void gdConfig::fetchOutChans(int menuItem) {

	channelsOut->clear();

	unsigned dev = kernelAudio::getDeviceByName(sounddevOut->text(sounddevOut->value()));
	unsigned chs = kernelAudio::getMaxOutChans(dev);

	if (chs == 0) {
		channelsOut->add("none");
		channelsOut->value(0);
		return;
	}
	for (unsigned i=0; i<chs; i+=2) {
		char str[16];
		sprintf(str, "%d-%d", (i+1), (i+2));
		channelsOut->add(str);
	}
	channelsOut->value(G_Conf.channelsOut);
}


/* ------------------------------------------------------------------ */


int gdConfig::findMenuDevice(gChoice *m, int device) {

	if (device == -1)
		return 0;

	if (G_audio_status == false)
		return 0;

	for (int i=0; i<m->size(); i++) {
		if (kernelAudio::getDeviceName(device) == NULL)
			continue;
		if (m->text(i) == NULL)
			continue;
		if (strcmp(m->text(i), kernelAudio::getDeviceName(device))==0)
			return i;
	}

	return 0;
}


/* ------------------------------------------------------------------ */


void gdConfig::__cb_cancel() {
	do_callback();
}
