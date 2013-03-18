/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2011 Giovanni A. Zuliani | Monocasual
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
#include "init.h"
#include "const.h"
#include "mixer.h"
#include "mixerHandler.h"
#include "kernelAudio.h"
#include "recorder.h"
#include "gui_utils.h"
#ifdef WITH_VST
#include "pluginHost.h"
#endif

pthread_t t_video;

void *thread_video(void *arg);

#ifdef WITH_VST
PluginHost 	G_PluginHost;
#endif

Mixer 	 		G_Mixer;
bool		 		G_quit;
bool		 		G_audio_status;



int main(int argc, char **argv) {

	G_quit = false;

	init_prepareParser();
	init_prepareKernelAudio();
	init_startGUI(argc, argv);
	Fl::lock();
	pthread_create(&t_video, NULL, thread_video, NULL);
	init_startKernelAudio();

	//return Fl::run();

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
