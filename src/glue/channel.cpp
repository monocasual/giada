/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/gui/elems/mainWindow/keyboard/channel.h"
#include "src/core/channels/channelFactory.h"
#include "src/core/engine.h"
#include "src/core/mixer.h"
#include "src/core/patch.h"
#include "src/glue/channel.h"
#include "src/glue/main.h"
#include "src/gui/dialogs/sampleEditor.h"
#include "src/gui/dialogs/warnings.h"
#include "src/gui/elems/basics/dial.h"
#include "src/gui/elems/basics/input.h"
#include "src/gui/elems/mainWindow/keyboard/channelButton.h"
#include "src/gui/elems/mainWindow/keyboard/keyboard.h"
#include "src/gui/elems/sampleEditor/pitchTool.h"
#include "src/gui/elems/sampleEditor/rangeTool.h"
#include "src/gui/elems/sampleEditor/waveTools.h"
#include "src/gui/elems/sampleEditor/waveform.h"
#include "src/gui/ui.h"
#include "src/utils/fs.h"
#include "src/utils/gui.h"
#include <functional>

extern giada::v::Ui*     g_ui;
extern giada::m::Engine* g_engine;

namespace giada::c::channel
{
namespace
{
void printLoadError_(int res)
{
	if (res == G_RES_ERR_WRONG_DATA)
		v::gdAlert(g_ui->getI18Text(v::LangMap::MESSAGE_CHANNEL_MULTICHANNOTSUPPORTED));
	else if (res == G_RES_ERR_IO)
		v::gdAlert(g_ui->getI18Text(v::LangMap::MESSAGE_CHANNEL_CANTREADSAMPLE));
	else if (res == G_RES_ERR_PATH_TOO_LONG)
		v::gdAlert(g_ui->getI18Text(v::LangMap::MESSAGE_CHANNEL_PATHTOOLONG));
	else if (res == G_RES_ERR_NO_DATA)
		v::gdAlert(g_ui->getI18Text(v::LangMap::MESSAGE_CHANNEL_NOFILESPECIFIED));
}

/* -------------------------------------------------------------------------- */

Data makeData_(ID channelId, const m::model::Track& modelTrack, std::size_t scene)
{
	const std::size_t channelIndex = modelTrack.getChannelIndex(channelId);
	const std::size_t trackIndex   = modelTrack.getIndex();
	return Data(g_engine->getChannelsApi().get(channelId), scene, trackIndex, channelIndex);
}

/* -------------------------------------------------------------------------- */

Track makeTrack_(const m::model::Track& modelTrack, std::size_t scene)
{
	Track track{modelTrack.getIndex(), modelTrack.width, {}};

	for (const m::Channel& channel : modelTrack.getChannels().getAll())
		track.channels.push_back(makeData_(channel.id, modelTrack, scene));

	return track;
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

SampleData::SampleData(const m::Channel& ch, std::size_t scene)
: waveId(ch.sampleChannel->getWaveId(scene))
, mode(ch.sampleChannel->mode)
, isLoop(ch.sampleChannel->isAnyLoopMode())
, pitch(ch.sampleChannel->getPitch(scene))
, range(ch.sampleChannel->getRange(scene))
, inputMonitor(ch.sampleChannel->inputMonitor)
, overdubProtection(ch.sampleChannel->overdubProtection)
, m_tracker(&ch.shared->tracker)
{
}

Frame SampleData::getTracker() const { return m_tracker->load(); }

/* -------------------------------------------------------------------------- */

MidiData::MidiData(const m::Channel& m)
: isOutputEnabled(m.midiChannel->outputEnabled)
, filter(m.midiChannel->outputFilter)
{
}

/* -------------------------------------------------------------------------- */

Data::Data(const m::Channel& c, std::size_t scene, std::size_t trackIndex, std::size_t channelIndex)
: id(c.id)
, trackIndex(trackIndex)
, channelIndex(channelIndex)
, plugins(c.plugins)
, type(c.type)
, height(c.height)
, name(c.getName(scene))
, volume(c.volume)
, pan(c.pan.asFloat())
, key(c.key)
, hasActions(c.hasActions)
, sendToMaster(c.sendToMaster)
, extraOutputsCount(c.extraOutputs.size())
, m_playStatus(&c.shared->playStatus)
, m_recStatus(&c.shared->recStatus)
, m_readActions(&c.shared->readActions)
{
	if (c.type == ChannelType::SAMPLE)
		sample = std::make_optional<SampleData>(c, scene);
	else if (c.type == ChannelType::MIDI)
		midi = std::make_optional<MidiData>(c);
}

ChannelStatus Data::getPlayStatus() const { return m_playStatus->load(); }
ChannelStatus Data::getRecStatus() const { return m_recStatus->load(); }
bool          Data::getReadActions() const { return m_readActions->load(); }
bool          Data::isRecordingInput() const { return g_engine->getMainApi().isRecordingInput(); }
bool          Data::isRecordingActions() const { return g_engine->getMainApi().isRecordingActions(); }
bool          Data::isMuted() const { return g_engine->getChannelsApi().get(id).isMuted(); }
bool          Data::isSoloed() const { return g_engine->getChannelsApi().get(id).isSoloed(); }
bool          Data::isArmed() const { return g_engine->getChannelsApi().get(id).armed; }
bool          Data::isActive() const { return g_engine->getChannelsApi().get(id).isActive(); }

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Data getData(ID channelId)
{
	const std::size_t scene = g_engine->getMainApi().getCurrentScene();
	return makeData_(channelId, g_engine->getChannelsApi().getTracks().getByChannel(channelId), scene);
}

std::vector<Track> getTracks()
{
	const std::size_t  scene = g_engine->getMainApi().getCurrentScene();
	std::vector<Track> out;
	for (const m::model::Track& modelTrack : g_engine->getChannelsApi().getTracks().getAll())
		if (!modelTrack.isInternal())
			out.push_back(makeTrack_(modelTrack, scene));
	return out;
}

/* -------------------------------------------------------------------------- */

RoutingData getRoutingData(ID channelId)
{
	const m::Channel&             channel   = g_engine->getChannelsApi().get(channelId);
	const m::KernelAudio::Device& outDevice = g_engine->getConfigApi().audio_getCurrentOutDevice();

	return {
	    .id                   = channelId,
	    .isGroup              = channel.type == ChannelType::GROUP,
	    .volume               = channel.volume,
	    .pan                  = channel.pan.asFloat(),
	    .sendToMaster         = channel.sendToMaster,
	    .outputMaxNumChannels = outDevice.maxOutputChannels,
	    .extraOutputs         = channel.extraOutputs,
	    .outputDeviceName     = outDevice.name};
}

/* -------------------------------------------------------------------------- */

void loadChannel(ID channelId, const std::string& fname)
{
	auto progress = g_ui->mainWindow->getScopedProgress(g_ui->getI18Text(v::LangMap::MESSAGE_CHANNEL_LOADINGSAMPLES));

	int res = g_engine->getChannelsApi().loadSampleChannel(channelId, fname);
	if (res != G_RES_OK)
		printLoadError_(res);

	if (auto* w = sampleEditor::getWindow(); w != nullptr)
		w->rebuild();
}

/* -------------------------------------------------------------------------- */

void addChannel(std::size_t trackIndex, ChannelType type)
{
	g_engine->getChannelsApi().add(type, trackIndex);
}

/* -------------------------------------------------------------------------- */

void addAndLoadChannels(std::size_t trackIndex, const std::vector<std::string>& fnames)
{
	auto progress    = g_ui->mainWindow->getScopedProgress(g_ui->getI18Text(v::LangMap::MESSAGE_CHANNEL_LOADINGSAMPLES));
	auto channelsApi = g_engine->getChannelsApi();

	int  i      = 0;
	bool errors = false;
	for (const std::string& f : fnames)
	{
		progress.setProgress(++i / static_cast<float>(fnames.size()));

		const m::Channel& ch  = channelsApi.add(ChannelType::SAMPLE, trackIndex);
		const int         res = channelsApi.loadSampleChannel(ch.id, f);
		if (res != G_RES_OK)
			errors = true;
	}

	if (errors)
		v::gdAlert(g_ui->getI18Text(v::LangMap::MESSAGE_CHANNEL_LOADINGSAMPLESERROR));
}

/* -------------------------------------------------------------------------- */

void deleteChannel(ID channelId)
{
	if (!v::gdConfirmWin(g_ui->getI18Text(v::LangMap::COMMON_WARNING), g_ui->getI18Text(v::LangMap::MESSAGE_CHANNEL_DELETE)))
		return;
	g_ui->closeAllSubwindows();
	g_engine->getChannelsApi().remove(channelId);
}

/* -------------------------------------------------------------------------- */

void freeChannel(ID channelId, bool allScenes)
{
	if (!v::gdConfirmWin(g_ui->getI18Text(v::LangMap::COMMON_WARNING), g_ui->getI18Text(v::LangMap::MESSAGE_CHANNEL_FREE)))
		return;
	g_ui->closeAllSubwindows();
	g_engine->getChannelsApi().freeSampleChannel(channelId, allScenes);
}

/* -------------------------------------------------------------------------- */

void setInputMonitor(ID channelId, bool value)
{
	g_engine->getChannelsApi().setInputMonitor(channelId, value);
}

/* -------------------------------------------------------------------------- */

void setOverdubProtection(ID channelId, bool value)
{
	g_engine->getChannelsApi().setOverdubProtection(channelId, value);
}

/* -------------------------------------------------------------------------- */

void cloneChannel(ID channelId)
{
	g_engine->getChannelsApi().clone(channelId);
}

/* -------------------------------------------------------------------------- */

void copyChannelToScene(ID channelId, std::size_t dstScene)
{
	g_engine->getChannelsApi().copyToScene(channelId, dstScene);
}

/* -------------------------------------------------------------------------- */

void moveChannel(ID channelId, std::size_t trackIndex, int newPosition)
{
	g_engine->getChannelsApi().move(channelId, trackIndex, newPosition);
}

/* -------------------------------------------------------------------------- */

void addTrack()
{
	g_engine->getChannelsApi().addTrack();
}

/* -------------------------------------------------------------------------- */

void deleteTrack(std::size_t index)
{
	if (g_engine->getChannelsApi().getTracks().getAll().size() == 1) // One track must stay
		return;
	g_engine->getChannelsApi().removeTrack(index);
}

/* -------------------------------------------------------------------------- */

void setTrackWidth(std::size_t index, int w)
{
	g_engine->getChannelsApi().setTrackWidth(index, w);
}

/* -------------------------------------------------------------------------- */

bool canRemoveTrack(std::size_t trackIndex)
{
	return g_engine->getChannelsApi().canRemoveTrack(trackIndex);
}

/* -------------------------------------------------------------------------- */

void setSamplePlayerMode(ID channelId, SamplePlayerMode mode)
{
	g_engine->getChannelsApi().setSamplePlayerMode(channelId, mode);
	g_ui->refreshSubWindow(WID_ACTION_EDITOR);
}

/* -------------------------------------------------------------------------- */

void setHeight(ID channelId, int p)
{
	g_engine->getChannelsApi().setHeight(channelId, p);
}

/* -------------------------------------------------------------------------- */

void setSendToMaster(ID channelId, bool value)
{
	g_engine->getChannelsApi().setSendToMaster(channelId, value);
}

/* -------------------------------------------------------------------------- */

void addExtraOutput(ID channelId, int offset)
{
	g_engine->getChannelsApi().addExtraOutput(channelId, offset);
}

void removeExtraOutput(ID channelId, int offset)
{
	g_engine->getChannelsApi().removeExtraOutput(channelId, offset);
}

/* -------------------------------------------------------------------------- */

void setName(ID channelId, const std::string& name)
{
	g_engine->getChannelsApi().setName(channelId, name);
}

/* -------------------------------------------------------------------------- */

void clearAllActions(ID channelId, bool allScenes)
{
	if (!v::gdConfirmWin(g_ui->getI18Text(v::LangMap::COMMON_WARNING),
	        g_ui->getI18Text(v::LangMap::MESSAGE_MAIN_CLEARALLACTIONS)))
		return;

	g_engine->getChannelsApi().clearAllActions(channelId, allScenes);
	g_ui->refreshSubWindow(WID_ACTION_EDITOR);
}

/* -------------------------------------------------------------------------- */

void pressChannel(ID channelId, float velocity, Thread t)
{
	g_engine->getChannelsApi().press(channelId, velocity);
	notifyChannelForMidiIn(t, channelId);
}

void releaseChannel(ID channelId, Thread t)
{
	g_engine->getChannelsApi().release(channelId);
	notifyChannelForMidiIn(t, channelId);
}

void killChannel(ID channelId, Thread t)
{
	g_engine->getChannelsApi().kill(channelId);
	notifyChannelForMidiIn(t, channelId);
}

/* -------------------------------------------------------------------------- */

float setChannelVolume(ID channelId, float v, Thread t, bool repaintMainUi)
{
	g_engine->getChannelsApi().setVolume(channelId, v);
	notifyChannelForMidiIn(t, channelId);

	if (t != Thread::MAIN || repaintMainUi)
		g_ui->pumpEvent([channelId, v]()
		{ g_ui->mainWindow->keyboard->setChannelVolume(channelId, v); });

	return v;
}

/* -------------------------------------------------------------------------- */

float setChannelPitch(ID channelId, float v, Thread t)
{
	g_engine->getChannelsApi().setPitch(channelId, v);
	g_ui->pumpEvent([v]()
	{
		if (auto* w = sampleEditor::getWindow(); w != nullptr)
			w->pitchTool->update(v); });
	notifyChannelForMidiIn(t, channelId);
	return v;
}

/* -------------------------------------------------------------------------- */

float setChannelPan(ID channelId, float v)
{
	g_engine->getChannelsApi().setPan(channelId, v);
	notifyChannelForMidiIn(Thread::MAIN, channelId); // Currently triggered only by the main thread
	return v;
}

/* -------------------------------------------------------------------------- */

void toggleMuteChannel(ID channelId, Thread t)
{
	g_engine->getChannelsApi().toggleMute(channelId);
	notifyChannelForMidiIn(t, channelId);
}

void toggleSoloChannel(ID channelId, Thread t)
{
	g_engine->getChannelsApi().toggleSolo(channelId);
	notifyChannelForMidiIn(t, channelId);
}

/* -------------------------------------------------------------------------- */

void toggleArmChannel(ID channelId, Thread t)
{
	g_engine->getChannelsApi().toggleArm(channelId);
	notifyChannelForMidiIn(t, channelId);
}

void toggleReadActionsChannel(ID channelId, Thread t)
{
	g_engine->getChannelsApi().toggleReadActions(channelId);
	notifyChannelForMidiIn(t, channelId);
}

void killReadActionsChannel(ID channelId, Thread t)
{
	g_engine->getChannelsApi().killReadActions(channelId);
	notifyChannelForMidiIn(t, channelId);
}

/* -------------------------------------------------------------------------- */

void sendMidiToChannel(ID channelId, const m::MidiEvent& e, Thread t)
{
	g_engine->getChannelsApi().sendMidi(channelId, e);
	notifyChannelForMidiIn(t, channelId);
}

/* -------------------------------------------------------------------------- */

void notifyChannelForMidiIn(Thread t, ID channelId)
{
	if (t == Thread::MIDI)
		g_ui->pumpEvent([channelId]()
		{ g_ui->mainWindow->keyboard->notifyMidiIn(channelId); });
}

} // namespace giada::c::channel
