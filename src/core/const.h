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


/* -- environment ----------------------------------------------------------- */
#if defined(_WIN32)
	#define G_OS_WINDOWS
#elif defined(__APPLE__)
	#define G_OS_MAC
#elif defined(__linux__)
	#define G_OS_LINUX
#endif

#ifndef BUILD_DATE
	#define BUILD_DATE __DATE__
#endif



/* -- version --------------------------------------------------------------- */
#define G_APP_NAME      "Giada"
#define G_VERSION_STR   "0.14.5"
#define G_VERSION_MAJOR 0
#define G_VERSION_MINOR 14
#define G_VERSION_PATCH 5

#define CONF_FILENAME "giada.conf"

#ifdef G_OS_WINDOWS
	#define G_SLASH '\\'
	#define G_SLASH_STR "\\"
#else
	#define G_SLASH '/'
	#define G_SLASH_STR "/"
#endif


/* -- GUI ------------------------------------------------------------------- */
#ifdef G_OS_WINDOWS
	#define G_GUI_SLEEP			1000/24
#else
	#define G_GUI_SLEEP			1000000/24 // == 1.000.000 / 24 == 1/24 sec == 24 Hz
#endif
#define G_GUI_WIDTH				   816
#define G_GUI_HEIGHT			   510
#define G_GUI_PLUGIN_RATE    0.05  // refresh rate for plugin GUIs
#define G_GUI_FONT_SIZE_BASE 12
#define G_GUI_INNER_MARGIN   4
#define G_GUI_OUTER_MARGIN   8
#define G_GUI_UNIT           20    // base unit for elements 
#define G_GUI_CHANNEL_H_1    G_GUI_UNIT
#define G_GUI_CHANNEL_H_2    G_GUI_UNIT * 2
#define G_GUI_CHANNEL_H_3    G_GUI_UNIT * 4
#define G_GUI_CHANNEL_H_4    G_GUI_UNIT * 6


#define G_COLOR_RED       fl_rgb_color(28,  32,  80)
#define G_COLOR_BLUE      fl_rgb_color(113, 31,  31)
#define G_COLOR_RED_ALERT fl_rgb_color(239, 75,  53)

#define G_COLOR_LIGHT_2  fl_rgb_color(200, 200, 200)
#define G_COLOR_LIGHT_1  fl_rgb_color(170, 170, 170)
#define G_COLOR_GREY_4   fl_rgb_color(78,  78,  78)
#define G_COLOR_GREY_3   fl_rgb_color(54,  54,  54)
#define G_COLOR_GREY_2   fl_rgb_color(37,  37,  37)
#define G_COLOR_GREY_1_5 fl_rgb_color(28,  28,  28)
#define G_COLOR_GREY_1   fl_rgb_color(25,  25,  25)
#define G_COLOR_BLACK    fl_rgb_color(0,   0,   0)



/* -- MIN/MAX values -------------------------------------------------------- */
#define G_MIN_BPM           20.0f
#define G_MAX_BPM           999.0f
#define G_MAX_BEATS				  32
#define G_MAX_BARS				  32
#define G_MAX_QUANTIZE      8
#define G_MIN_DB_SCALE      60.0f
#define G_MIN_COLUMN_WIDTH  140
#define G_MAX_BOOST_DB      20.0f
#define G_MAX_PITCH         4.0f
#define G_MAX_GRID_VAL      64
#define G_MIN_BUF_SIZE      8
#define G_MAX_BUF_SIZE      4096



/* -- kernel audio ---------------------------------------------------------- */
#define G_SYS_API_NONE		0x00  // 0000 0000
#define G_SYS_API_JACK		0x01  // 0000 0001
#define G_SYS_API_ALSA		0x02  // 0000 0010
#define G_SYS_API_DS			0x04  // 0000 0100
#define G_SYS_API_ASIO		0x08  // 0000 1000
#define G_SYS_API_CORE		0x10  // 0001 0000
#define G_SYS_API_PULSE   0x20  // 0010 0000
#define G_SYS_API_WASAPI  0x40  // 0100 0000
#define G_SYS_API_ANY     0x7F  // 0111 1111



