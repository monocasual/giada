/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_about
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


#include "gd_about.h"
#include "conf.h"
#include "kernelAudio.h"
#include "ge_mixed.h"
#include "graphics.h"
#include "gui_utils.h"


extern Conf G_Conf;


gdAbout::gdAbout()
#ifdef WITH_VST
: gWindow(340, 405, "About Giada") {
#else
: gWindow(340, 320, "About Giada") {
#endif

	if (G_Conf.aboutX)
		resize(G_Conf.aboutX, G_Conf.aboutY, w(), h());

	set_modal();

	logo  = new gBox(8, 10, 324, 86);
	text  = new gBox(8, 120, 324, 145);
	close = new gClick(252, h()-28, 80, 20, "Close");
#ifdef WITH_VST
	vstLogo = new gBox(8, 265, 324, 50);
	vstText = new gBox(8, 315, 324, 46);
#endif
	end();

	logo->image(new Fl_Pixmap(giada_logo_xpm));
	text->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_TOP);

	char message[512];
	sprintf(
	  message,
	  "Version " VERSIONE " (" __DATE__ ")\n\n"
		"Developed by Monocasual\n"
		"Based on FLTK (%d.%d.%d), RtAudio (%s),\n"
		"RtMidi, libsamplerate and libsndfile\n\n"
		"Released under the terms of the GNU General\n"
		"Public License (GPL v3)\n\n"
		"News, infos, contacts and documentation:\n"
		"www.giadamusic.com",
		FL_MAJOR_VERSION, FL_MINOR_VERSION, FL_PATCH_VERSION, kernelAudio::getRtAudioVersion().c_str());

	int tw = 0;
	int th = 0;
	fl_measure(message, tw, th);
	text->copy_label(message);
	text->size(text->w(), th);

#ifdef WITH_VST
	vstLogo->image(new Fl_Pixmap(vstLogo_xpm));
	vstLogo->position(vstLogo->x(), text->y()+text->h());
	vstText->label(
		"VST Plug-In Technology by Steinberg\n"
		"VST is a trademark of Steinberg\nMedia Technologies GmbH"
	);
	vstText->position(vstText->x(), vstLogo->y()+vstLogo->h());

#endif

	close->callback(cb_close, (void*)this);
	gu_setFavicon(this);
	setId(WID_ABOUT);
	show();
}


/* ------------------------------------------------------------------ */


gdAbout::~gdAbout() {
	G_Conf.aboutX = x();
	G_Conf.aboutY = y();
}


/* ------------------------------------------------------------------ */


void gdAbout::cb_close(Fl_Widget *w, void *p) { ((gdAbout*)p)->__cb_close(); }


/* ------------------------------------------------------------------ */


void gdAbout::__cb_close() {
	do_callback();
}
