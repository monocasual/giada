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


#include <stdlib.h>
#include "conf.h"
#include "../const.h"
#include "../utils.h"
#include "../log.h"


int Conf::openFileForReading()
{
	std::string path = gGetHomePath() + "/" + CONF_FILENAME;
	fp = fopen(path.c_str(), "r");
	if (fp == NULL) {
		gLog("[Conf::openFile] unable to open conf file for reading\n");
		return 0;
	}
	return 1;
}


/* -------------------------------------------------------------------------- */


int Conf::createConfigFolder(const char *path)
{
	if (gDirExists(path))
		return 1;

	gLog("[Conf] .giada folder not present. Updating...\n");

	if (gMkdir(path)) {
		gLog("[Conf] status: ok\n");
		return 1;
	}
	else {
		gLog("[Conf] status: error!\n");
		return 0;
	}
}


/* -------------------------------------------------------------------------- */


int Conf::openFileForWriting()
{
	/* writing config file. In windows is in the same dir of the .exe,
	 * in Linux and OS X in home */

#if defined(__linux__)

	char giadaPath[PATH_MAX];
	sprintf(giadaPath, "%s/.giada", getenv("HOME"));

	if (!createConfigFolder(giadaPath))
		return 0;

	char path[PATH_MAX];
	sprintf(path, "%s/%s", giadaPath, CONF_FILENAME);

#elif defined(_WIN32)

	const char *path = CONF_FILENAME;

#elif defined(__APPLE__)

	struct passwd *p = getpwuid(getuid());
	const char *home = p->pw_dir;
	char giadaPath[PATH_MAX];
	snprintf(giadaPath, PATH_MAX, "%s/Library/Application Support/Giada", home);

	if (!createConfigFolder(giadaPath))
		return 0;

	char path[PATH_MAX];
	sprintf(path, "%s/%s", giadaPath, CONF_FILENAME);

#endif

	fp = fopen(path, "w");
	if (fp == NULL)
		return 0;
	return 1;

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
	setDefault();

	if (!openFileForReading()) {
		gLog("[Conf] unreadable .conf file, using default parameters\n");
		return 0;
	}

	if (getValue("header") != "GIADACFG") {
		gLog("[Conf] corrupted .conf file, using default parameters\n");
		return -1;
	}

	logMode = atoi(getValue("logMode").c_str());

	soundSystem = atoi(getValue("soundSystem").c_str());
	if (!(soundSystem & SYS_API_ANY)) soundSystem = DEFAULT_SOUNDSYS;

	soundDeviceOut = atoi(getValue("soundDeviceOut").c_str());
	if (soundDeviceOut < 0) soundDeviceOut = DEFAULT_SOUNDDEV_OUT;

	soundDeviceIn = atoi(getValue("soundDeviceIn").c_str());
	if (soundDeviceIn < -1) soundDeviceIn = DEFAULT_SOUNDDEV_IN;

	channelsOut = atoi(getValue("channelsOut").c_str());
	channelsIn  = atoi(getValue("channelsIn").c_str());
	if (channelsOut < 0) channelsOut = 0;
	if (channelsIn < 0)  channelsIn  = 0;

	buffersize = atoi(getValue("buffersize").c_str());
	if (buffersize < 8) buffersize = DEFAULT_BUFSIZE;

	delayComp = atoi(getValue("delayComp").c_str());
	if (delayComp < 0) delayComp = DEFAULT_DELAYCOMP;

	midiSystem  = atoi(getValue("midiSystem").c_str());
	if (midiPortOut < -1) midiPortOut = DEFAULT_MIDI_SYSTEM;

	midiPortOut = atoi(getValue("midiPortOut").c_str());
	if (midiPortOut < -1) midiPortOut = DEFAULT_MIDI_PORT_OUT;

	midiPortIn = atoi(getValue("midiPortIn").c_str());
	if (midiPortIn < -1) midiPortIn = DEFAULT_MIDI_PORT_IN;

	noNoteOff = atoi(getValue("noNoteOff").c_str());

	std::string tmpMidiMapPath = getValue("midiMapPath");
	strncpy(midiMapPath, tmpMidiMapPath.c_str(), tmpMidiMapPath.size());
	midiMapPath[tmpMidiMapPath.size()] = '\0';	// strncpy doesn't add '\0'

	std::string tmplastFileMap = getValue("lastFileMap");
	strncpy(lastFileMap, tmplastFileMap.c_str(), tmplastFileMap.size());
	lastFileMap[tmplastFileMap.size()] = '\0';	// strncpy doesn't add '\0'

	midiSync  = atoi(getValue("midiSync").c_str());
	midiTCfps = atof(getValue("midiTCfps").c_str());

	midiInRewind     = strtoul(getValue("midiInRewind").c_str(), NULL, 10);
  midiInStartStop  = strtoul(getValue("midiInStartStop").c_str(), NULL, 10);
  midiInActionRec  = strtoul(getValue("midiInActionRec").c_str(), NULL, 10);
  midiInInputRec   = strtoul(getValue("midiInInputRec").c_str(), NULL, 10);
  midiInMetronome  = strtoul(getValue("midiInMetronome").c_str(), NULL, 10);
  midiInVolumeIn   = strtoul(getValue("midiInVolumeIn").c_str(), NULL, 10);
  midiInVolumeOut  = strtoul(getValue("midiInVolumeOut").c_str(), NULL, 10);
  midiInBeatDouble = strtoul(getValue("midiInBeatDouble").c_str(), NULL, 10);
  midiInBeatHalf   = strtoul(getValue("midiInBeatHalf").c_str(), NULL, 10);

	mainWindowX = atoi(getValue("mainWindowX").c_str());
	mainWindowY = atoi(getValue("mainWindowY").c_str());
	mainWindowW = atoi(getValue("mainWindowW").c_str());
	mainWindowH = atoi(getValue("mainWindowH").c_str());

	browserX = atoi(getValue("browserX").c_str());
	browserY = atoi(getValue("browserY").c_str());
	browserW = atoi(getValue("browserW").c_str());
	browserH = atoi(getValue("browserH").c_str());
	if (browserX < 0) browserX = 0;
	if (browserY < 0) browserY = 0;
	if (browserW < 396) browserW = 396;
	if (browserH < 302) browserH = 302;

	actionEditorX    = atoi(getValue("actionEditorX").c_str());
	actionEditorY    = atoi(getValue("actionEditorY").c_str());
	actionEditorW    = atoi(getValue("actionEditorW").c_str());
	actionEditorH    = atoi(getValue("actionEditorH").c_str());
	actionEditorZoom = atoi(getValue("actionEditorZoom").c_str());
	actionEditorGridVal = atoi(getValue("actionEditorGridVal").c_str());
	actionEditorGridOn  = atoi(getValue("actionEditorGridOn").c_str());
	if (actionEditorX < 0)      actionEditorX = 0;
	if (actionEditorY < 0)      actionEditorY = 0;
	if (actionEditorW < 640)    actionEditorW = 640;
	if (actionEditorH < 176)    actionEditorH = 176;
	if (actionEditorZoom < 100) actionEditorZoom = 100;
	if (actionEditorGridVal < 0) actionEditorGridVal = 0;
	if (actionEditorGridOn < 0)  actionEditorGridOn = 0;

	pianoRollY = atoi(getValue("pianoRollY").c_str());
	pianoRollH = atoi(getValue("pianoRollH").c_str());
	if (pianoRollH <= 0)
		pianoRollH = 422;

	sampleEditorX    = atoi(getValue("sampleEditorX").c_str());
	sampleEditorY    = atoi(getValue("sampleEditorY").c_str());
	sampleEditorW    = atoi(getValue("sampleEditorW").c_str());
	sampleEditorH    = atoi(getValue("sampleEditorH").c_str());
	sampleEditorGridVal = atoi(getValue("sampleEditorGridVal").c_str());
	sampleEditorGridOn  = atoi(getValue("sampleEditorGridOn").c_str());
  if (sampleEditorX < 0)   sampleEditorX = 0;
	if (sampleEditorY < 0)   sampleEditorY = 0;
	if (sampleEditorW < 500) sampleEditorW = 500;
	if (sampleEditorH < 292) sampleEditorH = 292;
	if (sampleEditorGridVal < 0) sampleEditorGridVal = 0;
	if (sampleEditorGridOn < 0)  sampleEditorGridOn = 0;

	configX = atoi(getValue("configX").c_str());
	configY = atoi(getValue("configY").c_str());
	if (configX < 0) configX = 0;
	if (configY < 0) configY = 0;

	pluginListX = atoi(getValue("pluginListX").c_str());
	pluginListY = atoi(getValue("pluginListY").c_str());
	if (pluginListX < 0) pluginListX = 0;
	if (pluginListY < 0) pluginListY = 0;

	bpmX = atoi(getValue("bpmX").c_str());
	bpmY = atoi(getValue("bpmY").c_str());
	if (bpmX < 0) bpmX = 0;
	if (bpmY < 0) bpmY = 0;

	beatsX = atoi(getValue("beatsX").c_str());
	beatsY = atoi(getValue("beatsY").c_str());
	if (beatsX < 0) beatsX = 0;
	if (beatsY < 0) beatsY = 0;

	aboutX = atoi(getValue("aboutX").c_str());
	aboutY = atoi(getValue("aboutY").c_str());
	if (aboutX < 0) aboutX = 0;
	if (aboutY < 0) aboutY = 0;

	samplerate = atoi(getValue("samplerate").c_str());
	if (samplerate < 8000) samplerate = DEFAULT_SAMPLERATE;

	limitOutput = atoi(getValue("limitOutput").c_str());
	rsmpQuality = atoi(getValue("rsmpQuality").c_str());

	std::string p = getValue("pluginPath");
	strncpy(pluginPath, p.c_str(), p.size());
	pluginPath[p.size()] = '\0';	// strncpy doesn't add '\0'

	p = getValue("patchPath");
	strncpy(patchPath, p.c_str(), p.size());
	patchPath[p.size()] = '\0';	// strncpy doesn't add '\0'

	p = getValue("samplePath");
	strncpy(samplePath, p.c_str(), p.size());
	samplePath[p.size()] = '\0';	// strncpy doesn't add '\0'

	recsStopOnChanHalt = atoi(getValue("recsStopOnChanHalt").c_str());
	chansStopOnSeqHalt = atoi(getValue("chansStopOnSeqHalt").c_str());
	treatRecsAsLoops   = atoi(getValue("treatRecsAsLoops").c_str());

	resizeRecordings = atoi(getValue("resizeRecordings").c_str());

	close();
	return 1;
}


