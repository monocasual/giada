/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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
constexpr auto CONF_KEY_HEADER                        = "header";
constexpr auto CONF_KEY_LOG_MODE                      = "log_mode";
constexpr auto CONF_KEY_SHOW_TOOLTIPS                 = "show_tooltips";
constexpr auto CONF_KEY_LANGMAP                       = "langmap";
constexpr auto CONF_KEY_SOUND_SYSTEM                  = "sound_system";
constexpr auto CONF_KEY_SOUND_DEVICE_IN               = "sound_device_in";
constexpr auto CONF_KEY_SOUND_DEVICE_OUT              = "sound_device_out";
constexpr auto CONF_KEY_CHANNELS_OUT_COUNT            = "channels_out_count";
constexpr auto CONF_KEY_CHANNELS_OUT_START            = "channels_out_start";
constexpr auto CONF_KEY_CHANNELS_IN_COUNT             = "channels_in_count";
constexpr auto CONF_KEY_CHANNELS_IN_START             = "channels_in_start";
constexpr auto CONF_KEY_SAMPLERATE                    = "samplerate";
constexpr auto CONF_KEY_BUFFER_SIZE                   = "buffer_size";
constexpr auto CONF_KEY_DELAY_COMPENSATION            = "delay_compensation";
constexpr auto CONF_KEY_LIMIT_OUTPUT                  = "limit_output";
constexpr auto CONF_KEY_RESAMPLE_QUALITY              = "resample_quality";
constexpr auto CONF_KEY_MIDI_SYSTEM                   = "midi_system";
constexpr auto CONF_KEY_MIDI_PORT_OUT                 = "midi_port_out";
constexpr auto CONF_KEY_MIDI_PORT_IN                  = "midi_port_in";
constexpr auto CONF_KEY_MIDIMAP_PATH                  = "midimap_path";
constexpr auto CONF_KEY_LAST_MIDIMAP                  = "last_midimap";
constexpr auto CONF_KEY_MIDI_SYNC                     = "midi_sync";
constexpr auto CONF_KEY_MIDI_TC_FPS                   = "midi_tc_fps";
constexpr auto CONF_KEY_MIDI_IN                       = "midi_in";
constexpr auto CONF_KEY_MIDI_IN_FILTER                = "midi_in_filter";
constexpr auto CONF_KEY_MIDI_IN_REWIND                = "midi_in_rewind";
constexpr auto CONF_KEY_MIDI_IN_START_STOP            = "midi_in_start_stop";
constexpr auto CONF_KEY_MIDI_IN_ACTION_REC            = "midi_in_action_rec";
constexpr auto CONF_KEY_MIDI_IN_INPUT_REC             = "midi_in_input_rec";
constexpr auto CONF_KEY_MIDI_IN_METRONOME             = "midi_in_metronome";
constexpr auto CONF_KEY_MIDI_IN_VOLUME_IN             = "midi_in_volume_in";
constexpr auto CONF_KEY_MIDI_IN_VOLUME_OUT            = "midi_in_volume_out";
constexpr auto CONF_KEY_MIDI_IN_BEAT_DOUBLE           = "midi_in_beat_doble";
constexpr auto CONF_KEY_MIDI_IN_BEAT_HALF             = "midi_in_beat_half";
constexpr auto CONF_KEY_CHANS_STOP_ON_SEQ_HALT        = "chans_stop_on_seq_halt";
constexpr auto CONF_KEY_TREAT_RECS_AS_LOOPS           = "treat_recs_as_loops";
constexpr auto CONF_KEY_INPUT_MONITOR_DEFAULT_ON      = "input_monitor_default_on";
constexpr auto CONF_KEY_OVERDUB_PROTECTION_DEFAULT_ON = "overdub_protection_default_on";
constexpr auto CONF_KEY_PLUGINS_PATH                  = "plugins_path";
constexpr auto CONF_KEY_PATCHES_PATH                  = "patches_path";
constexpr auto CONF_KEY_SAMPLES_PATH                  = "samples_path";
constexpr auto CONF_KEY_MAIN_WINDOW_X                 = "main_window_x";
constexpr auto CONF_KEY_MAIN_WINDOW_Y                 = "main_window_y";
constexpr auto CONF_KEY_MAIN_WINDOW_W                 = "main_window_w";
constexpr auto CONF_KEY_MAIN_WINDOW_H                 = "main_window_h";
constexpr auto CONF_KEY_SETTINGS_WINDOW_X             = "settings_window_x";
constexpr auto CONF_KEY_SETTINGS_WINDOW_Y             = "settings_window_y";
constexpr auto CONF_KEY_SETTINGS_WINDOW_W             = "settings_window_w";
constexpr auto CONF_KEY_SETTINGS_WINDOW_H             = "settings_window_h";
constexpr auto CONF_KEY_BROWSER_X                     = "browser_x";
constexpr auto CONF_KEY_BROWSER_Y                     = "browser_y";
constexpr auto CONF_KEY_BROWSER_W                     = "browser_w";
constexpr auto CONF_KEY_BROWSER_H                     = "browser_h";
constexpr auto CONF_KEY_BROWSER_POSITION              = "browser_position";
constexpr auto CONF_KEY_BROWSER_LAST_PATH             = "browser_last_path";
constexpr auto CONF_KEY_BROWSER_LAST_VALUE            = "browser_last_value";
constexpr auto CONF_KEY_ACTION_EDITOR_X               = "action_editor_x";
constexpr auto CONF_KEY_ACTION_EDITOR_Y               = "action_editor_y";
constexpr auto CONF_KEY_ACTION_EDITOR_W               = "action_editor_w";
constexpr auto CONF_KEY_ACTION_EDITOR_H               = "action_editor_h";
constexpr auto CONF_KEY_ACTION_EDITOR_ZOOM            = "action_editor_zoom";
constexpr auto CONF_KEY_ACTION_EDITOR_SPLIT_H         = "action_editor_split_h";
constexpr auto CONF_KEY_ACTION_EDITOR_GRID_VAL        = "action_editor_grid_val";
constexpr auto CONF_KEY_ACTION_EDITOR_GRID_ON         = "action_editor_grid_on";
constexpr auto CONF_KEY_SAMPLE_EDITOR_X               = "sample_editor_x";
constexpr auto CONF_KEY_SAMPLE_EDITOR_Y               = "sample_editor_y";
constexpr auto CONF_KEY_SAMPLE_EDITOR_W               = "sample_editor_w";
constexpr auto CONF_KEY_SAMPLE_EDITOR_H               = "sample_editor_h";
constexpr auto CONF_KEY_SAMPLE_EDITOR_GRID_VAL        = "sample_editor_grid_val";
constexpr auto CONF_KEY_SAMPLE_EDITOR_GRID_ON         = "sample_editor_grid_on";
constexpr auto CONF_KEY_ACTION_EDITOR_PIANO_ROLL_Y    = "piano_roll_y";
constexpr auto CONF_KEY_PLUGIN_LIST_X                 = "plugin_list_x";
constexpr auto CONF_KEY_PLUGIN_LIST_Y                 = "plugin_list_y";
constexpr auto CONF_KEY_PLUGIN_LIST_W                 = "plugin_list_w";
constexpr auto CONF_KEY_PLUGIN_LIST_H                 = "plugin_list_h";
constexpr auto CONF_KEY_PLUGIN_CHOOSER_X              = "plugin_chooser_x";
constexpr auto CONF_KEY_PLUGIN_CHOOSER_Y              = "plugin_chooser_y";
constexpr auto CONF_KEY_PLUGIN_CHOOSER_W              = "plugin_chooser_w";
constexpr auto CONF_KEY_PLUGIN_CHOOSER_H              = "plugin_chooser_h";
constexpr auto CONF_KEY_MIDI_INPUT_X                  = "midi_input_x";
constexpr auto CONF_KEY_MIDI_INPUT_Y                  = "midi_input_y";
constexpr auto CONF_KEY_MIDI_INPUT_W                  = "midi_input_w";
constexpr auto CONF_KEY_MIDI_INPUT_H                  = "midi_input_h";
constexpr auto CONF_KEY_PLUGIN_SORT_METHOD            = "plugin_sort_method";
constexpr auto CONF_KEY_PLUGIN_SORT_DIR               = "plugin_sort_dir";
constexpr auto CONF_KEY_REC_TRIGGER_MODE              = "rec_trigger_mode";
constexpr auto CONF_KEY_REC_TRIGGER_LEVEL             = "rec_trigger_level";
constexpr auto CONF_KEY_INPUT_REC_MODE                = "input_rec_mode";
constexpr auto CONF_KEY_BIND_PLAY                     = "key_bind_play";
constexpr auto CONF_KEY_BIND_REWIND                   = "key_bind_rewind";
constexpr auto CONF_KEY_BIND_RECORD_ACTIONS           = "key_bind_record_actions";
constexpr auto CONF_KEY_BIND_RECORD_INPUT             = "key_bind_record_input";
constexpr auto CONF_KEY_BIND_EXIT                     = "key_bind_record_exit";
constexpr auto CONF_KEY_UI_SCALING                    = "ui_scaling";

