/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <cstddef>
#include <string>
#if defined(_WIN32)
#include "../ext/resource.h"
#elif defined(__linux__) || defined(__FreeBSD__)
#include <X11/xpm.h>
#endif
#include "core/conf.h"
#include "core/graphics.h"
#include "core/mixer.h"
#include "core/mixerHandler.h"
#include "core/plugins/pluginHost.h"
#include "core/sequencer.h"
#include "gui.h"
#include "gui/dialogs/actionEditor/baseActionEditor.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/dialogs/warnings.h"
#include "gui/dialogs/window.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/elems/mainWindow/mainIO.h"
#include "gui/elems/mainWindow/mainTimer.h"
#include "gui/elems/mainWindow/mainTransport.h"
#include "gui/elems/mainWindow/sequencer.h"
#include "gui/elems/sampleEditor/waveTools.h"
#include "log.h"
#include "string.h"

namespace giada::u::gui
{
ScopedLock::ScopedLock()
{
	Fl::lock();
}

/* -------------------------------------------------------------------------- */

ScopedLock::~ScopedLock()
{
	Fl::unlock();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void setFavicon(v::gdWindow* w)
{
#if defined(__linux__) || defined(__FreeBSD__)

	fl_open_display();
	Pixmap p, mask;
	XpmCreatePixmapFromData(fl_display, DefaultRootWindow(fl_display),
	    (char**)giada_icon, &p, &mask, nullptr);
	w->icon((char*)p);

#elif defined(_WIN32)

	w->icon((char*)LoadIcon(fl_display, MAKEINTRESOURCE(IDI_ICON1)));

#endif
}

/* -------------------------------------------------------------------------- */

geompp::Rect<int> getStringRect(const std::string& s)
{
	int w = 0;
	int h = 0;
	fl_measure(s.c_str(), w, h);
	return {0, 0, w, h};
}

/* -------------------------------------------------------------------------- */

geompp::Rect<int> getCenterWinBounds(int w, int h)
{
	return {centerWindowX(w), centerWindowY(h), w, h};
}

/* -------------------------------------------------------------------------- */

std::string removeFltkChars(const std::string& s)
{
	std::string out = u::string::replace(s, "/", "-");
	out             = u::string::replace(out, "|", "-");
	out             = u::string::replace(out, "&", "-");
	out             = u::string::replace(out, "_", "-");
	return out;
}

/* -------------------------------------------------------------------------- */

std::string truncate(const std::string& s, Pixel width)
{
	if (s.empty() || getStringRect(s).w <= width)
		return s;

	std::string tmp  = s;
	std::size_t size = tmp.size();

	while (getStringRect(tmp + "...").w > width)
	{
		if (size == 0)
			return "";
		tmp.resize(--size);
	}

	return tmp + "...";
}

/* -------------------------------------------------------------------------- */

int centerWindowX(int w)
{
	return (Fl::w() / 2) - (w / 2);
}

int centerWindowY(int h)
{
	return (Fl::h() / 2) - (h / 2);
}
} // namespace giada::u::gui
