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

#include "gui/elems/mainWindow/keyboard/channel.h"
#include "core/actions/actionRecorder.h"
#include "core/channels/channelFactory.h"
#include "core/conf.h"
#include "core/engine.h"
#include "core/kernelAudio.h"
#include "core/mixer.h"
#include "core/model/model.h"
#include "core/patch.h"
#include "core/plugins/plugin.h"
#include "core/plugins/pluginHost.h"
#include "core/plugins/pluginManager.h"
#include "core/recorder.h"
#include "core/wave.h"
#include "core/waveFactory.h"
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
#include "gui/elems/sampleEditor/pitchTool.h"
#include "gui/elems/sampleEditor/rangeTool.h"
#include "gui/elems/sampleEditor/waveTools.h"
#include "gui/elems/sampleEditor/waveform.h"
#include "gui/ui.h"
#include "src/core/actions/actions.h"
#include "src/gui/elems/panTool.h"
#include "src/gui/elems/volumeTool.h"
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
		v::gdAlert(g_ui.langMapper.get(v::LangMap::MESSAGE_CHANNEL_MULTICHANNOTSUPPORTED));
	else if (res == G_RES_ERR_IO)
		v::gdAlert(g_ui.langMapper.get(v::LangMap::MESSAGE_CHANNEL_CANTREADSAMPLE));
	else if (res == G_RES_ERR_PATH_TOO_LONG)
		v::gdAlert(g_ui.langMapper.get(v::LangMap::MESSAGE_CHANNEL_PATHTOOLONG));
	else if (res == G_RES_ERR_NO_DATA)
		v::gdAlert(g_ui.langMapper.get(v::LangMap::MESSAGE_CHANNEL_NOFILESPECIFIED));
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

SampleData::SampleData(const m::Channel& ch)
: waveId(ch.samplePlayer->getWaveId())
, mode(ch.samplePlayer->mode)
, isLoop(ch.samplePlayer->isAnyLoopMode())
, pitch(ch.samplePlayer->pitch)
, begin(ch.samplePlayer->begin)
, end(ch.samplePlayer->end)
, inputMonitor(ch.audioReceiver->inputMonitor)
, overdubProtection(ch.audioReceiver->overdubProtection)
, m_channel(&ch)
{
}

Frame SampleData::getTracker() const { return m_channel->shared->tracker.load(); }

/* -------------------------------------------------------------------------- */

MidiData::MidiData(const m::Channel& m)
: isOutputEnabled(m.midiSender->enabled)
, filter(m.midiSender->filter)
{
}

/* -------------------------------------------------------------------------- */

Data::Data(const m::Channel& c)
: id(c.id)
, columnId(c.columnId)
, position(c.position)
, plugins(c.plugins)
, type(c.type)
, height(c.height)
, name(c.name)
, volume(c.volume)
, pan(c.pan)
, key(c.key)
, hasActions(c.hasActions)
, m_channel(&c)
{
	if (c.type == ChannelType::SAMPLE)
		sample = std::make_optional<SampleData>(c);
	else if (c.type == ChannelType::MIDI)
		midi = std::make_optional<MidiData>(c);
}

ChannelStatus Data::getPlayStatus() const { return m_channel->shared->playStatus.load(); }
ChannelStatus Data::getRecStatus() const { return m_channel->shared->recStatus.load(); }
bool          Data::getReadActions() const { return m_channel->shared->readActions.load(); }
bool          Data::isRecordingInput() const { return g_engine.getMainApi().isRecordingInput(); }
bool          Data::isRecordingActions() const { return g_engine.getMainApi().isRecordingActions(); }
bool          Data::isMuted() const { return m_channel->isMuted(); }
bool          Data::isSoloed() const { return m_channel->isSoloed(); }
bool          Data::isArmed() const { return m_channel->armed; }

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Data getData(ID channelId)
{
	return Data(g_engine.getChannelsApi().get(channelId));
}

std::vector<Data> getChannels()
{
	std::vector<Data> out;
	for (const m::Channel& ch : g_engine.getChannelsApi().getAll())
		if (!ch.isInternal())
			out.push_back(Data(ch));

	std::sort(out.begin(), out.end(), [](const Data& a, const Data& b)
	    { return a.position < b.position; });

	return out;
}

/* -------------------------------------------------------------------------- */

