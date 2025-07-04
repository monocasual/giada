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

#include "src/core/types.h"
#include "src/deps/rtaudio/RtAudio.h"
#include "src/types.h"
#include "src/version.h"
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
} // namespace giada

#endif
