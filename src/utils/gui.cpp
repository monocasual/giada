/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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
#include "../core/mixer.h"
#include "../core/clock.h"
#include "../core/pluginHost.h"
#include "../core/channel.h"
#include "../core/conf.h"
#include "../core/graphics.h"
#include "../gui/dialogs/gd_warnings.h"
#include "../gui/dialogs/gd_mainWindow.h"
#include "../gui/dialogs/actionEditor/baseActionEditor.h"
#include "../gui/dialogs/window.h"
#include "../gui/dialogs/sampleEditor.h"
#include "../gui/elems/mainWindow/mainIO.h"
#include "../gui/elems/mainWindow/mainTimer.h"
#include "../gui/elems/mainWindow/mainTransport.h"
#include "../gui/elems/mainWindow/beatMeter.h"
#include "../gui/elems/mainWindow/keyboard/keyboard.h"
#include "../gui/elems/mainWindow/keyboard/channel.h"
#include "../gui/elems/sampleEditor/waveTools.h"
#include "log.h"
#include "string.h"
#include "gui.h"


extern gdMainWindow* G_MainWin;


using std::string;
using namespace giada;
using namespace giada::m;
using namespace giada::v;


static int blinker = 0;


void gu_refreshUI()
{
	Fl::lock();

	/* update dynamic elements: in and out meters, beat meter and
	 * each channel */

	G_MainWin->mainIO->refresh();
	G_MainWin->beatMeter->redraw();
	G_MainWin->keyboard->refreshColumns();

	/* compute timer for blinker */

	blinker++;
	if (blinker > 12)
		blinker = 0;

	/* If Sample Editor is open, repaint it (for dynamic play head). */

	gdSampleEditor* se = static_cast<gdSampleEditor*>(gu_getSubwindow(G_MainWin, WID_SAMPLE_EDITOR));
	if (se != nullptr)
		se->waveTools->redrawWaveformAsync();

	/* redraw GUI */

	Fl::unlock();
	Fl::awake();
}


/* -------------------------------------------------------------------------- */


int gu_getBlinker()
{
	return blinker;
}


/* -------------------------------------------------------------------------- */


void gu_updateControls()
{
	for (const Channel* ch : mixer::channels)
		ch->guiChannel->update();

	G_MainWin->mainIO->setOutVol(mixer::outVol);
	G_MainWin->mainIO->setInVol(mixer::inVol);
#ifdef WITH_VST
	G_MainWin->mainIO->setMasterFxOutFull(pluginHost::getStack(pluginHost::StackType::MASTER_OUT).size() > 0);
	G_MainWin->mainIO->setMasterFxInFull(pluginHost::getStack(pluginHost::StackType::MASTER_IN).size() > 0);
#endif

	G_MainWin->mainTimer->setMeter(clock::getBeats(), clock::getBars());
	G_MainWin->mainTimer->setBpm(clock::getBpm());
	G_MainWin->mainTimer->setQuantizer(clock::getQuantize());

	G_MainWin->mainTransport->updatePlay(clock::isRunning());
	G_MainWin->mainTransport->updateMetronome(mixer::isMetronomeOn());
}


/* -------------------------------------------------------------------------- */


void gu_updateMainWinLabel(const string& s)
{
	std::string out = std::string(G_APP_NAME) + " - " + s;
	G_MainWin->copy_label(out.c_str());
}


/* -------------------------------------------------------------------------- */


void gu_setFavicon(Fl_Window* w)
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


void gu_openSubWindow(gdWindow* parent, gdWindow* child, int id)
{
	if (parent->hasWindow(id)) {
		gu_log("[GU] parent has subwindow with id=%d, deleting\n", id);
		parent->delSubWindow(id);
	}
	child->setId(id);
	parent->addSubWindow(child);
}


/* -------------------------------------------------------------------------- */


void gu_refreshActionEditor()
{
	gdBaseActionEditor* ae = static_cast<gdBaseActionEditor*>(G_MainWin->getChild(WID_ACTION_EDITOR));
	if (ae != nullptr)
		ae->rebuild();
}


/* -------------------------------------------------------------------------- */


gdWindow* gu_getSubwindow(gdWindow* parent, int id)
{
	if (parent->hasWindow(id))
		return parent->getChild(id);
	else
		return nullptr;
}


/* -------------------------------------------------------------------------- */


void gu_closeAllSubwindows()
{
	/* don't close WID_FILE_BROWSER, because it's the caller of this
	 * function */

	G_MainWin->delSubWindow(WID_ACTION_EDITOR);
	G_MainWin->delSubWindow(WID_SAMPLE_EDITOR);
	G_MainWin->delSubWindow(WID_FX_LIST);
	G_MainWin->delSubWindow(WID_FX);
}


/* -------------------------------------------------------------------------- */


int gu_getStringWidth(const std::string& s)
{
	int w = 0;
	int h = 0;
	fl_measure(s.c_str(), w, h);
	return w;
}


/* -------------------------------------------------------------------------- */


string gu_removeFltkChars(const string& s)
{
	string out = u::string::replace(s, "/", "-");
	out = u::string::replace(out, "|", "-");
	out = u::string::replace(out, "&", "-");
	out = u::string::replace(out, "_", "-");
	return out;
}