/* -------------------------------------------------------------------------- */


int Conf::write()
{
	if (!openFileForWriting())
		return 0;

	fprintf(fp, "# --- Giada configuration file --- \n");
	fprintf(fp, "header=GIADACFG\n");
	fprintf(fp, "version=%s\n", VERSIONE);

	fprintf(fp, "logMode=%d\n",    logMode);

	fprintf(fp, "soundSystem=%d\n",    soundSystem);
	fprintf(fp, "soundDeviceOut=%d\n", soundDeviceOut);
	fprintf(fp, "soundDeviceIn=%d\n",  soundDeviceIn);
	fprintf(fp, "channelsOut=%d\n",    channelsOut);
	fprintf(fp, "channelsIn=%d\n",     channelsIn);
	fprintf(fp, "buffersize=%d\n",     buffersize);
	fprintf(fp, "delayComp=%d\n",      delayComp);
	fprintf(fp, "samplerate=%d\n",     samplerate);
	fprintf(fp, "limitOutput=%d\n",    limitOutput);
	fprintf(fp, "rsmpQuality=%d\n",    rsmpQuality);

	fprintf(fp, "midiSystem=%d\n",  midiSystem);
	fprintf(fp, "midiPortOut=%d\n", midiPortOut);
	fprintf(fp, "midiPortIn=%d\n",  midiPortIn);
	fprintf(fp, "noNoteOff=%d\n",   noNoteOff);
	fprintf(fp, "midiMapPath=%s\n", midiMapPath);
	fprintf(fp, "lastFileMap=%s\n", lastFileMap);
	fprintf(fp, "midiSync=%d\n",    midiSync);
	fprintf(fp, "midiTCfps=%f\n",   midiTCfps);

	fprintf(fp, "midiInRewind=%u\n",     midiInRewind);
	fprintf(fp, "midiInStartStop=%u\n",  midiInStartStop);
	fprintf(fp, "midiInActionRec=%u\n",  midiInActionRec);
	fprintf(fp, "midiInInputRec=%u\n",   midiInInputRec);
	fprintf(fp, "midiInMetronome=%u\n",  midiInMetronome);
	fprintf(fp, "midiInVolumeIn=%u\n",   midiInVolumeIn);
	fprintf(fp, "midiInVolumeOut=%u\n",  midiInVolumeOut);
	fprintf(fp, "midiInBeatDouble=%u\n", midiInBeatDouble);
	fprintf(fp, "midiInBeatHalf=%u\n",   midiInBeatHalf);

	fprintf(fp, "pluginPath=%s\n", pluginPath);
	fprintf(fp, "patchPath=%s\n",  patchPath);
	fprintf(fp, "samplePath=%s\n", samplePath);

	fprintf(fp, "mainWindowX=%d\n", mainWindowX);
	fprintf(fp, "mainWindowY=%d\n", mainWindowY);
	fprintf(fp, "mainWindowW=%d\n", mainWindowW);
	fprintf(fp, "mainWindowH=%d\n", mainWindowH);

	fprintf(fp, "browserX=%d\n", browserX);
	fprintf(fp, "browserY=%d\n", browserY);
	fprintf(fp, "browserW=%d\n", browserW);
	fprintf(fp, "browserH=%d\n", browserH);

	fprintf(fp, "actionEditorX=%d\n",       actionEditorX);
	fprintf(fp, "actionEditorY=%d\n",       actionEditorY);
	fprintf(fp, "actionEditorW=%d\n",       actionEditorW);
	fprintf(fp, "actionEditorH=%d\n",       actionEditorH);
	fprintf(fp, "actionEditorZoom=%d\n",    actionEditorZoom);
	fprintf(fp, "actionEditorGridOn=%d\n",  actionEditorGridOn);
	fprintf(fp, "actionEditorGridVal=%d\n", actionEditorGridVal);

	fprintf(fp, "pianoRollY=%d\n", pianoRollY);
	fprintf(fp, "pianoRollH=%d\n", pianoRollH);

	fprintf(fp, "sampleEditorX=%d\n", sampleEditorX);
	fprintf(fp, "sampleEditorY=%d\n", sampleEditorY);
	fprintf(fp, "sampleEditorW=%d\n", sampleEditorW);
	fprintf(fp, "sampleEditorH=%d\n", sampleEditorH);
	fprintf(fp, "sampleEditorGridOn=%d\n",  sampleEditorGridOn);
	fprintf(fp, "sampleEditorGridVal=%d\n", sampleEditorGridVal);

	fprintf(fp, "configX=%d\n", configX);
	fprintf(fp, "configY=%d\n", configY);

	fprintf(fp, "pluginListX=%d\n", pluginListX);
	fprintf(fp, "pluginListY=%d\n", pluginListY);

	fprintf(fp, "bpmX=%d\n", bpmX);
	fprintf(fp, "bpmY=%d\n", bpmY);

	fprintf(fp, "beatsX=%d\n", beatsX);
	fprintf(fp, "beatsY=%d\n", beatsY);

	fprintf(fp, "aboutX=%d\n", aboutX);
	fprintf(fp, "aboutY=%d\n", aboutY);

	fprintf(fp, "recsStopOnChanHalt=%d\n", recsStopOnChanHalt);
	fprintf(fp, "chansStopOnSeqHalt=%d\n", chansStopOnSeqHalt);
	fprintf(fp, "treatRecsAsLoops=%d\n",   treatRecsAsLoops);

	fprintf(fp, "resizeRecordings=%d\n", resizeRecordings);

	close();
	return 1;
}



/* -------------------------------------------------------------------------- */


void Conf::close()
{
	if (fp != NULL)
		fclose(fp);
}


/* -------------------------------------------------------------------------- */


void Conf::setPath(char *path, const char *p)
{
	path[0] = '\0';
	strncpy(path, p, strlen(p));
	path[strlen(p)] = '\0';
}
