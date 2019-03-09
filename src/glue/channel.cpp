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
#include "core/model/data.h"
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
} // {anonymous}


/* -------------------------------------------------------------------------- */


int loadChannel(ID channelId, const std::string& fname)
{
	/* Save the patch and take the last browser's dir in order to re-use it the 
	next time. */

	m::conf::samplePath = gu_dirname(fname);

	int res = m::mh::loadChannel(channelId, fname);
	if (res != G_RES_OK)
		printLoadError_(res);
	
	return res;
}


/* -------------------------------------------------------------------------- */


m::Channel* addChannel(size_t columnIndex, ChannelType type, int size)
{
	return m::mh::addChannel(type, columnIndex);
}


/* -------------------------------------------------------------------------- */


void addAndLoadChannel(size_t columnIndex, int size, const std::string& fname)
{
	int res = m::mh::addAndLoadChannel(columnIndex, fname);
	if (res != G_RES_OK)
		printLoadError_(res);
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
	m::model::getLayout()->getChannel(channelId)->armed = value;
}


/* -------------------------------------------------------------------------- */


void setInputMonitor(ID channelId, bool value)
{
	static_cast<m::SampleChannel*>(m::model::getLayout()->getChannel(channelId))->inputMonitor = value;
}


/* -------------------------------------------------------------------------- */


void cloneChannel(ID channelId)
{
	m::mh::cloneChannel(channelId);
}


/* -------------------------------------------------------------------------- */


void setVolume(ID channelId, float value, bool gui, bool editor)
{
	m::model::getLayout()->getChannel(channelId)->volume = value;

	/* Changing channel volume? Update wave editor (if it's shown). */

	if (editor) {
		v::gdSampleEditor* gdEditor = static_cast<v::gdSampleEditor*>(u::gui::getSubwindow(G_MainWin, WID_SAMPLE_EDITOR));
		if (gdEditor != nullptr) {
			if (!gui) Fl::lock();
			gdEditor->volumeTool->rebuild();
			if (!gui) Fl::unlock();
		}
	}

	if (!gui) {
		Fl::lock();
		G_MainWin->keyboard->getChannel(channelId)->vol->value(value);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void setPitch(ID channelId, float val, bool gui)
{
	static_cast<m::SampleChannel*>(m::model::getLayout()->getChannel(channelId))->setPitch(val);

	v::gdSampleEditor* gdEditor = static_cast<v::gdSampleEditor*>(u::gui::getSubwindow(G_MainWin, WID_SAMPLE_EDITOR));
	if (gdEditor != nullptr) {
		if (!gui) Fl::lock();
		gdEditor->pitchTool->rebuild();
		if (!gui) Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void setPan(ID channelId, float val, bool gui)
{
	m::model::getLayout()->getChannel(channelId)->setPan(val);

	v::gdSampleEditor* gdEditor = static_cast<v::gdSampleEditor*>(u::gui::getSubwindow(G_MainWin, WID_SAMPLE_EDITOR));
	if (gdEditor != nullptr) {
		if (!gui) Fl::lock();
		gdEditor->panTool->rebuild();
		if (!gui) Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void setMute(ID channelId, bool value)
{
	m::model::getLayout()->getChannel(channelId)->setMute(value);
}


void toggleMute(ID channelId)
{
	setMute(channelId, !m::model::getLayout()->getChannel(channelId)->mute.load());
}


/* -------------------------------------------------------------------------- */


void setSampleMode(ID channelId, ChannelMode m)
{
	static_cast<m::SampleChannel*>(m::model::getLayout()->getChannel(channelId))->mode = m;
}


/* -------------------------------------------------------------------------- */


void setSolo(ID channelId, bool value)
{
	m::model::getLayout()->getChannel(channelId)->setSolo(value);
}


/* -------------------------------------------------------------------------- */


void start(ID channelId, int velocity, bool record)
{
	m::Channel* ch = m::model::getLayout()->getChannel(channelId);
	if (record && !ch->recordStart(m::clock::canQuantize()))
		return;
	ch->start(0, m::clock::canQuantize(), velocity); // On frame 0: it's a user-generated event
}


/* -------------------------------------------------------------------------- */


void kill(ID channelId, bool record)
{
	m::Channel* ch = m::model::getLayout()->getChannel(channelId);
	if (record && !ch->recordKill())
		return;
	ch->kill(0); // On frame 0: it's a user-generated event
}


/* -------------------------------------------------------------------------- */


void stop(ID channelId)
{
	m::Channel* ch = m::model::getLayout()->getChannel(channelId);
	ch->recordStop();
	ch->stop();
}


/* -------------------------------------------------------------------------- */


void setBoost(ID channelId, float val, bool gui)
{
	static_cast<m::SampleChannel*>(m::model::getLayout()->getChannel(channelId))->setBoost(val);

	v::gdSampleEditor* gdEditor = static_cast<v::gdSampleEditor*>(u::gui::getSubwindow(G_MainWin, WID_SAMPLE_EDITOR));
	if (gdEditor != nullptr) {
		if (!gui) Fl::lock();
		gdEditor->boostTool->rebuild();
		gdEditor->waveTools->rebuild();
		if (!gui) Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void setName(ID channelId, const std::string& name)
{
	m::mh::renameChannel(channelId, name);
}


/* -------------------------------------------------------------------------- */


void toggleReadingActions(ID channelId)
{
	const m::Channel* ch = m::model::getLayout()->getChannel(channelId);

	/* When you call startReadingRecs with conf::treatRecsAsLoops, the
	member value ch->readActions actually is not set to true immediately, because
	the channel is in wait mode (REC_WAITING). ch->readActions will become true on
	the next first beat. So a 'stop rec' command should occur also when
	ch->readActions is false but the channel is in wait mode; this check will
	handle the case of when you press 'R', the channel goes into REC_WAITING and
	then you press 'R' again to undo the status. */

	if (ch->readActions || (!ch->readActions && ch->recStatus == ChannelStatus::WAIT))
		stopReadingActions(channelId);
	else
		startReadingActions(channelId);
}


/* -------------------------------------------------------------------------- */


void startReadingActions(ID channelId)
{
	m::model::getLayout()->getChannel(channelId)->startReadingActions(m::conf::treatRecsAsLoops, 
		m::conf::recsStopOnChanHalt); 
}


/* -------------------------------------------------------------------------- */


void stopReadingActions(ID channelId)
{
	m::model::getLayout()->getChannel(channelId)->stopReadingActions(m::clock::isRunning(), 
		m::conf::treatRecsAsLoops, m::conf::recsStopOnChanHalt);
}

}}}; // giada::c::channel::
