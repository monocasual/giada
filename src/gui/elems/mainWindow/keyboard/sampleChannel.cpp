/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_sampleChannel
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


#include "../../../../core/mixer.h"
#include "../../../../core/conf.h"
#include "../../../../core/recorder.h"
#include "../../../../core/graphics.h"
#include "../../../../core/wave.h"
#include "../../../../core/sampleChannel.h"
#include "../../../../glue/main.h"
#include "../../../../glue/io.h"
#include "../../../../glue/channel.h"
#include "../../../../glue/storage.h"
#include "../../../../utils/string.h"
#include "../../../dialogs/gd_mainWindow.h"
#include "../../../dialogs/gd_keyGrabber.h"
#include "../../../dialogs/gd_editor.h"
#include "../../../dialogs/gd_actionEditor.h"
#include "../../../dialogs/gd_warnings.h"
#include "../../../dialogs/gd_browser.h"
#include "../../../dialogs/midiIO/midiOutputSampleCh.h"
#include "../../../dialogs/midiIO/midiInputChannel.h"
#include "../../basics/boxtypes.h"
#include "channelStatus.h"
#include "channelMode.h"
#include "keyboard.h"
#include "sampleChannel.h"


extern Mixer 		     G_Mixer;
extern Conf  		     G_Conf;
extern Recorder			 G_Recorder;
extern gdMainWindow *G_MainWin;


geSampleChannel::geSampleChannel(int X, int Y, int W, int H, SampleChannel *ch)
	: geChannel(X, Y, W, H, CHANNEL_SAMPLE, (Channel*) ch)
{
	begin();

	button      = new gButton(x(), y(), 20, 20, "", channelStop_xpm, channelPlay_xpm);
	arm         = new gClick(button->x()+button->w()+4, y(), 20, 20, "", armOff_xpm, armOn_xpm);
	status      = new geChannelStatus(arm->x()+arm->w()+4, y(), 20, 20, ch);
	mainButton  = new geSampleChannelButton(status->x()+status->w()+4, y(), 20, 20, "-- no sample --");
	readActions = new gClick(mainButton->x()+mainButton->w()+4, y(), 20, 20, "", readActionOff_xpm, readActionOn_xpm);
	modeBox     = new geChannelMode(readActions->x()+readActions->w()+4, y(), 20, 20, ch);
	mute        = new gClick(modeBox->x()+modeBox->w()+4, y(), 20, 20, "", muteOff_xpm, muteOn_xpm);
	solo        = new gClick(mute->x()+mute->w()+4, y(), 20, 20, "", soloOff_xpm, soloOn_xpm);
#ifdef WITH_VST
	fx          = new gFxButton(solo->x()+solo->w()+4, y(), 20, 20, fxOff_xpm, fxOn_xpm);
	vol         = new gDial(fx->x()+fx->w()+4, y(), 20, 20);
#else
	vol         = new gDial(solo->x()+solo->w()+4, y(), 20, 20);
#endif

	end();

  resizable(mainButton);

	update();

	button->callback(cb_button, (void*)this);
	button->when(FL_WHEN_CHANGED);   // do callback on keypress && on keyrelease

	arm->type(FL_TOGGLE_BUTTON);
	arm->callback(cb_arm, (void*)this);

#ifdef WITH_VST
	fx->callback(cb_openFxWindow, (void*)this);
#endif

	mute->type(FL_TOGGLE_BUTTON);
	mute->callback(cb_mute, (void*)this);

	solo->type(FL_TOGGLE_BUTTON);
	solo->callback(cb_solo, (void*)this);

	mainButton->callback(cb_openMenu, (void*)this);

	readActions->type(FL_TOGGLE_BUTTON);
	readActions->value(ch->readActions);
	readActions->callback(cb_readActions, (void*)this);

	vol->callback(cb_changeVol, (void*)this);

	ch->guiChannel = this;
}


/* -------------------------------------------------------------------------- */


void geSampleChannel::cb_button      (Fl_Widget *v, void *p) { ((geSampleChannel*)p)->__cb_button(); }
void geSampleChannel::cb_openMenu    (Fl_Widget *v, void *p) { ((geSampleChannel*)p)->__cb_openMenu(); }
void geSampleChannel::cb_readActions (Fl_Widget *v, void *p) { ((geSampleChannel*)p)->__cb_readActions(); }


/* -------------------------------------------------------------------------- */


void geSampleChannel::__cb_button()
{
	if (button->value())    // pushed
		glue_keyPress(ch, Fl::event_ctrl(), Fl::event_shift());
	else                    // released
		glue_keyRelease(ch, Fl::event_ctrl(), Fl::event_shift());
}


/* -------------------------------------------------------------------------- */


