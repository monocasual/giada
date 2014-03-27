/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2014 Giovanni A. Zuliani | Monocasual
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
 * ------------------------------------------------------------------ */


#include <pthread.h>
#if defined(__linux__) || defined(__APPLE__)
	#include <unistd.h>
#endif
#include "init.h"
#include "const.h"
#include "patch.h"
#include "conf.h"
#include "mixer.h"
#include "mixerHandler.h"
#include "kernelAudio.h"
#include "recorder.h"
#include "gui_utils.h"
#include "gd_mainWindow.h"
#ifdef WITH_VST
#include "pluginHost.h"
#endif


/* global variables. Yeah, we are nasty */

pthread_t     t_video;
Mixer         G_Mixer;
bool          G_quit;
bool	        G_audio_status;
bool          G_midiStatus;
Patch 	      G_Patch;
Conf		      G_Conf;
gdMainWindow *mainWin;

#ifdef WITH_VST
PluginHost    G_PluginHost;
#endif


void *thread_video(void *arg);


int main(int argc, char **argv) {

	G_quit = false;

	init_prepareParser();
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
