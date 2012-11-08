/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_pluginWindowGUI
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2012 Giovanni A. Zuliani | Monocasual
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

#ifdef WITH_VST

#include "gd_pluginWindowGUI.h"


extern PluginHost G_PluginHost;


gdPluginWindowGUI::gdPluginWindowGUI(Plugin *pPlugin)
 : gWindow(450, 300), pPlugin(pPlugin)
{
	gu_setFavicon(this);
	set_non_modal();
	resize(x(), y(), pPlugin->getGuiWidth(), pPlugin->getGuiHeight());
	show();

#if defined(__linux__)

	/* Fl::check(): Waits until "something happens" and then returns. It's
	 * mandatory on linux, otherwise X can't find 'this' window. */

	Fl::check();
	Window w = fl_xid(this);

#elif defined(_WIN32)

	HWND w = fl_xid(this);

#elif defined(__APPLE__)

	NSWindow w = fl_xid(this);

#endif

	pPlugin->openGui((void*)w);

	char name[256];
	pPlugin->getProduct(name);
	copy_label(name);

	pPlugin->idle();
}



gdPluginWindowGUI::~gdPluginWindowGUI() {
	pPlugin->closeGui();
}

#endif // #ifdef WITH_VST
