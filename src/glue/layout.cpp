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

#include "src/glue/layout.h"
#include "src/core/conf.h"
#include "src/core/engine.h"
#include "src/core/mixer.h"
#include "src/core/patch.h"
#include "src/core/sequencer.h"
#include "src/glue/channel.h"
#include "src/glue/config.h"
#include "src/glue/io.h"
#include "src/glue/storage.h"
#include "src/gui/dialogs/about.h"
#include "src/gui/dialogs/actionEditor/midiActionEditor.h"
#include "src/gui/dialogs/actionEditor/sampleActionEditor.h"
#include "src/gui/dialogs/beatsInput.h"
#include "src/gui/dialogs/bpmInput.h"
#include "src/gui/dialogs/browser/browserDir.h"
#include "src/gui/dialogs/browser/browserLoad.h"
#include "src/gui/dialogs/browser/browserSave.h"
#include "src/gui/dialogs/channelNameInput.h"
#include "src/gui/dialogs/channelRouting.h"
#include "src/gui/dialogs/config.h"
#include "src/gui/dialogs/keyGrabber.h"
#include "src/gui/dialogs/mainWindow.h"
#include "src/gui/dialogs/midiIO/midiInputChannel.h"
#include "src/gui/dialogs/midiIO/midiInputMaster.h"
#include "src/gui/dialogs/midiIO/midiOutputMidiCh.h"
#include "src/gui/dialogs/midiIO/midiOutputSampleCh.h"
#include "src/gui/dialogs/missingAssets.h"
#include "src/gui/dialogs/pluginChooser.h"
#include "src/gui/dialogs/pluginList.h"
#include "src/gui/dialogs/sampleEditor.h"
#include "src/gui/dialogs/warnings.h"
#include "src/gui/ui.h"
#include "src/utils/fs.h"
#include "src/gui/dialogs/browser/nativeProjectDialog.h"
#ifdef _WIN32
#include <FL/x.H>
#endif

extern giada::v::Ui*     g_ui;
extern giada::m::Engine* g_engine;

