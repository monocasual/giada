/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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
#include <FL/Fl.H>
#include "../gui/dialogs/gd_mainWindow.h"
#include "../gui/dialogs/sampleEditor.h"
#include "../gui/dialogs/gd_warnings.h"
#include "../gui/elems/basics/input.h"
#include "../gui/elems/basics/dial.h"
#include "../gui/elems/sampleEditor/waveTools.h"
#include "../gui/elems/sampleEditor/volumeTool.h"
#include "../gui/elems/sampleEditor/boostTool.h"
#include "../gui/elems/sampleEditor/panTool.h"
#include "../gui/elems/sampleEditor/pitchTool.h"
#include "../gui/elems/sampleEditor/rangeTool.h"
#include "../gui/elems/sampleEditor/waveform.h"
#include "../gui/elems/mainWindow/keyboard/keyboard.h"
#include "../gui/elems/mainWindow/keyboard/channel.h"
#include "../gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "../gui/elems/mainWindow/keyboard/channelButton.h"
#include "../utils/gui.h"
#include "../utils/fs.h"
#include "../utils/log.h"
#include "../core/kernelAudio.h"
#include "../core/mixerHandler.h"
#include "../core/mixer.h"
#include "../core/clock.h"
#include "../core/pluginHost.h"
#include "../core/conf.h"
#include "../core/wave.h"
#include "../core/channel.h"
#include "../core/sampleChannel.h"
#include "../core/midiChannel.h"
#include "../core/recorder/recorder.h"
#include "../core/plugin.h"
#include "../core/waveManager.h"
#include "main.h"
#include "channel.h"


extern gdMainWindow* G_MainWin;


using std::string;


