/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


#include <pthread.h>
#if defined(__linux__) || defined(__APPLE__)
	#include <unistd.h>
#endif
#include "core/init.h"
#include "core/const.h"
#include "core/patch_DEPR_.h"
#include "core/patch.h"
#include "core/conf.h"
#include "core/midiMapConf.h"
#include "core/mixer.h"
#include "core/clock.h"
#include "core/mixerHandler.h"
#include "core/kernelAudio.h"
#include "core/kernelMidi.h"
#include "core/recorder.h"
#include "utils/gui.h"
#include "gui/dialogs/gd_mainWindow.h"
#include "core/pluginHost.h"


/* global variables. Yeah, we are nasty */

Conf          G_Conf;
pthread_t     G_videoThread;
bool          G_quit;
bool	        G_audio_status;
bool          G_midiStatus;
Patch_DEPR_   G_Patch_DEPR_;
Patch         G_Patch;
Mixer         G_Mixer;
Recorder      G_Recorder;
MidiMapConf   G_MidiMap;
gdMainWindow *G_MainWin;

#ifdef WITH_VST
PluginHost G_PluginHost;
#endif


void *videoThreadCb(void *arg);


int main(int argc, char **argv)
{
	G_quit = false;

	init_prepareParser();
	init_prepareMidiMap();
	init_prepareKernelAudio();

  /* TODO - move somewhere else */
  giada::clock::init(&G_Conf);
  /* TODO */

	init_prepareKernelMIDI();
	init_startGUI(argc, argv);
	Fl::lock();
	pthread_create(&G_videoThread, nullptr, videoThreadCb, nullptr);
	init_startKernelAudio();

#ifdef WITH_VST
	juce::initialiseJuce_GUI();
#endif

	int ret = Fl::run();

#ifdef WITH_VST
	juce::shutdownJuce_GUI();
#endif

	pthread_join(G_videoThread, nullptr);
	return ret;
}


void *videoThreadCb(void *arg)
{
	if (G_audio_status)
		while (!G_quit)	{
			gu_refreshUI();
#ifdef _WIN32
			Sleep(GUI_SLEEP);
#else
			usleep(GUI_SLEEP);
#endif
		}
	pthread_exit(nullptr);
	return 0;
}