/* -- kernel midi ----------------------------------------------------------- */
#define G_MIDI_API_JACK		0x01  // 0000 0001
#define G_MIDI_API_ALSA		0x02  // 0000 0010



/* -- default system -------------------------------------------------------- */
#if defined(G_OS_LINUX)
	#define G_DEFAULT_SOUNDSYS	G_SYS_API_NONE
#elif defined(G_OS_WINDOWS)
	#define G_DEFAULT_SOUNDSYS 	G_SYS_API_DS
#elif defined(G_OS_MAC)
	#define G_DEFAULT_SOUNDSYS 	G_SYS_API_CORE
#endif

#define G_DEFAULT_SOUNDDEV_OUT    0      // FIXME - please override with rtAudio::getDefaultDevice (or similar)
#define G_DEFAULT_SOUNDDEV_IN    -1      // no recording by default: input disabled
#define G_DEFAULT_MIDI_SYSTEM     0
#define G_DEFAULT_MIDI_PORT_IN   -1
#define G_DEFAULT_MIDI_PORT_OUT  -1
#define G_DEFAULT_SAMPLERATE      44100
#define G_DEFAULT_BUFSIZE         1024
#define G_DEFAULT_DELAYCOMP       0
#define G_DEFAULT_BIT_DEPTH       32     // float
#define G_DEFAULT_AUDIO_CHANS     2      // stereo for internal processing
#define G_DEFAULT_VOL             1.0f
#define G_DEFAULT_PITCH           1.0f
#define G_DEFAULT_BOOST           1.0f
#define G_DEFAULT_OUT_VOL         1.0f
#define G_DEFAULT_IN_VOL          1.0f
#define G_DEFAULT_CHANMODE       SINGLE_BASIC
#define G_DEFAULT_BPM             120.0f
#define G_DEFAULT_BEATS           4
#define G_DEFAULT_BARS            1
#define G_DEFAULT_QUANTIZE        0      // quantizer off
#define G_DEFAULT_FADEOUT_STEP    0.01f  // micro-fadeout speed
#define G_DEFAULT_COLUMN_WIDTH    380
#define G_DEFAULT_PATCH_NAME      "(default patch)"
#define G_DEFAULT_MIDI_INPUT_UI_W 300
#define G_DEFAULT_MIDI_INPUT_UI_H 350



/* -- mixer statuses and modes ---------------------------------------------- */
#define LOOP_BASIC			 0x01 // 0000 0001  chanMode
#define LOOP_ONCE				 0x02 // 0000 0010  chanMode
#define	SINGLE_BASIC		 0x04 // 0000 0100  chanMode
#define SINGLE_PRESS		 0x08 // 0000 1000	chanMode
#define SINGLE_RETRIG		 0x10 // 0001 0000	chanMode
#define LOOP_REPEAT			 0x20 // 0010 0000  chanMode
#define SINGLE_ENDLESS   0x40 // 0100 0000  chanMode
#define LOOP_ONCE_BAR    0x80 // 1000 0000  chanMode

#define LOOP_ANY				 0xA3 // 1010 0011  chanMode - any loop mode
#define SINGLE_ANY		   0x5C // 0101 1100  chanMode - any single mode

#define	STATUS_ENDING		 0x01 // 0000 0001  chanStatus - ending            (loop mode only)
#define	STATUS_WAIT			 0x02 // 0000 0010  chanStatus - waiting for start (loop mode only)
#define	STATUS_PLAY			 0x04 // 0000 0100  chanStatus - playing
#define STATUS_OFF			 0x08 // 0000 1000  chanStatus - off
#define STATUS_EMPTY	   0x10 // 0001 0000  chanStatus - not loaded (empty chan)
#define STATUS_MISSING   0x20 // 0010 0000  chanStatus - not found
#define STATUS_WRONG     0x40 // 0100 0000  chanStatus - something wrong (freq, bitrate, ...)