void geSampleChannel::__cb_openMenu()
{
	/* If you're recording (input or actions) no menu is allowed; you can't do
	anything, especially deallocate the channel */

	if (G_Mixer.recording || G_Recorder.active)
		return;

	/* the following is a trash workaround for a FLTK menu. We need a gMenu
	 * widget asap */

	Fl_Menu_Item rclick_menu[] = {
		{"Load new sample..."},                     // 0
		{"Export sample to file..."},               // 1
		{"Setup keyboard input..."},                // 2
		{"Setup MIDI input..."},                    // 3
		{"Setup MIDI output..."},                   // 4
		{"Edit sample..."},                         // 5
		{"Edit actions..."},                        // 6
		{"Clear actions", 0, 0, 0, FL_SUBMENU},     // 7
			{"All"},                                  // 8
			{"Mute"},                                 // 9
			{"Volume"},                               // 10
			{"Start/Stop"},                           // 11
			{0},                                      // 12
		{"Clone channel"},                          // 13
		{"Free channel"},                           // 14
		{"Delete channel"},                         // 15
		{0}
	};

	if (ch->status & (STATUS_EMPTY | STATUS_MISSING)) {
		rclick_menu[1].deactivate();
		rclick_menu[5].deactivate();
		rclick_menu[14].deactivate();
	}

	/* no 'clear actions' if there are no actions */

	if (!ch->hasActions)
		rclick_menu[7].deactivate();

	/* no 'clear start/stop actions' for those channels in loop mode:
	 * they cannot have start/stop actions. */

	if (((SampleChannel*)ch)->mode & LOOP_ANY)
		rclick_menu[11].deactivate();

	Fl_Menu_Button *b = new Fl_Menu_Button(0, 0, 100, 50);
	b->box(G_CUSTOM_BORDER_BOX);
	b->textsize(GUI_FONT_SIZE_BASE);
	b->textcolor(COLOR_TEXT_0);
	b->color(COLOR_BG_0);

	const Fl_Menu_Item *m = rclick_menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, b);
	if (!m) return;

	if (strcmp(m->label(), "Load new sample...") == 0) {
    gWindow *w = new gdLoadBrowser(G_Conf.browserX, G_Conf.browserY,
      G_Conf.browserW, G_Conf.browserH, "Browse sample",
      G_Conf.samplePath.c_str(), glue_loadSample, ch);
    gu_openSubWindow(G_MainWin, w, WID_FILE_BROWSER);
		return;
	}

	if (strcmp(m->label(), "Setup keyboard input...") == 0) {
		new gdKeyGrabber(ch); /// FIXME - use gu_openSubWindow
		return;
	}

	if (strcmp(m->label(), "Setup MIDI input...") == 0) {
		gu_openSubWindow(G_MainWin, new gdMidiInputChannel(ch), 0);
		return;
	}

	if (strcmp(m->label(), "Setup MIDI output...") == 0) {
		gu_openSubWindow(G_MainWin, new gdMidiOutputSampleCh((SampleChannel*) ch), 0);
		return;
	}

	if (strcmp(m->label(), "Edit sample...") == 0) {
		gu_openSubWindow(G_MainWin, new gdEditor((SampleChannel*) ch), WID_SAMPLE_EDITOR); /// FIXME title it's up to gdEditor
		return;
	}

	if (strcmp(m->label(), "Export sample to file...") == 0) {
    gWindow *w = new gdSaveBrowser(G_Conf.browserX, G_Conf.browserY,
      G_Conf.browserW, G_Conf.browserH, "Save sample", \
      G_Conf.samplePath.c_str(), "", glue_saveSample, ch);
    gu_openSubWindow(G_MainWin, w, WID_FILE_BROWSER);
		return;
	}

	if (strcmp(m->label(), "Delete channel") == 0) {
		if (!gdConfirmWin("Warning", "Delete channel: are you sure?"))
			return;
		glue_deleteChannel(ch);
		return;
	}

	if (strcmp(m->label(), "Free channel") == 0) {
		if (ch->status == STATUS_PLAY) {
			if (!gdConfirmWin("Warning", "This action will stop the channel: are you sure?"))
				return;
		}
		else if (!gdConfirmWin("Warning", "Free channel: are you sure?"))
			return;

		glue_freeChannel(ch);

		/* delete any related subwindow */

		/** FIXME - use gu_closeAllSubwindows() */

		G_MainWin->delSubWindow(WID_FILE_BROWSER);
		G_MainWin->delSubWindow(WID_ACTION_EDITOR);
		G_MainWin->delSubWindow(WID_SAMPLE_EDITOR);
		G_MainWin->delSubWindow(WID_FX_LIST);

		return;
	}

	if (strcmp(m->label(), "Clone channel") == 0) {
		glue_cloneChannel(ch);
		return;
	}

	if (strcmp(m->label(), "Mute") == 0) {
		if (!gdConfirmWin("Warning", "Clear all mute actions: are you sure?"))
			return;
		G_Recorder.clearAction(ch->index, ACTION_MUTEON | ACTION_MUTEOFF);
    ch->hasActions = G_Recorder.hasActions(ch->index);

		if (!ch->hasActions)
			hideActionButton();

		/* TODO - set mute=false */

		gu_refreshActionEditor(); // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "Start/Stop") == 0) {
		if (!gdConfirmWin("Warning", "Clear all start/stop actions: are you sure?"))
			return;
		G_Recorder.clearAction(ch->index, ACTION_KEYPRESS | ACTION_KEYREL | ACTION_KILLCHAN);
    ch->hasActions = G_Recorder.hasActions(ch->index);

		if (!ch->hasActions)
			hideActionButton();
		gu_refreshActionEditor();  // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "Volume") == 0) {
		if (!gdConfirmWin("Warning", "Clear all volume actions: are you sure?"))
			return;
		G_Recorder.clearAction(ch->index, ACTION_VOLUME);
    ch->hasActions = G_Recorder.hasActions(ch->index);
		if (!ch->hasActions)
			hideActionButton();
		gu_refreshActionEditor();  // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "All") == 0) {
		if (!gdConfirmWin("Warning", "Clear all actions: are you sure?"))
			return;
		G_Recorder.clearChan(ch->index);
    ch->hasActions = false;
		hideActionButton();
		gu_refreshActionEditor(); // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "Edit actions...") == 0) {
		gu_openSubWindow(G_MainWin, new gdActionEditor(ch),	WID_ACTION_EDITOR);
		return;
	}
}


