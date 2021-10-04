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

#ifndef G_ENGINE_H
#define G_ENGINE_H

#include "core/actions/actionRecorder.h"
#include "core/actions/actions.h"
#include "core/channels/channelManager.h"
#include "core/conf.h"
#include "core/eventDispatcher.h"
#include "core/init.h"
#include "core/jackTransport.h"
#include "core/kernelAudio.h"
#include "core/kernelMidi.h"
#include "core/midiDispatcher.h"
#include "core/midiMapper.h"
#include "core/mixer.h"
#include "core/mixerHandler.h"
#include "core/model/model.h"
#include "core/patch.h"
#include "core/plugins/pluginHost.h"
#include "core/plugins/pluginManager.h"
#include "core/recorder.h"
#include "core/sequencer.h"
#include "core/synchronizer.h"
#include "core/waveManager.h"

namespace giada::m
{
class Engine final
{
public:
	/* Engine()
    Prepares all sub-components by constructing them and setting up the required
    callback for inter-component communication. It doesn't start the engine yet. */

	Engine();

	/* store
	Saves the current state to a Patch, then saves it to file. Returns true
	on success. */

	bool store(const std::string& projectName, const std::string& projectPath,
	    const std::string& patchPath);

	/* load
	Reads a Patch from file and then de-serialize its content into the model. 
	Returns G_PATCH_OK on success or any G_PATCH_* on failure. */

	int load(const std::string& projectPath, const std::string& patchPath);

	/* updateMixerModel
	Updates some values in model::Mixer data struct needed by m::Mixer for the
	audio rendering. Call this whenever the audio configuration changes. */

	void updateMixerModel();

	/* init
    Initializes all sub-components. If KernelAudio fails to start, the process
    interrupts and Giada is put in an invalid state. */

	void init();

	/* reset
    Resets all sub-components to the initial state. Useful when Giada needs to
    be brought back to the starup state. */

	void reset();

	/* shutdown
    Closes the current audio device. */

	void shutdown();

	model::Model    model;
	Conf            conf;
	Patch           patch;
	KernelAudio     kernelAudio;
	KernelMidi      kernelMidi;
	JackTransport   jackTransport;
	WaveManager     waveManager;
	EventDispatcher eventDispatcher;
	MidiMapper      midiMapper;
	ChannelManager  channelManager;
	PluginManager   pluginManager;
	MidiDispatcher  midiDispatcher;
	ActionRecorder  actionRecorder;
	Synchronizer    synchronizer;
	Sequencer       sequencer;
	Mixer           mixer;
	MixerHandler    mixerHandler;
	PluginHost      pluginHost;
	Recorder        recorder;

private:
	int audioCallback(KernelAudio::CallbackInfo);
};
} // namespace giada::m

#endif
