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


#include <functional>
#include <cmath>
#include <cassert>
#include <FL/Fl.H>
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/dialogs/warnings.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/sampleEditor/waveTools.h"
#include "gui/elems/sampleEditor/volumeTool.h"
#include "gui/elems/sampleEditor/boostTool.h"
#include "gui/elems/sampleEditor/panTool.h"
#include "gui/elems/sampleEditor/pitchTool.h"
#include "gui/elems/sampleEditor/rangeTool.h"
#include "gui/elems/sampleEditor/waveform.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "gui/elems/mainWindow/keyboard/channelButton.h"
#include "utils/gui.h"
#include "utils/fs.h"
#include "utils/log.h"
#include "core/model/model.h"
#include "core/kernelAudio.h"
#include "core/mixerHandler.h"
#include "core/mixer.h"
#include "core/clock.h"
#include "core/pluginHost.h"
#include "core/conf.h"
#include "core/wave.h"
#include "core/recorder.h"
#include "core/recManager.h"
#include "core/plugin.h"
#include "core/waveManager.h"
#include "main.h"
#include "channel.h"


extern giada::v::gdMainWindow* G_MainWin;


namespace giada {
namespace c {
namespace channel 
{
namespace
{
void printLoadError_(int res)
{
	if      (res == G_RES_ERR_WRONG_DATA)
		v::gdAlert("Multichannel samples not supported.");
	else if (res == G_RES_ERR_IO)
		v::gdAlert("Unable to read this sample.");
	else if (res == G_RES_ERR_PATH_TOO_LONG)
		v::gdAlert("File path too long.");
	else if (res == G_RES_ERR_NO_DATA)
		v::gdAlert("No file specified.");
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


SampleData::SampleData(const m::SamplePlayer& s, const m::AudioReceiver& a)
: waveId         (s.getWaveId())
, mode           (s.state->mode.load())
, isLoop         (s.state->isAnyLoopMode())
, pitch          (s.state->pitch.load())
, m_samplePlayer (&s)
, m_audioReceiver(&a)
{
}


Frame SampleData::a_getTracker() const           { return a_get(m_samplePlayer->state->tracker); }
Frame SampleData::a_getBegin() const             { return a_get(m_samplePlayer->state->begin); }
Frame SampleData::a_getEnd() const               { return a_get(m_samplePlayer->state->end); }
bool  SampleData::a_getInputMonitor() const      { return a_get(m_audioReceiver->state->inputMonitor); }
bool  SampleData::a_getOverdubProtection() const { return a_get(m_audioReceiver->state->overdubProtection); }


/* -------------------------------------------------------------------------- */


MidiData::MidiData(const m::MidiSender& m)
: m_midiSender(&m)
{
}

bool MidiData::a_isOutputEnabled() const { return a_get(m_midiSender->state->enabled); }
int  MidiData::a_getFilter() const       { return a_get(m_midiSender->state->filter); }


/* -------------------------------------------------------------------------- */


Data::Data(const m::Channel& c)
: id         (c.id)
, columnId   (c.getColumnId())
#ifdef WITH_VST
, pluginIds  (c.pluginIds)
#endif
, type       (c.getType())
, height     (c.state->height)
, name       (c.state->name)
, volume     (c.state->volume.load())
, pan        (c.state->pan.load())
, key        (c.state->key.load())
, hasActions (c.state->hasActions)
, m_channel  (c)
{
	if (c.getType() == ChannelType::SAMPLE)
		sample = std::make_optional<SampleData>(*c.samplePlayer, *c.audioReceiver);
	else
	if (c.getType() == ChannelType::MIDI)
		midi   = std::make_optional<MidiData>(*c.midiSender);
}


bool          Data::a_getSolo() const           { return a_get(m_channel.state->solo); }
bool          Data::a_getMute() const           { return a_get(m_channel.state->mute); }
ChannelStatus Data::a_getPlayStatus() const     { return a_get(m_channel.state->playStatus); }
ChannelStatus Data::a_getRecStatus() const      { return a_get(m_channel.state->recStatus); }
bool          Data::a_getReadActions() const    { return a_get(m_channel.state->readActions); }
bool          Data::a_isArmed() const           { return a_get(m_channel.state->armed); }
bool          Data::a_isRecordingInput() const  { return m::recManager::isRecordingInput(); }
bool          Data::a_isRecordingAction() const { return m::recManager::isRecordingAction(); }


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


Data getData(ID channelId)
{
	namespace mm = m::model;

	mm::ChannelsLock cl(mm::channels);
	return Data(mm::get(mm::channels, channelId));
}


std::vector<Data> getChannels()
{
	namespace mm = m::model;
	mm::ChannelsLock cl(mm::channels);

	std::vector<Data> out;
	for (const m::Channel* ch : mm::channels)
		if (!ch->isInternal()) 
			out.push_back(Data(*ch));
	
	return out;
}


/* -------------------------------------------------------------------------- */


int loadChannel(ID channelId, const std::string& fname)
{
	/* Save the patch and take the last browser's dir in order to re-use it the 
	next time. */

	m::conf::conf.samplePath = u::fs::dirname(fname);

	int res = m::mh::loadChannel(channelId, fname);
	if (res != G_RES_OK)
		printLoadError_(res);
	
	return res;
}


/* -------------------------------------------------------------------------- */


void addChannel(ID columnId, ChannelType type)
{
	m::mh::addChannel(type, columnId);
}


/* -------------------------------------------------------------------------- */


void addAndLoadChannel(ID columnId, const std::string& fpath)
{
	int res = m::mh::addAndLoadChannel(columnId, fpath);
	if (res != G_RES_OK)
		printLoadError_(res);
}


void addAndLoadChannels(ID columnId, const std::vector<std::string>& fpaths)
{
	if (fpaths.size() == 1)
		return addAndLoadChannel(columnId, fpaths[0]);

	bool errors = false;
	for (const std::string& f : fpaths)
		if (m::mh::addAndLoadChannel(columnId, f) != G_RES_OK)
			errors = true;

	if (errors)
		v::gdAlert("Some files weren't loaded sucessfully.");
}


/* -------------------------------------------------------------------------- */


void deleteChannel(ID channelId)
{
	if (!v::gdConfirmWin("Warning", "Delete channel: are you sure?"))
		return;
	u::gui::closeAllSubwindows();
	m::recorder::clearChannel(channelId);
	m::mh::deleteChannel(channelId);
}


/* -------------------------------------------------------------------------- */


void freeChannel(ID channelId)
{
	if (!v::gdConfirmWin("Warning", "Free channel: are you sure?"))
		return;
	u::gui::closeAllSubwindows();
	m::recorder::clearChannel(channelId);
	m::mh::freeChannel(channelId);
}


/* -------------------------------------------------------------------------- */


void setInputMonitor(ID channelId, bool value)
{
	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c) 
	{ 
		c.audioReceiver->state->inputMonitor.store(value);
	});
}


/* -------------------------------------------------------------------------- */


void setOverdubProtection(ID channelId, bool value)
{
	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c) 
	{
		c.audioReceiver->state->overdubProtection.store(value);
		if (value == true && c.state->armed.load() == true)
			c.state->armed.store(false);
	});	
}


/* -------------------------------------------------------------------------- */


void cloneChannel(ID channelId)
{
	m::mh::cloneChannel(channelId);
}


/* -------------------------------------------------------------------------- */


void setSamplePlayerMode(ID channelId, SamplePlayerMode m)
{
	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c)
	{
		c.samplePlayer->state->mode.store(m);
	});

	/* TODO - brutal rebuild! Just rebuild the specific channel instead */
	G_MainWin->keyboard->rebuild();

	u::gui::refreshActionEditor();
}


/* -------------------------------------------------------------------------- */


void setHeight(ID channelId, Pixel p)
{
	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c)
	{
		c.state->height = p;
	});	
}


/* -------------------------------------------------------------------------- */


void setName(ID channelId, const std::string& name)
{
	m::mh::renameChannel(channelId, name);
}
}}} // giada::c::channel::
