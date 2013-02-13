/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * const.h
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2013 Giovanni A. Zuliani | Monocasual
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
#define VERSIONE 				"0.6.0"
#define VERSIONE_STR 		"Giada"
#define VERSIONE_FLOAT	0.60f

#define CONF_FILENAME		"giada.conf"



/* -- GUI ----------------------------------------------------------- */
#ifdef _WIN32
	#define GUI_SLEEP			1000/24
#else
	#define GUI_SLEEP			1000000/24 // == 1.000.000 / 24 == 1/24 sec == 24 refresh per second
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
#define MAX_NUM_CHAN 		   32
#define MAX_BEATS				   32
#define MAX_BARS				   32
#define MAX_PATCHNAME_LEN	 32
#define DB_MIN_SCALE		   60.0f



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



/* -- default keys -------------------------------------------------- */
#define DEFAULT_KEY_1		'1'
#define DEFAULT_KEY_2		'2'
#define DEFAULT_KEY_3		'3'
#define DEFAULT_KEY_4		'4'
#define DEFAULT_KEY_5		'5'
#define DEFAULT_KEY_6		'6'
#define DEFAULT_KEY_7		'7'
#define DEFAULT_KEY_8		'8'
#define DEFAULT_KEY_9		'q'
#define DEFAULT_KEY_10	'w'
#define DEFAULT_KEY_11	'e'
#define DEFAULT_KEY_12	'r'
#define DEFAULT_KEY_13	't'
#define DEFAULT_KEY_14	'y'
#define DEFAULT_KEY_15	'u'
#define DEFAULT_KEY_16	'i'
#define DEFAULT_KEY_17	'a'
#define DEFAULT_KEY_18	's'
#define DEFAULT_KEY_19	'd'
#define DEFAULT_KEY_20	'f'
#define DEFAULT_KEY_21	'g'
#define DEFAULT_KEY_22	'h'
#define DEFAULT_KEY_23	'j'
#define DEFAULT_KEY_24	'k'
#define DEFAULT_KEY_25	'z'
#define DEFAULT_KEY_26	'x'
#define DEFAULT_KEY_27	'c'
#define DEFAULT_KEY_28	'v'
#define DEFAULT_KEY_29	'b'
#define DEFAULT_KEY_30	'n'
#define DEFAULT_KEY_31	'm'
#define DEFAULT_KEY_32	','

#define DEFAULT_KEY_ARRAY { \
	DEFAULT_KEY_1,  DEFAULT_KEY_2,  DEFAULT_KEY_3,  DEFAULT_KEY_4,  \
	DEFAULT_KEY_5,  DEFAULT_KEY_6,  DEFAULT_KEY_7,  DEFAULT_KEY_8,  \
	DEFAULT_KEY_9,  DEFAULT_KEY_10, DEFAULT_KEY_11, DEFAULT_KEY_12, \
	DEFAULT_KEY_13, DEFAULT_KEY_14, DEFAULT_KEY_15, DEFAULT_KEY_16, \
	DEFAULT_KEY_17, DEFAULT_KEY_18, DEFAULT_KEY_19, DEFAULT_KEY_20, \
	DEFAULT_KEY_21, DEFAULT_KEY_22, DEFAULT_KEY_23, DEFAULT_KEY_24, \
	DEFAULT_KEY_25, DEFAULT_KEY_26, DEFAULT_KEY_27, DEFAULT_KEY_28, \
	DEFAULT_KEY_29, DEFAULT_KEY_30, DEFAULT_KEY_31, DEFAULT_KEY_32  \
}

#if defined(__linux__)
	#define DEFAULT_SOUNDSYS	SYS_API_ALSA
#elif defined(_WIN32)
	#define DEFAULT_SOUNDSYS 	SYS_API_DS
#elif defined(__APPLE__)
	#define DEFAULT_SOUNDSYS 	SYS_API_CORE
#endif

