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

#include "glue/layout.h"
#include "core/conf.h"
#include "core/engine.h"
#include "core/patch.h"
#include "core/sequencer.h"
#include "glue/config.h"
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

extern giada::v::Ui     g_ui;
extern giada::m::Engine g_engine;

namespace giada::c::layout
{
void openBrowserForProjectLoad()
{
	v::gdWindow* childWin = new v::gdBrowserLoad("Open project", g_engine.conf.data.patchPath,
	    c::storage::loadProject, 0, g_engine.conf.data);
	g_ui.openSubWindow(*g_ui.mainWindow.get(), childWin, WID_FILE_BROWSER);
}

/* -------------------------------------------------------------------------- */

void openBrowserForProjectSave()
{
	v::gdWindow* childWin = new v::gdBrowserSave("Save project", g_engine.conf.data.patchPath,
	    g_engine.patch.data.name, c::storage::saveProject, 0, g_engine.conf.data);
	g_ui.openSubWindow(*g_ui.mainWindow.get(), childWin, WID_FILE_BROWSER);
}

/* -------------------------------------------------------------------------- */

void openBrowserForSampleLoad(ID channelId)
{
	v::gdWindow* w = new v::gdBrowserLoad("Browse sample",
	    g_engine.conf.data.samplePath.c_str(), c::storage::loadSample, channelId, g_engine.conf.data);
	g_ui.openSubWindow(*g_ui.mainWindow.get(), w, WID_FILE_BROWSER);
}

/* -------------------------------------------------------------------------- */

void openBrowserForSampleSave(ID channelId)
{
	v::gdWindow* w = new v::gdBrowserSave("Save sample",
	    g_engine.conf.data.samplePath.c_str(), "", c::storage::saveSample, channelId, g_engine.conf.data);
	g_ui.openSubWindow(*g_ui.mainWindow.get(), w, WID_FILE_BROWSER);
}

/* -------------------------------------------------------------------------- */

void openAboutWindow()
{
	g_ui.openSubWindow(*g_ui.mainWindow.get(), new v::gdAbout(), WID_ABOUT);
}

/* -------------------------------------------------------------------------- */

void openKeyGrabberWindow(const c::channel::Data& data)
{
	g_ui.openSubWindow(*g_ui.mainWindow.get(), new v::gdKeyGrabber(data), WID_KEY_GRABBER);
}

/* -------------------------------------------------------------------------- */

void openBpmWindow(std::string bpmValue)
{
	g_ui.openSubWindow(*g_ui.mainWindow.get(), new v::gdBpmInput(bpmValue.c_str()), WID_BPM);
}

/* -------------------------------------------------------------------------- */

void openBeatsWindow(int beats, int bars)
{
	g_ui.openSubWindow(*g_ui.mainWindow.get(), new v::gdBeatsInput(beats, bars), WID_BEATS);
}

/* -------------------------------------------------------------------------- */

void openConfigWindow()
{
	g_ui.openSubWindow(*g_ui.mainWindow.get(), new v::gdConfig(400, 370, g_engine.conf.data), WID_CONFIG);
}

/* -------------------------------------------------------------------------- */

void openMasterMidiInputWindow()
{
	g_ui.openSubWindow(*g_ui.mainWindow.get(), new v::gdMidiInputMaster(g_engine.conf.data), WID_MIDI_INPUT);
}

/* -------------------------------------------------------------------------- */

void openChannelMidiInputWindow(ID channelId)
{
	g_ui.openSubWindow(*g_ui.mainWindow.get(), new v::gdMidiInputChannel(channelId, g_engine.conf.data),
	    WID_MIDI_INPUT);
}

/* -------------------------------------------------------------------------- */

void openSampleChannelMidiOutputWindow(ID channelId)
{
	g_ui.openSubWindow(*g_ui.mainWindow.get(), new v::gdMidiOutputSampleCh(channelId),
	    WID_MIDI_OUTPUT);
}

/* -------------------------------------------------------------------------- */

void openMidiChannelMidiOutputWindow(ID channelId)
{
	g_ui.openSubWindow(*g_ui.mainWindow.get(), new v::gdMidiOutputMidiCh(channelId), WID_MIDI_OUTPUT);
}

/* -------------------------------------------------------------------------- */

void openSampleActionEditor(ID channelId)
{
	g_ui.openSubWindow(*g_ui.mainWindow.get(),
	    new v::gdSampleActionEditor(channelId, g_engine.conf.data), WID_ACTION_EDITOR);
}

/* -------------------------------------------------------------------------- */

void openMidiActionEditor(ID channelId)
{
	g_ui.openSubWindow(*g_ui.mainWindow.get(),
	    new v::gdMidiActionEditor(channelId, g_engine.conf.data), WID_ACTION_EDITOR);
}

/* -------------------------------------------------------------------------- */

void openSampleEditor(ID channelId)
{
	g_ui.openSubWindow(*g_ui.mainWindow.get(), new v::gdSampleEditor(channelId, g_engine.conf.data),
	    WID_SAMPLE_EDITOR);
}

/* -------------------------------------------------------------------------- */

void openRenameChannelWindow(const c::channel::Data& data)
{
	g_ui.openSubWindow(*g_ui.mainWindow.get(), new v::gdChannelNameInput(data),
	    WID_SAMPLE_NAME);
}

/* -------------------------------------------------------------------------- */

void openMissingAssetsWindow(const m::LoadState& state)
{
	g_ui.openSubWindow(*g_ui.mainWindow.get(), new v::gdMissingAssets(state),
	    WID_MISSING_ASSETS);
}

/* -------------------------------------------------------------------------- */

#ifdef WITH_VST

void openBrowserForPlugins(v::gdWindow& parent)
{
	v::gdBrowserDir* browser = new v::gdBrowserDir("Add plug-ins directory",
	    g_engine.conf.data.patchPath, c::config::setPluginPathCb, g_engine.conf.data);
	parent.addSubWindow(browser);
}

/* -------------------------------------------------------------------------- */

void openChannelPluginListWindow(ID channelId)
{
	g_ui.openSubWindow(*g_ui.mainWindow.get(), new v::gdPluginList(channelId, g_engine.conf.data),
	    WID_FX_LIST);
}

/* -------------------------------------------------------------------------- */

void openMasterInPluginListWindow()
{
	g_ui.openSubWindow(*g_ui.mainWindow.get(), new v::gdPluginList(m::Mixer::MASTER_IN_CHANNEL_ID, g_engine.conf.data),
	    WID_FX_LIST);
}

/* -------------------------------------------------------------------------- */

void openMasterOutPluginListWindow()
{
	g_ui.openSubWindow(*g_ui.mainWindow.get(), new v::gdPluginList(m::Mixer::MASTER_OUT_CHANNEL_ID, g_engine.conf.data),
	    WID_FX_LIST);
}

/* -------------------------------------------------------------------------- */

void openPluginChooser(ID channelId)
{
	g_ui.openSubWindow(*g_ui.mainWindow.get(),
	    new v::gdPluginChooser(g_engine.conf.data.pluginChooserX,
	        g_engine.conf.data.pluginChooserY, g_engine.conf.data.pluginChooserW,
	        g_engine.conf.data.pluginChooserH, channelId, g_engine.conf.data),
	    WID_FX_CHOOSER);
}

#endif
} // namespace giada::c::layout
