/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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
#include "core/conf.h"
#include "core/engine.h"
#include "core/init.h"
#include "core/mixer.h"
#include "core/model/model.h"
#include "core/patch.h"
#include "core/plugins/plugin.h"
#include "core/plugins/pluginHost.h"
#include "core/plugins/pluginManager.h"
#include "core/sequencer.h"
#include "core/wave.h"
#include "core/waveFactory.h"
#include "glue/layout.h"
#include "glue/main.h"
#include "gui/dialogs/browser/browserLoad.h"
#include "gui/dialogs/browser/browserSave.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/warnings.h"
#include "gui/elems/basics/progress.h"
#include "gui/elems/mainWindow/keyboard/column.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/ui.h"
#include "src/core/actions/actionRecorder.h"
#include "storage.h"
#include "utils/fs.h"
#include "utils/gui.h"
#include "utils/log.h"
#include "utils/string.h"
#include <cassert>

extern giada::m::Engine g_engine;
extern giada::v::Ui     g_ui;

namespace giada::c::storage
{
void loadProject(void* data)
{
	v::gdBrowserLoad* browser = static_cast<v::gdBrowserLoad*>(data);

	const std::string projectPath = browser->getSelectedItem();
	const std::string patchPath   = u::fs::join(projectPath, u::fs::stripExt(u::fs::basename(projectPath)) + ".gptc");

	auto progress   = g_ui.mainWindow->getScopedProgress(g_ui.langMapper.get(v::LangMap::MESSAGE_STORAGE_LOADINGPROJECT));
	auto progressCb = [&p = progress.get()](float v) {
		p.setProgress(v);
	};

	/* Close all sub-windows first, in case there are VST editors visible. VST
	editors must be closed before deleting their plug-in processors. */

	g_ui.closeAllSubwindows();

	m::LoadState state = g_engine.load(projectPath, patchPath, progressCb);

	if (state.patch != G_FILE_OK)
	{
		if (state.patch == G_FILE_UNREADABLE)
			v::gdAlert(g_ui.langMapper.get(v::LangMap::MESSAGE_STORAGE_PATCHUNREADABLE));
		else if (state.patch == G_FILE_INVALID)
			v::gdAlert(g_ui.langMapper.get(v::LangMap::MESSAGE_STORAGE_PATCHINVALID));
		else if (state.patch == G_FILE_UNSUPPORTED)
			v::gdAlert(g_ui.langMapper.get(v::LangMap::MESSAGE_STORAGE_PATCHUNSUPPORTED));
		return;
	}

	/* Update UI. */

	g_ui.load(g_engine.patch.data);

	if (!state.isGood())
		layout::openMissingAssetsWindow(state);

	browser->do_callback();
}

/* -------------------------------------------------------------------------- */

void saveProject(void* data)
{
	v::gdBrowserSave* browser = static_cast<v::gdBrowserSave*>(data);

	const std::string projectName = u::fs::stripExt(browser->getName());
	const std::string projectPath = u::fs::join(browser->getCurrentPath(), projectName + ".gprj");
	const std::string patchPath   = u::fs::join(projectPath, projectName + ".gptc");

	if (projectName == "")
	{
		v::gdAlert(g_ui.langMapper.get(v::LangMap::MESSAGE_STORAGE_CHOOSEPROJECTNAME));
		return;
	}

	if (u::fs::dirExists(projectPath) &&
	    !v::gdConfirmWin(g_ui.langMapper.get(v::LangMap::COMMON_WARNING), g_ui.langMapper.get(v::LangMap::MESSAGE_STORAGE_PROJECTEXISTS)))
		return;

	auto progress   = g_ui.mainWindow->getScopedProgress(g_ui.langMapper.get(v::LangMap::MESSAGE_STORAGE_SAVINGPROJECT));
	auto progressCb = [&p = progress.get()](float v) {
		p.setProgress(v);
	};

	g_ui.store(projectName, g_engine.patch.data);

	if (!g_engine.store(projectName, projectPath, patchPath, progressCb))
	{
		v::gdAlert(g_ui.langMapper.get(v::LangMap::MESSAGE_STORAGE_SAVINGPROJECTERROR));
		return;
	}

	browser->do_callback();
}

/* -------------------------------------------------------------------------- */

void loadSample(void* data)
{
	v::gdBrowserLoad* browser  = static_cast<v::gdBrowserLoad*>(data);
	std::string       fullPath = browser->getSelectedItem();

	if (fullPath.empty())
		return;

	auto progress = g_ui.mainWindow->getScopedProgress(g_ui.langMapper.get(v::LangMap::MESSAGE_STORAGE_LOADINGSAMPLE));

	if (int res = c::channel::loadChannel(browser->getChannelId(), fullPath); res == G_RES_OK)
	{
		g_engine.conf.data.samplePath = u::fs::dirname(fullPath);
		browser->do_callback();
		g_ui.mainWindow->delSubWindow(WID_SAMPLE_EDITOR); // if editor is open
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
		v::gdAlert(g_ui.langMapper.get(v::LangMap::MESSAGE_STORAGE_CHOOSEFILENAME));
		return;
	}

	std::string filePath = u::fs::join(folderPath, u::fs::stripExt(name) + ".wav");

	if (u::fs::fileExists(filePath) &&
	    !v::gdConfirmWin(g_ui.langMapper.get(v::LangMap::COMMON_WARNING),
	        g_ui.langMapper.get(v::LangMap::MESSAGE_STORAGE_FILEEXISTS)))
		return;

	ID       waveId = g_engine.model.get().getChannel(channelId).samplePlayer->getWaveId();
	m::Wave* wave   = g_engine.model.findShared<m::Wave>(waveId);

	assert(wave != nullptr);

	if (!g_engine.waveFactory.save(*wave, filePath))
	{
		v::gdAlert(g_ui.langMapper.get(v::LangMap::MESSAGE_STORAGE_SAVINGFILEERROR));
		return;
	}

	u::log::print("[saveSample] sample saved to %s\n", filePath);

	/* Update last used path in conf, so that it can be reused next time. */

	g_engine.conf.data.samplePath = u::fs::dirname(filePath);

	/* Update logical and edited states in Wave. */

	m::model::DataLock lock = g_engine.model.lockData();
	wave->setLogical(false);
	wave->setEdited(false);

	/* Finally close the browser. */

	browser->do_callback();
}
} // namespace giada::c::storage