#define REC_WAITING			 0x01 // 0000 0001
#define REC_ENDING       0x02 // 0000 0010
#define REC_READING      0x04 // 0000 0100
#define REC_STOPPED      0x08 // 0000 1000



/* -- preview modes --------------------------------------------------------- */
#define G_PREVIEW_NONE   0x00
#define G_PREVIEW_NORMAL 0x01
#define G_PREVIEW_LOOP   0x02



/* -- actions --------------------------------------------------------------- */
#define G_ACTION_KEYPRESS		0x01 // 0000 0001
#define G_ACTION_KEYREL			0x02 // 0000 0010
#define G_ACTION_KILL		    0x04 // 0000 0100
#define G_ACTION_MUTEON			0x08 // 0000 1000
#define G_ACTION_MUTEOFF		0x10 // 0001 0000
#define G_ACTION_VOLUME     0x20 // 0010 0000
#define G_ACTION_MIDI       0x40 // 0100 0000

#define G_ACTION_KEYS       0x03 // 0000 0011 any key
#define G_ACTION_MUTES      0x24 // 0001 1000 any mute

#define G_RANGE_CHAR        0x01 // range for MIDI (0-127)
#define G_RANGE_FLOAT       0x02 // range for volumes and VST params (0.0-1.0)



/* -- responses and return codes -------------------------------------------- */
#define G_RES_ERR_PROCESSING    -6
#define G_RES_ERR_WRONG_DATA    -5
#define G_RES_ERR_NO_DATA       -4
#define G_RES_ERR_PATH_TOO_LONG -3
#define G_RES_ERR_IO            -2
#define G_RES_ERR_MEMORY        -1
#define G_RES_ERR                0
#define G_RES_OK                 1



/* -- log modes ------------------------------------------------------------- */
#define LOG_MODE_STDOUT 0x01
#define LOG_MODE_FILE   0x02
#define LOG_MODE_MUTE   0x04



/* -- channel types --------------------------------------------------------- */
#define CHANNEL_SAMPLE 0x01
#define CHANNEL_MIDI   0x02



/* -- unique IDs of mainWin's subwindows ------------------------------------ */
/* -- wid > 0 are reserved by gg_keyboard ----------------------------------- */
#define WID_BEATS         -1
#define WID_BPM           -2
#define WID_ABOUT         -3
#define WID_FILE_BROWSER  -4
#define WID_CONFIG        -5
#define WID_FX_LIST       -6
#define WID_ACTION_EDITOR -7
#define WID_SAMPLE_EDITOR -8
#define WID_FX            -9
#define WID_KEY_GRABBER   -10
#define WID_SAMPLE_NAME   -11



/* -- patch signals --------------------------------------------------------- */
#define PATCH_UNREADABLE    0x01
#define PATCH_INVALID       0x02
#define PATCH_READ_OK       0x04
#define PATCH_WRONG_PLUGINS 0x08  // currently unused
#define PATCH_WRONG_SAMPLES 0x10  // currently unused



/* -- midimap signals ------------------------------------------------------- */
#define MIDIMAP_NOT_SPECIFIED 0x00
#define MIDIMAP_UNREADABLE    0x01
#define MIDIMAP_INVALID       0x02
#define MIDIMAP_READ_OK       0x04



/* -- MIDI signals -------------------------------------------------------------
All signals are set to channel 0 (where channels are considered). It's up to the
caller to bitmask them with the proper channel number.
Channel voices messages - controller (0xB0) is a special subset of this family:
it drives knobs, volume, faders and such. */

#define MIDI_CONTROLLER     0xB0 << 24
#define MIDI_NOTE_ON        0x90 << 24
#define MIDI_NOTE_OFF       0x80 << 24
#define MIDI_VELOCITY       0x3F <<  8
#define MIDI_ALL_NOTES_OFF (MIDI_CONTROLLER) | (0x7B << 16)
#define MIDI_VOLUME        (MIDI_CONTROLLER) | (0x07 << 16)

