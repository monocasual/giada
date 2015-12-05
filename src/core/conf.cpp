/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * conf
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2015 Giovanni A. Zuliani | Monocasual
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


#include <string>
#include "conf.h"
#include "const.h"
#include "../utils/utils.h"
#include "../utils/log.h"


using std::string;


Conf::Conf()
{
	/* Initialize confFilePath, i.e. the configuration file. In windows it is in
	 * the same dir of the .exe, while in Linux and OS X in ~/.giada */

#if defined(__linux__) || defined(__APPLE__)

	confFilePath = gGetHomePath() + gGetSlash() + CONF_FILENAME;
	confDirPath  = gGetHomePath() + gGetSlash();

#elif defined__(_WIN32)

	confFilePath = CONF_FILENAME;
	confDirPath  = "";

#endif
}


/* -------------------------------------------------------------------------- */


int Conf::createConfigFolder()
{
#if defined(__linux__) || defined(__APPLE__)

	if (gDirExists(confDirPath))
		return 1;

	gLog("[Conf::createConfigFolder] .giada folder not present. Updating...\n");

	if (gMkdir(confDirPath)) {
		gLog("[Conf::createConfigFolder] status: ok\n");
		return 1;
	}
	else {
		gLog("[Conf::createConfigFolder] status: error!\n");
		return 0;
	}

#endif
}


/* -------------------------------------------------------------------------- */


void Conf::setDefault()
{
	logMode = LOG_MODE_MUTE;

	soundSystem    = DEFAULT_SOUNDSYS;
	soundDeviceOut = DEFAULT_SOUNDDEV_OUT;
	soundDeviceIn  = DEFAULT_SOUNDDEV_IN;
	samplerate     = DEFAULT_SAMPLERATE;
	buffersize     = DEFAULT_BUFSIZE;
	delayComp      = DEFAULT_DELAYCOMP;
	limitOutput    = false;
	rsmpQuality    = 0;

	midiPortIn     = DEFAULT_MIDI_PORT_IN;
	noNoteOff      = false;
	midiMapPath[0] = '\0';
	midiPortOut    = DEFAULT_MIDI_PORT_OUT;
	midiSync       = MIDI_SYNC_NONE;
	midiTCfps      = 25.0f;

	midiInRewind     = 0x0;
	midiInStartStop  = 0x0;
	midiInActionRec  = 0x0;
	midiInInputRec   = 0x0;
	midiInVolumeIn   = 0x0;
	midiInVolumeOut  = 0x0;
	midiInBeatDouble = 0x0;
	midiInBeatHalf   = 0x0;
	midiInMetronome  = 0x0;

	pluginPath[0]  = '\0';
	patchPath [0]  = '\0';
	samplePath[0]  = '\0';

	recsStopOnChanHalt = false;
	chansStopOnSeqHalt = false;
	treatRecsAsLoops   = false;

	resizeRecordings = true;

	actionEditorZoom    = 100;
	actionEditorGridOn  = false;
	actionEditorGridVal = 1;

	mainWindowX = 0;
	mainWindowY = 0;
	mainWindowW = GUI_WIDTH;
	mainWindowH = GUI_HEIGHT;

	pianoRollY = -1;
	pianoRollH = 422;
}



/* -------------------------------------------------------------------------- */


int Conf::read()
{
	return 1;
}


/* -------------------------------------------------------------------------- */


int Conf::write()
{
	return 1;
}


/* -------------------------------------------------------------------------- */


void Conf::setPath(char *path, const char *p)
{
	/*
	path[0] = '\0';
	strncpy(path, p, strlen(p));
	path[strlen(p)] = '\0';*/
}