/* -------------------------------------------------------------------------- */

Conf parse_(std::ifstream& ifs)
{
	Conf           conf;
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
	conf.midiDevicesOut             = j.value(CONF_KEY_MIDI_PORT_OUT, conf.midiDevicesOut);
	conf.midiDevicesIn              = j.value(CONF_KEY_MIDI_PORT_IN, conf.midiDevicesIn);
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
	conf.settingsBounds.x           = j.value(CONF_KEY_SETTINGS_WINDOW_X, conf.settingsBounds.x);
	conf.settingsBounds.y           = j.value(CONF_KEY_SETTINGS_WINDOW_Y, conf.settingsBounds.y);
	conf.settingsBounds.w           = j.value(CONF_KEY_SETTINGS_WINDOW_W, conf.settingsBounds.w);
	conf.settingsBounds.h           = j.value(CONF_KEY_SETTINGS_WINDOW_H, conf.settingsBounds.h);
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

	conf.keyBindPlay          = j.value(CONF_KEY_BIND_PLAY, 0);
	conf.keyBindRewind        = j.value(CONF_KEY_BIND_REWIND, 0);
	conf.keyBindRecordActions = j.value(CONF_KEY_BIND_RECORD_ACTIONS, 0);
	conf.keyBindRecordInput   = j.value(CONF_KEY_BIND_RECORD_INPUT, 0);
	conf.keyBindExit          = j.value(CONF_KEY_BIND_EXIT, 0);

	conf.pluginChooserBounds.x = j.value(CONF_KEY_PLUGIN_CHOOSER_X, conf.pluginChooserBounds.x);
	conf.pluginChooserBounds.y = j.value(CONF_KEY_PLUGIN_CHOOSER_Y, conf.pluginChooserBounds.y);
	conf.pluginChooserBounds.w = j.value(CONF_KEY_PLUGIN_CHOOSER_W, conf.pluginChooserBounds.w);
	conf.pluginChooserBounds.h = j.value(CONF_KEY_PLUGIN_CHOOSER_H, conf.pluginChooserBounds.h);
	conf.pluginSortMode.method = j.value(CONF_KEY_PLUGIN_SORT_METHOD, conf.pluginSortMode.method);
	conf.pluginSortMode.dir    = j.value(CONF_KEY_PLUGIN_SORT_DIR, conf.pluginSortMode.dir);

	conf.uiScaling = j.value(CONF_KEY_UI_SCALING, conf.uiScaling);

	return conf;
}

