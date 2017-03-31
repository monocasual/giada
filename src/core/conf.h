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


#ifndef CONF_H
#define CONF_H


#include <string>


namespace giada {
namespace conf
{
int read();
int write();

extern std::string header;

extern int  logMode;
extern int  soundSystem;
extern int  soundDeviceOut;
extern int  soundDeviceIn;
extern int  channelsOut;
extern int  channelsIn;
extern int  samplerate;
extern int  buffersize;
extern int  delayComp;
extern bool limitOutput;
extern int  rsmpQuality;

extern int  midiSystem;
extern int  midiPortOut;
extern int  midiPortIn;
extern bool noNoteOff;
extern std::string midiMapPath;
extern std::string lastFileMap;
extern int   midiSync;  // see const.h
extern float midiTCfps;

extern uint32_t midiInRewind;
extern uint32_t midiInStartStop;
extern uint32_t midiInActionRec;
extern uint32_t midiInInputRec;
extern uint32_t midiInMetronome;
extern uint32_t midiInVolumeIn;
extern uint32_t midiInVolumeOut;
extern uint32_t midiInBeatDouble;
extern uint32_t midiInBeatHalf;

extern bool recsStopOnChanHalt;
extern bool chansStopOnSeqHalt;
extern bool treatRecsAsLoops;
extern bool resizeRecordings;

extern std::string pluginPath;
extern std::string patchPath;
extern std::string samplePath;

extern int mainWindowX, mainWindowY, mainWindowW, mainWindowH;

extern int browserX, browserY, browserW, browserH, browserPosition, browserLastValue;
extern std::string browserLastPath;

extern int actionEditorX, actionEditorY, actionEditorW, actionEditorH, actionEditorZoom;
extern int actionEditorGridVal;
extern int actionEditorGridOn;

extern int sampleEditorX, sampleEditorY, sampleEditorW, sampleEditorH;
extern int sampleEditorGridVal;
extern int sampleEditorGridOn;

extern int midiInputX, midiInputY, midiInputW, midiInputH;

extern int pianoRollY, pianoRollH;
extern int pluginListX, pluginListY;
extern int configX, configY;
extern int bpmX, bpmY;
extern int beatsX, beatsY;
extern int aboutX, aboutY;

#ifdef WITH_VST

extern int pluginChooserX, pluginChooserY, pluginChooserW, pluginChooserH;
extern int pluginSortMethod;

#endif
}}; // giada::conf::

#endif
