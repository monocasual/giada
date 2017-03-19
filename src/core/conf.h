/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * conf
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


#ifndef __CONF_H__
#define __CONF_H__


#include <string>
#include "dataStorageJson.h"


class Conf : public DataStorageJson
{
private:

	std::string confFilePath;
	std::string confDirPath;

	/* init
   * Init Conf with default values. */

  void init();

	/* sanitize
	 * Avoid funky values from config file. */

	void sanitize();

	/* createConfigFolder
	 * Create local folder where to put the configuration file. Path differs from
	 * OS to OS. */

	int createConfigFolder();

public:

	Conf();

	std::string header;

	int  logMode;
	int  soundSystem;
	int  soundDeviceOut;
	int  soundDeviceIn;
	int  channelsOut;
	int  channelsIn;
	int  samplerate;
	int  buffersize;
	int  delayComp;
	bool limitOutput;
	int  rsmpQuality;

	int  midiSystem;
	int  midiPortOut;
	int  midiPortIn;
	bool noNoteOff;
	std::string midiMapPath;
	std::string lastFileMap;
	int   midiSync;  // see const.h
	float midiTCfps;

	uint32_t midiInRewind;
	uint32_t midiInStartStop;
	uint32_t midiInActionRec;
	uint32_t midiInInputRec;
	uint32_t midiInMetronome;
	uint32_t midiInVolumeIn;
	uint32_t midiInVolumeOut;
	uint32_t midiInBeatDouble;
	uint32_t midiInBeatHalf;

	bool recsStopOnChanHalt;
	bool chansStopOnSeqHalt;
	bool treatRecsAsLoops;
	bool resizeRecordings;

	std::string pluginPath;
	std::string patchPath;
	std::string samplePath;

	int mainWindowX, mainWindowY, mainWindowW, mainWindowH;

	int browserX, browserY, browserW, browserH, browserPosition, browserLastValue;
	std::string browserLastPath;

	int actionEditorX, actionEditorY, actionEditorW, actionEditorH, actionEditorZoom;
	int actionEditorGridVal;
	int actionEditorGridOn;

	int sampleEditorX, sampleEditorY, sampleEditorW, sampleEditorH;
  int sampleEditorGridVal;
  int sampleEditorGridOn;

  int midiInputX, midiInputY, midiInputW, midiInputH;

	int pianoRollY, pianoRollH;
	int pluginListX, pluginListY;
	int configX, configY;
	int bpmX, bpmY;
	int beatsX, beatsY;
	int aboutX, aboutY;

#ifdef WITH_VST

	int pluginChooserX, pluginChooserY, pluginChooserW, pluginChooserH;
	int pluginSortMethod;

#endif

	int read();
	int write();
};

#endif
