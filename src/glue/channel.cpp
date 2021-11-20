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

#include "gui/elems/mainWindow/keyboard/channel.h"
#include "core/actions/actionRecorder.h"
#include "core/channels/channelManager.h"
#include "core/conf.h"
#include "core/engine.h"
#include "core/kernelAudio.h"
#include "core/mixer.h"
#include "core/mixerHandler.h"
#include "core/model/model.h"
#include "core/patch.h"
#include "core/plugins/plugin.h"
#include "core/plugins/pluginHost.h"
#include "core/plugins/pluginManager.h"
#include "core/recorder.h"
#include "core/wave.h"
#include "core/waveManager.h"
#include "glue/channel.h"
#include "glue/main.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/dialogs/warnings.h"
#include "gui/dispatcher.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/mainWindow/keyboard/channelButton.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "gui/elems/sampleEditor/boostTool.h"
#include "gui/elems/sampleEditor/panTool.h"
#include "gui/elems/sampleEditor/pitchTool.h"
#include "gui/elems/sampleEditor/rangeTool.h"
#include "gui/elems/sampleEditor/volumeTool.h"
#include "gui/elems/sampleEditor/waveTools.h"
#include "gui/elems/sampleEditor/waveform.h"
#include "gui/ui.h"
#include "src/core/actions/actions.h"
#include "utils/fs.h"
#include "utils/gui.h"
#include "utils/log.h"
#include <FL/Fl.H>
#include <cassert>
#include <cmath>
#include <functional>

extern giada::v::Ui     g_ui;
extern giada::m::Engine g_engine;

