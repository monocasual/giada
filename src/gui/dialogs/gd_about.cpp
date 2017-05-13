/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


#include <FL/Fl_Pixmap.H>
#include <FL/fl_draw.H>
#include <jansson.h>
#include "../../core/conf.h"
#include "../../core/const.h"
#include "../../core/kernelAudio.h"
#include "../../core/kernelMidi.h"
#include "../../core/graphics.h"
#ifdef WITH_VST
  #include "../../deps/juce-config.h"
#endif
#include "../../utils/gui.h"
#include "../elems/basics/button.h"
#include "../elems/basics/box.h"
#include "gd_about.h"


using namespace giada::m;


gdAbout::gdAbout()
#ifdef WITH_VST
: gdWindow(340, 435, "About Giada")
{
#else
: gdWindow(340, 350, "About Giada")
{
#endif

	if (conf::aboutX)
		resize(conf::aboutX, conf::aboutY, w(), h());

	set_modal();

	logo  = new geBox(8, 10, 324, 86);
	text  = new geBox(8, 120, 324, 145);
	close = new geButton(252, h()-28, 80, 20, "Close");
#ifdef WITH_VST
	vstLogo = new geBox(8, 265, 324, 50);
	vstText = new geBox(8, 315, 324, 46);
#endif
	end();

	logo->image(new Fl_Pixmap(giada_logo_xpm));
	text->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_TOP);

	char message[512];
	sprintf(
	  message,
	  "Version " G_VERSION_STR " (" BUILD_DATE ")\n\n"
		"Developed by Monocasual\n"
		"Based on FLTK (%d.%d.%d), RtAudio (%s),\n"
		"RtMidi (%s), Libsamplerate, Jansson (%s),\n"
		"Libsndfile"
#ifdef WITH_VST
		", JUCE (%d.%d.%d)\n\n"
#else
		"\n\n"
#endif
		"Released under the terms of the GNU General\n"
		"Public License (GPL v3)\n\n"
		"News, infos, contacts and documentation:\n"
		"www.giadamusic.com",
		FL_MAJOR_VERSION, FL_MINOR_VERSION, FL_PATCH_VERSION,
		kernelAudio::getRtAudioVersion().c_str(),
		kernelMidi::getRtMidiVersion().c_str(),
		JANSSON_VERSION
#ifdef WITH_VST
		, JUCE_MAJOR_VERSION, JUCE_MINOR_VERSION, JUCE_BUILDNUMBER);
#else
		);
#endif

	int tw = 0;
	int th = 0;
	fl_measure(message, tw, th);
	text->copy_label(message);
	text->size(text->w(), th);

#ifdef WITH_VST
	vstLogo->image(new Fl_Pixmap(vstLogo_xpm));
	vstLogo->position(vstLogo->x(), text->y()+text->h()+8);
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


/* -------------------------------------------------------------------------- */


gdAbout::~gdAbout()
{
	conf::aboutX = x();
	conf::aboutY = y();
}


/* -------------------------------------------------------------------------- */


void gdAbout::cb_close(Fl_Widget *w, void *p) { ((gdAbout*)p)->__cb_close(); }


/* -------------------------------------------------------------------------- */


void gdAbout::__cb_close()
{
	do_callback();
}
