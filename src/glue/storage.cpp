/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * glue
 * Intermediate layer GUI <-> CORE.
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


#include "../gui/elems/ge_column.h"
#include "../gui/elems/ge_keyboard.h"
#include "../gui/dialogs/gd_mainWindow.h"
#include "../gui/dialogs/gd_warnings.h"
#include "../core/mixer.h"
#include "../core/mixerHandler.h"
#include "../core/channel.h"
#include "../core/conf.h"
#include "../core/patch.h"
#include "../core/patch_DEPR_.h" // TODO - remove, used only for DEPR calls
#include "../core/sampleChannel.h"
#include "../core/midiChannel.h"
#include "../core/wave.h"
#include "../utils/gui_utils.h"
#include "glue.h" // TODO - remove, used only for DEPR calls
#include "storage.h"


using std::string;


extern gdMainWindow *mainWin;
extern Mixer	   		 G_Mixer;
extern Patch         G_Patch;
extern Conf          G_Conf;
extern Patch_DEPR_   G_Patch_DEPR_; // TODO - remove, used only for DEPR calls


int glue_savePatch(const string &fullPath, const string &name, bool isProject)
{
	G_Patch.init();

	__fillPatchGlobals__(name);
	__fillPatchChannels__(isProject);
	__fillPatchColumns__();

	if (G_Patch.write(fullPath)) {
		gu_update_win_label(name.c_str());
		gLog("[glue] patch saved as %s\n", fullPath.c_str());
		return 1;
	}
	return 0;
}


/* -------------------------------------------------------------------------- */


int glue_loadPatch(const string &fullPath, class gProgress *status, bool isProject)
{
	__setProgressBar__(status, 0.1f);

	/* try to load the new JSON-based patch. If it fails, fall back to deprecated
	* one. */

	int res = G_Patch.read(fullPath);
	if (res == PATCH_UNREADABLE) {
		gLog("[glue] failed reading JSON-based patch. Trying with the deprecated method\n");
		return glue_loadPatch__DEPR__(gBasename(fullPath).c_str(), fullPath.c_str(), status, isProject);
	}
	if (res != PATCH_OPEN_OK)
		return res;

	/* close all other windows. This prevents segfault if plugin
	 * windows GUIs are on. */

	gu_closeAllSubwindows();

	/* reset the system. False(1): don't update the gui right now. False(2): do
	 * not create empty columns. */

	glue_resetToInitState(false, false);

	__setProgressBar__(status, 0.1f);

	/* add common stuff, columns and channels */
	/* TODO - update press bar from 0.1 to 0.9 (0.8 / G_Patch.channels.size) */

	for (unsigned i=0; i<G_Patch.columns.size; i++) {
		mainWin->keyboard->addColumn();
		for (unsigned k=0; k<G_Patch.channels.size; k++) {
			if (G_Patch.channels.at(k).column == G_Patch.columns.at(i).index) {
				Channel *ch = glue_addChannel(G_Patch.channels.at(k).column, G_Patch.channels.at(k).type);
				ch->readPatch(k);	// TODO - grab return value and notify bad blugins
			}
		}
	}

	/* fill Mixer */

	mh_readPatch();

	/* let recorder recompute the actions' positions if the current
	 * samplerate != patch samplerate */

	recorder::updateSamplerate(G_Conf.samplerate, G_Patch.samplerate);

	/* save patchPath by taking the last dir of the broswer, in order to
	 * reuse it the next time */

	G_Conf.setPath(G_Conf.patchPath, gDirname(fullPath.c_str()).c_str());

	/* refresh GUI */

	gu_updateControls();
	gu_update_win_label(G_Patch.name.c_str());

	__setProgressBar__(status, 1.0f);

	return res;
}


/* -------------------------------------------------------------------------- */