/* system common / real-time messages. Single bytes */

#define MIDI_SYSEX          0xF0
#define MIDI_MTC_QUARTER    0xF1
#define MIDI_POSITION_PTR   0xF2
#define MIDI_CLOCK          0xF8
#define MIDI_START          0xFA
#define MIDI_CONTINUE       0xFB
#define MIDI_STOP           0xFC
#define MIDI_EOX            0xF7  // end of sysex

/* channels */

#define MIDI_CHAN_0         0x00 << 24
#define MIDI_CHAN_1         0x01 << 24
#define MIDI_CHAN_2         0x02 << 24
#define MIDI_CHAN_3         0x03 << 24
#define MIDI_CHAN_4         0x04 << 24
#define MIDI_CHAN_5         0x05 << 24
#define MIDI_CHAN_6         0x06 << 24
#define MIDI_CHAN_7         0x07 << 24
#define MIDI_CHAN_8         0x08 << 24
#define MIDI_CHAN_9         0x09 << 24
#define MIDI_CHAN_10        0x0A << 24
#define MIDI_CHAN_11        0x0B << 24
#define MIDI_CHAN_12        0x0C << 24
#define MIDI_CHAN_13        0x0D << 24
#define MIDI_CHAN_14        0x0E << 24
#define MIDI_CHAN_15        0x0F << 24

const int MIDI_CHANS[16] = {
	MIDI_CHAN_0,  MIDI_CHAN_1,	MIDI_CHAN_2,  MIDI_CHAN_3,
	MIDI_CHAN_4,  MIDI_CHAN_5,	MIDI_CHAN_6,  MIDI_CHAN_7,
	MIDI_CHAN_8,  MIDI_CHAN_9,	MIDI_CHAN_10, MIDI_CHAN_11,
	MIDI_CHAN_12, MIDI_CHAN_13,	MIDI_CHAN_14, MIDI_CHAN_15
};

/* midi sync constants */

#define MIDI_SYNC_NONE      0x00
#define MIDI_SYNC_CLOCK_M   0x01  // master
#define MIDI_SYNC_CLOCK_S   0x02  // slave
#define MIDI_SYNC_MTC_M     0x04  // master
#define MIDI_SYNC_MTC_S     0x08  // slave

/* JSON patch keys */