#define DEFAULT_SOUNDDEV_OUT -1      /// FIXME - please override with rtAudio::getDefaultDevice (or similar)
#define DEFAULT_SOUNDDEV_IN  -1			// no recording by default: input disabled
#define DEFAULT_SAMPLERATE   44100
#define DEFAULT_BUFSIZE		   1024
#define DEFAULT_DELAYCOMP		 0
#define DEFAULT_VOL				   0.0f
#define DEFAULT_BOOST			   0.0f
#define gDEFAULT_PITCH			 1.0f   // ugly and temporary fix to avoid conflicts with wingdi.h (Windows only).
#define DEFAULT_OUT_VOL  	   1.0f
#define DEFAULT_IN_VOL  	   0.0f
#define DEFAULT_CHANMODE	   LOOP_BASIC
#define DEFAULT_BPM				   120.0f
#define DEFAULT_BEATS			   4
#define DEFAULT_BARS			   1
#define DEFAULT_QUANTIZE     0		  // quantizer off
#define DEFAULT_FADEOUT_STEP 0.01f  // micro-fadeout speed



/* -- status e mode Mixer ------------------------------------------- */
#define LOOP_BASIC			0x01 // 0000 0001  chanMode
#define LOOP_ONCE				0x02 //	0000 0010  chanMode
#define	SINGLE_BASIC		0x04 //	0000 0100  chanMode
#define SINGLE_PRESS		0x08 //	0000 1000	 chanMode
#define SINGLE_RETRIG		0x10 //	0001 0000	 chanMode
#define LOOP_REPEAT			0x20 // 0010 0000  chanMode

#define LOOP_ANY				0x23 // 0010 0011  chanMode - any loop mode
#define SINGLE_ANY		  0x1C // 0001 1100  chanMode - any single mode

#define	STATUS_ENDING		0x01 // 0000 0001  chanStatus - ending            (loop mode only)
#define	STATUS_WAIT			0x02 // 0000 0010  chanStatus - waiting for start (loop mode only)
#define	STATUS_PLAY			0x04 // 0000 0100  chanStatus - playing
#define STATUS_OFF			0x08 // 0000 1000  chanStatus - off
#define STATUS_EMPTY	  0x10 // 0001 0000  chanStatus - not loaded (empty chan)
#define STATUS_MISSING  0x20 // 0010 0000  chanStatus - not found
#define STATUS_WRONG    0x40 // 0100 0000  chanStatus - something wrong (freq, bitrate, ...)

#define REC_WAITING			0x80   // 1000 0000
#define REC_ENDING      0x100
#define REC_READING     0x200
#define REC_STOPPED     0x400



/* -- actions ------------------------------------------------------- */
#define ACTION_KEYPRESS		0x01 // 0000 0001
#define ACTION_KEYREL			0x02 // 0000 0010
#define ACTION_KILLCHAN		0x04 // 0000 0100
#define ACTION_MUTEON			0x08 // 0000 1000
#define ACTION_MUTEOFF		0x10 // 0001 0000

#define ACTION_KEYS       0x03 // 0000 0011 any key
#define ACTION_MUTES      0x24 // 0001 1000 any mute



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



/* -- unique IDs of mainWin's subwindows ---------------------------- */
/* -- 0-31 are reserved by ge_keyboard ------------------------------ */
#define WID_BEATS         32
#define WID_BPM           33
#define WID_ABOUT         34
//#define WID_MASTER_OUT_FX 35
#define WID_FILE_BROWSER  36
#define WID_CONFIG        37
#define WID_FX_LIST       38
#define WID_ACTION_EDITOR 39
#define WID_SAMPLE_EDITOR 40
#define WID_FX            41



/* -- patch signals ------------------------------------------------- */
#define PATCH_UNREADABLE  0
#define PATCH_INVALID    -1
#define PATCH_OPEN_OK     1

/** TODO - addo to PATCH_ serie the signals for saving/loading */

#endif
