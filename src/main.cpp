/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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
#include "core/mixerHandler.h"
#include "core/kernelAudio.h"
#include "core/recorder.h"
#include "utils/gui_utils.h"
#include "gui/dialogs/gd_mainWindow.h"
#ifdef WITH_VST
#include "core/pluginHost_DEPR_.h"
#endif


/* global variables. Yeah, we are nasty */

pthread_t     t_video;
Mixer         G_Mixer;
bool          G_quit;
bool	        G_audio_status;
bool          G_midiStatus;
Patch_DEPR_   G_Patch_DEPR_;
Patch         G_Patch;
Conf          G_Conf;
MidiMapConf   G_MidiMap;
gdMainWindow *mainWin;

#ifdef WITH_VST
PluginHost_DEPR_ G_PluginHost;
#endif


void *thread_video(void *arg);


int main(int argc, char **argv) {

	G_quit = false;

	init_prepareParser();
	init_prepareMidiMap();
	init_prepareKernelAudio();
	init_prepareKernelMIDI();
	init_startGUI(argc, argv);
	Fl::lock();
	pthread_create(&t_video, NULL, thread_video, NULL);
	init_startKernelAudio();

	int ret = Fl::run();

	pthread_join(t_video, NULL);
	return ret;
}



void *thread_video(void *arg) {
	if (G_audio_status)
		while (!G_quit)	{
			gu_refresh();
#ifdef _WIN32
			Sleep(GUI_SLEEP);
#else
			usleep(GUI_SLEEP);
#endif
		}
	pthread_exit(NULL);
	return 0;
}