#define PATCH_KEY_HEADER                       "header"
#define PATCH_KEY_VERSION                      "version"
#define PATCH_KEY_VERSION_MAJOR                "version_major"
#define PATCH_KEY_VERSION_MINOR                "version_minor"
#define PATCH_KEY_VERSION_PATCH                "version_patch"
#define PATCH_KEY_NAME                         "name"
#define PATCH_KEY_BPM                          "bpm"
#define PATCH_KEY_BARS                         "bars"
#define PATCH_KEY_BEATS                        "beats"
#define PATCH_KEY_QUANTIZE                     "quantize"
#define PATCH_KEY_MASTER_VOL_IN                "master_vol_in"
#define PATCH_KEY_MASTER_VOL_OUT               "master_vol_out"
#define PATCH_KEY_METRONOME                    "metronome"
#define PATCH_KEY_LAST_TAKE_ID                 "last_take_id"
#define PATCH_KEY_SAMPLERATE                   "samplerate"
#define PATCH_KEY_COLUMNS                      "columns"
#define PATCH_KEY_MASTER_OUT_PLUGINS           "master_out_plugins"
#define PATCH_KEY_MASTER_IN_PLUGINS            "master_in_plugins"
#define PATCH_KEY_CHANNELS                     "channels"
#define PATCH_KEY_CHANNEL_TYPE                 "type"
#define PATCH_KEY_CHANNEL_INDEX                "index"
#define PATCH_KEY_CHANNEL_SIZE                 "size"
#define PATCH_KEY_CHANNEL_NAME                 "name"
#define PATCH_KEY_CHANNEL_COLUMN               "column"
#define PATCH_KEY_CHANNEL_MUTE                 "mute"
#define PATCH_KEY_CHANNEL_MUTE_S               "mute_s"
#define PATCH_KEY_CHANNEL_SOLO                 "solo"
#define PATCH_KEY_CHANNEL_VOLUME               "volume"
#define PATCH_KEY_CHANNEL_PAN                  "pan"
#define PATCH_KEY_CHANNEL_MIDI_IN              "midi_in"
#define PATCH_KEY_CHANNEL_MIDI_IN_KEYPRESS     "midi_in_keypress"
#define PATCH_KEY_CHANNEL_MIDI_IN_KEYREL       "midi_in_keyrel"
#define PATCH_KEY_CHANNEL_MIDI_IN_KILL         "midi_in_kill"
#define PATCH_KEY_CHANNEL_MIDI_IN_ARM          "midi_in_arm"
#define PATCH_KEY_CHANNEL_MIDI_IN_VOLUME       "midi_in_volume"
#define PATCH_KEY_CHANNEL_MIDI_IN_MUTE         "midi_in_mute"
#define PATCH_KEY_CHANNEL_MIDI_IN_FILTER       "midi_in_filter"
#define PATCH_KEY_CHANNEL_MIDI_IN_SOLO         "midi_in_solo"
#define PATCH_KEY_CHANNEL_MIDI_OUT_L           "midi_out_l"
#define PATCH_KEY_CHANNEL_MIDI_OUT_L_PLAYING   "midi_out_l_playing"
#define PATCH_KEY_CHANNEL_MIDI_OUT_L_MUTE      "midi_out_l_mute"
#define PATCH_KEY_CHANNEL_MIDI_OUT_L_SOLO      "midi_out_l_solo"
#define PATCH_KEY_CHANNEL_SAMPLE_PATH          "sample_path"
#define PATCH_KEY_CHANNEL_KEY                  "key"
#define PATCH_KEY_CHANNEL_MODE                 "mode"
#define PATCH_KEY_CHANNEL_BEGIN                "begin"
#define PATCH_KEY_CHANNEL_END                  "end"
#define PATCH_KEY_CHANNEL_BOOST                "boost"
#define PATCH_KEY_CHANNEL_REC_ACTIVE           "rec_active"
#define PATCH_KEY_CHANNEL_PITCH                "pitch"
#define PATCH_KEY_CHANNEL_INPUT_MONITOR        "input_monitor"
#define PATCH_KEY_CHANNEL_MIDI_IN_READ_ACTIONS "midi_in_read_actions"
#define PATCH_KEY_CHANNEL_MIDI_IN_PITCH        "midi_in_pitch"
#define PATCH_KEY_CHANNEL_MIDI_OUT             "midi_out"
#define PATCH_KEY_CHANNEL_MIDI_OUT_CHAN        "midi_out_chan"
#define PATCH_KEY_CHANNEL_PLUGINS              "plugins"
#define PATCH_KEY_CHANNEL_ACTIONS              "actions"
#define PATCH_KEY_CHANNEL_ARMED                "armed"
#define PATCH_KEY_ACTION_TYPE                  "type"
#define PATCH_KEY_ACTION_FRAME                 "frame"
#define PATCH_KEY_ACTION_F_VALUE               "f_value"
#define PATCH_KEY_ACTION_I_VALUE               "i_value"
#define PATCH_KEY_PLUGIN_PATH                  "path"
#define PATCH_KEY_PLUGIN_BYPASS                "bypass"
#define PATCH_KEY_PLUGIN_PARAMS                "params"
#define PATCH_KEY_PLUGIN_MIDI_IN_PARAMS        "midi_in_params"
#define PATCH_KEY_COLUMN_INDEX                 "index"
#define PATCH_KEY_COLUMN_WIDTH                 "width"
#define PATCH_KEY_COLUMN_CHANNELS              "channels"

/* JSON config keys */

