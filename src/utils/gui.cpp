/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gui_utils
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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


#include "../core/mixer.h"
#include "../core/patch_DEPR_.h"
#include "../core/recorder.h"
#include "../core/wave.h"
#include "../core/pluginHost.h"
#include "../core/channel.h"
#include "../core/conf.h"
#include "../core/graphics.h"
#include "../gui/dialogs/gd_warnings.h"
#include "../gui/dialogs/gd_mainWindow.h"
#include "../gui/dialogs/gd_actionEditor.h"
#include "../gui/elems/ge_keyboard.h"
#include "../gui/elems/ge_window.h"
#include "../gui/elems/channel.h"
#include "log.h"
#include "string.h"
#include "gui.h"


extern Mixer 	       G_Mixer;
extern unsigned      G_beats;
extern bool 		     G_audio_status;
extern Patch_DEPR_   G_patch;
extern Conf          G_conf;
extern uint32_t      G_time;
extern gdMainWindow *G_MainWin;
#ifdef WITH_VST
extern PluginHost    G_PluginHost;
#endif


static int blinker = 0;


void gu_refreshUI()
{
	Fl::lock();

	/* update dynamic elements: in and out meters, beat meter and
	 * each channel */

	G_MainWin->inOut->refresh();
	G_MainWin->beatMeter->redraw();
	G_MainWin->keyboard->refreshColumns();

	/* compute timer for blinker */

	blinker++;
	if (blinker > 12)
		blinker = 0;

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
	for (unsigned i=0; i<G_Mixer.channels.size(); i++) {
		G_Mixer.channels.at(i)->guiChannel->update();
	}

	G_MainWin->inOut->setOutVol(G_Mixer.outVol);
	G_MainWin->inOut->setInVol(G_Mixer.inVol);
#ifdef WITH_VST
	G_MainWin->inOut->setMasterFxOutFull(G_PluginHost.getStack(PluginHost::MASTER_OUT)->size() > 0);
	G_MainWin->inOut->setMasterFxInFull(G_PluginHost.getStack(PluginHost::MASTER_IN)->size() > 0);
#endif

	G_MainWin->timing->setMeter(G_Mixer.beats, G_Mixer.bars);
	G_MainWin->timing->setBpm(G_Mixer.bpm);

	/* if you reset to init state while the seq is in play: it's better to
	 * update the button status */

	G_MainWin->controller->updatePlay(G_Mixer.running);
	G_MainWin->controller->updateMetronome(0);
}


/* -------------------------------------------------------------------------- */


void gu_updateMainWinLabel(const string &s)
{
	std::string out = std::string(G_APP_NAME) + " - " + s;
	G_MainWin->copy_label(out.c_str());
}


/* -------------------------------------------------------------------------- */


void gu_setFavicon(Fl_Window *w)
{
#if defined(__linux__)

	fl_open_display();
	Pixmap p, mask;
	XpmCreatePixmapFromData(fl_display, DefaultRootWindow(fl_display),
		(char **) giada_icon, &p, &mask, NULL);
	w->icon((char *)p);

#elif defined(_WIN32)

	w->icon((char *)LoadIcon(fl_display, MAKEINTRESOURCE(IDI_ICON1)));

#endif
}


/* -------------------------------------------------------------------------- */


void gu_openSubWindow(gWindow *parent, gWindow *child, int id)
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
	/** TODO - why don't we simply call WID_ACTION_EDITOR->redraw()? */

	gdActionEditor *aeditor = (gdActionEditor*) G_MainWin->getChild(WID_ACTION_EDITOR);
	if (aeditor) {
		Channel *chan = aeditor->chan;
		G_MainWin->delSubWindow(WID_ACTION_EDITOR);
		gu_openSubWindow(G_MainWin, new gdActionEditor(chan), WID_ACTION_EDITOR);
	}
}


/* -------------------------------------------------------------------------- */


gWindow *gu_getSubwindow(gWindow *parent, int id)
{
	if (parent->hasWindow(id))
		return parent->getChild(id);
	else
		return NULL;
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


string gu_removeFltkChars(const string &s)
{
	string out = gu_replace(s, "/", "-");
	out = gu_replace(out, "|", "-");
	out = gu_replace(out, "&", "-");
	out = gu_replace(out, "_", "-");
	return out;
}
