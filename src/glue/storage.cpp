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


#include <cassert>
#include "core/model/model.h"
#include "core/model/storage.h"
#include "core/mixer.h"
#include "core/wave.h"
#include "core/mixerHandler.h"
#include "core/recorderHandler.h"
#include "core/plugins/pluginManager.h"
#include "core/plugins/pluginHost.h"
#include "core/plugins/plugin.h"
#include "core/conf.h"
#include "core/patch.h"
#include "core/init.h"
#include "core/waveManager.h"
#include "core/clock.h"
#include "core/wave.h"
#include "utils/gui.h"
#include "utils/log.h"
#include "utils/string.h"
#include "utils/fs.h"
#include "gui/model.h"
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
std::string makeWavePath_(const std::string& base, const m::Wave& w, int k)
{
	return base + G_SLASH + w.getBasename(/*ext=*/false) + "-" + std::to_string(k) + "." +  w.getExtension();
} 


bool isWavePathUnique_(const m::Wave& skip, const std::string& path)
{
	m::model::WavesLock l(m::model::waves);

	for (const m::Wave* w : m::model::waves)
		if (w->id != skip.id && w->getPath() == path)
			return false;
	return true;
}

std::string makeUniqueWavePath_(const std::string& base, const m::Wave& w)
{
	std::string path = base + G_SLASH + w.getBasename(/*ext=*/true);
	if (isWavePathUnique_(w, path))
		return path;

	// TODO - just use a timestamp. e.g. makeWavePath_(..., ..., getTimeStamp())
	int k = 0;
	path = makeWavePath_(base, w, k);
	while (!isWavePathUnique_(w, path))
		path = makeWavePath_(base, w, k++);
	
	return path;
}


/* -------------------------------------------------------------------------- */


bool savePatch_(const std::string& path, const std::string& name)
{
	m::patch::init();
	m::patch::patch.name = name;
	m::model::store(m::patch::patch);
	v::model::store(m::patch::patch);

	if (!m::patch::write(path))
		return false;

	u::gui::updateMainWinLabel(name);
	m::conf::conf.patchPath = u::fs::getUpDir(u::fs::getUpDir(path));
	u::log::print("[savePatch] patch saved as %s\n", path);

	return true;
}


/* -------------------------------------------------------------------------- */