namespace giada {
namespace c     {
namespace channel 
{
int loadChannel(m::SampleChannel* ch, const string& fname)
{
	using namespace giada::m;

	/* Always stop a channel before loading a new sample in it. This will prevent
	issues if tracker is outside the boundaries of the new sample -> segfault. */

	if (ch->isPlaying())
		ch->kill(0);

	/* Save the patch and take the last browser's dir in order to re-use it the 
	next time. */

	conf::samplePath = gu_dirname(fname);

	Wave* wave = nullptr;
	int result = waveManager::create(fname, &wave); 
	if (result != G_RES_OK)
		return result;

	if (wave->getRate() != conf::samplerate) {
		gu_log("[loadChannel] input rate (%d) != system rate (%d), conversion needed\n",
			wave->getRate(), conf::samplerate);
		result = waveManager::resample(wave, conf::rsmpQuality, conf::samplerate); 
		if (result != G_RES_OK) {
			delete wave;
			return result;
		}
	}

	ch->pushWave(wave);

	G_MainWin->keyboard->updateChannel(ch->guiChannel);

	return result;
}


/* -------------------------------------------------------------------------- */


m::Channel* addChannel(int column, ChannelType type, int size)
{
	m::Channel* ch = m::mh::addChannel(type);
	geChannel* gch = G_MainWin->keyboard->addChannel(column, ch, size);
	ch->guiChannel = gch;
	return ch;
}


/* -------------------------------------------------------------------------- */


void deleteChannel(m::Channel* ch)
{
	using namespace giada::m;

	if (!gdConfirmWin("Warning", "Delete channel: are you sure?"))
		return;
	recorder::clearChannel(ch->index);
	ch->hasActions = false;
#ifdef WITH_VST
	pluginHost::freeStack(pluginHost::CHANNEL, &mixer::mutex, ch);
#endif
	Fl::lock();
	G_MainWin->keyboard->deleteChannel(ch->guiChannel);
	Fl::unlock();
	mh::deleteChannel(ch);
	gu_closeAllSubwindows();
}


/* -------------------------------------------------------------------------- */


void freeChannel(m::Channel* ch)
{
	if (ch->isPlaying()) {
		if (!gdConfirmWin("Warning", "This action will stop the channel: are you sure?"))
			return;
	}
	else
	if (!gdConfirmWin("Warning", "Free channel: are you sure?"))
		return;

	G_MainWin->keyboard->freeChannel(ch->guiChannel);
	m::recorder::clearChannel(ch->index);
	ch->empty();

	/* delete any related subwindow */
	/** TODO - use gu_closeAllSubwindows()   */
	G_MainWin->delSubWindow(WID_FILE_BROWSER);
	G_MainWin->delSubWindow(WID_ACTION_EDITOR);
	G_MainWin->delSubWindow(WID_SAMPLE_EDITOR);
	G_MainWin->delSubWindow(WID_FX_LIST);
}


/* -------------------------------------------------------------------------- */


void toggleArm(m::Channel* ch, bool gui)
{
	ch->armed = !ch->armed;
	if (!gui)
		ch->guiChannel->arm->value(ch->armed);
}


/* -------------------------------------------------------------------------- */


void toggleInputMonitor(m::Channel* ch)
{
	m::SampleChannel* sch = static_cast<m::SampleChannel*>(ch);
	sch->inputMonitor = !sch->inputMonitor;
}


/* -------------------------------------------------------------------------- */


int cloneChannel(m::Channel* src)
{
	using namespace giada::m;

	Channel* ch    = mh::addChannel(src->type);
	geChannel* gch = G_MainWin->keyboard->addChannel(src->guiChannel->getColumnIndex(), 
		ch, src->guiChannel->getSize());

	ch->guiChannel = gch;
	ch->copy(src, &mixer::mutex);

	G_MainWin->keyboard->updateChannel(ch->guiChannel);
	return true;
}


/* -------------------------------------------------------------------------- */


void setVolume(m::Channel* ch, float v, bool gui, bool editor)
{
	ch->volume = v;

	/* Changing channel volume? Update wave editor (if it's shown). */

	if (!editor) {
		gdSampleEditor* gdEditor = static_cast<gdSampleEditor*>(gu_getSubwindow(G_MainWin, WID_SAMPLE_EDITOR));
		if (gdEditor) {
			Fl::lock();
			gdEditor->volumeTool->refresh();
			Fl::unlock();
		}
	}

	if (!gui) {
		Fl::lock();
		ch->guiChannel->vol->value(v);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void setPitch(m::SampleChannel* ch, float val)
{
	ch->setPitch(val);
	gdSampleEditor* gdEditor = static_cast<gdSampleEditor*>(gu_getSubwindow(G_MainWin, WID_SAMPLE_EDITOR));
	if (gdEditor) {
		Fl::lock();
		gdEditor->pitchTool->refresh();
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void setPanning(m::SampleChannel* ch, float val)
{
	ch->setPan(val);
	gdSampleEditor* gdEditor = static_cast<gdSampleEditor*>(gu_getSubwindow(G_MainWin, WID_SAMPLE_EDITOR));
	if (gdEditor) {
		Fl::lock();
		gdEditor->panTool->refresh();
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void toggleMute(m::Channel* ch, bool gui)
{
	ch->setMute(!ch->mute);
	if (!gui) {
		Fl::lock();
		ch->guiChannel->mute->value(ch->mute);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void toggleSolo(m::Channel* ch, bool gui)
{
	ch->setSolo(!ch->solo);
	if (!gui) {
		Fl::lock();
		ch->guiChannel->solo->value(ch->solo);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void kill(m::Channel* ch)
{
	ch->kill(0); // on frame 0: it's a user-generated event
}


/* -------------------------------------------------------------------------- */


void setBoost(m::SampleChannel* ch, float val)
{
	ch->setBoost(val);
	gdSampleEditor *gdEditor = static_cast<gdSampleEditor*>(gu_getSubwindow(G_MainWin, WID_SAMPLE_EDITOR));
	if (gdEditor) {
		Fl::lock();
		gdEditor->boostTool->refresh();
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void setName(m::Channel* ch, const string& name)
{
	ch->name = name;
	ch->guiChannel->update();
}


/* -------------------------------------------------------------------------- */


void toggleReadingActions(m::Channel* ch, bool gui)
{

	/* When you call startReadingRecs with conf::treatRecsAsLoops, the
	member value ch->readActions actually is not set to true immediately, because
	the channel is in wait mode (REC_WAITING). ch->readActions will become true on
	the next first beat. So a 'stop rec' command should occur also when
	ch->readActions is false but the channel is in wait mode; this check will
	handle the case of when you press 'R', the channel goes into REC_WAITING and
	then you press 'R' again to undo the status. */

	if (ch->readActions || (!ch->readActions && ch->recStatus == ChannelStatus::WAIT))
		stopReadingActions(ch, gui);
	else
		startReadingActions(ch, gui);
}


/* -------------------------------------------------------------------------- */


void startReadingActions(m::Channel* ch, bool gui)
{
	using namespace giada::m;

	ch->startReadingActions(conf::treatRecsAsLoops, conf::recsStopOnChanHalt); 

	if (!gui) {
		Fl::lock();
		static_cast<geSampleChannel*>(ch->guiChannel)->readActions->value(1);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void stopReadingActions(m::Channel* ch, bool gui)
{
	using namespace giada::m;

	ch->stopReadingActions(clock::isRunning(), conf::treatRecsAsLoops, 
		conf::recsStopOnChanHalt);

	if (!gui) {
		Fl::lock();
		static_cast<geSampleChannel*>(ch->guiChannel)->readActions->value(0);
		Fl::unlock();
	}
}

}}}; // giada::c::channel::
