/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "glue/layout.h"
#include "core/conf.h"
#include "core/patch.h"
#include "core/sequencer.h"
#include "glue/channel.h"
#include "glue/config.h"
#include "glue/io.h"
#include "glue/storage.h"
#include "gui/dialogs/about.h"
#include "gui/dialogs/actionEditor/midiActionEditor.h"
#include "gui/dialogs/actionEditor/sampleActionEditor.h"
#include "gui/dialogs/beatsInput.h"
#include "gui/dialogs/bpmInput.h"
#include "gui/dialogs/browser/browserDir.h"
#include "gui/dialogs/browser/browserLoad.h"
#include "gui/dialogs/browser/browserSave.h"
#include "gui/dialogs/channelNameInput.h"
#include "gui/dialogs/channelRouting.h"
#include "gui/dialogs/config.h"
#include "gui/dialogs/keyGrabber.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/midiIO/midiInputChannel.h"
#include "gui/dialogs/midiIO/midiInputMaster.h"
#include "gui/dialogs/midiIO/midiOutputMidiCh.h"
#include "gui/dialogs/midiIO/midiOutputSampleCh.h"
#include "gui/dialogs/missingAssets.h"
#include "gui/dialogs/pluginChooser.h"
#include "gui/dialogs/pluginList.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/ui.h"

extern giada::v::Ui g_ui;

