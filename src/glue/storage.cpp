/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/glue/storage.h"
#include "src/core/actions/ActionManager.h"
#include "src/core/channels/channel.h"
#include "src/core/conf.h"
#include "src/core/engine.h"
#include "src/core/init.h"
#include "src/core/mixer.h"
#include "src/core/model/model.h"
#include "src/core/patch.h"
#include "src/core/plugins/plugin.h"
#include "src/core/plugins/pluginHost.h"
#include "src/core/plugins/pluginManager.h"
#include "src/core/sequencer.h"
#include "src/core/wave.h"
#include "src/core/waveFactory.h"
#include "src/deps/mcl-utils/src/fs.hpp"
#include "src/glue/layout.h"
#include "src/glue/main.h"
#include "src/gui/dialogs/browser/browserLoad.h"
#include "src/gui/dialogs/browser/browserSave.h"
#include "src/gui/dialogs/mainWindow.h"
#include "src/gui/dialogs/warnings.h"
#include "src/gui/elems/basics/progress.h"
#include "src/gui/elems/mainWindow/keyboard/keyboard.h"
#include "src/gui/elems/mainWindow/keyboard/track.h"
#include "src/gui/ui.h"
#include "src/utils/gui.h"
#include "src/utils/fs.h"
#include "src/utils/log.h"
#include "src/utils/string.h"
#include <cassert>
#include <filesystem>

extern giada::m::Engine* g_engine;
extern giada::v::Ui*     g_ui;

namespace utils = mcl::utils;

namespace giada::c::storage
{
namespace
{
void printLoadError_(int res)
{
	if (res == G_FILE_UNREADABLE)
		v::gdAlert(g_ui->getI18Text(v::LangMap::MESSAGE_STORAGE_PATCHUNREADABLE));
	else if (res == G_FILE_INVALID)
		v::gdAlert(g_ui->getI18Text(v::LangMap::MESSAGE_STORAGE_PATCHINVALID));
	else if (res == G_FILE_UNSUPPORTED)
		v::gdAlert(g_ui->getI18Text(v::LangMap::MESSAGE_STORAGE_PATCHUNSUPPORTED));
}

/* -------------------------------------------------------------------------- */

bool validateFileName_(const std::string& name)
{
	if (name == "")
	{
		v::gdAlert(g_ui->getI18Text(v::LangMap::MESSAGE_STORAGE_CHOOSEFILENAME));
		return false;
	}
	if (!utils::fs::isValidFileName(name))
	{
		v::gdAlert(g_ui->getI18Text(v::LangMap::MESSAGE_STORAGE_FILEHASINVALIDCHARS));
		return false;
	}
	return true;
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void loadProject(void* data)
{
	v::gdBrowserLoad* browser = static_cast<v::gdBrowserLoad*>(data);
	loadProjectPath(browser->getSelectedItem());
}

void loadProjectPath(const std::string& projectPath)
{

	/* Close all plug-ins sub-windows first, in case there are VST editors visible.
	VST editors must be closed before deleting their plug-in processors. */

	g_ui->closeAllPluginsEditors();

	/* Stop the UI updater: the current model is about to change, but the UI
	still retains the old one. Updating during this stage would trigger
	assertions/crashes. */

	g_ui->stopUpdater();

	auto uiProgress     = g_ui->mainWindow->getScopedProgress(g_ui->getI18Text(v::LangMap::MESSAGE_STORAGE_LOADINGPROJECT));
	auto engineProgress = [&uiProgress](float v)
	{ uiProgress.setProgress(v); };

	m::model::LoadState state = g_engine->getStorageApi().loadProject(projectPath, engineProgress);

	if (state.patch.status != G_FILE_OK)
	{
		printLoadError_(state.patch.status);
		g_ui->startUpdater();
		return;
	}

	g_ui->model.patchPath = utils::fs::getUpDir(projectPath);
	g_ui->load(state.patch);
	g_ui->startUpdater();

	if (!state.isGood())
		layout::openMissingAssetsWindow(state);
}

/* -------------------------------------------------------------------------- */

void saveProject(void* data)
{
	v::gdBrowserSave* browser = static_cast<v::gdBrowserSave*>(data);
	if (saveProjectPath(browser->getCurrentPath(), browser->getName()))
		browser->do_callback();
}

bool saveProjectPath(const std::string& directory, const std::string& name)
{
	// Validate before joining so a typed path cannot escape the chosen folder.
	std::string projectName = u::fs::isProject(directory)
	    ? utils::fs::stripExt(utils::fs::basename(directory)) : name;
	if (utils::fs::getExt(projectName) == G_PROJECT_EXT)
		projectName = utils::fs::stripExt(projectName);

	if (!validateFileName_(projectName))
		return false;
	const std::string projectPath = u::fs::getProjectSavePath(directory, projectName);

	std::error_code pathError;
	const bool exists = std::filesystem::exists(projectPath, pathError);
	if (pathError)
	{
		v::gdAlert(g_ui->getI18Text(v::LangMap::MESSAGE_STORAGE_SAVINGPROJECTERROR));
		return false;
	}
	if (exists &&
	    !v::gdConfirmWin(g_ui->getI18Text(v::LangMap::COMMON_WARNING),
	        g_ui->getI18Text(v::LangMap::MESSAGE_STORAGE_PROJECTEXISTS)))
		return false;

	auto uiProgress     = g_ui->mainWindow->getScopedProgress(g_ui->getI18Text(v::LangMap::MESSAGE_STORAGE_SAVINGPROJECT));
	auto engineProgress = [&uiProgress](float v)
	{ uiProgress.setProgress(v); };

	const std::string previousName = g_ui->model.projectName;
	g_ui->model.projectName = projectName;

	bool saved = false;
	try
	{
		saved = g_engine->getStorageApi().storeProject(projectPath, g_ui->model, engineProgress);
	}
	catch (const std::filesystem::filesystem_error& error)
	{
		u::log::print("[saveProjectPath] {}\n", error.what());
	}
	if (saved)
	{
		g_ui->mainWindow->setProjectTitle(projectName);
		g_ui->model.patchPath = utils::fs::getUpDir(projectPath);
		return true;
	}
	g_ui->model.projectName = previousName;
	v::gdAlert(g_ui->getI18Text(v::LangMap::MESSAGE_STORAGE_SAVINGPROJECTERROR));
	return false;
}

/* -------------------------------------------------------------------------- */

void loadSample(void* data)
{
	v::gdBrowserLoad* browser  = static_cast<v::gdBrowserLoad*>(data);
	std::string       fullPath = browser->getSelectedItem();
	loadSamplePath(browser->getChannelId(), fullPath);
	if (!fullPath.empty())
		browser->do_callback();
}

void loadSamplePath(ID channelId, const std::string& fullPath)
{
	if (fullPath.empty())
		return;

	g_ui->model.samplePath = utils::fs::dirname(fullPath);

	c::channel::loadChannel(channelId, fullPath);
}
} // namespace giada::c::storage