int glue_loadPatch__DEPR__(const char *fname, const char *fpath, gProgress *status, bool isProject)
{
	/* update browser's status bar with % 0.1 */

	status->show();
	status->value(0.1f);
	//Fl::check();
	Fl::wait(0);

	/* is it a valid patch? */

	int res = G_Patch_DEPR_.open(fpath);
	if (res != PATCH_OPEN_OK)
		return res;

	/* close all other windows. This prevents segfault if plugin windows
	 * GUI are on. */

	if (res)
		gu_closeAllSubwindows();

	/* reset the system. False(1): don't update the gui right now. False(2): do
	 * not create empty columns. */

	glue_resetToInitState(false, false);

	status->value(0.2f);  // progress status: % 0.2
	//Fl::check();
	Fl::wait(0);

	/* mixerHandler will update the samples inside Mixer */

	mh_loadPatch_DEPR_(isProject, fpath);

	/* take the patch name and update the main window's title */

	G_Patch_DEPR_.getName();
	gu_update_win_label(G_Patch_DEPR_.name);

	status->value(0.4f);  // progress status: 0.4
	//Fl::check();
	Fl::wait(0);

	G_Patch_DEPR_.readRecs();
	status->value(0.6f);  // progress status: 0.6
	//Fl::check();
	Fl::wait(0);

#ifdef WITH_VST
	int resPlugins = G_Patch_DEPR_.readPlugins();
	status->value(0.8f);  // progress status: 0.8
	//Fl::check();
	Fl::wait(0);
#endif

	/* this one is vital: let recorder recompute the actions' positions if
	 * the current samplerate != patch samplerate */

	recorder::updateSamplerate(G_Conf.samplerate, G_Patch_DEPR_.samplerate);

	/* update gui */

	gu_updateControls();

	status->value(1.0f);  // progress status: 1.0 (done)
	//Fl::check();
	Fl::wait(0);

	/* save patchPath by taking the last dir of the broswer, in order to
	 * reuse it the next time */

	G_Conf.setPath(G_Conf.patchPath, gDirname(fpath).c_str());

	gLog("[glue] patch %s loaded\n", fname);

#ifdef WITH_VST
	if (resPlugins != 1)
		gdAlert("Some VST plugins were not loaded successfully.");
#endif

	return res;
}


/* -------------------------------------------------------------------------- */


void __fillPatchGlobals__(const string &name)
{
	G_Patch.version      = G_VERSION_STR;
	G_Patch.versionMajor = G_VERSION_MAJOR;
	G_Patch.versionMinor = G_VERSION_MINOR;
	G_Patch.versionPatch = G_VERSION_PATCH;
	G_Patch.name         = name;
	G_Patch.bpm          = G_Mixer.bpm;
	G_Patch.bars         = G_Mixer.bars;
	G_Patch.beats        = G_Mixer.beats;
	G_Patch.quantize     = G_Mixer.quantize;
	G_Patch.masterVolIn  = G_Mixer.inVol;
  G_Patch.masterVolOut = G_Mixer.outVol;
  G_Patch.metronome    = G_Mixer.metronome;
}


/* -------------------------------------------------------------------------- */


void __fillPatchChannels__(bool isProject)
{
	for (unsigned i=0; i<G_Mixer.channels.size; i++) {
		G_Mixer.channels.at(i)->writePatch(i, isProject);
	}
}


/* -------------------------------------------------------------------------- */


void __fillPatchColumns__()
{
	for (unsigned i=0; i<mainWin->keyboard->getTotalColumns(); i++) {
		gColumn *gCol = mainWin->keyboard->getColumn(i);
		Patch::column_t pCol;
		pCol.index = gCol->getIndex();
		pCol.width = gCol->w();
		for (unsigned k=0; k<gCol->countChannels(); k++) {
			Channel *colChannel = gCol->getChannel(k);
			for (unsigned j=0; j<G_Mixer.channels.size; j++) {
				Channel *mixerChannel = G_Mixer.channels.at(j);
				if (colChannel == mixerChannel) {
					pCol.channels.add(mixerChannel->index);
					break;
				}
			}
		}
		G_Patch.columns.add(pCol);
	}
}


/* -------------------------------------------------------------------------- */


void __setProgressBar__(class gProgress *status, float v)
{
	status->value(v);
	//Fl::check();
	Fl::wait(0);
}


/* -------------------------------------------------------------------------- */


int glue_saveProject(const string &folderPath, const string &projName)
{
	if (!gMkdir(folderPath.c_str())) {
		gLog("[glue] unable to make project directory!\n");
		return 0;
	}

	/* copy all samples inside the folder. Takes and logical ones are saved
	 * via glue_saveSample() */

	for (unsigned i=0; i<G_Mixer.channels.size; i++) {

		if (G_Mixer.channels.at(i)->type == CHANNEL_MIDI)
			continue;

		SampleChannel *ch = (SampleChannel*) G_Mixer.channels.at(i);

		if (ch->wave == NULL)
			continue;

		/* update the new samplePath: everything now comes from the project
		 * folder (folderPath). Also remove any existing file. */

		string samplePath = folderPath + gGetSlash() + ch->wave->basename() + "." + ch->wave->extension();

		if (gFileExists(samplePath.c_str()))
			remove(samplePath.c_str());
		if (ch->save(samplePath.c_str()))
			ch->wave->pathfile = samplePath;
	}

	string gptcPath = folderPath + gGetSlash() + gStripExt(projName.c_str()) + ".gptc";
	glue_savePatch(gptcPath, projName, true); // true == it's a project

	return 1;
}
