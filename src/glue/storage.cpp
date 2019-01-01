/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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
#include "../core/mixerHandler.h"
#include "../core/channel.h"
#include "../core/recorderHandler.h"
#include "../core/pluginManager.h"
#include "../core/pluginHost.h"
#include "../core/plugin.h"
#include "../core/conf.h"
#include "../core/patch.h"
#include "../core/sampleChannel.h"
#include "../core/midiChannel.h"
#include "../core/waveManager.h"
#include "../core/clock.h"
#include "../core/wave.h"
#include "../utils/gui.h"
#include "../utils/log.h"
#include "../utils/string.h"
#include "../utils/fs.h"
#include "../gui/elems/basics/progress.h"
#include "../gui/elems/mainWindow/keyboard/column.h"
#include "../gui/elems/mainWindow/keyboard/keyboard.h"
#include "../gui/dialogs/gd_mainWindow.h"
#include "../gui/dialogs/gd_warnings.h"
#include "../gui/dialogs/browser/browserSave.h"
#include "../gui/dialogs/browser/browserLoad.h"
#include "main.h"
#include "channel.h"
#include "storage.h"


extern gdMainWindow *G_MainWin;


using std::string;
using std::vector;

namespace giada {
namespace c {
namespace storage
{
namespace
{
#ifdef WITH_VST

void fillPatchGlobalsPlugins_(std::vector<m::Plugin*> stack, 
	vector<m::patch::plugin_t>& patch)
{
	using namespace giada::m;

	for (const Plugin* plugin : stack) {
		patch::plugin_t ppl;
		ppl.path   = plugin->getUniqueId();
		ppl.bypass = plugin->isBypassed();
		for (int k=0; k<plugin->getNumParameters(); k++)
			ppl.params.push_back(plugin->getParameter(k));
		patch.push_back(ppl);
	}
}

#endif


/* -------------------------------------------------------------------------- */


void fillPatchColumns_()
{
	using namespace giada::m;

	for (unsigned i=0; i<G_MainWin->keyboard->getTotalColumns(); i++) {
		geColumn* gCol = G_MainWin->keyboard->getColumn(i);
		patch::column_t pCol;
		pCol.index = gCol->getIndex();
		pCol.width = gCol->w();
		for (int k=0; k<gCol->countChannels(); k++) {
			Channel* colChannel = gCol->getChannel(k);
			for (const Channel* mixerChannel : mixer::channels) {
				if (colChannel == mixerChannel) {
					pCol.channels.push_back(mixerChannel->index);
					break;
				}
			}
		}
		patch::columns.push_back(pCol);
	}
}


/* -------------------------------------------------------------------------- */


void fillPatchChannels_(bool isProject)
{
	using namespace giada::m;

	for (unsigned i=0; i<mixer::channels.size(); i++)
		mixer::channels.at(i)->writePatch(i, isProject);
}


/* -------------------------------------------------------------------------- */


void fillPatchGlobals_(const string& name)
{
	using namespace giada::m;

	patch::version      = G_VERSION_STR;
	patch::versionMajor = G_VERSION_MAJOR;
	patch::versionMinor = G_VERSION_MINOR;
	patch::versionPatch = G_VERSION_PATCH;
	patch::name         = name;
	patch::bpm          = clock::getBpm();
	patch::bars         = clock::getBars();
	patch::beats        = clock::getBeats();
	patch::quantize     = clock::getQuantize();
	patch::masterVolIn  = mixer::inVol;
	patch::masterVolOut = mixer::outVol;
	patch::metronome    = mixer::metronome;

#ifdef WITH_VST

	fillPatchGlobalsPlugins_(pluginHost::getStack(pluginHost::StackType::MASTER_IN),
			patch::masterInPlugins);
	fillPatchGlobalsPlugins_(pluginHost::getStack(pluginHost::StackType::MASTER_OUT),
			patch::masterOutPlugins);

#endif
}


/* -------------------------------------------------------------------------- */


bool savePatch_(const string& fullPath, const string& name, bool isProject)
{
	using namespace giada::m;

	patch::init();

	fillPatchGlobals_(name);
	fillPatchChannels_(isProject);
	fillPatchColumns_();

	if (patch::write(fullPath)) {
		gu_updateMainWinLabel(name);
		gu_log("[savePatch] patch saved as %s\n", fullPath.c_str());
		return true;
	}
	return false;
}


/* -------------------------------------------------------------------------- */


string makeSamplePath_(const string& base, const Wave& w, int k)
{
	return base + G_SLASH + w.getBasename(false) + "-" + gu_iToString(k) + "." +  w.getExtension();
} 


string makeUniqueSamplePath_(const string& base, const m::SampleChannel* ch)
{
	using namespace giada::m;

	string path = base + G_SLASH + ch->wave->getBasename(true);
	if (mh::uniqueSamplePath(ch, path))
		return path;

	int k = 0;
	path = makeSamplePath_(base, *ch->wave.get(), k);
	while (!mh::uniqueSamplePath(ch, path))
		path = makeSamplePath_(base, *ch->wave.get(), k++);
	return path;
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void savePatch(void* data)
{
	gdBrowserSave* browser = (gdBrowserSave*) data;
	string name            = gu_stripExt(browser->getName());
	string fullPath        = browser->getCurrentPath() + G_SLASH + name + ".gptc";

	if (name == "") {
		gdAlert("Please choose a file name.");
		return;
	}

	if (gu_fileExists(fullPath))
		if (!gdConfirmWin("Warning", "File exists: overwrite?"))
			return;

	if (savePatch_(fullPath, name, false)) {  // false == not a project
		m::conf::patchPath = gu_dirname(fullPath);
		browser->do_callback();
	}
	else
		gdAlert("Unable to save the patch!");
}


/* -------------------------------------------------------------------------- */


void loadPatch(void* data)
{
	using namespace giada::m;

	gdBrowserLoad* browser = (gdBrowserLoad*) data;
	string fullPath        = browser->getSelectedItem();
	bool isProject         = gu_isProject(browser->getSelectedItem());

	browser->showStatusBar();

	gu_log("[glue] loading %s...\n", fullPath.c_str());

	string fileToLoad = fullPath;  // patch file to read from
	string basePath   = "";        // base path, in case of reading from a project
	if (isProject) {
		fileToLoad = fullPath + G_SLASH + gu_stripExt(gu_basename(fullPath)) + ".gptc";
		basePath   = fullPath + G_SLASH;
	}

	int res = patch::read(fileToLoad);
	if (res != PATCH_READ_OK) {
		if (res == PATCH_UNREADABLE)
			isProject ? gdAlert("This project is unreadable.") : gdAlert("This patch is unreadable.");
		else
		if (res == PATCH_INVALID)
			isProject ? gdAlert("This project is not valid.") : gdAlert("This patch is not valid.");
		browser->hideStatusBar();
		return;
	}

	/* Close all other windows. This prevents problems if plugin windows are 
	open. */

	gu_closeAllSubwindows();

	/* Reset the system. False(1): don't update the gui right now. False(2): do 
	not create empty columns. */

	c::main::resetToInitState(false, false);

	browser->setStatusBar(0.1f);

	/* Add common stuff, columns and channels. Also increment the progress bar by 
	0.8 / total_channels steps. */

	float steps = 0.8 / patch::channels.size();
	
	for (const patch::column_t& col : patch::columns) {
		G_MainWin->keyboard->addColumn(col.width);
		for (const patch::channel_t& pch : patch::channels) {
			if (pch.column == col.index) {
				Channel* ch = c::channel::addChannel(pch.column, static_cast<ChannelType>(pch.type), pch.size);
				ch->readPatch(basePath, pch);
			}
			browser->setStatusBar(steps);
		}
	}

	/* Prepare Mixer and Recorder. The latter has to recompute the actions' 
	positions if the current samplerate != patch samplerate.*/

	mh::updateSoloCount();
	mh::readPatch();
	recorderHandler::updateSamplerate(conf::samplerate, patch::samplerate);

	/* Save patchPath by taking the last dir of the broswer, in order to reuse it 
	the next time. */

	conf::patchPath = gu_dirname(fullPath);

	/* Refresh GUI. */

	gu_updateControls();
	gu_updateMainWinLabel(patch::name);

	browser->setStatusBar(0.1f);

	gu_log("[glue] patch loaded successfully\n");

#ifdef WITH_VST

	if (pluginManager::hasMissingPlugins())
		gdAlert("Some plugins were not loaded successfully.\nCheck the plugin browser to know more.");

#endif

	browser->do_callback();
}


/* -------------------------------------------------------------------------- */


void saveProject(void* data)
{
	using namespace giada::m;

	gdBrowserSave* browser = (gdBrowserSave*) data;
	string name            = gu_stripExt(browser->getName());
	string folderPath      = browser->getCurrentPath();
	string fullPath        = folderPath + G_SLASH + name + ".gprj";

	if (name == "") {
		gdAlert("Please choose a project name.");
		return;
	}

	if (gu_isProject(fullPath) && !gdConfirmWin("Warning", "Project exists: overwrite?"))
		return;

	if (!gu_dirExists(fullPath) && !gu_mkdir(fullPath)) {
		gu_log("[saveProject] Unable to make project directory!\n");
		return;
	}

	gu_log("[saveProject] Project dir created: %s\n", fullPath.c_str());

	/* Copy all samples inside the folder. Takes and logical ones are saved via 
	saveSample(). Update the new sample path: everything now comes from the 
	project folder (folderPath). Also make sure the file path is unique inside the 
	project folder.*/

	for (const Channel* ch : mixer::channels) {

		if (ch->type == ChannelType::MIDI)
			continue;

		const SampleChannel* sch = static_cast<const SampleChannel*>(ch);

		if (sch->wave == nullptr)
			continue;

		sch->wave->setPath(makeUniqueSamplePath_(fullPath, sch));

		gu_log("[saveProject] Save file to %s\n", sch->wave->getPath().c_str());

		waveManager::save(sch->wave.get(), sch->wave->getPath()); // TODO - error checking	
	}

	string gptcPath = fullPath + G_SLASH + name + ".gptc";
	if (savePatch_(gptcPath, name, true)) // true == it's a project
		browser->do_callback();
	else
		gdAlert("Unable to save the project!");
}


/* -------------------------------------------------------------------------- */


void loadSample(void* data)
{
	gdBrowserLoad* browser = (gdBrowserLoad*) data;
	string fullPath        = browser->getSelectedItem();

	if (fullPath.empty())
		return;

	int res = c::channel::loadChannel(static_cast<m::SampleChannel*>(browser->getChannel()), 
		fullPath);

	if (res == G_RES_OK) {
		m::conf::samplePath = gu_dirname(fullPath);
		browser->do_callback();
		G_MainWin->delSubWindow(WID_SAMPLE_EDITOR); // if editor is open
	}
	else
		G_MainWin->keyboard->printChannelMessage(res);
}


/* -------------------------------------------------------------------------- */


void saveSample(void* data)
{
	using namespace giada::m;

	gdBrowserSave* browser = (gdBrowserSave*) data;
	string name            = browser->getName();
	string folderPath      = browser->getCurrentPath();

	if (name == "") {
		gdAlert("Please choose a file name.");
		return;
	}

	/* bruteforce check extension. */

	string filePath = folderPath + G_SLASH + gu_stripExt(name) + ".wav";

	if (gu_fileExists(filePath))
		if (!gdConfirmWin("Warning", "File exists: overwrite?"))
			return;

	SampleChannel* ch = static_cast<SampleChannel*>(browser->getChannel());

	if (waveManager::save(ch->wave.get(), filePath)) {
		gu_log("[saveSample] sample saved to %s\n", filePath.c_str());
		conf::samplePath = gu_dirname(filePath);
		browser->do_callback();
	}
	else
		gdAlert("Unable to save this sample!");
}

}}} // giada::c::storage::