#define CONF_KEY_HEADER                   "header"
#define CONF_KEY_LOG_MODE                 "log_mode"
#define CONF_KEY_SOUND_SYSTEM             "sound_system"
#define CONF_KEY_SOUND_DEVICE_IN          "sound_device_in"
#define CONF_KEY_SOUND_DEVICE_OUT         "sound_device_out"
#define CONF_KEY_CHANNELS_IN              "channels_in"
#define CONF_KEY_CHANNELS_OUT             "channels_out"
#define CONF_KEY_SAMPLERATE               "samplerate"
#define CONF_KEY_BUFFER_SIZE              "buffer_size"
#define CONF_KEY_DELAY_COMPENSATION       "delay_compensation"
#define CONF_KEY_LIMIT_OUTPUT             "limit_output"
#define CONF_KEY_RESAMPLE_QUALITY         "resample_quality"
#define CONF_KEY_MIDI_SYSTEM              "midi_system"
#define CONF_KEY_MIDI_PORT_OUT            "midi_port_out"
#define CONF_KEY_MIDI_PORT_IN             "midi_port_in"
#define CONF_KEY_NO_NOTE_OFF              "no_note_off"
#define CONF_KEY_MIDIMAP_PATH             "midimap_path"
#define CONF_KEY_LAST_MIDIMAP             "last_midimap"
#define CONF_KEY_MIDI_SYNC                "midi_sync"
#define CONF_KEY_MIDI_TC_FPS              "midi_tc_fps"
#define CONF_KEY_MIDI_IN                  "midi_in"
#define CONF_KEY_MIDI_IN_FILTER           "midi_in_filter"
#define CONF_KEY_MIDI_IN_REWIND           "midi_in_rewind"
#define CONF_KEY_MIDI_IN_START_STOP       "midi_in_start_stop"
#define CONF_KEY_MIDI_IN_ACTION_REC       "midi_in_action_rec"
#define CONF_KEY_MIDI_IN_INPUT_REC        "midi_in_input_rec"
#define CONF_KEY_MIDI_IN_METRONOME        "midi_in_metronome"
#define CONF_KEY_MIDI_IN_VOLUME_IN        "midi_in_volume_in"
#define CONF_KEY_MIDI_IN_VOLUME_OUT       "midi_in_volume_out"
#define CONF_KEY_MIDI_IN_BEAT_DOUBLE      "midi_in_beat_doble"
#define CONF_KEY_MIDI_IN_BEAT_HALF        "midi_in_beat_half"
#define CONF_KEY_RECS_STOP_ON_CHAN_HALT   "recs_stop_on_chan_halt"
#define CONF_KEY_CHANS_STOP_ON_SEQ_HALT   "chans_stop_on_seq_halt"
#define CONF_KEY_TREAT_RECS_AS_LOOPS      "treat_recs_as_loops"
#define CONF_KEY_RESIZE_RECORDINGS        "resize_recordings"
#define CONF_KEY_INPUT_MONITOR_DEFAULT_ON "input_monitor_default_on"
#define CONF_KEY_PLUGINS_PATH             "plugins_path"
#define CONF_KEY_PATCHES_PATH             "patches_path"
#define CONF_KEY_SAMPLES_PATH             "samples_path"
#define CONF_KEY_MAIN_WINDOW_X            "main_window_x"
#define CONF_KEY_MAIN_WINDOW_Y            "main_window_y"
#define CONF_KEY_MAIN_WINDOW_W            "main_window_w"
#define CONF_KEY_MAIN_WINDOW_H            "main_window_h"
#define CONF_KEY_BROWSER_X                "browser_x"
#define CONF_KEY_BROWSER_Y                "browser_y"
#define CONF_KEY_BROWSER_W                "browser_w"
#define CONF_KEY_BROWSER_H                "browser_h"
#define CONF_KEY_BROWSER_POSITION         "browser_position"
#define CONF_KEY_BROWSER_LAST_PATH        "browser_last_path"
#define CONF_KEY_BROWSER_LAST_VALUE       "browser_last_value"
#define CONF_KEY_ACTION_EDITOR_X          "action_editor_x"
#define CONF_KEY_ACTION_EDITOR_Y          "action_editor_y"
#define CONF_KEY_ACTION_EDITOR_W          "action_editor_w"
#define CONF_KEY_ACTION_EDITOR_H          "action_editor_h"
#define CONF_KEY_ACTION_EDITOR_ZOOM       "action_editor_zoom"
#define CONF_KEY_ACTION_EDITOR_GRID_VAL   "action_editor_grid_val"
#define CONF_KEY_ACTION_EDITOR_GRID_ON    "action_editor_grid_on"
#define CONF_KEY_SAMPLE_EDITOR_X          "sample_editor_x"
#define CONF_KEY_SAMPLE_EDITOR_Y          "sample_editor_y"
#define CONF_KEY_SAMPLE_EDITOR_W          "sample_editor_w"
#define CONF_KEY_SAMPLE_EDITOR_H          "sample_editor_h"
#define CONF_KEY_SAMPLE_EDITOR_GRID_VAL   "sample_editor_grid_val"
#define CONF_KEY_SAMPLE_EDITOR_GRID_ON    "sample_editor_grid_on"
#define CONF_KEY_PIANO_ROLL_Y             "piano_roll_y"
#define CONF_KEY_PIANO_ROLL_H             "piano_roll_h"
#define CONF_KEY_PLUGIN_LIST_X            "plugin_list_x"
#define CONF_KEY_PLUGIN_LIST_Y            "plugin_list_y"
#define CONF_KEY_CONFIG_X                 "config_x"
#define CONF_KEY_CONFIG_Y                 "config_y"
#define CONF_KEY_BPM_X                    "bpm_x"
#define CONF_KEY_BPM_Y                    "bpm_y"
#define CONF_KEY_BEATS_X                  "beats_x"
#define CONF_KEY_BEATS_Y                  "beats_y"
#define CONF_KEY_ABOUT_X                  "about_x"
#define CONF_KEY_ABOUT_Y                  "about_y"
#define CONF_KEY_NAME_X                   "name_x"
#define CONF_KEY_NAME_Y                   "name_y"
#define CONF_KEY_PLUGIN_CHOOSER_X         "plugin_chooser_x"
#define CONF_KEY_PLUGIN_CHOOSER_Y         "plugin_chooser_y"
#define CONF_KEY_PLUGIN_CHOOSER_W         "plugin_chooser_w"
#define CONF_KEY_PLUGIN_CHOOSER_H         "plugin_chooser_h"
#define CONF_KEY_MIDI_INPUT_X             "midi_input_x"
#define CONF_KEY_MIDI_INPUT_Y             "midi_input_y"
#define CONF_KEY_MIDI_INPUT_W             "midi_input_w"
#define CONF_KEY_MIDI_INPUT_H             "midi_input_h"
#define CONF_KEY_PLUGIN_SORT_METHOD       "plugin_sort_method"

/* JSON midimaps keys */

#define MIDIMAP_KEY_BRAND          "brand"
#define MIDIMAP_KEY_DEVICE         "device"
#define MIDIMAP_KEY_INIT_COMMANDS  "init_commands"
#define MIDIMAP_KEY_MUTE_ON        "mute_on"
#define MIDIMAP_KEY_MUTE_OFF       "mute_off"
#define MIDIMAP_KEY_SOLO_ON        "solo_on"
#define MIDIMAP_KEY_SOLO_OFF       "solo_off"
#define MIDIMAP_KEY_WAITING        "waiting"
#define MIDIMAP_KEY_PLAYING        "playing"
#define MIDIMAP_KEY_STOPPING       "stopping"
#define MIDIMAP_KEY_STOPPED        "stopped"
#define MIDIMAP_KEY_CHANNEL        "channel"
#define MIDIMAP_KEY_MESSAGE        "message"

#endif
