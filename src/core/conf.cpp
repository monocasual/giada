/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#include <fstream>
#include <cassert>
#include <string>
#include <FL/Fl.H>
#include "deps/json/single_include/nlohmann/json.hpp"
#include "utils/fs.h"
#include "utils/log.h"
#include "core/const.h"
#include "core/types.h"
#include "conf.h"


namespace nl = nlohmann;


namespace giada {
namespace m {
namespace conf
{
namespace
{
std::string confFilePath_ = "";
std::string confDirPath_  = "";


/* -------------------------------------------------------------------------- */


/* createConfigFolder
Creates local folder where to put the configuration file. Path differs from OS
to OS. */

int createConfigFolder_()
{
#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)

	if (u::fs::dirExists(confDirPath_))
		return 1;

	u::log::print("[conf::createConfigFolder] .giada folder not present. Updating...\n");

	if (u::fs::mkdir(confDirPath_)) {
		u::log::print("[conf::createConfigFolder] status: ok\n");
		return 1;
	}
	else {
		u::log::print("[conf::createConfigFolder] status: error!\n");
		return 0;
	}

#else // Windows: nothing to do

	return 1;

#endif
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


Conf conf;


/* -------------------------------------------------------------------------- */


void init()
{
	conf = Conf();

	/* Initialize confFilePath_, i.e. the configuration file. In windows it is in
	 * the same dir of the .exe, while in Linux and OS X in ~/.giada */

#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)

	confFilePath_ = u::fs::getHomePath() + G_SLASH + CONF_FILENAME;
	confDirPath_  = u::fs::getHomePath() + G_SLASH;

#elif defined(_WIN32)

	confFilePath_ = CONF_FILENAME;
	confDirPath_  = "";

#endif
}


/* -------------------------------------------------------------------------- */


bool read()
{
	init();

	std::ifstream ifs(confFilePath_);
	if (!ifs.good())
		return false;

	nl::json j = nl::json::parse(ifs);

	conf.logMode                    =  j.value(CONF_KEY_LOG_MODE, conf.logMode);
	conf.soundSystem                =  j.value(CONF_KEY_SOUND_SYSTEM, conf.soundSystem);
	conf.soundDeviceOut             =  j.value(CONF_KEY_SOUND_DEVICE_OUT, conf.soundDeviceOut);
	conf.soundDeviceIn              =  j.value(CONF_KEY_SOUND_DEVICE_IN, conf.soundDeviceIn);
	conf.channelsOut                =  j.value(CONF_KEY_CHANNELS_OUT, conf.channelsOut);
	conf.channelsInCount            =  j.value(CONF_KEY_CHANNELS_IN_COUNT, conf.channelsInCount);
	conf.channelsInStart            =  j.value(CONF_KEY_CHANNELS_IN_START, conf.channelsInStart);
	conf.samplerate                 =  j.value(CONF_KEY_SAMPLERATE, conf.samplerate);
	conf.buffersize                 =  j.value(CONF_KEY_BUFFER_SIZE, conf.buffersize);
	conf.limitOutput                =  j.value(CONF_KEY_LIMIT_OUTPUT, conf.limitOutput);
	conf.rsmpQuality                =  j.value(CONF_KEY_RESAMPLE_QUALITY, conf.rsmpQuality);
	conf.midiSystem                 =  j.value(CONF_KEY_MIDI_SYSTEM, conf.midiSystem);
	conf.midiPortOut                =  j.value(CONF_KEY_MIDI_PORT_OUT, conf.midiPortOut);
	conf.midiPortIn                 =  j.value(CONF_KEY_MIDI_PORT_IN, conf.midiPortIn);
	conf.midiMapPath                =  j.value(CONF_KEY_MIDIMAP_PATH, conf.midiMapPath);
	conf.lastFileMap                =  j.value(CONF_KEY_LAST_MIDIMAP, conf.lastFileMap);
	conf.midiSync                   =  j.value(CONF_KEY_MIDI_SYNC, conf.midiSync);
	conf.midiTCfps                  =  j.value(CONF_KEY_MIDI_TC_FPS, conf.midiTCfps);
	conf.chansStopOnSeqHalt         =  j.value(CONF_KEY_CHANS_STOP_ON_SEQ_HALT, conf.chansStopOnSeqHalt);
	conf.treatRecsAsLoops           =  j.value(CONF_KEY_TREAT_RECS_AS_LOOPS, conf.treatRecsAsLoops);
	conf.inputMonitorDefaultOn      =  j.value(CONF_KEY_INPUT_MONITOR_DEFAULT_ON, conf.inputMonitorDefaultOn);
	conf.overdubProtectionDefaultOn =  j.value(CONF_KEY_OVERDUB_PROTECTION_DEFAULT_ON, conf.overdubProtectionDefaultOn);
	conf.pluginPath                 =  j.value(CONF_KEY_PLUGINS_PATH, conf.pluginPath);
	conf.patchPath                  =  j.value(CONF_KEY_PATCHES_PATH, conf.patchPath);
	conf.samplePath                 =  j.value(CONF_KEY_SAMPLES_PATH, conf.samplePath);
	conf.mainWindowX                =  j.value(CONF_KEY_MAIN_WINDOW_X, conf.mainWindowX);
	conf.mainWindowY                =  j.value(CONF_KEY_MAIN_WINDOW_Y, conf.mainWindowY);
	conf.mainWindowW                =  j.value(CONF_KEY_MAIN_WINDOW_W, conf.mainWindowW);
	conf.mainWindowH                =  j.value(CONF_KEY_MAIN_WINDOW_H, conf.mainWindowH);
	conf.browserX                   =  j.value(CONF_KEY_BROWSER_X, conf.browserX);
	conf.browserY                   =  j.value(CONF_KEY_BROWSER_Y, conf.browserY);
	conf.browserW                   =  j.value(CONF_KEY_BROWSER_W, conf.browserW);
	conf.browserH                   =  j.value(CONF_KEY_BROWSER_H, conf.browserH);
	conf.browserPosition            =  j.value(CONF_KEY_BROWSER_POSITION, conf.browserPosition);
	conf.browserLastPath            =  j.value(CONF_KEY_BROWSER_LAST_PATH, conf.browserLastPath);
	conf.browserLastValue           =  j.value(CONF_KEY_BROWSER_LAST_VALUE, conf.browserLastValue);
	conf.actionEditorX              =  j.value(CONF_KEY_ACTION_EDITOR_X, conf.actionEditorX);
	conf.actionEditorY              =  j.value(CONF_KEY_ACTION_EDITOR_Y, conf.actionEditorY);
	conf.actionEditorW              =  j.value(CONF_KEY_ACTION_EDITOR_W, conf.actionEditorW);
	conf.actionEditorH              =  j.value(CONF_KEY_ACTION_EDITOR_H, conf.actionEditorH);
	conf.actionEditorZoom           =  j.value(CONF_KEY_ACTION_EDITOR_ZOOM, conf.actionEditorZoom);
	conf.actionEditorGridVal        =  j.value(CONF_KEY_ACTION_EDITOR_GRID_VAL, conf.actionEditorGridVal);
	conf.actionEditorGridOn         =  j.value(CONF_KEY_ACTION_EDITOR_GRID_ON, conf.actionEditorGridOn);
	conf.sampleEditorX              =  j.value(CONF_KEY_SAMPLE_EDITOR_X, conf.sampleEditorX);
	conf.sampleEditorY              =  j.value(CONF_KEY_SAMPLE_EDITOR_Y, conf.sampleEditorY);
	conf.sampleEditorW              =  j.value(CONF_KEY_SAMPLE_EDITOR_W, conf.sampleEditorW);
	conf.sampleEditorH              =  j.value(CONF_KEY_SAMPLE_EDITOR_H, conf.sampleEditorH);
	conf.sampleEditorGridVal        =  j.value(CONF_KEY_SAMPLE_EDITOR_GRID_VAL, conf.sampleEditorGridVal);
	conf.sampleEditorGridOn         =  j.value(CONF_KEY_SAMPLE_EDITOR_GRID_ON, conf.sampleEditorGridOn);
	conf.pianoRollY                 =  j.value(CONF_KEY_PIANO_ROLL_Y, conf.pianoRollY);
	conf.pianoRollH                 =  j.value(CONF_KEY_PIANO_ROLL_H, conf.pianoRollH);
	conf.sampleActionEditorH        =  j.value(CONF_KEY_SAMPLE_ACTION_EDITOR_H, conf.sampleActionEditorH);
	conf.velocityEditorH            =  j.value(CONF_KEY_VELOCITY_EDITOR_H, conf.velocityEditorH);
	conf.envelopeEditorH            =  j.value(CONF_KEY_ENVELOPE_EDITOR_H, conf.envelopeEditorH);
	conf.pluginListX                =  j.value(CONF_KEY_PLUGIN_LIST_X, conf.pluginListX);
	conf.pluginListY                =  j.value(CONF_KEY_PLUGIN_LIST_Y, conf.pluginListY);
	conf.midiInputX                 =  j.value(CONF_KEY_MIDI_INPUT_X, conf.midiInputX);
	conf.midiInputY                 =  j.value(CONF_KEY_MIDI_INPUT_Y, conf.midiInputY);
	conf.midiInputW                 =  j.value(CONF_KEY_MIDI_INPUT_W, conf.midiInputW);
	conf.midiInputH                 =  j.value(CONF_KEY_MIDI_INPUT_H, conf.midiInputH);
	conf.recTriggerMode             =  j.value(CONF_KEY_REC_TRIGGER_MODE, conf.recTriggerMode);
	conf.recTriggerLevel            =  j.value(CONF_KEY_REC_TRIGGER_LEVEL, conf.recTriggerLevel);
	conf.midiInEnabled              =  j.value(CONF_KEY_MIDI_IN, conf.midiInEnabled);
	conf.midiInFilter               =  j.value(CONF_KEY_MIDI_IN_FILTER, conf.midiInFilter);
	conf.midiInRewind               =  j.value(CONF_KEY_MIDI_IN_REWIND, conf.midiInRewind);
	conf.midiInStartStop            =  j.value(CONF_KEY_MIDI_IN_START_STOP, conf.midiInStartStop);
	conf.midiInActionRec            =  j.value(CONF_KEY_MIDI_IN_ACTION_REC, conf.midiInActionRec);
	conf.midiInInputRec             =  j.value(CONF_KEY_MIDI_IN_INPUT_REC, conf.midiInInputRec);
	conf.midiInMetronome            =  j.value(CONF_KEY_MIDI_IN_METRONOME, conf.midiInMetronome);
	conf.midiInVolumeIn             =  j.value(CONF_KEY_MIDI_IN_VOLUME_IN, conf.midiInVolumeIn);
	conf.midiInVolumeOut            =  j.value(CONF_KEY_MIDI_IN_VOLUME_OUT, conf.midiInVolumeOut);
	conf.midiInBeatDouble           =  j.value(CONF_KEY_MIDI_IN_BEAT_DOUBLE, conf.midiInBeatDouble);
	conf.midiInBeatHalf             =  j.value(CONF_KEY_MIDI_IN_BEAT_HALF, conf.midiInBeatHalf);
#ifdef WITH_VST
	conf.pluginChooserX             = j.value(CONF_KEY_PLUGIN_CHOOSER_X, conf.pluginChooserX);
	conf.pluginChooserY             = j.value(CONF_KEY_PLUGIN_CHOOSER_Y, conf.pluginChooserY);
	conf.pluginChooserW             = j.value(CONF_KEY_PLUGIN_CHOOSER_W, conf.pluginChooserW);
	conf.pluginChooserH             = j.value(CONF_KEY_PLUGIN_CHOOSER_H, conf.pluginChooserH);
	conf.pluginSortMethod           = j.value(CONF_KEY_PLUGIN_SORT_METHOD, conf.pluginSortMethod);
#endif

	return true;
}


/* -------------------------------------------------------------------------- */


bool write()
{
	if (!createConfigFolder_())
		return false;

	nl::json j;

	j[CONF_KEY_HEADER]                        = "GIADACFG";
	j[CONF_KEY_LOG_MODE]                      = conf.logMode;
	j[CONF_KEY_SOUND_SYSTEM]                  = conf.soundSystem;
	j[CONF_KEY_SOUND_DEVICE_OUT]              = conf.soundDeviceOut;
	j[CONF_KEY_SOUND_DEVICE_IN]               = conf.soundDeviceIn;
	j[CONF_KEY_CHANNELS_OUT]                  = conf.channelsOut;
	j[CONF_KEY_CHANNELS_IN_COUNT]             = conf.channelsInCount;
	j[CONF_KEY_CHANNELS_IN_START]             = conf.channelsInStart;
	j[CONF_KEY_SAMPLERATE]                    = conf.samplerate;
	j[CONF_KEY_BUFFER_SIZE]                   = conf.buffersize;
	j[CONF_KEY_LIMIT_OUTPUT]                  = conf.limitOutput;
	j[CONF_KEY_RESAMPLE_QUALITY]              = conf.rsmpQuality;
	j[CONF_KEY_MIDI_SYSTEM]                   = conf.midiSystem;
	j[CONF_KEY_MIDI_PORT_OUT]                 = conf.midiPortOut;
	j[CONF_KEY_MIDI_PORT_IN]                  = conf.midiPortIn;
	j[CONF_KEY_MIDIMAP_PATH]                  = conf.midiMapPath;
	j[CONF_KEY_LAST_MIDIMAP]                  = conf.lastFileMap;
	j[CONF_KEY_MIDI_SYNC]                     = conf.midiSync;
	j[CONF_KEY_MIDI_TC_FPS]                   = conf.midiTCfps;
	j[CONF_KEY_MIDI_IN]                       = conf.midiInEnabled;
	j[CONF_KEY_MIDI_IN_FILTER]                = conf.midiInFilter;
	j[CONF_KEY_MIDI_IN_REWIND]                = conf.midiInRewind;
	j[CONF_KEY_MIDI_IN_START_STOP]            = conf.midiInStartStop;
	j[CONF_KEY_MIDI_IN_ACTION_REC]            = conf.midiInActionRec;
	j[CONF_KEY_MIDI_IN_INPUT_REC]             = conf.midiInInputRec;
	j[CONF_KEY_MIDI_IN_METRONOME]             = conf.midiInMetronome;
	j[CONF_KEY_MIDI_IN_VOLUME_IN]             = conf.midiInVolumeIn;
	j[CONF_KEY_MIDI_IN_VOLUME_OUT]            = conf.midiInVolumeOut;
	j[CONF_KEY_MIDI_IN_BEAT_DOUBLE]           = conf.midiInBeatDouble;
	j[CONF_KEY_MIDI_IN_BEAT_HALF]             = conf.midiInBeatHalf;
	j[CONF_KEY_CHANS_STOP_ON_SEQ_HALT]        = conf.chansStopOnSeqHalt;
	j[CONF_KEY_TREAT_RECS_AS_LOOPS]           = conf.treatRecsAsLoops;
	j[CONF_KEY_INPUT_MONITOR_DEFAULT_ON]      = conf.inputMonitorDefaultOn;
	j[CONF_KEY_OVERDUB_PROTECTION_DEFAULT_ON] = conf.overdubProtectionDefaultOn;
	j[CONF_KEY_PLUGINS_PATH]                  = conf.pluginPath;
	j[CONF_KEY_PATCHES_PATH]                  = conf.patchPath;
	j[CONF_KEY_SAMPLES_PATH]                  = conf.samplePath;
	j[CONF_KEY_MAIN_WINDOW_X]                 = conf.mainWindowX;
	j[CONF_KEY_MAIN_WINDOW_Y]                 = conf.mainWindowY;
	j[CONF_KEY_MAIN_WINDOW_W]                 = conf.mainWindowW;
	j[CONF_KEY_MAIN_WINDOW_H]                 = conf.mainWindowH;
	j[CONF_KEY_BROWSER_X]                     = conf.browserX;
	j[CONF_KEY_BROWSER_Y]                     = conf.browserY;
	j[CONF_KEY_BROWSER_W]                     = conf.browserW;
	j[CONF_KEY_BROWSER_H]                     = conf.browserH;
	j[CONF_KEY_BROWSER_POSITION]              = conf.browserPosition;
	j[CONF_KEY_BROWSER_LAST_PATH]             = conf.browserLastPath;
	j[CONF_KEY_BROWSER_LAST_VALUE]            = conf.browserLastValue;
	j[CONF_KEY_ACTION_EDITOR_X]               = conf.actionEditorX;
	j[CONF_KEY_ACTION_EDITOR_Y]               = conf.actionEditorY;
	j[CONF_KEY_ACTION_EDITOR_W]               = conf.actionEditorW;
	j[CONF_KEY_ACTION_EDITOR_H]               = conf.actionEditorH;
	j[CONF_KEY_ACTION_EDITOR_ZOOM]            = conf.actionEditorZoom;
	j[CONF_KEY_ACTION_EDITOR_GRID_VAL]        = conf.actionEditorGridVal;
	j[CONF_KEY_ACTION_EDITOR_GRID_ON]         = conf.actionEditorGridOn;
	j[CONF_KEY_SAMPLE_EDITOR_X]               = conf.sampleEditorX;
	j[CONF_KEY_SAMPLE_EDITOR_Y]               = conf.sampleEditorY;
	j[CONF_KEY_SAMPLE_EDITOR_W]               = conf.sampleEditorW;
	j[CONF_KEY_SAMPLE_EDITOR_H]               = conf.sampleEditorH;
	j[CONF_KEY_SAMPLE_EDITOR_GRID_VAL]        = conf.sampleEditorGridVal;
	j[CONF_KEY_SAMPLE_EDITOR_GRID_ON]         = conf.sampleEditorGridOn;
	j[CONF_KEY_PIANO_ROLL_Y]                  = conf.pianoRollY;
	j[CONF_KEY_PIANO_ROLL_H]                  = conf.pianoRollH;
	j[CONF_KEY_SAMPLE_ACTION_EDITOR_H]        = conf.sampleActionEditorH;
	j[CONF_KEY_VELOCITY_EDITOR_H]             = conf.velocityEditorH;
	j[CONF_KEY_ENVELOPE_EDITOR_H]             = conf.envelopeEditorH;
	j[CONF_KEY_PLUGIN_LIST_X]                 = conf.pluginListX;
	j[CONF_KEY_PLUGIN_LIST_Y]                 = conf.pluginListY;
	j[CONF_KEY_MIDI_INPUT_X]                  = conf.midiInputX;
	j[CONF_KEY_MIDI_INPUT_Y]                  = conf.midiInputY;
	j[CONF_KEY_MIDI_INPUT_W]                  = conf.midiInputW;
	j[CONF_KEY_MIDI_INPUT_H]                  = conf.midiInputH;
	j[CONF_KEY_REC_TRIGGER_MODE]              = static_cast<int>(conf.recTriggerMode);
	j[CONF_KEY_REC_TRIGGER_LEVEL]             = conf.recTriggerLevel;
#ifdef WITH_VST
	j[CONF_KEY_PLUGIN_CHOOSER_X]              = conf.pluginChooserX;
	j[CONF_KEY_PLUGIN_CHOOSER_Y]              = conf.pluginChooserY;
	j[CONF_KEY_PLUGIN_CHOOSER_W]              = conf.pluginChooserW;
	j[CONF_KEY_PLUGIN_CHOOSER_H]              = conf.pluginChooserH;
	j[CONF_KEY_PLUGIN_SORT_METHOD]            = conf.pluginSortMethod;
#endif

    std::ofstream ofs(confFilePath_);
	if (!ofs.good()) {
		u::log::print("[conf::write] unable to write configuration file!\n");
		return false;
	}

    ofs << j;
	return true;
}
}}} // giada::m::conf::