namespace giada::c::channel
{
namespace
{
void printLoadError_(int res)
{
	if (res == G_RES_ERR_WRONG_DATA)
		v::gdAlert("Multichannel samples not supported.");
	else if (res == G_RES_ERR_IO)
		v::gdAlert("Unable to read this sample.");
	else if (res == G_RES_ERR_PATH_TOO_LONG)
		v::gdAlert("File path too long.");
	else if (res == G_RES_ERR_NO_DATA)
		v::gdAlert("No file specified.");
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

// TODO - just pass const Channel&
SampleData::SampleData(const m::Channel& ch)
: waveId(ch.samplePlayer->getWaveId())
, mode(ch.samplePlayer->mode)
, isLoop(ch.samplePlayer->isAnyLoopMode())
, pitch(ch.samplePlayer->pitch)
, m_channel(&ch)
{
}

Frame SampleData::getTracker() const { return m_channel->shared->tracker.load(); }
/* TODO - useless methods, turn them into member vars */
Frame SampleData::getBegin() const { return m_channel->samplePlayer->begin; }
Frame SampleData::getEnd() const { return m_channel->samplePlayer->end; }
bool  SampleData::getInputMonitor() const { return m_channel->audioReceiver->inputMonitor; }
bool  SampleData::getOverdubProtection() const { return m_channel->audioReceiver->overdubProtection; }

/* -------------------------------------------------------------------------- */

MidiData::MidiData(const m::Channel& m)
: m_channel(&m)
{
}

/* TODO - useless methods, turn them into member vars */
bool MidiData::isOutputEnabled() const { return m_channel->midiSender->enabled; }
int  MidiData::getFilter() const { return m_channel->midiSender->filter; }

/* -------------------------------------------------------------------------- */

Data::Data(const m::Channel& c)
: viewDispatcher(g_ui.dispatcher)
, id(c.id)
, columnId(c.columnId)
#ifdef WITH_VST
, plugins(c.plugins)
#endif
, type(c.type)
, height(c.height)
, name(c.name)
, volume(c.volume)
, pan(c.pan)
, key(c.key)
, hasActions(c.hasActions)
, m_channel(c)
{
	if (c.type == ChannelType::SAMPLE)
		sample = std::make_optional<SampleData>(c);
	else if (c.type == ChannelType::MIDI)
		midi = std::make_optional<MidiData>(c);
}

ChannelStatus Data::getPlayStatus() const { return m_channel.shared->playStatus.load(); }
ChannelStatus Data::getRecStatus() const { return m_channel.shared->recStatus.load(); }
bool          Data::getReadActions() const { return m_channel.shared->readActions.load(); }
bool          Data::isRecordingInput() const { return g_engine.recorder.isRecordingInput(); }
bool          Data::isRecordingAction() const { return g_engine.recorder.isRecordingAction(); }
/* TODO - useless methods, turn them into member vars */
bool Data::getSolo() const { return m_channel.isSoloed(); }
bool Data::getMute() const { return m_channel.isMuted(); }
bool Data::isArmed() const { return m_channel.armed; }

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Data getData(ID channelId)
{
	return Data(g_engine.model.get().getChannel(channelId));
}

std::vector<Data> getChannels()
{
	std::vector<Data> out;
	for (const m::Channel& ch : g_engine.model.get().channels)
		if (!ch.isInternal())
			out.push_back(Data(ch));
	return out;
}

/* -------------------------------------------------------------------------- */

int loadChannel(ID channelId, const std::string& fname)
{
	m::WaveManager::Result res = g_engine.waveManager.createFromFile(fname, /*id=*/0,
	    g_engine.kernelAudio.getSampleRate(), g_engine.conf.data.rsmpQuality);

	if (res.status != G_RES_OK)
	{
		printLoadError_(res.status);
		return res.status;
	}

	/* Save the patch and take the last browser's dir in order to re-use it the 
	next time. */

	g_engine.conf.data.samplePath = u::fs::dirname(fname);
	g_engine.mixerHandler.loadChannel(channelId, std::move(res.wave));

	return G_RES_OK;
}

/* -------------------------------------------------------------------------- */

void addChannel(ID columnId, ChannelType type)
{
	g_engine.mixerHandler.addChannel(type, columnId, g_engine.kernelAudio.getBufferSize(), g_engine.channelManager);
}

/* -------------------------------------------------------------------------- */

void addAndLoadChannel(ID columnId, const std::string& fname)
{
	m::WaveManager::Result res = g_engine.waveManager.createFromFile(fname, /*id=*/0,
	    g_engine.kernelAudio.getSampleRate(), g_engine.conf.data.rsmpQuality);
	if (res.status == G_RES_OK)
		g_engine.mixerHandler.addAndLoadChannel(columnId, std::move(res.wave), g_engine.kernelAudio.getBufferSize(),
		    g_engine.channelManager);
	else
		printLoadError_(res.status);
}

void addAndLoadChannels(ID columnId, const std::vector<std::string>& fnames)
{
	bool errors = false;
	for (const std::string& f : fnames)
	{
		m::WaveManager::Result res = g_engine.waveManager.createFromFile(f, /*id=*/0,
		    g_engine.kernelAudio.getSampleRate(), g_engine.conf.data.rsmpQuality);
		if (res.status == G_RES_OK)
			g_engine.mixerHandler.addAndLoadChannel(columnId, std::move(res.wave), g_engine.kernelAudio.getBufferSize(),
			    g_engine.channelManager);
		else
			errors = true;
	}

	if (errors)
		v::gdAlert("Some files weren't loaded successfully.");
}

/* -------------------------------------------------------------------------- */

void deleteChannel(ID channelId)
{
	if (!v::gdConfirmWin("Warning", "Delete channel: are you sure?"))
		return;
	g_ui.closeAllSubwindows();

#ifdef WITH_VST
	const std::vector<m::Plugin*> plugins = g_engine.model.get().getChannel(channelId).plugins;
#endif
	g_engine.mixerHandler.deleteChannel(channelId);
	g_engine.actionRecorder.clearChannel(channelId);
#ifdef WITH_VST
	g_engine.pluginHost.freePlugins(plugins);
#endif
}

/* -------------------------------------------------------------------------- */

void freeChannel(ID channelId)
{
	if (!v::gdConfirmWin("Warning", "Free channel: are you sure?"))
		return;
	g_ui.closeAllSubwindows();
	g_engine.actionRecorder.clearChannel(channelId);
	g_engine.mixerHandler.freeChannel(channelId);
}

/* -------------------------------------------------------------------------- */

void setInputMonitor(ID channelId, bool value)
{
	g_engine.model.get().getChannel(channelId).audioReceiver->inputMonitor = value;
	g_engine.model.swap(m::model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void setOverdubProtection(ID channelId, bool value)
{
	m::Channel& ch                      = g_engine.model.get().getChannel(channelId);
	ch.audioReceiver->overdubProtection = value;
	if (value == true && ch.armed)
		ch.armed = false;
	g_engine.model.swap(m::model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void cloneChannel(ID channelId)
{
	g_engine.actionRecorder.cloneActions(channelId, g_engine.channelManager.getNextId());
#ifdef WITH_VST
	g_engine.mixerHandler.cloneChannel(channelId, g_engine.patch.data.samplerate,
	    g_engine.kernelAudio.getBufferSize(), g_engine.channelManager, g_engine.waveManager,
	    g_engine.sequencer, g_engine.pluginManager);
#else
	g_engine.mixerHandler.cloneChannel(channelId, g_engine.kernelAudio.getBufferSize(),
	    g_engine.channelManager, g_engine.waveManager);
#endif
}

/* -------------------------------------------------------------------------- */

void setSamplePlayerMode(ID channelId, SamplePlayerMode mode)
{
	g_engine.model.get().getChannel(channelId).samplePlayer->mode = mode;
	g_engine.model.swap(m::model::SwapType::HARD); // TODO - SOFT should be enough, fix geChannel refresh method
	g_ui.refreshSubWindow(WID_ACTION_EDITOR);
}

/* -------------------------------------------------------------------------- */

void setHeight(ID channelId, Pixel p)
{
	g_engine.model.get().getChannel(channelId).height = p;
	g_engine.model.swap(m::model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void setName(ID channelId, const std::string& name)
{
	g_engine.mixerHandler.renameChannel(channelId, name);
}
} // namespace giada::c::channel
