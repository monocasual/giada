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
 * will be useful, but WITHOUT ANY WARRANTY without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------- */

#ifndef G_CONST_H
#define G_CONST_H

#include "core/types.h"
#include "deps/rtaudio/RtAudio.h"
#include "version.h"
#include <RtMidi.h>
#include <cstdint>

namespace giada
{
/* -- environment ----------------------------------------------------------- */
#if defined(_WIN32)
#define G_OS_WINDOWS
#elif defined(__APPLE__)
#define G_OS_MAC
#elif defined(__linux__)
#define G_OS_LINUX
#elif defined(__FreeBSD__)
#define G_OS_FREEBSD
#endif

#ifndef NDEBUG
#define G_DEBUG_MODE
#endif

#ifndef BUILD_DATE
#define BUILD_DATE __DATE__
#endif

/* -- version --------------------------------------------------------------- */
constexpr auto    G_APP_NAME = "Giada";
constexpr Version G_VERSION(1, 2, 1);

constexpr auto G_CONF_FILENAME = "giada.conf";

/* -- Engine ---------------------------------------------------------------- */
/* G_EVENT_DISPATCHER_RATE_MS
The amount of sleep between each Event Dispatcher cycle. It should be lower
than the audio thread sleep time. Note: this value will obviously increase the
live input latency, keep it small! */
constexpr int G_EVENT_DISPATCHER_RATE_MS = 5;

/* G_KERNEL_MIDI_OUTPUT_RATE_MS
The rate at which KernelMidi spits out MIDI events. Note: this value will
obviously increase the MIDI output latency, keep it small!*/
constexpr int G_KERNEL_MIDI_OUTPUT_RATE_MS = 3;

/* G_KERNEL_MIDI_INPUT_RATE_MS
The rate at which KernelMidi checks for MIDI events received from the devices.
Note: this value will obviously increase the MIDI latency, keep it small! */
constexpr int G_KERNEL_MIDI_INPUT_RATE_MS = 3;

/* -- GUI ------------------------------------------------------------------- */
constexpr int   G_GUI_FPS            = 30;
constexpr float G_GUI_REFRESH_RATE   = 1 / static_cast<float>(G_GUI_FPS);
constexpr int   G_GUI_FONT_SIZE_BASE = 12;
constexpr int   G_GUI_INNER_MARGIN   = 4;
constexpr int   G_GUI_OUTER_MARGIN   = 8;
constexpr int   G_GUI_UNIT           = 20; // base unit for elements
constexpr int   G_GUI_ZOOM_FACTOR    = 2;

#define G_COLOR_RED fl_rgb_color(28, 32, 80)
#define G_COLOR_BLUE fl_rgb_color(113, 31, 31)
#define G_COLOR_LIGHT_2 fl_rgb_color(200, 200, 200)
#define G_COLOR_LIGHT_1 fl_rgb_color(170, 170, 170)
#define G_COLOR_GREY_5 fl_rgb_color(110, 110, 110)
#define G_COLOR_GREY_4 fl_rgb_color(78, 78, 78)
#define G_COLOR_GREY_3 fl_rgb_color(54, 54, 54)
#define G_COLOR_GREY_2 fl_rgb_color(37, 37, 37)
#define G_COLOR_GREY_1_5 fl_rgb_color(28, 28, 28)
#define G_COLOR_GREY_1 fl_rgb_color(25, 25, 25)
#define G_COLOR_BLACK fl_rgb_color(0, 0, 0)

/* -- MIN/MAX values -------------------------------------------------------- */
constexpr float G_MIN_BPM               = 20.0f;
constexpr float G_MAX_BPM               = 999.0f;
constexpr int   G_MAX_BEATS             = 32;
constexpr int   G_MAX_BARS              = 32;
constexpr int   G_MAX_QUANTIZE          = 8;
constexpr float G_MIN_DB_SCALE          = 60.0f;
constexpr int   G_MIN_TRACK_WIDTH       = 140;
constexpr float G_MIN_PITCH             = 0.1f;
constexpr float G_MAX_PITCH             = 4.0f;
constexpr float G_MAX_PAN               = 1.0f;
constexpr float G_MAX_VOLUME            = 1.0f;
constexpr int   G_MIN_GUI_WIDTH         = 816;
constexpr int   G_MIN_GUI_HEIGHT        = 510;
constexpr int   G_MAX_IO_CHANS          = 2;
constexpr int   G_MAX_VELOCITY          = 0x7F;
constexpr float G_MAX_VELOCITY_FLOAT    = 1.0f;
constexpr int   G_MAX_MIDI_CHANS        = 16;
constexpr int   G_MAX_DISPATCHER_EVENTS = 32;
constexpr int   G_MAX_SEQUENCER_EVENTS  = 128;  // Per block
constexpr float G_MIN_UI_SCALING        = 0.0f; // Auto: FLTK will figure it out
constexpr float G_MAX_UI_SCALING        = 4.0f;

/* -- default values -------------------------------------------------------- */
constexpr RtAudio::Api G_DEFAULT_SOUNDSYS            = RtAudio::Api::UNSPECIFIED;
constexpr int          G_DEFAULT_SOUNDDEV_OUT        = -1; // auto by default: RtAudio will figure it out
constexpr int          G_DEFAULT_SOUNDDEV_IN         = -1; // auto by default: RtAudio will figure it out
constexpr RtMidi::Api  G_DEFAULT_MIDI_API            = RtMidi::Api::UNSPECIFIED;
constexpr int          G_DEFAULT_MIDI_PORT_IN        = -1;
constexpr int          G_DEFAULT_MIDI_PORT_OUT       = -1;
constexpr int          G_DEFAULT_SAMPLERATE          = 44100;
constexpr int          G_DEFAULT_BUFSIZE             = 1024;
constexpr int          G_DEFAULT_BIT_DEPTH           = 32;
constexpr float        G_DEFAULT_VOL                 = 1.0f;
constexpr float        G_DEFAULT_PAN                 = 0.5f;
constexpr float        G_DEFAULT_PITCH               = 1.0f;
constexpr float        G_DEFAULT_BPM                 = 120.0f;
constexpr int          G_DEFAULT_BEATS               = 4;
constexpr int          G_DEFAULT_BARS                = 1;
constexpr int          G_DEFAULT_QUANTIZE            = 0;     // quantizer off
constexpr float        G_DEFAULT_FADEOUT_STEP        = 0.01f; // micro-fadeout speed
constexpr int          G_DEFAULT_TRACK_WIDTH         = 380;
constexpr auto         G_DEFAULT_PATCH_NAME          = "(default patch)";
constexpr int          G_DEFAULT_ACTION_SIZE         = 8192; // frames
constexpr int          G_DEFAULT_ZOOM_RATIO          = 128;
constexpr float        G_DEFAULT_REC_TRIGGER_LEVEL   = -10.0f;
constexpr int          G_DEFAULT_SUBWINDOW_W         = 640;
constexpr int          G_DEFAULT_SUBWINDOW_H         = 480;
constexpr int          G_DEFAULT_VST_MIDIBUFFER_SIZE = 1024; // TODO - not 100% sure about this size
constexpr float        G_DEFAULT_UI_SCALING          = G_MIN_UI_SCALING;

/* -- responses and return codes -------------------------------------------- */
constexpr int G_RES_ERR_PROCESSING    = -6;
constexpr int G_RES_ERR_WRONG_DATA    = -5;
constexpr int G_RES_ERR_NO_DATA       = -4;
constexpr int G_RES_ERR_PATH_TOO_LONG = -3;
constexpr int G_RES_ERR_IO            = -2;
constexpr int G_RES_ERR_MEMORY        = -1;
constexpr int G_RES_ERR               = 0;
constexpr int G_RES_OK                = 1;

/* -- log modes ------------------------------------------------------------- */
constexpr int LOG_MODE_MUTE   = 0;
constexpr int LOG_MODE_STDOUT = 1;
constexpr int LOG_MODE_FILE   = 2;

/* -- unique IDs of mainWin's subwindows ------------------------------------ */
constexpr ID WID_BEATS           = 1;
constexpr ID WID_BPM             = 2;
constexpr ID WID_ABOUT           = 3;
constexpr ID WID_FILE_BROWSER    = 4;
constexpr ID WID_CONFIG          = 5;
constexpr ID WID_FX_LIST         = 6;
constexpr ID WID_ACTION_EDITOR   = 7;
constexpr ID WID_SAMPLE_EDITOR   = 8;
constexpr ID WID_KEY_GRABBER     = 10;
constexpr ID WID_SAMPLE_NAME     = 11;
constexpr ID WID_FX_CHOOSER      = 12;
constexpr ID WID_MIDI_INPUT      = 13;
constexpr ID WID_MIDI_OUTPUT     = 14;
constexpr ID WID_MISSING_ASSETS  = 15;
constexpr ID WID_CHANNEL_ROUTING = 16;
constexpr ID WID_FX              = 1000; // Plug-in windows have ID == WID_FX + [pluginId]

/* -- File signals ---------------------------------------------------------- */
constexpr int G_FILE_NOT_SPECIFIED = -3;
constexpr int G_FILE_UNSUPPORTED   = -2;
constexpr int G_FILE_UNREADABLE    = -1;
constexpr int G_FILE_INVALID       = 0;
constexpr int G_FILE_OK            = 1;

/* -- File system ----------------------------------------------------------- */
constexpr auto G_PATCH_EXT   = ".gptc";
constexpr auto G_PROJECT_EXT = ".gprj";

/* -- MIDI in parameters (for MIDI learning) -------------------------------- */
constexpr int G_MIDI_IN_ENABLED      = 1;
constexpr int G_MIDI_IN_FILTER       = 2;
constexpr int G_MIDI_IN_REWIND       = 3;
constexpr int G_MIDI_IN_START_STOP   = 4;
constexpr int G_MIDI_IN_ACTION_REC   = 5;
constexpr int G_MIDI_IN_INPUT_REC    = 6;
constexpr int G_MIDI_IN_METRONOME    = 7;
constexpr int G_MIDI_IN_VOLUME_IN    = 8;
constexpr int G_MIDI_IN_VOLUME_OUT   = 9;
constexpr int G_MIDI_IN_BEAT_DOUBLE  = 10;
constexpr int G_MIDI_IN_BEAT_HALF    = 11;
constexpr int G_MIDI_IN_KEYPRESS     = 12;
constexpr int G_MIDI_IN_KEYREL       = 13;
constexpr int G_MIDI_IN_KILL         = 14;
constexpr int G_MIDI_IN_ARM          = 15;
constexpr int G_MIDI_IN_MUTE         = 16;
constexpr int G_MIDI_IN_SOLO         = 17;
constexpr int G_MIDI_IN_VOLUME       = 18;
constexpr int G_MIDI_IN_PITCH        = 19;
constexpr int G_MIDI_IN_READ_ACTIONS = 20;

/* -- MIDI out parameters (for MIDI output and lightning) ------------------- */
constexpr int G_MIDI_OUT_ENABLED   = 1;
constexpr int G_MIDI_OUT_L_ENABLED = 2;
constexpr int G_MIDI_OUT_L_PLAYING = 3;
constexpr int G_MIDI_OUT_L_MUTE    = 4;
constexpr int G_MIDI_OUT_L_SOLO    = 5;

/* -- MIDI signals -------------------------------------------------------------
Channel voices messages - controller (0xB0) is a special subset of this family:
it drives knobs, volume, faders and such. */

constexpr uint32_t G_MIDI_CONTROLLER    = static_cast<uint32_t>(0xB0 << 24);
constexpr uint32_t G_MIDI_ALL_NOTES_OFF = (G_MIDI_CONTROLLER) | (0x7B << 16);

/* MIDI sync constants */

constexpr int G_MIDI_SYNC_NONE         = 0;
constexpr int G_MIDI_SYNC_CLOCK_MASTER = 1;
constexpr int G_MIDI_SYNC_CLOCK_SLAVE  = 2;

/* JSON patch keys */

constexpr auto PATCH_KEY_HEADER                       = "header";
constexpr auto PATCH_KEY_VERSION_MAJOR                = "version_major";
constexpr auto PATCH_KEY_VERSION_MINOR                = "version_minor";
constexpr auto PATCH_KEY_VERSION_PATCH                = "version_patch";
constexpr auto PATCH_KEY_NAME                         = "name";
constexpr auto PATCH_KEY_BPM                          = "bpm";
constexpr auto PATCH_KEY_BARS                         = "bars";
constexpr auto PATCH_KEY_BEATS                        = "beats";
constexpr auto PATCH_KEY_QUANTIZE                     = "quantize";
constexpr auto PATCH_KEY_MASTER_VOL_IN                = "master_vol_in";
constexpr auto PATCH_KEY_MASTER_VOL_OUT               = "master_vol_out";
constexpr auto PATCH_KEY_METRONOME                    = "metronome";
constexpr auto PATCH_KEY_SAMPLERATE                   = "samplerate";
constexpr auto PATCH_KEY_TRACKS                       = "tracks";
constexpr auto PATCH_KEY_PLUGINS                      = "plugins";
constexpr auto PATCH_KEY_MASTER_OUT_PLUGINS           = "master_out_plugins";
constexpr auto PATCH_KEY_MASTER_IN_PLUGINS            = "master_in_plugins";
constexpr auto PATCH_KEY_CHANNELS                     = "channels";
constexpr auto PATCH_KEY_CHANNEL_TYPE                 = "type";
constexpr auto PATCH_KEY_CHANNEL_ID                   = "id";
constexpr auto PATCH_KEY_CHANNEL_SIZE                 = "size";
constexpr auto PATCH_KEY_CHANNEL_NAME                 = "name";
constexpr auto PATCH_KEY_CHANNEL_MUTE                 = "mute";
constexpr auto PATCH_KEY_CHANNEL_SOLO                 = "solo";
constexpr auto PATCH_KEY_CHANNEL_VOLUME               = "volume";
constexpr auto PATCH_KEY_CHANNEL_PAN                  = "pan";
constexpr auto PATCH_KEY_CHANNEL_MIDI_IN              = "midi_in";
constexpr auto PATCH_KEY_CHANNEL_MIDI_IN_VELO_AS_VOL  = "midi_in_velo_as_vol";
constexpr auto PATCH_KEY_CHANNEL_MIDI_IN_KEYPRESS     = "midi_in_keypress";
constexpr auto PATCH_KEY_CHANNEL_MIDI_IN_KEYREL       = "midi_in_keyrel";
constexpr auto PATCH_KEY_CHANNEL_MIDI_IN_KILL         = "midi_in_kill";
constexpr auto PATCH_KEY_CHANNEL_MIDI_IN_ARM          = "midi_in_arm";
constexpr auto PATCH_KEY_CHANNEL_MIDI_IN_VOLUME       = "midi_in_volume";
constexpr auto PATCH_KEY_CHANNEL_MIDI_IN_MUTE         = "midi_in_mute";
constexpr auto PATCH_KEY_CHANNEL_MIDI_IN_FILTER       = "midi_in_filter";
constexpr auto PATCH_KEY_CHANNEL_MIDI_IN_SOLO         = "midi_in_solo";
constexpr auto PATCH_KEY_CHANNEL_MIDI_OUT_L           = "midi_out_l";
constexpr auto PATCH_KEY_CHANNEL_MIDI_OUT_L_PLAYING   = "midi_out_l_playing";
constexpr auto PATCH_KEY_CHANNEL_MIDI_OUT_L_MUTE      = "midi_out_l_mute";
constexpr auto PATCH_KEY_CHANNEL_MIDI_OUT_L_SOLO      = "midi_out_l_solo";
constexpr auto PATCH_KEY_CHANNEL_WAVE_ID              = "wave_id";
constexpr auto PATCH_KEY_CHANNEL_KEY                  = "key";
constexpr auto PATCH_KEY_CHANNEL_MODE                 = "mode";
constexpr auto PATCH_KEY_CHANNEL_BEGIN                = "begin";
constexpr auto PATCH_KEY_CHANNEL_END                  = "end";
constexpr auto PATCH_KEY_CHANNEL_SHIFT                = "shift";
constexpr auto PATCH_KEY_CHANNEL_HAS_ACTIONS          = "has_actions";
constexpr auto PATCH_KEY_CHANNEL_READ_ACTIONS         = "read_actions";
constexpr auto PATCH_KEY_CHANNEL_PITCH                = "pitch";
constexpr auto PATCH_KEY_CHANNEL_INPUT_MONITOR        = "input_monitor";
constexpr auto PATCH_KEY_CHANNEL_OVERDUB_PROTECTION   = "overdub_protection";
constexpr auto PATCH_KEY_CHANNEL_MIDI_IN_READ_ACTIONS = "midi_in_read_actions";
constexpr auto PATCH_KEY_CHANNEL_MIDI_IN_PITCH        = "midi_in_pitch";
constexpr auto PATCH_KEY_CHANNEL_MIDI_OUT             = "midi_out";
constexpr auto PATCH_KEY_CHANNEL_MIDI_OUT_CHAN        = "midi_out_chan";
constexpr auto PATCH_KEY_CHANNEL_PLUGINS              = "plugins";
constexpr auto PATCH_KEY_CHANNEL_PLUGIN_ID            = "plugin_id";
constexpr auto PATCH_KEY_CHANNEL_ARMED                = "armed";
constexpr auto PATCH_KEY_WAVES                        = "waves";
constexpr auto PATCH_KEY_WAVE_ID                      = "id";
constexpr auto PATCH_KEY_WAVE_PATH                    = "path";
constexpr auto PATCH_KEY_ACTIONS                      = "actions";
constexpr auto PATCH_KEY_ACTION_TYPE                  = "type";
constexpr auto PATCH_KEY_ACTION_FRAME                 = "frame";
constexpr auto PATCH_KEY_ACTION_F_VALUE               = "f_value";
constexpr auto PATCH_KEY_ACTION_I_VALUE               = "i_value";
constexpr auto PATCH_KEY_PLUGIN_ID                    = "id";
constexpr auto PATCH_KEY_PLUGIN_PATH                  = "path";
constexpr auto PATCH_KEY_PLUGIN_BYPASS                = "bypass";
constexpr auto PATCH_KEY_PLUGIN_PARAMS                = "params";
constexpr auto PATCH_KEY_PLUGIN_STATE                 = "state";
constexpr auto PATCH_KEY_PLUGIN_MIDI_IN_PARAMS        = "midi_in_params";
constexpr auto PATCH_KEY_TRACK_WIDTH                  = "width";
constexpr auto PATCH_KEY_TRACK_INTERNAL               = "internal";
constexpr auto PATCH_KEY_TRACK_CHANNELS               = "channels";
constexpr auto G_PATCH_KEY_ACTION_ID                  = "id";
constexpr auto G_PATCH_KEY_ACTION_CHANNEL             = "channel";
constexpr auto G_PATCH_KEY_ACTION_FRAME               = "frame";
constexpr auto G_PATCH_KEY_ACTION_EVENT               = "event";
constexpr auto G_PATCH_KEY_ACTION_PREV                = "prev";
constexpr auto G_PATCH_KEY_ACTION_NEXT                = "next";

/* JSON config keys */

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

/* JSON midimaps keys */

constexpr auto MIDIMAP_KEY_BRAND             = "brand";
constexpr auto MIDIMAP_KEY_DEVICE            = "device";
constexpr auto MIDIMAP_KEY_INIT_COMMANDS     = "init_commands";
constexpr auto MIDIMAP_KEY_MUTE_ON           = "mute_on";
constexpr auto MIDIMAP_KEY_MUTE_OFF          = "mute_off";
constexpr auto MIDIMAP_KEY_SOLO_ON           = "solo_on";
constexpr auto MIDIMAP_KEY_SOLO_OFF          = "solo_off";
constexpr auto MIDIMAP_KEY_WAITING           = "waiting";
constexpr auto MIDIMAP_KEY_PLAYING           = "playing";
constexpr auto MIDIMAP_KEY_PLAYING_INAUDIBLE = "playing_inaudible";
constexpr auto MIDIMAP_KEY_STOPPING          = "stopping";
constexpr auto MIDIMAP_KEY_STOPPED           = "stopped";
constexpr auto MIDIMAP_KEY_CHANNEL           = "channel";
constexpr auto MIDIMAP_KEY_MESSAGE           = "message";
} // namespace giada

#endif
