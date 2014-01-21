/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * const.h
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2014 Giovanni A. Zuliani | Monocasual
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
 * ------------------------------------------------------------------ */

#ifndef CONST_H
#define CONST_H



/* -- version ------------------------------------------------------- */
#define VERSIONE 				"0.8.3"
#define VERSIONE_STR 		"Giada"
#define VERSIONE_FLOAT	0.83f

#define CONF_FILENAME		"giada.conf"



/* -- GUI ----------------------------------------------------------- */
#ifdef _WIN32
	#define GUI_SLEEP			1000/24
#else
	#define GUI_SLEEP			1000000/24 // == 1.000.000 / 24 == 1/24 sec == 24 Hz
#endif
#define GUI_WIDTH				810
#define GUI_HEIGHT			510

#define COLOR_BD_0     fl_rgb_color(78,  78,  78)			// border off
#define COLOR_BD_1     fl_rgb_color(188, 188, 188)    // border on
#define COLOR_BG_0     fl_rgb_color(37,  37,  37)     // bg off
#define COLOR_BG_1     fl_rgb_color(78,  78,  78)     // bg on (clicked)
#define COLOR_BG_2     fl_rgb_color(177, 142, 142)    // bg active (play, for some widgets)
#define COLOR_BG_3     fl_rgb_color(28,  32,  80)     // bg input rec
#define COLOR_BG_4     fl_rgb_color(113, 31,  31)     // bg action rec
#define COLOR_ALERT    fl_rgb_color(239, 75,  53)     // peak meter alert
#define COLOR_TEXT_0   fl_rgb_color(200, 200, 200)
#define COLOR_TEXT_1   fl_rgb_color(25,  25,  25)
#define COLOR_BG_MAIN  fl_rgb_color(25,  25,  25)		   // windows background
#define COLOR_BG_DARK  fl_rgb_color(0,   0,   0)		   // inputs background



/* -- MIN/MAX values ------------------------------------------------ */
#define MAX_BEATS				   32
#define MAX_BARS				   32
#define MAX_PATCHNAME_LEN	 32
#define DB_MIN_SCALE		   60.0f
#define MAX_VST_EVENTS     32



/* -- kernel audio -------------------------------------------------- */
#define SYS_API_JACK		0x01  // 0000 0001
#define SYS_API_ALSA		0x02  // 0000 0010
#define SYS_API_DS			0x04  // 0000 0100
#define SYS_API_ASIO		0x08  // 0000 1000
#define SYS_API_CORE		0x10  // 0001 0000
#define SYS_API_PULSE   0x20  // 0010 0000
#define SYS_API_ANY     0x3F  // 0011 1111

#define KERNEL_OK					 0
#define KERNEL_UNDERRUN	  -1
#define KERNEL_CRITICAL	  -2



/* -- kernel midi --------------------------------------------------- */
#define MIDI_API_JACK		0x01  // 0000 0001
#define MIDI_API_ALSA		0x02  // 0000 0010



/* -- default system ------------------------------------------------ */
#if defined(__linux__)
	#define DEFAULT_SOUNDSYS	SYS_API_ALSA
#elif defined(_WIN32)
	#define DEFAULT_SOUNDSYS 	SYS_API_DS
#elif defined(__APPLE__)
	#define DEFAULT_SOUNDSYS 	SYS_API_CORE
#endif

#define DEFAULT_SOUNDDEV_OUT  0       /// FIXME - please override with rtAudio::getDefaultDevice (or similar)
#define DEFAULT_SOUNDDEV_IN   -1			// no recording by default: input disabled
#define DEFAULT_MIDI_SYSTEM   0
#define DEFAULT_MIDI_PORT_IN  -1
#define DEFAULT_MIDI_PORT_OUT -1
#define DEFAULT_SAMPLERATE   44100
#define DEFAULT_BUFSIZE		   1024
#define DEFAULT_DELAYCOMP		 0
#define DEFAULT_VOL				   0.0f
#define DEFAULT_BOOST			   0.0f
#define gDEFAULT_PITCH			 1.0f   // ugly and temporary fix to avoid conflicts with wingdi.h (Windows only).
#define DEFAULT_OUT_VOL  	   1.0f
#define DEFAULT_IN_VOL  	   0.0f
#define DEFAULT_CHANMODE	   SINGLE_BASIC
#define DEFAULT_BPM				   120.0f
#define DEFAULT_BEATS			   4
#define DEFAULT_BARS			   1
#define DEFAULT_QUANTIZE     0		  // quantizer off
#define DEFAULT_FADEOUT_STEP 0.01f  // micro-fadeout speed