/* -------------------------------------------------------------------------- */

void sanitize_(Conf& conf)
{
	conf.soundDeviceOut   = std::max(0, conf.soundDeviceOut);
	conf.soundDeviceIn    = std::max(0, conf.soundDeviceIn);
	conf.channelsOutCount = G_MAX_IO_CHANS;
	conf.channelsOutStart = std::max(0, conf.channelsOutStart);
	conf.channelsInCount  = std::max(1, conf.channelsInCount);
	conf.channelsInStart  = std::max(0, conf.channelsInStart);

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
	j[CONF_KEY_MIDI_PORT_OUT]                 = conf.midiDevicesOut;
	j[CONF_KEY_MIDI_PORT_IN]                  = conf.midiDevicesIn;
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
	j[CONF_KEY_SETTINGS_WINDOW_X]             = conf.settingsBounds.x;
	j[CONF_KEY_SETTINGS_WINDOW_Y]             = conf.settingsBounds.y;
	j[CONF_KEY_SETTINGS_WINDOW_W]             = conf.settingsBounds.w;
	j[CONF_KEY_SETTINGS_WINDOW_H]             = conf.settingsBounds.h;
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

	j[CONF_KEY_BIND_PLAY]           = conf.keyBindPlay;
	j[CONF_KEY_BIND_REWIND]         = conf.keyBindRewind;
	j[CONF_KEY_BIND_RECORD_ACTIONS] = conf.keyBindRecordActions;
	j[CONF_KEY_BIND_RECORD_INPUT]   = conf.keyBindRecordInput;
	j[CONF_KEY_BIND_EXIT]           = conf.keyBindExit;

	j[CONF_KEY_PLUGIN_CHOOSER_X]   = conf.pluginChooserBounds.x;
	j[CONF_KEY_PLUGIN_CHOOSER_Y]   = conf.pluginChooserBounds.y;
	j[CONF_KEY_PLUGIN_CHOOSER_W]   = conf.pluginChooserBounds.w;
	j[CONF_KEY_PLUGIN_CHOOSER_H]   = conf.pluginChooserBounds.h;
	j[CONF_KEY_PLUGIN_SORT_METHOD] = conf.pluginSortMode.method;
	j[CONF_KEY_PLUGIN_SORT_DIR]    = conf.pluginSortMode.dir;

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
	std::ifstream ifs(u::fs::getConfigFilePath());
	if (!ifs.good())
		return {};

	try
	{
		Conf conf = parse_(ifs);
		sanitize_(conf);
		return conf;
	}
	catch (const nlohmann::json::exception& e)
	{
		u::log::print("[confFactory::deserialize] Unable to read configuration file! Reason: {}\n", e.what());
		return {};
	}
}
} // namespace giada::m::confFactory