namespace giada::c::layout
{
void openBrowserForProjectLoad()
{
	v::gdWindow* childWin = new v::gdBrowserLoad(g_ui.getI18Text(v::LangMap::BROWSER_OPENPROJECT),
	    g_ui.model.patchPath, c::storage::loadProject, 0, g_ui.model);
	g_ui.openSubWindow(childWin, WID_FILE_BROWSER);
}

/* -------------------------------------------------------------------------- */

void openBrowserForProjectSave()
{
	v::gdWindow* childWin = new v::gdBrowserSave(g_ui.getI18Text(v::LangMap::BROWSER_SAVEPROJECT),
	    g_ui.model.patchPath, g_ui.model.projectName, c::storage::saveProject, 0, g_ui.model);
	g_ui.openSubWindow(childWin, WID_FILE_BROWSER);
}

/* -------------------------------------------------------------------------- */

void openBrowserForSampleLoad(ID channelId)
{
	v::gdWindow* w = new v::gdBrowserLoad(g_ui.getI18Text(v::LangMap::BROWSER_OPENSAMPLE),
	    g_ui.model.samplePath, c::storage::loadSample, channelId, g_ui.model);
	g_ui.openSubWindow(w, WID_FILE_BROWSER);
}

/* -------------------------------------------------------------------------- */

void openBrowserForSampleSave(ID channelId)
{
	v::gdWindow* w = new v::gdBrowserSave(g_ui.getI18Text(v::LangMap::BROWSER_SAVESAMPLE),
	    g_ui.model.samplePath, "", c::storage::saveSample, channelId, g_ui.model);
	g_ui.openSubWindow(w, WID_FILE_BROWSER);
}

/* -------------------------------------------------------------------------- */

void openAboutWindow()
{
	g_ui.openSubWindow(new v::gdAbout(), WID_ABOUT);
}

/* -------------------------------------------------------------------------- */

void openKeyGrabberWindow(int key, std::function<bool(int)> f)
{
	v::gdKeyGrabber* keyGrabber = new v::gdKeyGrabber(key);

	keyGrabber->onSetKey = f;

	g_ui.openSubWindow(keyGrabber, WID_KEY_GRABBER);
}

/* -------------------------------------------------------------------------- */

void openBpmWindow(float bpm)
{
	g_ui.openSubWindow(new v::gdBpmInput(bpm), WID_BPM);
}

/* -------------------------------------------------------------------------- */

void openBeatsWindow(int beats, int bars)
{
	g_ui.openSubWindow(new v::gdBeatsInput(beats, bars), WID_BEATS);
}

/* -------------------------------------------------------------------------- */

void openConfigWindow()
{
	g_ui.openSubWindow(new v::gdConfig(400, 370, g_ui.model), WID_CONFIG);
}

/* -------------------------------------------------------------------------- */

void openMasterMidiInputWindow()
{
	g_ui.openSubWindow(new v::gdMidiInputMaster(g_ui.model), WID_MIDI_INPUT);
}

/* -------------------------------------------------------------------------- */

void openChannelMidiInputWindow(ID channelId)
{
	g_ui.openSubWindow(new v::gdMidiInputChannel(channelId, g_ui.model), WID_MIDI_INPUT);
}

/* -------------------------------------------------------------------------- */

void openChannelRoutingWindow(ID channelId)
{
	g_ui.openSubWindow(new v::gdChannelRouting(channel::getData(channelId)), WID_CHANNEL_ROUTING);
}

/* -------------------------------------------------------------------------- */

void openSampleChannelMidiOutputWindow(ID channelId)
{
	g_ui.openSubWindow(new v::gdMidiOutputSampleCh(channelId), WID_MIDI_OUTPUT);
}

/* -------------------------------------------------------------------------- */

void openMidiChannelMidiOutputWindow(ID channelId)
{
	g_ui.openSubWindow(new v::gdMidiOutputMidiCh(channelId), WID_MIDI_OUTPUT);
}

/* -------------------------------------------------------------------------- */

void openSampleActionEditor(ID channelId)
{
	g_ui.openSubWindow(new v::gdSampleActionEditor(channelId, g_ui.model), WID_ACTION_EDITOR);
}

/* -------------------------------------------------------------------------- */

void openMidiActionEditor(ID channelId)
{
	g_ui.openSubWindow(new v::gdMidiActionEditor(channelId, g_ui.model), WID_ACTION_EDITOR);
}

/* -------------------------------------------------------------------------- */

void openSampleEditor(ID channelId)
{
	g_ui.openSubWindow(new v::gdSampleEditor(channelId, g_ui.model), WID_SAMPLE_EDITOR);
}

/* -------------------------------------------------------------------------- */

void openRenameChannelWindow(const c::channel::Data& data)
{
	g_ui.openSubWindow(new v::gdChannelNameInput(data), WID_SAMPLE_NAME);
}

/* -------------------------------------------------------------------------- */

void openMissingAssetsWindow(const m::model::LoadState& state)
{
	g_ui.openSubWindow(new v::gdMissingAssets(state), WID_MISSING_ASSETS);
}

/* -------------------------------------------------------------------------- */

void openBrowserForPlugins(v::gdWindow& parent)
{
	v::gdBrowserDir* browser = new v::gdBrowserDir(g_ui.getI18Text(v::LangMap::BROWSER_OPENPLUGINSDIR),
	    g_ui.model.patchPath, c::config::setPluginPathCb, g_ui.model);
	parent.addChild(browser, WID_FILE_BROWSER);
}

/* -------------------------------------------------------------------------- */

void openChannelPluginListWindow(ID channelId)
{
	g_ui.openSubWindow(new v::gdPluginList(channelId, g_ui.model.pluginListBounds),
	    WID_FX_LIST);
}

/* -------------------------------------------------------------------------- */

void openMasterInPluginListWindow()
{
	g_ui.openSubWindow(new v::gdPluginList(m::Mixer::MASTER_IN_CHANNEL_ID, g_ui.model.pluginListBounds),
	    WID_FX_LIST);
}

/* -------------------------------------------------------------------------- */

void openMasterOutPluginListWindow()
{
	g_ui.openSubWindow(new v::gdPluginList(m::Mixer::MASTER_OUT_CHANNEL_ID, g_ui.model.pluginListBounds),
	    WID_FX_LIST);
}

/* -------------------------------------------------------------------------- */

void openPluginChooser(ID channelId)
{
	g_ui.openSubWindow(new v::gdPluginChooser(channelId, g_ui.model), WID_FX_CHOOSER);
}
} // namespace giada::c::layout