void saveWavesToProject_(const std::string& basePath)
{
	/* No need for a hard Wave swap here: nobody is reading the path data. */

	m::model::WavesLock l(m::model::waves);

	for (m::Wave* w : m::model::waves) {
		w->setPath(makeUniqueWavePath_(basePath, *w));
		m::waveManager::save(*w, w->getPath()); // TODO - error checking			
	}
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void loadProject(void* data)
{
	v::gdBrowserLoad* browser = static_cast<v::gdBrowserLoad*>(data);
	std::string fullPath      = browser->getSelectedItem();
	bool isProject            = u::fs::isProject(browser->getSelectedItem());

	browser->showStatusBar();

	u::log::print("[loadProject] load from %s\n", fullPath);

	std::string fileToLoad = fullPath;  // patch file to read from
	std::string basePath   = "";        // base path, in case of reading from a project
	if (isProject) {
		fileToLoad = fullPath + G_SLASH + u::fs::stripExt(u::fs::basename(fullPath)) + ".gptc";
		basePath   = fullPath + G_SLASH;
	}

	/* Read the patch from file. */

	m::patch::init();
	int res = m::patch::read(fileToLoad, basePath);
	if (res != G_PATCH_OK) {
		if (res == G_PATCH_UNREADABLE)
			v::gdAlert("This patch is unreadable.");
		else
		if (res == G_PATCH_INVALID)
			v::gdAlert("This patch is not valid.");
		else
		if (res == G_PATCH_UNSUPPORTED)
			v::gdAlert("This patch format is no longer supported.");
		browser->hideStatusBar();
		return;
	}	

	if (!isProject)
		v::gdAlert("Support for raw patches is deprecated\nand will be removed soon!");

	/* Then reset the system (it disables mixer) and fill the model. */

	m::init::reset();
	m::model::load(m::patch::patch);
	v::model::load(m::patch::patch);

	/* Prepare the engine. Recorder has to recompute the actions positions if 
	the current samplerate != patch samplerate. Clock needs to update frames
	in sequencer. */

	m::mh::updateSoloCount();
	m::recorderHandler::updateSamplerate(m::conf::conf.samplerate, m::patch::patch.samplerate);
	m::clock::recomputeFrames();
	m::mixer::allocRecBuffer(m::clock::getFramesInLoop());

	/* Mixer is ready to go back online. */

	m::mixer::enable();

	/* Utilities and cosmetics. Save patchPath by taking the last dir of the 
	broswer, in order to reuse it the next time. Also update UI. */

	m::conf::conf.patchPath = u::fs::dirname(fullPath);
	u::gui::updateMainWinLabel(m::patch::patch.name);

#ifdef WITH_VST

	if (m::pluginManager::hasMissingPlugins())
		v::gdAlert("Some plugins were not loaded successfully.\nCheck the plugin browser to know more.");

#endif

	browser->do_callback();
}


/* -------------------------------------------------------------------------- */


void saveProject(void* data)
{
	v::gdBrowserSave* browser = static_cast<v::gdBrowserSave*>(data);
	std::string name          = u::fs::stripExt(browser->getName());
	std::string folderPath    = browser->getCurrentPath();
	std::string fullPath      = folderPath + G_SLASH + name + ".gprj";
	std::string gptcPath      = fullPath + G_SLASH + name + ".gptc";

	if (name == "") {
		v::gdAlert("Please choose a project name.");
		return;
	}

	if (u::fs::dirExists(fullPath) && !v::gdConfirmWin("Warning", "Project exists: overwrite?"))
		return;

	if (!u::fs::mkdir(fullPath)) {
		u::log::print("[saveProject] Unable to make project directory!\n");
		return;
	}

	u::log::print("[saveProject] Project dir created: %s\n", fullPath);

	saveWavesToProject_(fullPath);

	if (savePatch_(gptcPath, name))
		browser->do_callback();
	else
		v::gdAlert("Unable to save the project!");

}


/* -------------------------------------------------------------------------- */


void loadSample(void* data)
{
	v::gdBrowserLoad* browser  = static_cast<v::gdBrowserLoad*>(data);
	std::string       fullPath = browser->getSelectedItem();

	if (fullPath.empty())
		return;

	int res = c::channel::loadChannel(browser->getChannelId(), fullPath);

	if (res == G_RES_OK) {
		m::conf::conf.samplePath = u::fs::dirname(fullPath);
		browser->do_callback();
		G_MainWin->delSubWindow(WID_SAMPLE_EDITOR); // if editor is open
	}
}


/* -------------------------------------------------------------------------- */


void saveSample(void* data)
{
	v::gdBrowserSave* browser = static_cast<v::gdBrowserSave*>(data);
	std::string name          = browser->getName();
	std::string folderPath    = browser->getCurrentPath();
	ID channelId              = browser->getChannelId();

	if (name == "") {
		v::gdAlert("Please choose a file name.");
		return;
	}

	std::string filePath = folderPath + G_SLASH + u::fs::stripExt(name) + ".wav";

	if (u::fs::fileExists(filePath) && !v::gdConfirmWin("Warning", "File exists: overwrite?"))
		return;

	ID waveId;
	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c)
	{
		waveId = c.samplePlayer->getWaveId();
	});

	std::size_t waveIndex = m::model::getIndex(m::model::waves, waveId);

	std::unique_ptr<m::Wave> wave = m::model::waves.clone(waveIndex);

	if (!m::waveManager::save(*wave.get(), filePath)) {
		v::gdAlert("Unable to save this sample!");
		return;
	}

	u::log::print("[saveSample] sample saved to %s\n", filePath);

	/* Update last used path in conf, so that it can be reused next time. */

	m::conf::conf.samplePath = u::fs::dirname(filePath);

	/* Update logical and edited states in Wave. */

	wave->setLogical(false);
	wave->setEdited(false);

	m::model::waves.swap(std::move(wave), waveIndex);

	/* Finally close the browser. */

	browser->do_callback();
}
}}} // giada::c::storage::
