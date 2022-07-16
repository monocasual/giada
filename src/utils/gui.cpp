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

geompp::Rect<int> getCenterWinBounds(geompp::Rect<int> r)
{
	if (r.x == -1 || r.y == -1)
		return {centerWindowX(r.w), centerWindowY(r.h), r.w, r.h};
	return r;
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

/* -------------------------------------------------------------------------- */

std::string keyToString(int key)
{
	// https://github.com/fltk/fltk/blob/570a05a33c9dc42a16caa5a1a11cf34d4df1c1f9/FL/Enumerations.H
	// https://www.fltk.org/doc-1.3/group__fl__events.html#gafa17a5b4d8d9163631c88142e60447ed

	if (key == 0)
		return "[None]";

	switch (key)
	{
	case ' ':
		return "Space";
	case FL_BackSpace:
		return "Backspace";
	case FL_Tab:
		return "Tab";
	case FL_Enter:
		return "Enter";
	case FL_Pause:
		return "Pause";
	case FL_Scroll_Lock:
		return "Scroll lock";
	case FL_Escape:
		return "Escape";
	case FL_Home:
		return "Home";
	case FL_Left:
		return "Left";
	case FL_Up:
		return "Up";
	case FL_Right:
		return "Right";
	case FL_Down:
		return "Down";
	case FL_Page_Up:
		return "Page up";
	case FL_Page_Down:
		return "Page down";
	case FL_End:
		return "End";
	case FL_Print:
		return "Print";
	case FL_Insert:
		return "Insert";
	case FL_Menu:
		return "Menu";
	case FL_Help:
		return "Help";
	case FL_Num_Lock:
		return "Num lock";
	case FL_KP: // TODO ?
		return "";
	case FL_KP_Enter:
		return "KP Enter";
	case FL_F + 1:
		return "F1";
	case FL_F + 2:
		return "F2";
	case FL_F + 3:
		return "F3";
	case FL_F + 4:
		return "F4";
	case FL_F + 5:
		return "F5";
	case FL_F + 6:
		return "F6";
	case FL_F + 7:
		return "F7";
	case FL_F + 8:
		return "F8";
	case FL_F + 9:
		return "F9";
	case FL_F + 10:
		return "F10";
	case FL_F + 11:
		return "F11";
	case FL_F + 12:
		return "F12";
	case FL_Shift_L:
		return "Shift L";
	case FL_Shift_R:
		return "Shift R";
	case FL_Control_L:
		return "Control L";
	case FL_Control_R:
		return "Control R";
	case FL_Caps_Lock:
		return "Caps lock";
	case FL_Meta_L:
		return "Meta L";
	case FL_Meta_R:
		return "Meta R";
	case FL_Alt_L:
		return "Alt L";
	case FL_Alt_R:
		return "Alt R";
	case FL_Delete:
		return "Delete";
	default:
		return Fl::event_text();
	}
}
} // namespace giada::u::gui
