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


#include <cassert>
#include "core/model/model.h"
#include "core/channels/channel.h"
#include "core/channels/sampleChannel.h"
#include "core/channels/midiChannel.h"
#include "core/mixer.h"
#include "core/wave.h"
#include "core/mixerHandler.h"
#include "core/recorderHandler.h"
#include "core/pluginManager.h"
#include "core/pluginHost.h"
#include "core/plugin.h"
#include "core/conf.h"
#include "core/patch.h"
#include "core/waveManager.h"
#include "core/clock.h"
#include "core/wave.h"
#include "utils/gui.h"
#include "utils/log.h"
#include "utils/string.h"
#include "utils/fs.h"
#include "gui/elems/basics/progress.h"
#include "gui/elems/mainWindow/keyboard/column.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/warnings.h"
#include "gui/dialogs/browser/browserSave.h"
#include "gui/dialogs/browser/browserLoad.h"
#include "main.h"
#include "channel.h"
#include "storage.h"


extern giada::v::gdMainWindow* G_MainWin;


namespace giada {
namespace c {
namespace storage
{
namespace
{
std::string makeSamplePath_(const std::string& base, const m::Wave& w, int k)
{
	return base + G_SLASH + w.getBasename(false) + "-" + std::to_string(k) + "." +  w.getExtension();
} 


std::string makeUniqueSamplePath_(const std::string& base, const m::SampleChannel* ch)
{
	assert(false);
/*
	using namespace giada::m;

	std::string path = base + G_SLASH + ch->wave->getBasename(true);
	if (mh::uniqueSamplePath(ch, path))
		return path;

	int k = 0;
	path = makeSamplePath_(base, *ch->wave, k);
	while (!mh::uniqueSamplePath(ch, path))
		path = makeSamplePath_(base, *ch->wave, k++);
	return path;*/
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void savePatch(void* data)
{
	v::gdBrowserSave* browser = (v::gdBrowserSave*) data;
	std::string name          = gu_stripExt(browser->getName());
	std::string fullPath      = browser->getCurrentPath() + G_SLASH + name + ".gptc";

	if (name == "") {
		v::gdAlert("Please choose a file name.");
		return;
	}

	if (gu_fileExists(fullPath))
		if (!v::gdConfirmWin("Warning", "File exists: overwrite?"))
			return;

	if (m::patch::write(name, fullPath, /*isProject=*/false)) {
		u::gui::updateMainWinLabel(name);
		m::conf::patchPath = gu_dirname(fullPath);
		browser->do_callback();
		gu_log("[savePatch] patch saved as %s\n", fullPath.c_str());
	}
	else
		v::gdAlert("Unable to save the patch!");
}


/* -------------------------------------------------------------------------- */


void loadPatch(void* data)
{
	assert(false);
#if 0
	using namespace giada::m;

	v::gdBrowserLoad* browser = (v::gdBrowserLoad*) data;
	std::string fullPath      = browser->getSelectedItem();
	bool isProject            = gu_isProject(browser->getSelectedItem());

	browser->showStatusBar();

	gu_log("[glue] loading %s...\n", fullPath.c_str());

	std::string fileToLoad = fullPath;  // patch file to read from
	std::string basePath   = "";        // base path, in case of reading from a project
	if (isProject) {
		fileToLoad = fullPath + G_SLASH + gu_stripExt(gu_basename(fullPath)) + ".gptc";
		basePath   = fullPath + G_SLASH;
	}

	int res = patch::read(fileToLoad);
	if (res != PATCH_READ_OK) {
		if (res == PATCH_UNREADABLE)
			isProject ? v::gdAlert("This project is unreadable.") : v::gdAlert("This patch is unreadable.");
		else
		if (res == PATCH_INVALID)
			isProject ? v::gdAlert("This project is not valid.") : v::gdAlert("This patch is not valid.");
		browser->hideStatusBar();
		return;
	}

	/* Close all other windows. This prevents problems if plugin windows are 
	open. */

	u::gui::closeAllSubwindows();

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

	u::gui::updateControls();
	u::gui::updateMainWinLabel(patch::name);

	browser->setStatusBar(0.1f);

	gu_log("[glue] patch loaded successfully\n");

#ifdef WITH_VST

	if (pluginManager::hasMissingPlugins())
		v::gdAlert("Some plugins were not loaded successfully.\nCheck the plugin browser to know more.");

#endif

	browser->do_callback();
#endif
}


/* -------------------------------------------------------------------------- */


void saveProject(void* data)
{
	assert(false);
#if 0
	using namespace giada::m;

	v::gdBrowserSave* browser = (v::gdBrowserSave*) data;
	string name            = gu_stripExt(browser->getName());
	string folderPath      = browser->getCurrentPath();
	string fullPath        = folderPath + G_SLASH + name + ".gprj";

	if (name == "") {
		v::gdAlert("Please choose a project name.");
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

		waveManager::save(sch->wave, sch->wave->getPath()); // TODO - error checking	
	}

	string gptcPath = fullPath + G_SLASH + name + ".gptc";
	if (savePatch_(gptcPath, name, true)) // true == it's a project
		browser->do_callback();
	else
		v::gdAlert("Unable to save the project!");
#endif
}


/* -------------------------------------------------------------------------- */


void loadSample(void* data)
{
	v::gdBrowserLoad* browser  = (v::gdBrowserLoad*) data;
	std::string       fullPath = browser->getSelectedItem();

	if (fullPath.empty())
		return;

	int res = c::channel::loadChannel(browser->getChannelId(), fullPath);

	if (res == G_RES_OK) {
		m::conf::samplePath = gu_dirname(fullPath);
		browser->do_callback();
		G_MainWin->delSubWindow(WID_SAMPLE_EDITOR); // if editor is open
	}
}


/* -------------------------------------------------------------------------- */


void saveSample(void* data)
{
	v::gdBrowserSave* browser = (v::gdBrowserSave*) data;
	std::string name          = browser->getName();
	std::string folderPath    = browser->getCurrentPath();

	if (name == "") {
		v::gdAlert("Please choose a file name.");
		return;
	}

	std::string filePath = folderPath + G_SLASH + gu_stripExt(name) + ".wav";

	if (gu_fileExists(filePath) && !v::gdConfirmWin("Warning", "File exists: overwrite?"))
		return;

	ID waveId;
	m::model::onGet(m::model::channels, browser->getChannelId(), [&](m::Channel& c)
	{
		waveId = static_cast<m::SampleChannel&>(c).waveId;
	});

	size_t waveIndex = m::model::getIndex(m::model::waves, waveId);

	std::unique_ptr<m::Wave> wave = m::model::waves.clone(waveIndex);

	if (!m::waveManager::save(*wave.get(), filePath)) {
		v::gdAlert("Unable to save this sample!");
		return;
	}
	
	gu_log("[saveSample] sample saved to %s\n", filePath.c_str());
	
	/* Update last used path in conf, so that it can be reused next time. */

	m::conf::samplePath = gu_dirname(filePath);

	/* Update logical and edited states in Wave. */

	wave->setLogical(false);
	wave->setEdited(false);

	m::model::waves.swap(std::move(wave), waveIndex);

	/* Finally close the browser. */

	browser->do_callback();
}
}}} // giada::c::storage::