/* -------------------------------------------------------------------------- */


void geSampleChannel::__cb_readActions()
{
	glue_startStopReadingRecs((SampleChannel*) ch);
}


/* -------------------------------------------------------------------------- */


void geSampleChannel::refresh()
{
  if (!mainButton->visible()) // mainButton invisible? status too (see below)
    return;

	setColorsByStatus(ch->status, ch->recStatus);

	if (((SampleChannel*) ch)->wave != nullptr) {
		if (G_Mixer.recording && ch->armed)
			mainButton->setInputRecordMode();
		if (G_Recorder.active) {
			if (G_Recorder.canRec(ch, &G_Mixer))
				mainButton->setActionRecordMode();
		}
		status->redraw(); // status invisible? sampleButton too (see below)
	}
	mainButton->redraw();
}


/* -------------------------------------------------------------------------- */


void geSampleChannel::reset()
{
	hideActionButton();
	mainButton->setDefaultMode("-- no sample --");
 	mainButton->redraw();
	status->redraw();
}


/* -------------------------------------------------------------------------- */


void geSampleChannel::update()
{
	/* update sample button's label */

	switch (ch->status) {
		case STATUS_EMPTY:
			mainButton->label("-- no sample --");
			break;
		case STATUS_MISSING:
		case STATUS_WRONG:
			mainButton->label("* file not found! *");
			break;
		default:
			mainButton->label(((SampleChannel*) ch)->wave->name.c_str());
			break;
	}

	/* update channels. If you load a patch with recorded actions, the 'R'
	 * button must be shown. Moreover if the actions are active, the 'R'
	 * button must be activated accordingly. */

	if (ch->hasActions)
		showActionButton();
	else
		hideActionButton();

	/* updates modebox */

	modeBox->value(((SampleChannel*) ch)->mode);
	modeBox->redraw();

	/* update volumes+mute+solo */

	vol->value(ch->volume);
	mute->value(ch->mute);
	solo->value(ch->solo);

	mainButton->setKey(ch->key);

#ifdef WITH_VST
	fx->full = ch->plugins.size() > 0;
	fx->redraw();
#endif
}


/* -------------------------------------------------------------------------- */


void geSampleChannel::showActionButton()
{
	readActions->value(((SampleChannel*) ch)->readActions);
	readActions->show();
	packWidgets();
	redraw();
}


/* -------------------------------------------------------------------------- */


void geSampleChannel::hideActionButton()
{
	readActions->hide();
	packWidgets();
	redraw();
}


/* -------------------------------------------------------------------------- */


void geSampleChannel::resize(int X, int Y, int W, int H)
{
  geChannel::resize(X, Y, W, H);

	arm->hide();
	modeBox->hide();
	readActions->hide();
#ifdef WITH_VST
	fx->hide();
#endif

	if (w() > BREAK_ARM)
		arm->show();
#ifdef WITH_VST
	if (w() > BREAK_FX)
		fx->show();
#endif
	if (w() > BREAK_MODE_BOX)
		modeBox->show();
	if (w() > BREAK_READ_ACTIONS && ch->hasActions)
		readActions->show();

	packWidgets();
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


geSampleChannelButton::geSampleChannelButton(int x, int y, int w, int h, const char *l)
	: geChannelButton(x, y, w, h, l) {}


/* -------------------------------------------------------------------------- */


int geSampleChannelButton::handle(int e)
{
	int ret = gClick::handle(e);
	switch (e) {
		case FL_DND_ENTER:
		case FL_DND_DRAG:
		case FL_DND_RELEASE: {
			ret = 1;
			break;
		}
		case FL_PASTE: {
      geSampleChannel *gch = (geSampleChannel*) parent();   // parent is geSampleChannel
      SampleChannel   *ch  = (SampleChannel*) gch->ch;
      int result = glue_loadChannel(ch, gu_trim(gu_stripFileUrl(Fl::event_text())).c_str());
			if (result != SAMPLE_LOADED_OK)
				G_MainWin->keyboard->printChannelMessage(result);
			ret = 1;
			break;
		}
	}
	return ret;
}
