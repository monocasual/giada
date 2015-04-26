/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gui_utils
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


#include "mixer.h"
#include "patch.h"
#include "gui_utils.h"
#include "graphics.h"
#include "gd_warnings.h"
#include "ge_window.h"
#include "ge_channel.h"
#include "gd_mainWindow.h"
#include "gg_keyboard.h"
#include "gd_actionEditor.h"
#include "recorder.h"
#include "wave.h"
#include "pluginHost.h"
#include "channel.h"
#include "log.h"
#include "conf.h"


extern Mixer 	       G_Mixer;
extern unsigned      G_beats;
extern bool 		     G_audio_status;
extern Patch         G_patch;
extern Conf          G_conf;
extern uint32_t      G_time;
extern gdMainWindow *mainWin;
#ifdef WITH_VST
extern PluginHost		 G_PluginHost;
#endif


static int blinker = 0;


void gu_refresh()
{
	Fl::lock();

	/* update dynamic elements: in and out meters, beat meter and
	 * each channel */

	mainWin->inOut->refresh();
	mainWin->beatMeter->redraw();
	mainWin->keyboard->refreshColumns();

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
	for (unsigned i=0; i<G_Mixer.channels.size; i++)
		G_Mixer.channels.at(i)->guiChannel->update();

	mainWin->inOut->setOutVol(G_Mixer.outVol);
	mainWin->inOut->setInVol(G_Mixer.inVol);
#ifdef WITH_VST
	mainWin->inOut->setMasterFxOutFull(G_PluginHost.countPlugins(PluginHost::MASTER_OUT));
	mainWin->inOut->setMasterFxInFull(G_PluginHost.countPlugins(PluginHost::MASTER_IN));
#endif

	mainWin->timing->setMeter(G_Mixer.beats, G_Mixer.bars);
	mainWin->timing->setBpm(G_Mixer.bpm);

	/* if you reset to init state while the seq is in play: it's better to
	 * update the button status */

	mainWin->controller->updatePlay(G_Mixer.running);
	mainWin->controller->updateMetronome(0);
}


/* -------------------------------------------------------------------------- */


void gu_update_win_label(const char *c)
{
	std::string out = VERSIONE_STR;
	out += " - ";
	out += c;
	mainWin->copy_label(out.c_str());
}


/* -------------------------------------------------------------------------- */


void gu_setFavicon(Fl_Window *w)
{
#if defined(__linux__)
	fl_open_display();
	Pixmap p, mask;
	XpmCreatePixmapFromData(
		fl_display,
		DefaultRootWindow(fl_display),
		(char **)giada_icon,
		&p,
		&mask,
		NULL);
	w->icon((char *)p);
#elif defined(_WIN32)
	w->icon((char *)LoadIcon(fl_display, MAKEINTRESOURCE(IDI_ICON1)));
#endif
}


/* -------------------------------------------------------------------------- */


void gu_openSubWindow(gWindow *parent, gWindow *child, int id)
{
	if (parent->hasWindow(id)) {
		gLog("[GU] parent has subwindow with id=%d, deleting\n", id);
		parent->delSubWindow(id);
	}
	child->setId(id);
	parent->addSubWindow(child);
}


/* -------------------------------------------------------------------------- */


void gu_refreshActionEditor()
{
	/** TODO - why don't we simply call WID_ACTION_EDITOR->redraw()? */

	gdActionEditor *aeditor = (gdActionEditor*) mainWin->getChild(WID_ACTION_EDITOR);
	if (aeditor) {
		Channel *chan = aeditor->chan;
		mainWin->delSubWindow(WID_ACTION_EDITOR);
		gu_openSubWindow(mainWin, new gdActionEditor(chan), WID_ACTION_EDITOR);
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

	mainWin->delSubWindow(WID_ACTION_EDITOR);
	mainWin->delSubWindow(WID_SAMPLE_EDITOR);
	mainWin->delSubWindow(WID_FX_LIST);
	mainWin->delSubWindow(WID_FX);
}