namespace giada::c::layout
{
namespace
{
void openNativeProjectDialog_(bool save)
{
	std::string directory = g_ui->model.patchPath;
	std::string name = g_ui->model.projectName.empty() ? "Untitled" : g_ui->model.projectName;
	for (;;)
	{
		void* owner = nullptr;
#ifdef _WIN32
		owner = reinterpret_cast<void*>(fl_xid(g_ui->mainWindow.get()));
#endif
		const auto selection = v::showNativeProjectDialog(save,
		    owner,
		    g_ui->getI18Text(save ? v::LangMap::BROWSER_SAVEPROJECT : v::LangMap::BROWSER_OPENPROJECT),
		    g_ui->getI18Text(save ? v::LangMap::COMMON_SAVE : v::LangMap::COMMON_LOAD),
		    directory, name, g_ui->getI18Text(v::LangMap::BROWSER_PROJECTNAME),
		    g_ui->getI18Text(save ? v::LangMap::BROWSER_SAVEIN : v::LangMap::BROWSER_PROJECTFOLDER));
		if (selection.status == v::NativeProjectSelection::Status::CANCELLED)
			return;
		if (selection.status == v::NativeProjectSelection::Status::FAILED_TO_OPEN)
		{
			v::gdAlert(g_ui->getI18Text(v::LangMap::BROWSER_SYSTEMDIALOGERROR));
			return;
		}
		directory = selection.directory;
		name = selection.name;
		if (save)
		{
			if (c::storage::saveProjectPath(directory, name))
				return;
		}
		else if (u::fs::isProject(directory))
		{
			c::storage::loadProjectPath(directory);
			return;
		}
		else
			v::gdAlert(g_ui->getI18Text(v::LangMap::BROWSER_SELECTPROJECT));
	}
}
} // namespace

void openBrowserForProjectLoad()
{
	openNativeProjectDialog_(false);
}

/* -------------------------------------------------------------------------- */

void openBrowserForProjectSave()
{
	openNativeProjectDialog_(true);
}

/* -------------------------------------------------------------------------- */

void openBrowserForSampleLoad(ID channelId)
{
	if (g_ui->model.useNativeSampleDialogs)
	{
		void* owner = nullptr;
#ifdef _WIN32
		owner = reinterpret_cast<void*>(fl_xid(g_ui->mainWindow.get()));
#endif
		const auto selection = v::showNativeSampleDialog(owner,
		    g_ui->getI18Text(v::LangMap::BROWSER_OPENSAMPLE),
		    g_ui->getI18Text(v::LangMap::COMMON_LOAD), g_ui->model.samplePath);
		if (selection.status == v::NativeSampleSelection::Status::SELECTED)
			c::storage::loadSamplePath(channelId, selection.path);
		else if (selection.status == v::NativeSampleSelection::Status::FAILED_TO_OPEN)
			v::gdAlert(g_ui->getI18Text(v::LangMap::BROWSER_SYSTEMDIALOGERROR));
		return;
	}
	v::gdWindow* w = new v::gdBrowserLoad(g_ui->getI18Text(v::LangMap::BROWSER_OPENSAMPLE),
	    g_ui->model.samplePath, c::storage::loadSample, channelId, g_ui->model, true);
	g_ui->openSubWindow(w);
}

/* -------------------------------------------------------------------------- */

void openAboutWindow()
{
	g_ui->openSubWindow(new v::gdAbout());
}

/* -------------------------------------------------------------------------- */

void openKeyGrabberWindow(int key, std::function<bool(int)> f)
{
	v::gdKeyGrabber* keyGrabber = new v::gdKeyGrabber(key);

	keyGrabber->onSetKey = f;

	g_ui->openSubWindow(keyGrabber);
}

/* -------------------------------------------------------------------------- */

void openBpmWindow(float bpm)
{
	g_ui->openSubWindow(new v::gdBpmInput(bpm));
}

/* -------------------------------------------------------------------------- */

void openBeatsWindow(TimeSignature t)
{
	g_ui->openSubWindow(new v::gdBeatsInput(t));
}

/* -------------------------------------------------------------------------- */

void openConfigWindow()
{
	g_ui->openSubWindow(new v::gdConfig(g_ui->model));
}

/* -------------------------------------------------------------------------- */

void openMasterMidiInputWindow()
{
	g_ui->openSubWindow(new v::gdMidiInputMaster(g_ui->model));
}

/* -------------------------------------------------------------------------- */

void openChannelMidiInputWindow(ID channelId)
{
	g_ui->openSubWindow(new v::gdMidiInputChannel(channelId, g_ui->model));
}

/* -------------------------------------------------------------------------- */

void openChannelRoutingWindow(ID channelId)
{
	g_ui->openSubWindow(new v::gdChannelRouting(channelId, g_ui->model));
}

/* -------------------------------------------------------------------------- */

void openSampleChannelMidiOutputWindow(ID channelId)
{
	g_ui->openSubWindow(new v::gdMidiOutputSampleCh(channelId));
}

/* -------------------------------------------------------------------------- */

void openMidiChannelMidiOutputWindow(ID channelId)
{
	g_ui->openSubWindow(new v::gdMidiOutputMidiCh(channelId));
}

/* -------------------------------------------------------------------------- */

void openSampleActionEditor(ID channelId)
{
	g_ui->openSubWindow(new v::gdSampleActionEditor(channelId, g_ui->model));
}

/* -------------------------------------------------------------------------- */

void openMidiActionEditor(ID channelId)
{
	g_ui->openSubWindow(new v::gdMidiActionEditor(channelId, g_ui->model));
}

/* -------------------------------------------------------------------------- */

void openSampleEditor(ID channelId)
{
	/* Close it first, in case it is open. This is to prevent the Window class
	from deleting the Sample Editor window if open, losing the loaded Wave and
	resulting in a broken Editor (gdSampleEditor's destructor frees the loaded
	sample). */
	g_ui->closeSubWindow(WID_SAMPLE_EDITOR);
	g_ui->openSubWindow(new v::gdSampleEditor(channelId, g_ui->model));
}

/* -------------------------------------------------------------------------- */

void openRenameChannelWindow(const c::channel::Data& data, bool allscenes)
{
	g_ui->openSubWindow(new v::gdChannelNameInput(data, allscenes));
}

/* -------------------------------------------------------------------------- */

void openMissingAssetsWindow(const m::model::LoadState& state)
{
	g_ui->openSubWindow(new v::gdMissingAssets(state));
}

/* -------------------------------------------------------------------------- */

void openBrowserForPlugins(v::gdWindow& parent)
{
	v::gdBrowserDir* browser = new v::gdBrowserDir(g_ui->getI18Text(v::LangMap::BROWSER_OPENPLUGINSDIR),
	    g_ui->model.patchPath, c::config::setPluginPathCb, g_ui->model);
	parent.addChild(browser);
}

/* -------------------------------------------------------------------------- */

void openChannelPluginListWindow(ID channelId)
{
	g_ui->openSubWindow(new v::gdPluginList(channelId, g_ui->model.pluginListBounds));
}

/* -------------------------------------------------------------------------- */

void openMasterInPluginListWindow()
{
	g_ui->openSubWindow(new v::gdPluginList(MASTER_IN_CHANNEL_ID, g_ui->model.pluginListBounds));
}

/* -------------------------------------------------------------------------- */

void openMasterOutPluginListWindow()
{
	g_ui->openSubWindow(new v::gdPluginList(MASTER_OUT_CHANNEL_ID, g_ui->model.pluginListBounds));
}

/* -------------------------------------------------------------------------- */

void openPluginChooser(ID channelId)
{
	g_engine->getPluginsApi().sort(g_ui->model.pluginChooserSortMode);
	g_ui->openSubWindow(new v::gdPluginChooser(channelId, g_ui->model));
}

/* -------------------------------------------------------------------------- */

void openInfoWindow(std::string_view text, int w, int h)
{
	g_ui->openSubWindow(new v::gdInfo(text, w, h));
}
} // namespace giada::c::layout
