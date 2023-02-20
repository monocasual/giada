/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "core/confFactory.h"
#include "core/const.h"
#include "utils/fs.h"
#include "utils/log.h"
#include <fstream>
#include <nlohmann/json.hpp>

namespace giada::m::confFactory
{
namespace
{
void sanitize_(Conf& conf)
{
	conf.soundDeviceOut   = std::max(0, conf.soundDeviceOut);
	conf.channelsOutCount = G_MAX_IO_CHANS;
	conf.channelsOutStart = std::max(0, conf.channelsOutStart);
	conf.channelsInCount  = std::max(1, conf.channelsInCount);
	conf.channelsInStart  = std::max(0, conf.channelsInStart);

	conf.midiPortOut = std::max(-1, conf.midiPortOut);
	conf.midiPortIn  = std::max(-1, conf.midiPortIn);

	conf.uiScaling = std::clamp(conf.uiScaling, G_MIN_UI_SCALING, G_MAX_UI_SCALING);
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

bool serialize(const Conf& conf)
{
	if (!u::fs::createConfigFolder())
		return false;

	nlohmann::json j;

	j[CONF_KEY_HEADER]                        = "GIADACFG";
	j[CONF_KEY_LOG_MODE]                      = conf.logMode;
	j[CONF_KEY_SHOW_TOOLTIPS]                 = conf.showTooltips;
	j[CONF_KEY_LANGMAP]                       = conf.langMap;
	j[CONF_KEY_SOUND_SYSTEM]                  = conf.soundSystem;
	j[CONF_KEY_SOUND_DEVICE_OUT]              = conf.soundDeviceOut;
	j[CONF_KEY_SOUND_DEVICE_IN]               = conf.soundDeviceIn;
	j[CONF_KEY_CHANNELS_OUT_COUNT]            = conf.channelsOutCount;
	j[CONF_KEY_CHANNELS_OUT_START]            = conf.channelsOutStart;
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
	j[CONF_KEY_MAIN_WINDOW_X]                 = conf.mainWindowBounds.x;
	j[CONF_KEY_MAIN_WINDOW_Y]                 = conf.mainWindowBounds.y;
	j[CONF_KEY_MAIN_WINDOW_W]                 = conf.mainWindowBounds.w;
	j[CONF_KEY_MAIN_WINDOW_H]                 = conf.mainWindowBounds.h;
	j[CONF_KEY_BROWSER_X]                     = conf.browserBounds.x;
	j[CONF_KEY_BROWSER_Y]                     = conf.browserBounds.y;
	j[CONF_KEY_BROWSER_W]                     = conf.browserBounds.w;
	j[CONF_KEY_BROWSER_H]                     = conf.browserBounds.h;
	j[CONF_KEY_BROWSER_POSITION]              = conf.browserPosition;
	j[CONF_KEY_BROWSER_LAST_PATH]             = conf.browserLastPath;
	j[CONF_KEY_BROWSER_LAST_VALUE]            = conf.browserLastValue;
	j[CONF_KEY_ACTION_EDITOR_X]               = conf.actionEditorBounds.x;
	j[CONF_KEY_ACTION_EDITOR_Y]               = conf.actionEditorBounds.y;
	j[CONF_KEY_ACTION_EDITOR_W]               = conf.actionEditorBounds.w;
	j[CONF_KEY_ACTION_EDITOR_H]               = conf.actionEditorBounds.h;
	j[CONF_KEY_ACTION_EDITOR_ZOOM]            = conf.actionEditorZoom;
	j[CONF_KEY_ACTION_EDITOR_SPLIT_H]         = conf.actionEditorSplitH;
	j[CONF_KEY_ACTION_EDITOR_GRID_VAL]        = conf.actionEditorGridVal;
	j[CONF_KEY_ACTION_EDITOR_GRID_ON]         = conf.actionEditorGridOn;
	j[CONF_KEY_ACTION_EDITOR_PIANO_ROLL_Y]    = conf.actionEditorPianoRollY;
	j[CONF_KEY_SAMPLE_EDITOR_X]               = conf.sampleEditorBounds.x;
	j[CONF_KEY_SAMPLE_EDITOR_Y]               = conf.sampleEditorBounds.y;
	j[CONF_KEY_SAMPLE_EDITOR_W]               = conf.sampleEditorBounds.w;
	j[CONF_KEY_SAMPLE_EDITOR_H]               = conf.sampleEditorBounds.h;
	j[CONF_KEY_SAMPLE_EDITOR_GRID_VAL]        = conf.sampleEditorGridVal;
	j[CONF_KEY_SAMPLE_EDITOR_GRID_ON]         = conf.sampleEditorGridOn;
	j[CONF_KEY_PLUGIN_LIST_X]                 = conf.pluginListBounds.x;
	j[CONF_KEY_PLUGIN_LIST_Y]                 = conf.pluginListBounds.y;
	j[CONF_KEY_PLUGIN_LIST_W]                 = conf.pluginListBounds.w;
	j[CONF_KEY_PLUGIN_LIST_H]                 = conf.pluginListBounds.h;
	j[CONF_KEY_MIDI_INPUT_X]                  = conf.midiInputBounds.x;
	j[CONF_KEY_MIDI_INPUT_Y]                  = conf.midiInputBounds.y;
	j[CONF_KEY_MIDI_INPUT_W]                  = conf.midiInputBounds.w;
	j[CONF_KEY_MIDI_INPUT_H]                  = conf.midiInputBounds.h;
	j[CONF_KEY_REC_TRIGGER_MODE]              = static_cast<int>(conf.recTriggerMode);
	j[CONF_KEY_REC_TRIGGER_LEVEL]             = conf.recTriggerLevel;
	j[CONF_KEY_INPUT_REC_MODE]                = static_cast<int>(conf.inputRecMode);

	j[CONF_KEY_BIND_PLAY]           = conf.keyBindings[Conf::KEY_BIND_PLAY];
	j[CONF_KEY_BIND_REWIND]         = conf.keyBindings[Conf::KEY_BIND_REWIND];
	j[CONF_KEY_BIND_RECORD_ACTIONS] = conf.keyBindings[Conf::KEY_BIND_RECORD_ACTIONS];
	j[CONF_KEY_BIND_RECORD_INPUT]   = conf.keyBindings[Conf::KEY_BIND_RECORD_INPUT];
	j[CONF_KEY_BIND_EXIT]           = conf.keyBindings[Conf::KEY_BIND_EXIT];

	j[CONF_KEY_PLUGIN_CHOOSER_X]   = conf.pluginChooserBounds.x;
	j[CONF_KEY_PLUGIN_CHOOSER_Y]   = conf.pluginChooserBounds.y;
	j[CONF_KEY_PLUGIN_CHOOSER_W]   = conf.pluginChooserBounds.w;
	j[CONF_KEY_PLUGIN_CHOOSER_H]   = conf.pluginChooserBounds.h;
	j[CONF_KEY_PLUGIN_SORT_METHOD] = conf.pluginSortMethod;

	j[CONF_KEY_UI_SCALING] = conf.uiScaling;

	std::ofstream ofs(u::fs::getConfigFilePath());
	if (!ofs.good())
	{
		u::log::print("[confFactory::serialize] unable to write configuration file!\n");
		return false;
	}

	ofs << j;
	return true;
}

/* -------------------------------------------------------------------------- */

Conf deserialize()
{
	Conf          conf;
	std::ifstream ifs(u::fs::getConfigFilePath());

	if (!ifs.good())
		return {};

	nlohmann::json j = nlohmann::json::parse(ifs);

	conf.valid                      = true;
	conf.logMode                    = j.value(CONF_KEY_LOG_MODE, conf.logMode);
	conf.showTooltips               = j.value(CONF_KEY_SHOW_TOOLTIPS, conf.showTooltips);
	conf.langMap                    = j.value(CONF_KEY_LANGMAP, conf.langMap);
	conf.soundSystem                = j.value(CONF_KEY_SOUND_SYSTEM, conf.soundSystem);
	conf.soundDeviceOut             = j.value(CONF_KEY_SOUND_DEVICE_OUT, conf.soundDeviceOut);
	conf.soundDeviceIn              = j.value(CONF_KEY_SOUND_DEVICE_IN, conf.soundDeviceIn);
	conf.channelsOutCount           = j.value(CONF_KEY_CHANNELS_OUT_COUNT, conf.channelsOutCount);
	conf.channelsOutStart           = j.value(CONF_KEY_CHANNELS_OUT_START, conf.channelsOutStart);
	conf.channelsInCount            = j.value(CONF_KEY_CHANNELS_IN_COUNT, conf.channelsInCount);
	conf.channelsInStart            = j.value(CONF_KEY_CHANNELS_IN_START, conf.channelsInStart);
	conf.samplerate                 = j.value(CONF_KEY_SAMPLERATE, conf.samplerate);
	conf.buffersize                 = j.value(CONF_KEY_BUFFER_SIZE, conf.buffersize);
	conf.limitOutput                = j.value(CONF_KEY_LIMIT_OUTPUT, conf.limitOutput);
	conf.rsmpQuality                = j.value(CONF_KEY_RESAMPLE_QUALITY, conf.rsmpQuality);
	conf.midiSystem                 = j.value(CONF_KEY_MIDI_SYSTEM, conf.midiSystem);
	conf.midiPortOut                = j.value(CONF_KEY_MIDI_PORT_OUT, conf.midiPortOut);
	conf.midiPortIn                 = j.value(CONF_KEY_MIDI_PORT_IN, conf.midiPortIn);
	conf.midiMapPath                = j.value(CONF_KEY_MIDIMAP_PATH, conf.midiMapPath);
	conf.midiSync                   = j.value(CONF_KEY_MIDI_SYNC, conf.midiSync);
	conf.midiTCfps                  = j.value(CONF_KEY_MIDI_TC_FPS, conf.midiTCfps);
	conf.chansStopOnSeqHalt         = j.value(CONF_KEY_CHANS_STOP_ON_SEQ_HALT, conf.chansStopOnSeqHalt);
	conf.treatRecsAsLoops           = j.value(CONF_KEY_TREAT_RECS_AS_LOOPS, conf.treatRecsAsLoops);
	conf.inputMonitorDefaultOn      = j.value(CONF_KEY_INPUT_MONITOR_DEFAULT_ON, conf.inputMonitorDefaultOn);
	conf.overdubProtectionDefaultOn = j.value(CONF_KEY_OVERDUB_PROTECTION_DEFAULT_ON, conf.overdubProtectionDefaultOn);
	conf.pluginPath                 = j.value(CONF_KEY_PLUGINS_PATH, conf.pluginPath);
	conf.patchPath                  = j.value(CONF_KEY_PATCHES_PATH, conf.patchPath);
	conf.samplePath                 = j.value(CONF_KEY_SAMPLES_PATH, conf.samplePath);
	conf.mainWindowBounds.x         = j.value(CONF_KEY_MAIN_WINDOW_X, conf.mainWindowBounds.x);
	conf.mainWindowBounds.y         = j.value(CONF_KEY_MAIN_WINDOW_Y, conf.mainWindowBounds.y);
	conf.mainWindowBounds.w         = j.value(CONF_KEY_MAIN_WINDOW_W, conf.mainWindowBounds.w);
	conf.mainWindowBounds.h         = j.value(CONF_KEY_MAIN_WINDOW_H, conf.mainWindowBounds.h);
	conf.browserBounds.x            = j.value(CONF_KEY_BROWSER_X, conf.browserBounds.x);
	conf.browserBounds.y            = j.value(CONF_KEY_BROWSER_Y, conf.browserBounds.y);
	conf.browserBounds.w            = j.value(CONF_KEY_BROWSER_W, conf.browserBounds.w);
	conf.browserBounds.h            = j.value(CONF_KEY_BROWSER_H, conf.browserBounds.h);
	conf.browserPosition            = j.value(CONF_KEY_BROWSER_POSITION, conf.browserPosition);
	conf.browserLastPath            = j.value(CONF_KEY_BROWSER_LAST_PATH, conf.browserLastPath);
	conf.browserLastValue           = j.value(CONF_KEY_BROWSER_LAST_VALUE, conf.browserLastValue);
	conf.actionEditorBounds.x       = j.value(CONF_KEY_ACTION_EDITOR_X, conf.actionEditorBounds.x);
	conf.actionEditorBounds.y       = j.value(CONF_KEY_ACTION_EDITOR_Y, conf.actionEditorBounds.y);
	conf.actionEditorBounds.w       = j.value(CONF_KEY_ACTION_EDITOR_W, conf.actionEditorBounds.w);
	conf.actionEditorBounds.h       = j.value(CONF_KEY_ACTION_EDITOR_H, conf.actionEditorBounds.h);
	conf.actionEditorZoom           = j.value(CONF_KEY_ACTION_EDITOR_ZOOM, conf.actionEditorZoom);
	conf.actionEditorSplitH         = j.value(CONF_KEY_ACTION_EDITOR_SPLIT_H, conf.actionEditorSplitH);
	conf.actionEditorGridVal        = j.value(CONF_KEY_ACTION_EDITOR_GRID_VAL, conf.actionEditorGridVal);
	conf.actionEditorGridOn         = j.value(CONF_KEY_ACTION_EDITOR_GRID_ON, conf.actionEditorGridOn);
	conf.actionEditorPianoRollY     = j.value(CONF_KEY_ACTION_EDITOR_PIANO_ROLL_Y, conf.actionEditorPianoRollY);
	conf.sampleEditorBounds.x       = j.value(CONF_KEY_SAMPLE_EDITOR_X, conf.sampleEditorBounds.x);
	conf.sampleEditorBounds.y       = j.value(CONF_KEY_SAMPLE_EDITOR_Y, conf.sampleEditorBounds.y);
	conf.sampleEditorBounds.w       = j.value(CONF_KEY_SAMPLE_EDITOR_W, conf.sampleEditorBounds.w);
	conf.sampleEditorBounds.h       = j.value(CONF_KEY_SAMPLE_EDITOR_H, conf.sampleEditorBounds.h);
	conf.sampleEditorGridVal        = j.value(CONF_KEY_SAMPLE_EDITOR_GRID_VAL, conf.sampleEditorGridVal);
	conf.sampleEditorGridOn         = j.value(CONF_KEY_SAMPLE_EDITOR_GRID_ON, conf.sampleEditorGridOn);
	conf.pluginListBounds.x         = j.value(CONF_KEY_PLUGIN_LIST_X, conf.pluginListBounds.x);
	conf.pluginListBounds.y         = j.value(CONF_KEY_PLUGIN_LIST_Y, conf.pluginListBounds.y);
	conf.pluginListBounds.w         = j.value(CONF_KEY_PLUGIN_LIST_W, conf.pluginListBounds.w);
	conf.pluginListBounds.h         = j.value(CONF_KEY_PLUGIN_LIST_H, conf.pluginListBounds.h);
	conf.midiInputBounds.x          = j.value(CONF_KEY_MIDI_INPUT_X, conf.midiInputBounds.x);
	conf.midiInputBounds.y          = j.value(CONF_KEY_MIDI_INPUT_Y, conf.midiInputBounds.y);
	conf.midiInputBounds.w          = j.value(CONF_KEY_MIDI_INPUT_W, conf.midiInputBounds.w);
	conf.midiInputBounds.h          = j.value(CONF_KEY_MIDI_INPUT_H, conf.midiInputBounds.h);
	conf.recTriggerMode             = j.value(CONF_KEY_REC_TRIGGER_MODE, conf.recTriggerMode);
	conf.recTriggerLevel            = j.value(CONF_KEY_REC_TRIGGER_LEVEL, conf.recTriggerLevel);
	conf.inputRecMode               = j.value(CONF_KEY_INPUT_REC_MODE, conf.inputRecMode);
	conf.midiInEnabled              = j.value(CONF_KEY_MIDI_IN, conf.midiInEnabled);
	conf.midiInFilter               = j.value(CONF_KEY_MIDI_IN_FILTER, conf.midiInFilter);
	conf.midiInRewind               = j.value(CONF_KEY_MIDI_IN_REWIND, conf.midiInRewind);
	conf.midiInStartStop            = j.value(CONF_KEY_MIDI_IN_START_STOP, conf.midiInStartStop);
	conf.midiInActionRec            = j.value(CONF_KEY_MIDI_IN_ACTION_REC, conf.midiInActionRec);
	conf.midiInInputRec             = j.value(CONF_KEY_MIDI_IN_INPUT_REC, conf.midiInInputRec);
	conf.midiInMetronome            = j.value(CONF_KEY_MIDI_IN_METRONOME, conf.midiInMetronome);
	conf.midiInVolumeIn             = j.value(CONF_KEY_MIDI_IN_VOLUME_IN, conf.midiInVolumeIn);
	conf.midiInVolumeOut            = j.value(CONF_KEY_MIDI_IN_VOLUME_OUT, conf.midiInVolumeOut);
	conf.midiInBeatDouble           = j.value(CONF_KEY_MIDI_IN_BEAT_DOUBLE, conf.midiInBeatDouble);
	conf.midiInBeatHalf             = j.value(CONF_KEY_MIDI_IN_BEAT_HALF, conf.midiInBeatHalf);

	conf.keyBindings[Conf::KEY_BIND_PLAY]           = j.value(CONF_KEY_BIND_PLAY, 0);
	conf.keyBindings[Conf::KEY_BIND_REWIND]         = j.value(CONF_KEY_BIND_REWIND, 0);
	conf.keyBindings[Conf::KEY_BIND_RECORD_ACTIONS] = j.value(CONF_KEY_BIND_RECORD_ACTIONS, 0);
	conf.keyBindings[Conf::KEY_BIND_RECORD_INPUT]   = j.value(CONF_KEY_BIND_RECORD_INPUT, 0);
	conf.keyBindings[Conf::KEY_BIND_EXIT]           = j.value(CONF_KEY_BIND_EXIT, 0);

	conf.pluginChooserBounds.x = j.value(CONF_KEY_PLUGIN_CHOOSER_X, conf.pluginChooserBounds.x);
	conf.pluginChooserBounds.y = j.value(CONF_KEY_PLUGIN_CHOOSER_Y, conf.pluginChooserBounds.y);
	conf.pluginChooserBounds.w = j.value(CONF_KEY_PLUGIN_CHOOSER_W, conf.pluginChooserBounds.w);
	conf.pluginChooserBounds.h = j.value(CONF_KEY_PLUGIN_CHOOSER_H, conf.pluginChooserBounds.h);
	conf.pluginSortMethod      = j.value(CONF_KEY_PLUGIN_SORT_METHOD, conf.pluginSortMethod);

	conf.uiScaling = j.value(CONF_KEY_UI_SCALING, conf.uiScaling);

	sanitize_(conf);

	return conf;
}
} // namespace giada::m::confFactory