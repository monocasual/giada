/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#include <string>
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#if defined(_WIN32)
	#include "../ext/resource.h"
#elif defined(__linux__) || defined(__FreeBSD__)
	#include <X11/xpm.h>
#endif
#include "core/mixer.h"
#include "core/mixerHandler.h"
#include "core/clock.h"
#include "core/pluginHost.h"
#include "core/conf.h"
#include "core/graphics.h"
#include "gui/dialogs/warnings.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/actionEditor/baseActionEditor.h"
#include "gui/dialogs/window.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/elems/mainWindow/mainIO.h"
#include "gui/elems/mainWindow/mainTimer.h"
#include "gui/elems/mainWindow/mainTransport.h"
#include "gui/elems/mainWindow/beatMeter.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "gui/elems/sampleEditor/waveTools.h"
#include "log.h"
#include "string.h"
#include "gui.h"


extern giada::v::gdMainWindow* G_MainWin;


namespace giada {
namespace u {
namespace gui 
{
namespace
{
int blinker_ = 0;
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void rebuildSubWindow(int wid)
{
	v::gdWindow* w = getSubwindow(G_MainWin, wid);
	if(w != nullptr)  // If its open
		w->rebuild();	
}


/* -------------------------------------------------------------------------- */


void refreshSubWindow(int wid)
{
	v::gdWindow* w = getSubwindow(G_MainWin, wid);
	if(w != nullptr)  // If its open
		w->refresh();		
}


/* -------------------------------------------------------------------------- */


void refresh()
{
	/* Update dynamic elements inside main window: in and out meters, beat meter
	and each channel. */

	G_MainWin->refresh();

	/* Compute timer for blinker. */

	blinker_ = (blinker_ + 1) % 12;

	/* Refresh Sample Editor (if open) for dynamic play head. */

	refreshSubWindow(WID_SAMPLE_EDITOR);
}


/* -------------------------------------------------------------------------- */


void rebuild()
{
	G_MainWin->rebuild();
	rebuildSubWindow(WID_FX_LIST);
	rebuildSubWindow(WID_SAMPLE_EDITOR);
	rebuildSubWindow(WID_ACTION_EDITOR);
}


/* -------------------------------------------------------------------------- */


bool shouldBlink()
{
	return blinker_ > 6;
}


/* -------------------------------------------------------------------------- */


void updateStaticWidgets()
{
	using namespace giada::m;

	G_MainWin->mainIO->setOutVol(mh::getOutVol());
	G_MainWin->mainIO->setInVol(mh::getInVol());

#ifdef WITH_VST

//	G_MainWin->mainIO->setMasterFxOutFull(pluginHost::getStack(pluginHost::StackType::MASTER_OUT).plugins.size() > 0);
//	G_MainWin->mainIO->setMasterFxInFull(pluginHost::getStack(pluginHost::StackType::MASTER_IN).plugins.size() > 0);
	
#endif

	G_MainWin->mainTimer->setMeter(clock::getBeats(), clock::getBars());
	G_MainWin->mainTimer->setBpm(clock::getBpm());
	G_MainWin->mainTimer->setQuantizer(clock::getQuantizerValue());
}


/* -------------------------------------------------------------------------- */


void updateMainWinLabel(const std::string& s)
{
	std::string out = std::string(G_APP_NAME) + " - " + s;
	G_MainWin->copy_label(out.c_str());
}


/* -------------------------------------------------------------------------- */


void setFavicon(v::gdWindow* w)
{
#if defined(__linux__) || defined(__FreeBSD__)

	fl_open_display();
	Pixmap p, mask;
	XpmCreatePixmapFromData(fl_display, DefaultRootWindow(fl_display),
		(char **) giada_icon, &p, &mask, nullptr);
	w->icon((char *)p);

#elif defined(_WIN32)

	w->icon((char *)LoadIcon(fl_display, MAKEINTRESOURCE(IDI_ICON1)));

#endif
}


/* -------------------------------------------------------------------------- */


void openSubWindow(v::gdWindow* parent, v::gdWindow* child, int id)
{
	if (parent->hasWindow(id)) {
		u::log::print("[GU] parent has subwindow with id=%d, deleting\n", id);
		parent->delSubWindow(id);
	}
	child->setId(id);
	parent->addSubWindow(child);
}


/* -------------------------------------------------------------------------- */


void refreshActionEditor()
{
	v::gdBaseActionEditor* ae = static_cast<v::gdBaseActionEditor*>(G_MainWin->getChild(WID_ACTION_EDITOR));
	if (ae != nullptr)
		ae->rebuild();
}


/* -------------------------------------------------------------------------- */


v::gdWindow* getSubwindow(v::gdWindow* parent, int id)
{
	if (parent->hasWindow(id))
		return parent->getChild(id);
	else
		return nullptr;
}


/* -------------------------------------------------------------------------- */


void closeAllSubwindows()
{
	/* don't close WID_FILE_BROWSER, because it's the caller of this
	 * function */

	G_MainWin->delSubWindow(WID_ACTION_EDITOR);
	G_MainWin->delSubWindow(WID_SAMPLE_EDITOR);
	G_MainWin->delSubWindow(WID_FX_LIST);
	G_MainWin->delSubWindow(WID_FX);
}


/* -------------------------------------------------------------------------- */


int getStringWidth(const std::string& s)
{
	int w = 0;
	int h = 0;
	fl_measure(s.c_str(), w, h);
	return w;
}


/* -------------------------------------------------------------------------- */


std::string removeFltkChars(const std::string& s)
{
	std::string out = u::string::replace(s, "/", "-");
	out = u::string::replace(out, "|", "-");
	out = u::string::replace(out, "&", "-");
	out = u::string::replace(out, "_", "-");
	return out;
}


/* -------------------------------------------------------------------------- */


std::string truncate(const std::string& s, Pixel width)
{
	if (getStringWidth(s) <= width) 
		return s;
	
	std::string tmp  = s;
	std::size_t size = tmp.size();
	while (getStringWidth(tmp) > width)
		tmp.resize(--size);

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
}}} // giada::u::gui::
