/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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


#include <cassert>
#include <string>
#include <FL/Fl.H>
#include "utils/fs.h"
#include "utils/log.h"
#include "utils/json.h"
#include "core/const.h"
#include "core/types.h"
#include "conf.h"


namespace giada {
namespace m {
namespace conf
{
namespace
{
std::string confFilePath = "";
std::string confDirPath  = "";


/* -------------------------------------------------------------------------- */

/* sanitize
Avoids funky values from config file. */

void sanitize()
{
	if (!(soundSystem & G_SYS_API_ANY)) soundSystem = G_DEFAULT_SOUNDSYS;
	if (soundDeviceOut < 0) soundDeviceOut = G_DEFAULT_SOUNDDEV_OUT;
	if (soundDeviceIn < -1) soundDeviceIn = G_DEFAULT_SOUNDDEV_IN;
	if (channelsOut < 0) channelsOut = 0;
	if (channelsIn < 0)  channelsIn  = 0;
	if (buffersize < G_MIN_BUF_SIZE || buffersize > G_MAX_BUF_SIZE) buffersize = G_DEFAULT_BUFSIZE;
	if (midiPortOut < -1) midiPortOut = G_DEFAULT_MIDI_SYSTEM;
	if (midiPortOut < -1) midiPortOut = G_DEFAULT_MIDI_PORT_OUT;
	if (midiPortIn < -1) midiPortIn = G_DEFAULT_MIDI_PORT_IN;
	if (browserX < 0) browserX = 0;
	if (browserY < 0) browserY = 0;
	if (browserW < 396) browserW = 396;
	if (browserH < 302) browserH = 302;
	if (actionEditorX < 0) actionEditorX = 0;
	if (actionEditorY < 0) actionEditorY = 0;
	if (actionEditorW < 640) actionEditorW = 640;
	if (actionEditorH < 176) actionEditorH = 176;
	if (actionEditorZoom < 100) actionEditorZoom = 100;
	if (actionEditorGridVal < 0 || actionEditorGridVal > G_MAX_GRID_VAL) actionEditorGridVal = 0;
	if (actionEditorGridOn < 0) actionEditorGridOn = 0;
	if (pianoRollH <= 0) pianoRollH = 422;
	if (sampleActionEditorH <= 0) sampleActionEditorH = 40;
	if (velocityEditorH <= 0) velocityEditorH = 40;
	if (envelopeEditorH <= 0) envelopeEditorH = 40;
	if (sampleEditorX < 0) sampleEditorX = 0;
	if (sampleEditorY < 0) sampleEditorY = 0;
	if (sampleEditorW < 500) sampleEditorW = 500;
	if (sampleEditorH < 292) sampleEditorH = 292;
	if (sampleEditorGridVal < 0 || sampleEditorGridVal > G_MAX_GRID_VAL) sampleEditorGridVal = 0;
	if (sampleEditorGridOn < 0) sampleEditorGridOn = 0;
	if (midiInputX < 0) midiInputX = 0;
	if (midiInputY < 0) midiInputY = 0;
	if (midiInputW < G_DEFAULT_MIDI_INPUT_UI_W) midiInputW = G_DEFAULT_MIDI_INPUT_UI_W;
	if (midiInputH < G_DEFAULT_MIDI_INPUT_UI_H) midiInputH = G_DEFAULT_MIDI_INPUT_UI_H;
	if (configX < 0) configX = 0;
	if (configY < 0) configY = 0;
	if (pluginListX < 0) pluginListX = 0;
	if (pluginListY < 0) pluginListY = 0;
#ifdef WITH_VST
	if (pluginChooserW < 640) pluginChooserW = 640;
	if (pluginChooserH < 480) pluginChooserW = 480;
#endif
	if (bpmX < 0) bpmX = 0;
	if (bpmY < 0) bpmY = 0;
	if (beatsX < 0) beatsX = 0;
	if (beatsY < 0) beatsY = 0;
	if (aboutX < 0) aboutX = 0;
	if (aboutY < 0) aboutY = 0;
	if (samplerate < 8000) samplerate = G_DEFAULT_SAMPLERATE;
	if (rsmpQuality < 0 || rsmpQuality > 4) rsmpQuality = 0;
}


/* -------------------------------------------------------------------------- */


/* createConfigFolder
Creates local folder where to put the configuration file. Path differs from OS
to OS. */

int createConfigFolder()
{
#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)

	if (gu_dirExists(confDirPath))
		return 1;

	gu_log("[conf::createConfigFolder] .giada folder not present. Updating...\n");

	if (gu_mkdir(confDirPath)) {
		gu_log("[conf::createConfigFolder] status: ok\n");
		return 1;
	}
	else {
		gu_log("[conf::createConfigFolder] status: error!\n");
		return 0;
	}

#else // windows

	return 1;

#endif
}

}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


std::string header = "GIADACFG";

int  logMode        = LOG_MODE_MUTE;
int  soundSystem    = G_DEFAULT_SOUNDSYS;
int  soundDeviceOut = G_DEFAULT_SOUNDDEV_OUT;
int  soundDeviceIn  = G_DEFAULT_SOUNDDEV_IN;
int  channelsOut    = 0;
int  channelsIn     = 0;
int  samplerate     = G_DEFAULT_SAMPLERATE;
int  buffersize     = G_DEFAULT_BUFSIZE;
bool limitOutput    = false;
int  rsmpQuality    = 0;

int         midiSystem  = 0;
int         midiPortOut = G_DEFAULT_MIDI_PORT_OUT;
int         midiPortIn  = G_DEFAULT_MIDI_PORT_IN;
std::string midiMapPath = "";
std::string lastFileMap = "";
int         midiSync    = MIDI_SYNC_NONE;
float       midiTCfps   = 25.0f;

/* TODO - move these into a RCUList */
std::atomic<bool>     midiIn          (false);
std::atomic<int>      midiInFilter    (-1);
std::atomic<uint32_t> midiInRewind    (0x0);
std::atomic<uint32_t> midiInStartStop (0x0);
std::atomic<uint32_t> midiInActionRec (0x0);
std::atomic<uint32_t> midiInInputRec  (0x0);
std::atomic<uint32_t> midiInVolumeIn  (0x0);
std::atomic<uint32_t> midiInVolumeOut (0x0);
std::atomic<uint32_t> midiInBeatDouble(0x0);
std::atomic<uint32_t> midiInBeatHalf  (0x0);
std::atomic<uint32_t> midiInMetronome (0x0);

bool recsStopOnChanHalt    = false;
bool chansStopOnSeqHalt    = false;
bool treatRecsAsLoops      = false;
bool inputMonitorDefaultOn = false;

std::string pluginPath = "";
std::string patchPath  = "";
std::string samplePath = "";

int mainWindowX = (Fl::w() / 2) - (G_MIN_GUI_WIDTH / 2);
int mainWindowY = (Fl::h() / 2) - (G_MIN_GUI_HEIGHT / 2);
int mainWindowW = G_MIN_GUI_WIDTH;
int mainWindowH = G_MIN_GUI_HEIGHT;

int         browserX         = 0;
int         browserY         = 0;
int         browserW         = 640;
int         browserH         = 480;
int         browserPosition  = 0;
int         browserLastValue = 0;
std::string browserLastPath = "";

int actionEditorX       = 0;
int actionEditorY       = 0;
int actionEditorW       = 640;
int actionEditorH       = 480;
int actionEditorZoom    = 100;
int actionEditorGridVal = 1;
int actionEditorGridOn  = false;

int sampleEditorX = 0;
int sampleEditorY = 0;
int sampleEditorW = 640;
int sampleEditorH = 480;
int sampleEditorGridVal = 0;
int sampleEditorGridOn  = false;

int midiInputX = 0;
int midiInputY = 0;
int midiInputW = G_DEFAULT_MIDI_INPUT_UI_W;
int midiInputH = G_DEFAULT_MIDI_INPUT_UI_H;

int pianoRollY = -1;
int pianoRollH = 422;

int sampleActionEditorH = 40; 
int velocityEditorH     = 40; 
int envelopeEditorH     = 40; 

int pluginListX = 0;
int pluginListY = 0;

int configX = 0;
int configY = 0;

int bpmX = 0;
int bpmY = 0;

int beatsX = 0;
int beatsY = 0;

int aboutX = 0;
int aboutY = 0;

int nameX = 0;
int nameY = 0;

int   recTriggerMode  = static_cast<int>(RecTriggerMode::NORMAL);
float recTriggerLevel = G_DEFAULT_REC_TRIGGER_LEVEL;

#ifdef WITH_VST

int pluginChooserX   = 0;
int pluginChooserY   = 0;
int pluginChooserW   = 640;
int pluginChooserH   = 480;
int pluginSortMethod = 0;

#endif


/* -------------------------------------------------------------------------- */


void init()
{
	/* Initialize confFilePath, i.e. the configuration file. In windows it is in
	 * the same dir of the .exe, while in Linux and OS X in ~/.giada */

#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)

	confFilePath = gu_getHomePath() + G_SLASH + CONF_FILENAME;
	confDirPath  = gu_getHomePath() + G_SLASH;

#elif defined(_WIN32)

	confFilePath = CONF_FILENAME;
	confDirPath  = "";

#endif
}


/* -------------------------------------------------------------------------- */


bool isMidiInAllowed(int c)
{
	return midiInFilter == -1 || midiInFilter == c;
}


/* -------------------------------------------------------------------------- */


bool read()
{
	namespace uj = u::json;

	init();

	json_t* j = uj::load(confFilePath);
	if (j == nullptr)
		return false;

	if (!uj::isObject(j)) {
		json_decref(j);
		return false;
	}

	header         = uj::readString(j, CONF_KEY_HEADER);
	logMode        = uj::readInt(j, CONF_KEY_LOG_MODE);
	soundSystem    = uj::readInt(j, CONF_KEY_SOUND_SYSTEM);
	soundDeviceOut = uj::readInt(j, CONF_KEY_SOUND_DEVICE_OUT);
	soundDeviceIn  = uj::readInt(j, CONF_KEY_SOUND_DEVICE_IN);
	channelsOut    = uj::readInt(j, CONF_KEY_CHANNELS_OUT);
	channelsIn     = uj::readInt(j, CONF_KEY_CHANNELS_IN);
	samplerate     = uj::readInt(j, CONF_KEY_SAMPLERATE);
	buffersize     = uj::readInt(j, CONF_KEY_BUFFER_SIZE);
	limitOutput    = uj::readBool(j, CONF_KEY_LIMIT_OUTPUT);
	rsmpQuality    = uj::readInt(j, CONF_KEY_RESAMPLE_QUALITY);
	midiSystem     = uj::readInt(j, CONF_KEY_MIDI_SYSTEM);
	midiPortOut    = uj::readInt(j, CONF_KEY_MIDI_PORT_OUT);
	midiPortIn     = uj::readInt(j, CONF_KEY_MIDI_PORT_IN);
	midiMapPath    = uj::readString(j, CONF_KEY_MIDIMAP_PATH);
	lastFileMap    = uj::readString(j, CONF_KEY_LAST_MIDIMAP);
	midiSync       = uj::readInt(j, CONF_KEY_MIDI_SYNC);
	midiTCfps      = uj::readFloat(j, CONF_KEY_MIDI_TC_FPS);
	midiIn           = uj::readBool(j, CONF_KEY_MIDI_IN);
	midiInFilter     = uj::readInt(j, CONF_KEY_MIDI_IN_FILTER); 
	midiInRewind     = uj::readInt(j, CONF_KEY_MIDI_IN_REWIND);
	midiInStartStop  = uj::readInt(j, CONF_KEY_MIDI_IN_START_STOP);
	midiInActionRec  = uj::readInt(j, CONF_KEY_MIDI_IN_ACTION_REC);
	midiInInputRec   = uj::readInt(j, CONF_KEY_MIDI_IN_INPUT_REC);
	midiInMetronome  = uj::readInt(j, CONF_KEY_MIDI_IN_METRONOME);
	midiInVolumeIn   = uj::readInt(j, CONF_KEY_MIDI_IN_VOLUME_IN);
	midiInVolumeOut  = uj::readInt(j, CONF_KEY_MIDI_IN_VOLUME_OUT);
	midiInBeatDouble = uj::readInt(j, CONF_KEY_MIDI_IN_BEAT_DOUBLE);
	midiInBeatHalf   = uj::readInt(j, CONF_KEY_MIDI_IN_BEAT_HALF);
	recsStopOnChanHalt = uj::readBool(j, CONF_KEY_RECS_STOP_ON_CHAN_HALT);
	chansStopOnSeqHalt = uj::readBool(j, CONF_KEY_CHANS_STOP_ON_SEQ_HALT);
	treatRecsAsLoops = uj::readBool(j, CONF_KEY_TREAT_RECS_AS_LOOPS);
	inputMonitorDefaultOn = uj::readBool(j, CONF_KEY_INPUT_MONITOR_DEFAULT_ON);
	pluginPath = uj::readString(j, CONF_KEY_PLUGINS_PATH);
	patchPath = uj::readString(j, CONF_KEY_PATCHES_PATH);
	samplePath = uj::readString(j, CONF_KEY_SAMPLES_PATH);
	mainWindowX = uj::readInt(j, CONF_KEY_MAIN_WINDOW_X);
	mainWindowY = uj::readInt(j, CONF_KEY_MAIN_WINDOW_Y);
	mainWindowW = uj::readInt(j, CONF_KEY_MAIN_WINDOW_W);
	mainWindowH = uj::readInt(j, CONF_KEY_MAIN_WINDOW_H);
	browserX = uj::readInt(j, CONF_KEY_BROWSER_X);
	browserY = uj::readInt(j, CONF_KEY_BROWSER_Y);
	browserW = uj::readInt(j, CONF_KEY_BROWSER_W);
	browserH = uj::readInt(j, CONF_KEY_BROWSER_H);
	browserPosition = uj::readInt(j, CONF_KEY_BROWSER_POSITION);
	browserLastPath = uj::readString(j, CONF_KEY_BROWSER_LAST_PATH);
	browserLastValue = uj::readInt(j, CONF_KEY_BROWSER_LAST_VALUE);
	actionEditorX = uj::readInt(j, CONF_KEY_ACTION_EDITOR_X);
	actionEditorY = uj::readInt(j, CONF_KEY_ACTION_EDITOR_Y);
	actionEditorW = uj::readInt(j, CONF_KEY_ACTION_EDITOR_W);
	actionEditorH = uj::readInt(j, CONF_KEY_ACTION_EDITOR_H);
	actionEditorZoom = uj::readInt(j, CONF_KEY_ACTION_EDITOR_ZOOM);
	actionEditorGridVal = uj::readInt(j, CONF_KEY_ACTION_EDITOR_GRID_VAL);
	actionEditorGridOn = uj::readInt(j, CONF_KEY_ACTION_EDITOR_GRID_ON);
	sampleEditorX = uj::readInt(j, CONF_KEY_SAMPLE_EDITOR_X);
	sampleEditorY = uj::readInt(j, CONF_KEY_SAMPLE_EDITOR_Y);
	sampleEditorW = uj::readInt(j, CONF_KEY_SAMPLE_EDITOR_W);
	sampleEditorH = uj::readInt(j, CONF_KEY_SAMPLE_EDITOR_H);
	sampleEditorGridVal = uj::readInt(j, CONF_KEY_SAMPLE_EDITOR_GRID_VAL);
	sampleEditorGridOn = uj::readInt(j, CONF_KEY_SAMPLE_EDITOR_GRID_ON);
	pianoRollY = uj::readInt(j, CONF_KEY_PIANO_ROLL_Y);
	pianoRollH = uj::readInt(j, CONF_KEY_PIANO_ROLL_H);
	sampleActionEditorH = uj::readInt(j, CONF_KEY_SAMPLE_ACTION_EDITOR_H);
	velocityEditorH = uj::readInt(j, CONF_KEY_VELOCITY_EDITOR_H);
	envelopeEditorH = uj::readInt(j, CONF_KEY_ENVELOPE_EDITOR_H);
	pluginListX = uj::readInt(j, CONF_KEY_PLUGIN_LIST_X);
	pluginListY = uj::readInt(j, CONF_KEY_PLUGIN_LIST_Y);
	configX = uj::readInt(j, CONF_KEY_CONFIG_X);
	configY = uj::readInt(j, CONF_KEY_CONFIG_Y);
	bpmX = uj::readInt(j, CONF_KEY_BPM_X);
	bpmY = uj::readInt(j, CONF_KEY_BPM_Y);
	beatsX = uj::readInt(j, CONF_KEY_BEATS_X);
	beatsY = uj::readInt(j, CONF_KEY_BEATS_Y);
	aboutX = uj::readInt(j, CONF_KEY_ABOUT_X);
	aboutY = uj::readInt(j, CONF_KEY_ABOUT_Y);
	nameX = uj::readInt(j, CONF_KEY_NAME_X);
	nameY = uj::readInt(j, CONF_KEY_NAME_Y);
	midiInputX = uj::readInt(j, CONF_KEY_MIDI_INPUT_X);
	midiInputY = uj::readInt(j, CONF_KEY_MIDI_INPUT_Y);
	midiInputW = uj::readInt(j, CONF_KEY_MIDI_INPUT_W);
	midiInputH = uj::readInt(j, CONF_KEY_MIDI_INPUT_H);
	recTriggerMode = uj::readInt(j, CONF_KEY_REC_TRIGGER_MODE);
	recTriggerLevel = uj::readFloat(j, CONF_KEY_REC_TRIGGER_LEVEL);

#ifdef WITH_VST

	pluginChooserX   = uj::readInt(j, CONF_KEY_PLUGIN_CHOOSER_X);
	pluginChooserY   = uj::readInt(j, CONF_KEY_PLUGIN_CHOOSER_Y);
	pluginChooserW   = uj::readInt(j, CONF_KEY_PLUGIN_CHOOSER_W);
	pluginChooserH   = uj::readInt(j, CONF_KEY_PLUGIN_CHOOSER_H);
	pluginSortMethod = uj::readInt(j, CONF_KEY_PLUGIN_SORT_METHOD);

#endif

	json_decref(j);

	sanitize();

	return true;
}


/* -------------------------------------------------------------------------- */


bool write()
{
	if (!createConfigFolder())
		return false;

	json_t* j = json_object();

	json_object_set_new(j, CONF_KEY_HEADER,                    json_string(header.c_str()));
	json_object_set_new(j, CONF_KEY_LOG_MODE,                  json_integer(logMode));
	json_object_set_new(j, CONF_KEY_SOUND_SYSTEM,              json_integer(soundSystem));
	json_object_set_new(j, CONF_KEY_SOUND_DEVICE_OUT,          json_integer(soundDeviceOut));
	json_object_set_new(j, CONF_KEY_SOUND_DEVICE_IN,           json_integer(soundDeviceIn));
	json_object_set_new(j, CONF_KEY_CHANNELS_OUT,              json_integer(channelsOut));
	json_object_set_new(j, CONF_KEY_CHANNELS_IN,               json_integer(channelsIn));
	json_object_set_new(j, CONF_KEY_SAMPLERATE,                json_integer(samplerate));
	json_object_set_new(j, CONF_KEY_BUFFER_SIZE,               json_integer(buffersize));
	json_object_set_new(j, CONF_KEY_LIMIT_OUTPUT,              json_boolean(limitOutput));
	json_object_set_new(j, CONF_KEY_RESAMPLE_QUALITY,          json_integer(rsmpQuality));
	json_object_set_new(j, CONF_KEY_MIDI_SYSTEM,               json_integer(midiSystem));
	json_object_set_new(j, CONF_KEY_MIDI_PORT_OUT,             json_integer(midiPortOut));
	json_object_set_new(j, CONF_KEY_MIDI_PORT_IN,              json_integer(midiPortIn));
	json_object_set_new(j, CONF_KEY_MIDIMAP_PATH,              json_string(midiMapPath.c_str()));
	json_object_set_new(j, CONF_KEY_LAST_MIDIMAP,              json_string(lastFileMap.c_str()));
	json_object_set_new(j, CONF_KEY_MIDI_SYNC,                 json_integer(midiSync));
	json_object_set_new(j, CONF_KEY_MIDI_TC_FPS,               json_real(midiTCfps));
	json_object_set_new(j, CONF_KEY_MIDI_IN,                   json_boolean(midiIn));
	json_object_set_new(j, CONF_KEY_MIDI_IN_FILTER,            json_integer(midiInFilter));
	json_object_set_new(j, CONF_KEY_MIDI_IN_REWIND,            json_integer(midiInRewind));
	json_object_set_new(j, CONF_KEY_MIDI_IN_START_STOP,        json_integer(midiInStartStop));
	json_object_set_new(j, CONF_KEY_MIDI_IN_ACTION_REC,        json_integer(midiInActionRec));
	json_object_set_new(j, CONF_KEY_MIDI_IN_INPUT_REC,         json_integer(midiInInputRec));
	json_object_set_new(j, CONF_KEY_MIDI_IN_METRONOME,         json_integer(midiInMetronome));
	json_object_set_new(j, CONF_KEY_MIDI_IN_VOLUME_IN,         json_integer(midiInVolumeIn));
	json_object_set_new(j, CONF_KEY_MIDI_IN_VOLUME_OUT,        json_integer(midiInVolumeOut));
	json_object_set_new(j, CONF_KEY_MIDI_IN_BEAT_DOUBLE,       json_integer(midiInBeatDouble));
	json_object_set_new(j, CONF_KEY_MIDI_IN_BEAT_HALF,         json_integer(midiInBeatHalf));
	json_object_set_new(j, CONF_KEY_RECS_STOP_ON_CHAN_HALT,    json_boolean(recsStopOnChanHalt));
	json_object_set_new(j, CONF_KEY_CHANS_STOP_ON_SEQ_HALT,    json_boolean(chansStopOnSeqHalt));
	json_object_set_new(j, CONF_KEY_TREAT_RECS_AS_LOOPS,       json_boolean(treatRecsAsLoops));
	json_object_set_new(j, CONF_KEY_INPUT_MONITOR_DEFAULT_ON,  json_boolean(inputMonitorDefaultOn));
	json_object_set_new(j, CONF_KEY_PLUGINS_PATH,              json_string(pluginPath.c_str()));
	json_object_set_new(j, CONF_KEY_PATCHES_PATH,              json_string(patchPath.c_str()));
	json_object_set_new(j, CONF_KEY_SAMPLES_PATH,              json_string(samplePath.c_str()));
	json_object_set_new(j, CONF_KEY_MAIN_WINDOW_X,             json_integer(mainWindowX));
	json_object_set_new(j, CONF_KEY_MAIN_WINDOW_Y,             json_integer(mainWindowY));
	json_object_set_new(j, CONF_KEY_MAIN_WINDOW_W,             json_integer(mainWindowW));
	json_object_set_new(j, CONF_KEY_MAIN_WINDOW_H,             json_integer(mainWindowH));
	json_object_set_new(j, CONF_KEY_BROWSER_X,                 json_integer(browserX));
	json_object_set_new(j, CONF_KEY_BROWSER_Y,                 json_integer(browserY));
	json_object_set_new(j, CONF_KEY_BROWSER_W,                 json_integer(browserW));
	json_object_set_new(j, CONF_KEY_BROWSER_H,                 json_integer(browserH));
	json_object_set_new(j, CONF_KEY_BROWSER_POSITION,          json_integer(browserPosition));
	json_object_set_new(j, CONF_KEY_BROWSER_LAST_PATH,         json_string(browserLastPath.c_str()));
	json_object_set_new(j, CONF_KEY_BROWSER_LAST_VALUE,        json_integer(browserLastValue));
	json_object_set_new(j, CONF_KEY_ACTION_EDITOR_X,           json_integer(actionEditorX));
	json_object_set_new(j, CONF_KEY_ACTION_EDITOR_Y,           json_integer(actionEditorY));
	json_object_set_new(j, CONF_KEY_ACTION_EDITOR_W,           json_integer(actionEditorW));
	json_object_set_new(j, CONF_KEY_ACTION_EDITOR_H,           json_integer(actionEditorH));
	json_object_set_new(j, CONF_KEY_ACTION_EDITOR_ZOOM,        json_integer(actionEditorZoom));
	json_object_set_new(j, CONF_KEY_ACTION_EDITOR_GRID_VAL,    json_integer(actionEditorGridVal));
	json_object_set_new(j, CONF_KEY_ACTION_EDITOR_GRID_ON,     json_integer(actionEditorGridOn));
	json_object_set_new(j, CONF_KEY_SAMPLE_EDITOR_X,           json_integer(sampleEditorX));
	json_object_set_new(j, CONF_KEY_SAMPLE_EDITOR_Y,           json_integer(sampleEditorY));
	json_object_set_new(j, CONF_KEY_SAMPLE_EDITOR_W,           json_integer(sampleEditorW));
	json_object_set_new(j, CONF_KEY_SAMPLE_EDITOR_H,           json_integer(sampleEditorH));
	json_object_set_new(j, CONF_KEY_SAMPLE_EDITOR_GRID_VAL,    json_integer(sampleEditorGridVal));
	json_object_set_new(j, CONF_KEY_SAMPLE_EDITOR_GRID_ON,     json_integer(sampleEditorGridOn));
	json_object_set_new(j, CONF_KEY_PIANO_ROLL_Y,              json_integer(pianoRollY));
	json_object_set_new(j, CONF_KEY_PIANO_ROLL_H,              json_integer(pianoRollH));
	json_object_set_new(j, CONF_KEY_SAMPLE_ACTION_EDITOR_H,    json_integer(sampleActionEditorH));
	json_object_set_new(j, CONF_KEY_VELOCITY_EDITOR_H,         json_integer(velocityEditorH));
	json_object_set_new(j, CONF_KEY_ENVELOPE_EDITOR_H,         json_integer(envelopeEditorH));
	json_object_set_new(j, CONF_KEY_PLUGIN_LIST_X,             json_integer(pluginListX));
	json_object_set_new(j, CONF_KEY_PLUGIN_LIST_Y,             json_integer(pluginListY));
	json_object_set_new(j, CONF_KEY_CONFIG_X,                  json_integer(configX));
	json_object_set_new(j, CONF_KEY_CONFIG_Y,                  json_integer(configY));
	json_object_set_new(j, CONF_KEY_BPM_X,                     json_integer(bpmX));
	json_object_set_new(j, CONF_KEY_BPM_Y,                     json_integer(bpmY));
	json_object_set_new(j, CONF_KEY_BEATS_X,                   json_integer(beatsX));
	json_object_set_new(j, CONF_KEY_BEATS_Y,                   json_integer(beatsY));
	json_object_set_new(j, CONF_KEY_ABOUT_X,                   json_integer(aboutX));
	json_object_set_new(j, CONF_KEY_ABOUT_Y,                   json_integer(aboutY));	
	json_object_set_new(j, CONF_KEY_NAME_X,                    json_integer(nameX));
	json_object_set_new(j, CONF_KEY_NAME_Y,                    json_integer(nameY));
	json_object_set_new(j, CONF_KEY_MIDI_INPUT_X,              json_integer(midiInputX));
	json_object_set_new(j, CONF_KEY_MIDI_INPUT_Y,              json_integer(midiInputY));
	json_object_set_new(j, CONF_KEY_MIDI_INPUT_W,              json_integer(midiInputW));
	json_object_set_new(j, CONF_KEY_MIDI_INPUT_H,              json_integer(midiInputH));
	json_object_set_new(j, CONF_KEY_REC_TRIGGER_MODE,          json_integer(recTriggerMode));
	json_object_set_new(j, CONF_KEY_REC_TRIGGER_LEVEL,         json_real(recTriggerLevel));

#ifdef WITH_VST

	json_object_set_new(j, CONF_KEY_PLUGIN_CHOOSER_X,   json_integer(pluginChooserX));
	json_object_set_new(j, CONF_KEY_PLUGIN_CHOOSER_Y,   json_integer(pluginChooserY));
	json_object_set_new(j, CONF_KEY_PLUGIN_CHOOSER_W,   json_integer(pluginChooserW));
	json_object_set_new(j, CONF_KEY_PLUGIN_CHOOSER_H,   json_integer(pluginChooserH));
	json_object_set_new(j, CONF_KEY_PLUGIN_SORT_METHOD, json_integer(pluginSortMethod));

#endif

  if (json_dump_file(j, confFilePath.c_str(), JSON_INDENT(2)) != 0) {
	gu_log("[conf::write] unable to write configuration file!\n");
	return false;
  }
  return true;
}
}}}; // giada::m::conf::