void loadChannel(ID channelId, const std::string& fname)
{
	auto progress = g_ui.mainWindow->getScopedProgress(g_ui.langMapper.get(v::LangMap::MESSAGE_CHANNEL_LOADINGSAMPLES));

	int res = g_engine.getChannelsApi().loadSampleChannel(channelId, fname);
	if (res != G_RES_OK)
		printLoadError_(res);

	if (auto* w = sampleEditor::getWindow(); w != nullptr)
		w->rebuild();
}

/* -------------------------------------------------------------------------- */

void addChannel(ID columnId, ChannelType type)
{
	g_engine.getChannelsApi().add(columnId, type);
}

/* -------------------------------------------------------------------------- */

void addAndLoadChannels(ID columnId, const std::vector<std::string>& fnames)
{
	auto progress      = g_ui.mainWindow->getScopedProgress(g_ui.langMapper.get(v::LangMap::MESSAGE_CHANNEL_LOADINGSAMPLES));
	auto channelEngine = g_engine.getChannelsApi();

	int  i      = 0;
	bool errors = false;
	for (const std::string& f : fnames)
	{
		progress.setProgress(++i / static_cast<float>(fnames.size()));

		const m::Channel& ch  = channelEngine.add(columnId, ChannelType::SAMPLE);
		const int         res = channelEngine.loadSampleChannel(ch.id, f);
		if (res != G_RES_OK)
			errors = true;
	}

	if (errors)
		v::gdAlert(g_ui.langMapper.get(v::LangMap::MESSAGE_CHANNEL_LOADINGSAMPLESERROR));
}

/* -------------------------------------------------------------------------- */

void deleteChannel(ID channelId)
{
	if (!v::gdConfirmWin(g_ui.langMapper.get(v::LangMap::COMMON_WARNING), g_ui.langMapper.get(v::LangMap::MESSAGE_CHANNEL_DELETE)))
		return;
	g_ui.closeAllSubwindows();
	g_engine.getChannelsApi().remove(channelId);
}

/* -------------------------------------------------------------------------- */

void freeChannel(ID channelId)
{
	if (!v::gdConfirmWin(g_ui.langMapper.get(v::LangMap::COMMON_WARNING), g_ui.langMapper.get(v::LangMap::MESSAGE_CHANNEL_FREE)))
		return;
	g_ui.closeAllSubwindows();
	g_engine.getChannelsApi().freeSampleChannel(channelId);
}

/* -------------------------------------------------------------------------- */

void setInputMonitor(ID channelId, bool value)
{
	g_engine.getChannelsApi().setInputMonitor(channelId, value);
}

/* -------------------------------------------------------------------------- */

void setOverdubProtection(ID channelId, bool value)
{
	g_engine.getChannelsApi().setOverdubProtection(channelId, value);
}

/* -------------------------------------------------------------------------- */

void cloneChannel(ID channelId)
{
	g_engine.getChannelsApi().clone(channelId);
}

/* -------------------------------------------------------------------------- */

void moveChannel(ID channelId, ID columnId, int position)
{
	g_engine.getChannelsApi().move(channelId, columnId, position);
}

/* -------------------------------------------------------------------------- */

void setSamplePlayerMode(ID channelId, SamplePlayerMode mode)
{
	g_engine.getChannelsApi().setSamplePlayerMode(channelId, mode);
	g_ui.refreshSubWindow(WID_ACTION_EDITOR);
}

/* -------------------------------------------------------------------------- */

void setHeight(ID channelId, Pixel p)
{
	g_engine.getChannelsApi().setHeight(channelId, p);
}

/* -------------------------------------------------------------------------- */

void setName(ID channelId, const std::string& name)
{
	g_engine.getChannelsApi().setName(channelId, name);
}

/* -------------------------------------------------------------------------- */

void clearAllActions(ID channelId)
{
	if (!v::gdConfirmWin(g_ui.langMapper.get(v::LangMap::COMMON_WARNING),
	        g_ui.langMapper.get(v::LangMap::MESSAGE_MAIN_CLEARALLACTIONS)))
		return;

	g_engine.getChannelsApi().clearAllActions(channelId);
	g_ui.refreshSubWindow(WID_ACTION_EDITOR);
}

/* -------------------------------------------------------------------------- */

void setCallbacks(m::Channel& ch)
{
	auto onSendMidiCb = [channelId = ch.id]()
	{
		g_ui.pumpEvent([channelId]()
		    { g_ui.mainWindow->keyboard->notifyMidiOut(channelId); });
	};

	ch.midiLighter.onSend = onSendMidiCb;
	if (ch.midiSender)
		ch.midiSender->onSend = onSendMidiCb;
}
} // namespace giada::c::channel