/* -- mixer statuses and modes -------------------------------------- */
#define LOOP_BASIC			 0x01 // 0000 0001  chanMode
#define LOOP_ONCE				 0x02 // 0000 0010  chanMode
#define	SINGLE_BASIC		 0x04 // 0000 0100  chanMode
#define SINGLE_PRESS		 0x08 // 0000 1000	 chanMode
#define SINGLE_RETRIG		 0x10 // 0001 0000	 chanMode
#define LOOP_REPEAT			 0x20 // 0010 0000  chanMode
#define SINGLE_ENDLESS   0x40 // 0100 0000  chanMode

#define LOOP_ANY				 0x23 // 0010 0011  chanMode - any loop mode
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



/* -- actions ------------------------------------------------------- */
#define ACTION_MIDI       0x40 // 0000 0000 WHY NOT 0x40?
#define ACTION_KEYPRESS		0x01 // 0000 0001
#define ACTION_KEYREL			0x02 // 0000 0010
#define ACTION_KILLCHAN		0x04 // 0000 0100
#define ACTION_MUTEON			0x08 // 0000 1000
#define ACTION_MUTEOFF		0x10 // 0001 0000
#define ACTION_VOLUME     0x20 // 0010 0000

#define ACTION_KEYS       0x03 // 0000 0011 any key
#define ACTION_MUTES      0x24 // 0001 1000 any mute

#define RANGE_CHAR        0x01 // range for MIDI (0-127)
#define RANGE_FLOAT       0x02 // range for volumes and VST params (0.0-1.0)



/* -- mixerHandler signals ------------------------------------------ */
#define SAMPLE_LOADED_OK      1
#define SAMPLE_LEFT_EMPTY     0
#define SAMPLE_NOT_VALID     -1
#define SAMPLE_MULTICHANNEL  -2
#define SAMPLE_WRONG_BIT     -4
#define SAMPLE_WRONG_ENDIAN  -5
#define SAMPLE_WRONG_FORMAT  -6
#define SAMPLE_READ_ERROR    -8
#define SAMPLE_PATH_TOO_LONG -9

/** FIXME - add to SAMPLE_ series those for when exporting */



/* -- browser types ------------------------------------------------- */
#define BROWSER_LOAD_PATCH   0x00
#define BROWSER_LOAD_SAMPLE  0x01
#define BROWSER_SAVE_PATCH   0x02
#define BROWSER_SAVE_SAMPLE  0x04
#define BROWSER_SAVE_PROJECT 0x08
#define BROWSER_LOAD_PLUGIN  0x10



/* -- channel types ------------------------------------------------- */
#define CHANNEL_SAMPLE 0x01
#define CHANNEL_MIDI   0x02



/* -- unique IDs of mainWin's subwindows ---------------------------- */
/* -- wid > 0 are reserved by gg_keyboard --------------------------- */
#define WID_BEATS         -1
#define WID_BPM           -2
#define WID_ABOUT         -3
#define WID_FILE_BROWSER  -4
#define WID_CONFIG        -5
#define WID_FX_LIST       -6
#define WID_ACTION_EDITOR -7
#define WID_SAMPLE_EDITOR -8
#define WID_FX            -9


/* -- patch signals ------------------------------------------------- */
#define PATCH_UNREADABLE  0
#define PATCH_INVALID    -1
#define PATCH_OPEN_OK     1

/** TODO - addo to PATCH_ serie the signals for saving/loading */



/* -- MIDI signals -----------------------------------------------------
 * all signals are set to channel 0 (where channels are considered).
 * It's up to the caller to bitmask them with the proper channel number. */

/* channel voices messages - controller (0xB0) is a special subset of
 * this family: it drives knobs, volume, faders and such. */

#define MIDI_CONTROLLER     0xB0 << 24
#define MIDI_NOTE_ON        0x90 << 24
#define MIDI_NOTE_OFF       0x80 << 24
#define MIDI_ALL_NOTES_OFF (MIDI_CONTROLLER) | (0x7B << 16)
#define MIDI_VOLUME        (MIDI_CONTROLLER) | (0x07 << 16)

/* system common / real-time messages */

#define MIDI_TIMING_CLOCK   0xF8 << 24
#define MIDI_START          0xFA << 24
#define MIDI_CONTINUE       0xFB << 24
#define MIDI_STOP           0xFC << 24

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

#endif
