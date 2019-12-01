/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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
#include "core/channels/channel.h"
#include "core/channels/sampleChannel.h"
#include "core/channels/midiChannel.h"
#include "core/model/model.h"
#include "core/kernelAudio.h"
#include "core/mixerHandler.h"
#include "core/mixer.h"
#include "core/clock.h"
#include "core/pluginHost.h"
#include "core/conf.h"
#include "core/wave.h"
#include "core/recorder.h"
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


/* -------------------------------------------------------------------------- */


void onRefreshSampleEditor_(bool gui, std::function<void(v::gdSampleEditor*)> f)
{
	v::gdSampleEditor* gdEditor = static_cast<v::gdSampleEditor*>(u::gui::getSubwindow(G_MainWin, WID_SAMPLE_EDITOR));
	if (gdEditor == nullptr) 
		return;
	if (!gui) Fl::lock();
	f(gdEditor);
	if (!gui) Fl::unlock();
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


int loadChannel(ID channelId, const std::string& fname)
{
	/* Save the patch and take the last browser's dir in order to re-use it the 
	next time. */

	m::conf::samplePath = u::fs::dirname(fname);

	int res = m::mh::loadChannel(channelId, fname);
	if (res != G_RES_OK)
		printLoadError_(res);
	
	return res;
}


/* -------------------------------------------------------------------------- */


void addChannel(ID columnId, ChannelType type, int size)
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


void setArm(ID channelId, bool value)
{
	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& c) { c.armed = value; });
}


void toggleArm(ID channelId)
{
	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& c) { c.armed = !c.armed; });
}


/* -------------------------------------------------------------------------- */


void setInputMonitor(ID channelId, bool value)
{
	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& c) 
	{ 
		static_cast<m::SampleChannel&>(c).inputMonitor = value;
	});
}


/* -------------------------------------------------------------------------- */


void cloneChannel(ID channelId)
{
	m::mh::cloneChannel(channelId);
}


/* -------------------------------------------------------------------------- */


void setVolume(ID channelId, float value, bool gui, bool editor)
{
	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c) { c.volume = value; });

	/* Changing channel volume? Update wave editor (if it's shown). */

	if (editor) 
		onRefreshSampleEditor_(gui, [](v::gdSampleEditor* e) { e->volumeTool->rebuild(); });

	if (!gui) {
		Fl::lock();
		G_MainWin->keyboard->getChannel(channelId)->vol->value(value);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void setPitch(ID channelId, float val, bool gui)
{	
	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c)
	{ 
		static_cast<m::SampleChannel&>(c).setPitch(val); 
	});
	
	onRefreshSampleEditor_(gui, [](v::gdSampleEditor* e) { e->pitchTool->rebuild(); });
}


/* -------------------------------------------------------------------------- */


void setPan(ID channelId, float val, bool gui)
{
	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c) { c.setPan(val); });

	onRefreshSampleEditor_(gui, [](v::gdSampleEditor* e) { e->panTool->rebuild(); });
}


/* -------------------------------------------------------------------------- */


void setMute(ID channelId, bool value)
{
	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& ch) { ch.setMute(value); });
}


void toggleMute(ID channelId)
{
	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& ch) { ch.setMute(!ch.mute); });
}


/* -------------------------------------------------------------------------- */


void setSampleMode(ID channelId, ChannelMode m)
{
	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& ch)
	{
		static_cast<m::SampleChannel&>(ch).mode = m;
	});

	u::gui::refreshActionEditor();
}


/* -------------------------------------------------------------------------- */


void setSolo(ID channelId, bool value)
{	
	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& ch) { ch.setSolo(value); });
	m::mh::updateSoloCount();
}


void toggleSolo(ID channelId)
{	
	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& ch) { ch.setSolo(!ch.solo); });
	m::mh::updateSoloCount();
}

/* -------------------------------------------------------------------------- */


void start(ID channelId, int velocity, bool record)
{
	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& ch)
	{
		if (record && !ch.recordStart(m::clock::canQuantize()))
			return;
		ch.start(/*localFrame=*/0, m::clock::canQuantize(), velocity); // Frame 0: user-generated event
	});
}


/* -------------------------------------------------------------------------- */


void kill(ID channelId, bool record)
{
	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& ch)
	{
		if (record && !ch.recordKill())
			return;
		ch.kill(/*localFrame=*/0); // Frame 0: user-generated event
	});
}


/* -------------------------------------------------------------------------- */


void stop(ID channelId)
{	
	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& ch)
	{
		ch.recordStop();
		ch.stop();
	});
}


/* -------------------------------------------------------------------------- */


void setName(ID channelId, const std::string& name)
{
	m::mh::renameChannel(channelId, name);
}


/* -------------------------------------------------------------------------- */


void toggleReadingActions(ID channelId)
{
	/* When you call startReadingRecs with conf::treatRecsAsLoops, the
	member value ch->readActions actually is not set to true immediately, because
	the channel is in wait mode (REC_WAITING). ch->readActions will become true on
	the next first beat. So a 'stop rec' command should occur also when
	ch->readActions is false but the channel is in wait mode; this check will
	handle the case of when you press 'R', the channel goes into REC_WAITING and
	then you press 'R' again to undo the status. */

	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& ch)
	{
		if (ch.readActions || (!ch.readActions && ch.recStatus == ChannelStatus::WAIT))
			ch.stopReadingActions(m::clock::isRunning(), m::conf::treatRecsAsLoops, 
				m::conf::recsStopOnChanHalt);
		else
			ch.startReadingActions(m::conf::treatRecsAsLoops, m::conf::recsStopOnChanHalt);
	});
}


/* -------------------------------------------------------------------------- */


void startReadingActions(ID channelId)
{
	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& ch)
	{
		ch.startReadingActions(m::conf::treatRecsAsLoops, m::conf::recsStopOnChanHalt);
	});
}


/* -------------------------------------------------------------------------- */


void stopReadingActions(ID channelId)
{
	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& ch)
	{
		ch.stopReadingActions(m::clock::isRunning(), m::conf::treatRecsAsLoops, 
			m::conf::recsStopOnChanHalt);
	});
}

}}}; // giada::c::channel::
