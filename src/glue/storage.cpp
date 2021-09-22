/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#include "core/model/storage.h"
#include "channel.h"
#include "core/clock.h"
#include "core/conf.h"
#include "core/init.h"
#include "core/mixer.h"
#include "core/mixerHandler.h"
#include "core/model/model.h"
#include "core/patch.h"
#include "core/plugins/plugin.h"
#include "core/plugins/pluginHost.h"
#include "core/plugins/pluginManager.h"
#include "core/recorderHandler.h"
#include "core/wave.h"
#include "core/waveManager.h"
#include "gui/dialogs/browser/browserLoad.h"
#include "gui/dialogs/browser/browserSave.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/warnings.h"
#include "gui/elems/basics/progress.h"
#include "gui/elems/mainWindow/keyboard/column.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/model.h"
#include "main.h"
#include "storage.h"
#include "utils/fs.h"
#include "utils/gui.h"
#include "utils/log.h"
#include "utils/string.h"
#include <cassert>

extern giada::v::gdMainWindow* G_MainWin;

namespace giada
{
namespace c
{
namespace storage
{
namespace
{
std::string makeWavePath_(const std::string& base, const m::Wave& w, int k)
{
	return base + G_SLASH + w.getBasename(/*ext=*/false) + "-" + std::to_string(k) + w.getExtension();
}

bool isWavePathUnique_(const m::Wave& skip, const std::string& path)
{
	for (const auto& w : m::model::getAll<m::model::WavePtrs>())
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
	path  = makeWavePath_(base, w, k);
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
	for (const std::unique_ptr<m::Wave>& w : m::model::getAll<m::model::WavePtrs>())
	{
		w->setPath(makeUniqueWavePath_(basePath, *w));
		m::waveManager::save(*w, w->getPath()); // TODO - error checking
	}
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void loadProject(void* data)
{
	v::gdBrowserLoad* browser  = static_cast<v::gdBrowserLoad*>(data);
	std::string       fullPath = browser->getSelectedItem();

	browser->showStatusBar();

	u::log::print("[loadProject] load from %s\n", fullPath);

	std::string fileToLoad = fullPath + G_SLASH + u::fs::stripExt(u::fs::basename(fullPath)) + ".gptc";
	std::string basePath   = fullPath + G_SLASH;

	/* Read the patch from file. */

	m::patch::init();
	int res = m::patch::read(fileToLoad, basePath);
	if (res != G_PATCH_OK)
	{
		if (res == G_PATCH_UNREADABLE)
			v::gdAlert("This patch is unreadable.");
		else if (res == G_PATCH_INVALID)
			v::gdAlert("This patch is not valid.");
		else if (res == G_PATCH_UNSUPPORTED)
			v::gdAlert("This patch format is no longer supported.");
		browser->hideStatusBar();
		return;
	}

	/* Then reset the system (it disables mixer) and fill the model. */

	m::init::reset();
	v::model::load(m::patch::patch);
	m::model::load(m::patch::patch);

	/* Prepare the engine. Recorder has to recompute the actions positions if 
	the current samplerate != patch samplerate. Clock needs to update frames
	in sequencer. */

	m::mh::updateSoloCount();
	m::recorderHandler::updateSamplerate(m::conf::conf.samplerate, m::patch::patch.samplerate);
	m::clock::recomputeFrames();
	m::mixer::allocRecBuffer(m::clock::getMaxFramesInLoop());

	/* Mixer is ready to go back online. */

	m::mixer::enable();

	/* Utilities and cosmetics. Save patchPath by taking the last dir of the 
	browser, in order to reuse it the next time. Also update UI. */

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
	v::gdBrowserSave* browser    = static_cast<v::gdBrowserSave*>(data);
	std::string       name       = u::fs::stripExt(browser->getName());
	std::string       folderPath = browser->getCurrentPath();
	std::string       fullPath   = folderPath + G_SLASH + name + ".gprj";
	std::string       gptcPath   = fullPath + G_SLASH + name + ".gptc";

	if (name == "")
	{
		v::gdAlert("Please choose a project name.");
		return;
	}

	if (u::fs::dirExists(fullPath) && !v::gdConfirmWin("Warning", "Project exists: overwrite?"))
		return;

	if (!u::fs::mkdir(fullPath))
	{
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

	if (res == G_RES_OK)
	{
		m::conf::conf.samplePath = u::fs::dirname(fullPath);
		browser->do_callback();
		G_MainWin->delSubWindow(WID_SAMPLE_EDITOR); // if editor is open
	}
}

/* -------------------------------------------------------------------------- */

void saveSample(void* data)
{
	v::gdBrowserSave* browser    = static_cast<v::gdBrowserSave*>(data);
	std::string       name       = browser->getName();
	std::string       folderPath = browser->getCurrentPath();
	ID                channelId  = browser->getChannelId();

	if (name == "")
	{
		v::gdAlert("Please choose a file name.");
		return;
	}

	std::string filePath = folderPath + G_SLASH + u::fs::stripExt(name) + ".wav";

	if (u::fs::fileExists(filePath) && !v::gdConfirmWin("Warning", "File exists: overwrite?"))
		return;

	ID       waveId = m::model::get().getChannel(channelId).samplePlayer->getWaveId();
	m::Wave* wave   = m::model::find<m::Wave>(waveId);

	assert(wave != nullptr);

	if (!m::waveManager::save(*wave, filePath))
	{
		v::gdAlert("Unable to save this sample!");
		return;
	}

	u::log::print("[saveSample] sample saved to %s\n", filePath);

	/* Update last used path in conf, so that it can be reused next time. */

	m::conf::conf.samplePath = u::fs::dirname(filePath);

	/* Update logical and edited states in Wave. */

	m::model::DataLock lock;
	wave->setLogical(false);
	wave->setEdited(false);

	/* Finally close the browser. */

	browser->do_callback();
}
} // namespace storage
} // namespace c
} // namespace giada
