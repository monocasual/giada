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

#include "gui/langMapper.h"
#include "utils/fs.h"
#include "utils/log.h"
#include <filesystem>
#include <fstream>

namespace nl = nlohmann;

namespace giada::v
{
LangMap::LangMap()
: m_default("[Not set]")
{
	m_data[COMMON_OK]         = "Ok";
	m_data[COMMON_CANCEL]     = "Cancel";
	m_data[COMMON_YES]        = "Yes";
	m_data[COMMON_NO]         = "No";
	m_data[COMMON_OFF]        = "Off";
	m_data[COMMON_SELECT]     = "Select";
	m_data[COMMON_LOAD]       = "Load";
	m_data[COMMON_SAVE]       = "Save";
	m_data[COMMON_WARNING]    = "Warning";
	m_data[COMMON_CLOSE]      = "Close";
	m_data[COMMON_ADD]        = "Add";
	m_data[COMMON_CLEAR]      = "Clear";
	m_data[COMMON_RELOAD]     = "Reload";
	m_data[COMMON_RESET]      = "Reset";
	m_data[COMMON_ZOOMIN]     = "Zoom in";
	m_data[COMMON_ZOOMOUT]    = "Zoom out";
	m_data[COMMON_GRIDRES]    = "Grid resolution";
	m_data[COMMON_SNAPTOGRID] = "Snap to grid";
	m_data[COMMON_BIND]       = "Bind";
	m_data[COMMON_LEARN]      = "Learn";
	m_data[COMMON_NOTSET]     = "(not set)";
	m_data[COMMON_NONE]       = "None";
	m_data[COMMON_APPLY]      = "Apply";

	m_data[MESSAGE_MAIN_FREEALLSAMPLES]           = "Free all Sample channels: are you sure?";
	m_data[MESSAGE_MAIN_CLEARALLACTIONS]          = "Clear all actions: are you sure?";
	m_data[MESSAGE_MAIN_CLEARALLVOLUMEACTIONS]    = "Clear all volume actions: are you sure?";
	m_data[MESSAGE_MAIN_CLEARALLSTARTSTOPACTIONS] = "Clear all start/stop actions: are you sure?";
	m_data[MESSAGE_MAIN_CLOSEPROJECT]             = "Close project: are you sure?";

	m_data[MESSAGE_INIT_WRONGSYSTEM] = "Your soundcard isn't configured correctly!";
	m_data[MESSAGE_INIT_QUITGIADA]   = "Quit Giada: are you sure?";

	m_data[MESSAGE_CHANNEL_MULTICHANNOTSUPPORTED] = "Multichannel samples not supported.";
	m_data[MESSAGE_CHANNEL_CANTREADSAMPLE]        = "Unable to read this sample.";
	m_data[MESSAGE_CHANNEL_PATHTOOLONG]           = "File path too long.";
	m_data[MESSAGE_CHANNEL_NOFILESPECIFIED]       = "No file specified.";
	m_data[MESSAGE_CHANNEL_LOADINGSAMPLES]        = "Loading samples...";
	m_data[MESSAGE_CHANNEL_LOADINGSAMPLESERROR]   = "Some files weren't loaded successfully.";
	m_data[MESSAGE_CHANNEL_DELETE]                = "Delete channel: are you sure?";
	m_data[MESSAGE_CHANNEL_FREE]                  = "Free channel: are you sure?";

	m_data[MESSAGE_STORAGE_PATCHUNREADABLE]     = "This patch is unreadable.";
	m_data[MESSAGE_STORAGE_PATCHINVALID]        = "This patch is not valid.";
	m_data[MESSAGE_STORAGE_PATCHUNSUPPORTED]    = "This patch format is no longer supported.";
	m_data[MESSAGE_STORAGE_PROJECTEXISTS]       = "Project exists: overwrite?";
	m_data[MESSAGE_STORAGE_LOADINGPROJECT]      = "Loading project...";
	m_data[MESSAGE_STORAGE_LOADINGSAMPLE]       = "Loading sample...";
	m_data[MESSAGE_STORAGE_SAVINGPROJECT]       = "Saving project...";
	m_data[MESSAGE_STORAGE_SAVINGPROJECTERROR]  = "Unable to save the project!";
	m_data[MESSAGE_STORAGE_CHOOSEFILENAME]      = "Please choose a file name.";
	m_data[MESSAGE_STORAGE_FILEHASINVALIDCHARS] = "The file name contains invalid characters.";
	m_data[MESSAGE_STORAGE_FILEEXISTS]          = "File exists: overwrite?";
	m_data[MESSAGE_STORAGE_SAVINGFILEERROR]     = "Unable to save this sample!";

	m_data[MAIN_MENU_FILE]                 = "File";
	m_data[MAIN_MENU_FILE_OPENPROJECT]     = "Open project...";
	m_data[MAIN_MENU_FILE_SAVEPROJECT]     = "Save project...";
	m_data[MAIN_MENU_FILE_CLOSEPROJECT]    = "Close project";
	m_data[MAIN_MENU_FILE_DEBUGSTATS]      = "Debug stats";
	m_data[MAIN_MENU_FILE_QUIT]            = "Quit Giada";
	m_data[MAIN_MENU_EDIT]                 = "Edit";
	m_data[MAIN_MENU_EDIT_FREEALLSAMPLES]  = "Free all Sample channels";
	m_data[MAIN_MENU_EDIT_CLEARALLACTIONS] = "Clear all actions";
	m_data[MAIN_MENU_EDIT_SETUPMIDIINPUT]  = "Setup global MIDI input...";
	m_data[MAIN_MENU_CONFIG]               = "Setup";
	m_data[MAIN_MENU_ABOUT]                = "About";

	m_data[MAIN_IO_LABEL_OUTMETER]     = "Main output meter";
	m_data[MAIN_IO_LABEL_INMETER]      = "Main input meter";
	m_data[MAIN_IO_LABEL_OUTVOL]       = "Main output volume";
	m_data[MAIN_IO_LABEL_INVOL]        = "Main input volume";
	m_data[MAIN_IO_LABEL_INTOOUT]      = "Stream linker\n\nConnects input to output to enable \"hear what you're playing\" mode.";
	m_data[MAIN_IO_LABEL_FXOUT]        = "Main output plug-ins";
	m_data[MAIN_IO_LABEL_FXIN]         = "Main input plug-ins";
	m_data[MAIN_IO_LABEL_MIDIACTIVITY] = "Master MIDI I/O activity\n\nNotifies MIDI messages sent (top) or "
	                                     "received (bottom) globally.";
	m_data[MAIN_IO_LABEL_MIDIINACTIVITY]  = "Master MIDI input activity\n\nNotifies MIDI messages received globally.";
	m_data[MAIN_IO_LABEL_MIDIOUTACTIVITY] = "Master MIDI output activity\n\nNotifies MIDI messages sent globally.";

	m_data[MAIN_TIMER_LABEL_BPM]        = "Beats per minute (BPM)";
	m_data[MAIN_TIMER_LABEL_METER]      = "Beats and bars";
	m_data[MAIN_TIMER_LABEL_QUANTIZER]  = "Live quantizer";
	m_data[MAIN_TIMER_LABEL_MULTIPLIER] = "Beat multiplier";
	m_data[MAIN_TIMER_LABEL_DIVIDER]    = "Beat divider";

	m_data[MAIN_SEQUENCER_LABEL] = "Main sequencer";

	m_data[MAIN_TRANSPORT_LABEL_REWIND]         = "Rewind";
	m_data[MAIN_TRANSPORT_LABEL_PLAY]           = "Play/Stop";
	m_data[MAIN_TRANSPORT_LABEL_RECTRIGGERMODE] = "Record-on-signal mode\n\nAction and audio recording will start only when a signal (key press or audio) "
	                                              "is detected.\n\nAvailable if the sequencer is not running.";
	m_data[MAIN_TRANSPORT_LABEL_RECACTIONS]   = "Record actions";
	m_data[MAIN_TRANSPORT_LABEL_RECINPUT]     = "Record audio\n\nAvailable if:\n - input device is enabled\n - there is at least one empty and armed Sample channel";
	m_data[MAIN_TRANSPORT_LABEL_RECINPUTMODE] = "Free loop-length mode\n\nThe sequencer will adjust to the length of your first audio recording.\n\n"
	                                            "Available if there are no other audio samples in the project.";
	m_data[MAIN_TRANSPORT_LABEL_METRONOME] = "Metronome";

	m_data[MAIN_COLUMN_HELP] = "Drop audio files or right-click here";

	m_data[MAIN_COLUMN_BUTTON]                  = "Edit column";
	m_data[MAIN_COLUMN_BUTTON_ADDSAMPLECHANNEL] = "Add Sample channel";
	m_data[MAIN_COLUMN_BUTTON_ADDMIDICHANNEL]   = "Add MIDI channel";
	m_data[MAIN_COLUMN_BUTTON_REMOVE]           = "Remove";
	m_data[MAIN_COLUMN_BUTTON_ADD_COLUMN]       = "Add column";

	m_data[MAIN_CHANNEL_NOSAMPLE]          = "-- no sample --";
	m_data[MAIN_CHANNEL_SAMPLENOTFOUND]    = "* file not found! *";
	m_data[MAIN_CHANNEL_LABEL_PLAY]        = "Play/stop";
	m_data[MAIN_CHANNEL_LABEL_ARM]         = "Arm for recording";
	m_data[MAIN_CHANNEL_LABEL_STATUS]      = "Progress bar";
	m_data[MAIN_CHANNEL_LABEL_READACTIONS] = "Read actions\n\nToggles playback of pre-recorded "
	                                         "actions (key press, key release, ...).";
	m_data[MAIN_CHANNEL_LABEL_MODEBOX]      = "Mode";
	m_data[MAIN_CHANNEL_LABEL_MUTE]         = "Mute";
	m_data[MAIN_CHANNEL_LABEL_SOLO]         = "Solo";
	m_data[MAIN_CHANNEL_LABEL_FX]           = "Plug-ins";
	m_data[MAIN_CHANNEL_LABEL_VOLUME]       = "Volume";
	m_data[MAIN_CHANNEL_LABEL_MIDIACTIVITY] = "MIDI I/O activity\n\nNotifies MIDI messages sent (top) or "
	                                          "received (bottom) by this channel.";

	m_data[MAIN_CHANNEL_MENU_INPUTMONITOR]           = "Input monitor";
	m_data[MAIN_CHANNEL_MENU_OVERDUBPROTECTION]      = "Overdub protection";
	m_data[MAIN_CHANNEL_MENU_LOADSAMPLE]             = "Load new sample...";
	m_data[MAIN_CHANNEL_MENU_EXPORTSAMPLE]           = "Export sample to file...";
	m_data[MAIN_CHANNEL_MENU_KEYBOARDINPUT]          = "Setup keyboard input...";
	m_data[MAIN_CHANNEL_MENU_MIDIINPUT]              = "Setup MIDI input...";
	m_data[MAIN_CHANNEL_MENU_MIDIOUTPUT]             = "Setup MIDI output...";
	m_data[MAIN_CHANNEL_MENU_EDITROUTING]            = "Edit routing...";
	m_data[MAIN_CHANNEL_MENU_EDITSAMPLE]             = "Edit sample...";
	m_data[MAIN_CHANNEL_MENU_EDITACTIONS]            = "Edit actions...";
	m_data[MAIN_CHANNEL_MENU_CLEARACTIONS]           = "Clear actions";
	m_data[MAIN_CHANNEL_MENU_CLEARACTIONS_ALL]       = "All";
	m_data[MAIN_CHANNEL_MENU_CLEARACTIONS_VOLUME]    = "Volume";
	m_data[MAIN_CHANNEL_MENU_CLEARACTIONS_STARTSTOP] = "Start/Stop";
	m_data[MAIN_CHANNEL_MENU_RENAME]                 = "Rename";
	m_data[MAIN_CHANNEL_MENU_CLONE]                  = "Clone";
	m_data[MAIN_CHANNEL_MENU_FREE]                   = "Free";
	m_data[MAIN_CHANNEL_MENU_DELETE]                 = "Delete";

	m_data[MISSINGASSETS_INTRO]      = "This project contains missing assets.";
	m_data[MISSINGASSETS_AUDIOFILES] = "Audio files not found in the project folder:";
	m_data[MISSINGASSETS_PLUGINS]    = "Audio plug-ins not found globally:";

	m_data[PLUGINCHOOSER_TITLE]               = "Available plugins";
	m_data[PLUGINCHOOSER_HEADER]              = "NAME\tMANUFACTURER\tCATEGORY\tFORMAT\tUID";
	m_data[PLUGINCHOOSER_SORTBY]              = "Sort by";
	m_data[PLUGINCHOOSER_SORTBY_NAME]         = "Name";
	m_data[PLUGINCHOOSER_SORTBY_CATEGORY]     = "Category";
	m_data[PLUGINCHOOSER_SORTBY_MANUFACTURER] = "Manufacturer";
	m_data[PLUGINCHOOSER_SORTBY_FORMAT]       = "Format";

	m_data[PLUGINLIST_TITLE_MASTEROUT] = "Master Out Plug-ins";
	m_data[PLUGINLIST_TITLE_MASTERIN]  = "Master In Plug-ins";
	m_data[PLUGINLIST_TITLE_CHANNEL]   = "Channel Plug-ins";
	m_data[PLUGINLIST_ADDPLUGIN]       = "-- add new plugin --";
	m_data[PLUGINLIST_NOPROGRAMS]      = "-- no programs --";

	m_data[CHANNELNAME_TITLE] = "New channel name";

	m_data[KEYGRABBER_TITLE] = "Key configuration";
	m_data[KEYGRABBER_BODY]  = "Press a key.\n\nCurrent binding: ";

	m_data[SAMPLEEDITOR_TITLE]                = "Sample Editor";
	m_data[SAMPLEEDITOR_RELOAD]               = "Reload";
	m_data[SAMPLEEDITOR_LOOP]                 = "Loop";
	m_data[SAMPLEEDITOR_INFO]                 = "File: {}\nSize: {} frames\nDuration {} seconds\nBit depth: {}\nFrequency: {} Hz";
	m_data[SAMPLEEDITOR_PAN]                  = "Pan";
	m_data[SAMPLEEDITOR_PITCH]                = "Pitch";
	m_data[SAMPLEEDITOR_PITCH_TOBAR]          = "To bar";
	m_data[SAMPLEEDITOR_PITCH_TOSONG]         = "To song";
	m_data[SAMPLEEDITOR_RANGE]                = "Range";
	m_data[SAMPLEEDITOR_SHIFT]                = "Shift";
	m_data[SAMPLEEDITOR_VOLUME]               = "Volume";
	m_data[SAMPLEEDITOR_TOOLS_CUT]            = "Cut";
	m_data[SAMPLEEDITOR_TOOLS_COPY]           = "Copy";
	m_data[SAMPLEEDITOR_TOOLS_PASTE]          = "Paste";
	m_data[SAMPLEEDITOR_TOOLS_TRIM]           = "Trim";
	m_data[SAMPLEEDITOR_TOOLS_SILENCE]        = "Silence";
	m_data[SAMPLEEDITOR_TOOLS_REVERSE]        = "Reverse";
	m_data[SAMPLEEDITOR_TOOLS_NORMALIZE]      = "Normalize";
	m_data[SAMPLEEDITOR_TOOLS_FADE_IN]        = "Fade in";
	m_data[SAMPLEEDITOR_TOOLS_FADE_OUT]       = "Fade out";
	m_data[SAMPLEEDITOR_TOOLS_SMOOTH_EDGES]   = "Smooth edges";
	m_data[SAMPLEEDITOR_TOOLS_SET_BEGIN_END]  = "Set begin/end here";
	m_data[SAMPLEEDITOR_TOOLS_TO_NEW_CHANNEL] = "Copy to new channel";

	m_data[ACTIONEDITOR_TITLE]            = "Action Editor";
	m_data[ACTIONEDITOR_VOLUME]           = "Volume";
	m_data[ACTIONEDITOR_KEYPRESS]         = "Key press";
	m_data[ACTIONEDITOR_KEYRELEASE]       = "Key release";
	m_data[ACTIONEDITOR_STOPSAMPLE]       = "Stop sample";
	m_data[ACTIONEDITOR_STARTSTOP]        = "Start/stop";
	m_data[ACTIONEDITOR_VELOCITY]         = "Velocity";
	m_data[ACTIONEDITOR_LABEL_ACTIONTYPE] = "Action type to add";

	m_data[BROWSER_SHOWHIDDENFILES] = "Show hidden files";
	m_data[BROWSER_OPENPROJECT]     = "Open project";
	m_data[BROWSER_SAVEPROJECT]     = "Save project";
	m_data[BROWSER_OPENSAMPLE]      = "Open sample";
	m_data[BROWSER_SAVESAMPLE]      = "Save sample";
	m_data[BROWSER_OPENPLUGINSDIR]  = "Open plug-ins directory";

	m_data[MIDIINPUT_MASTER_TITLE]           = "MIDI Input Setup (global)";
	m_data[MIDIINPUT_MASTER_ENABLE]          = "Enable MIDI input";
	m_data[MIDIINPUT_MASTER_LEARN_REWIND]    = "Rewind";
	m_data[MIDIINPUT_MASTER_LEARN_PLAYSTOP]  = "Play/stop";
	m_data[MIDIINPUT_MASTER_LEARN_ACTIONREC] = "Action recording";
	m_data[MIDIINPUT_MASTER_LEARN_INPUTREC]  = "Input recording";
	m_data[MIDIINPUT_MASTER_LEARN_METRONOME] = "metronome";
	m_data[MIDIINPUT_MASTER_LEARN_INVOLUME]  = "Input volume";
	m_data[MIDIINPUT_MASTER_LEARN_OUTVOLUME] = "Output volume";
	m_data[MIDIINPUT_MASTER_LEARN_SEQDOUBLE] = "Sequencer ×2";
	m_data[MIDIINPUT_MASTER_LEARN_SEQHALF]   = "Sequencer ÷2";

	m_data[MIDIINPUT_CHANNEL_TITLE]             = "MIDI Input Setup";
	m_data[MIDIINPUT_CHANNEL_ENABLE]            = "Enable MIDI input";
	m_data[MIDIINPUT_CHANNEL_VELOCITYDRIVESVOL] = "Velocity drives volume (Sample Channels)";
	m_data[MIDIINPUT_CHANNEL_LEARN_CHANNEL]     = "Channel";
	m_data[MIDIINPUT_CHANNEL_LEARN_KEYPRESS]    = "Key press";
	m_data[MIDIINPUT_CHANNEL_LEARN_KEYREL]      = "Key release";
	m_data[MIDIINPUT_CHANNEL_LEARN_KEYKILL]     = "Key kill";
	m_data[MIDIINPUT_CHANNEL_LEARN_ARM]         = "Arm";
	m_data[MIDIINPUT_CHANNEL_LEARN_MUTE]        = "Mute";
	m_data[MIDIINPUT_CHANNEL_LEARN_SOLO]        = "Solo";
	m_data[MIDIINPUT_CHANNEL_LEARN_VOLUME]      = "Volume";
	m_data[MIDIINPUT_CHANNEL_LEARN_PITCH]       = "Pitch";
	m_data[MIDIINPUT_CHANNEL_LEARN_READACTIONS] = "Read actions";

	m_data[MIDIOUTPUT_CHANNEL_TITLE]            = "Midi Output Setup";
	m_data[MIDIOUTPUT_CHANNEL_ENABLE]           = "Enable MIDI output";
	m_data[MIDIOUTPUT_CHANNEL_ENABLE_LIGHTNING] = "Enable MIDI lightning output";
	m_data[MIDIOUTPUT_CHANNEL_LEARN_PLAYING]    = "Playing";
	m_data[MIDIOUTPUT_CHANNEL_LEARN_MUTE]       = "Mute";
	m_data[MIDIOUTPUT_CHANNEL_LEARN_SOLO]       = "Solo";

	m_data[ABOUT_TITLE] = "About Giada";
	m_data[ABOUT_BODY]  = "Version {} ({} build) {}\n\n"
	                     "Developed by Monocasual Laboratories\n\n"
	                     "Released under the terms of the GNU General\n"
	                     "Public License (GPL v3)\n\n"
	                     "News, infos, contacts and documentation:\n"
	                     "www.giadamusic.com";
	m_data[ABOUT_BODY_VST] = "VST Plug-In Technology by Steinberg\n"
	                         "VST is a trademark of Steinberg\nMedia Technologies GmbH";

	m_data[CONFIG_TITLE]        = "Configuration";
	m_data[CONFIG_RESTARTGIADA] = "Restart Giada for the changes to take effect.";

	m_data[CONFIG_AUDIO_TITLE]                 = "Audio";
	m_data[CONFIG_AUDIO_SYSTEM]                = "System";
	m_data[CONFIG_AUDIO_BUFFERSIZE]            = "Buffer size";
	m_data[CONFIG_AUDIO_SAMPLERATE]            = "Sample rate";
	m_data[CONFIG_AUDIO_OUTPUTDEVICE]          = "Output device";
	m_data[CONFIG_AUDIO_OUTPUTCHANNELS]        = "Output channels";
	m_data[CONFIG_AUDIO_LIMITOUTPUT]           = "Limit output";
	m_data[CONFIG_AUDIO_INPUTDEVICE]           = "Input device";
	m_data[CONFIG_AUDIO_INPUTCHANNELS]         = "Input channels";
	m_data[CONFIG_AUDIO_RECTHRESHOLD]          = "Rec threshold (dB)";
	m_data[CONFIG_AUDIO_ENABLEINPUT]           = "Enable Input";
	m_data[CONFIG_AUDIO_RESAMPLING]            = "Resampling";
	m_data[CONFIG_AUDIO_RESAMPLING_SINCBEST]   = "Sinc best quality (very slow)";
	m_data[CONFIG_AUDIO_RESAMPLING_SINCMEDIUM] = "Sinc medium quality (slow)";
	m_data[CONFIG_AUDIO_RESAMPLING_SINCBASIC]  = "Sinc basic quality (medium)";
	m_data[CONFIG_AUDIO_RESAMPLING_ZEROORDER]  = "Zero Order Hold (fast)";
	m_data[CONFIG_AUDIO_RESAMPLING_LINEAR]     = "Linear (very fast)";
	m_data[CONFIG_AUDIO_NODEVICESFOUND]        = "-- no devices found --";

	m_data[CONFIG_MIDI_TITLE]           = "MIDI";
	m_data[CONFIG_MIDI_SYSTEM]          = "System";
	m_data[CONFIG_MIDI_OUTPUTPORT]      = "Output port";
	m_data[CONFIG_MIDI_INPUTPORT]       = "Input port";
	m_data[CONFIG_MIDI_NOPORTSFOUND]    = "-- no ports found --";
	m_data[CONFIG_MIDI_OUTPUTMIDIMAP]   = "Output MIDI Map";
	m_data[CONFIG_MIDI_NOMIDIMAPSFOUND] = "(no MIDI maps available)";
	m_data[CONFIG_MIDI_SYNC]            = "Sync";
	m_data[CONFIG_MIDI_LABEL_ENABLEOUT] = "Enable Output port";
	m_data[CONFIG_MIDI_LABEL_ENABLEIN]  = "Enable Input port";
	m_data[CONFIG_MIDI_LABEL_WRONGMIDI] = "Could not apply MIDI configuration. Error:";

	m_data[CONFIG_BEHAVIORS_TITLE]                      = "Behaviors";
	m_data[CONFIG_BEHAVIORS_CHANSSTOPONSEQHALT]         = "Dynamic channels stop immediately when the sequencer is halted";
	m_data[CONFIG_BEHAVIORS_TREATRECSASLOOPS]           = "Treat one shot channels with actions as loops";
	m_data[CONFIG_BEHAVIORS_INPUTMONITORDEFAULTON]      = "New sample channels have input monitor on by default";
	m_data[CONFIG_BEHAVIORS_OVERDUBPROTECTIONDEFAULTON] = "New sample channels have overdub protection on by default";

	m_data[CONFIG_BINDINGS_TITLE]         = "Key Bindings";
	m_data[CONFIG_BINDINGS_PLAY]          = "Play";
	m_data[CONFIG_BINDINGS_REWIND]        = "Rewind";
	m_data[CONFIG_BINDINGS_RECORDACTIONS] = "Record actions";
	m_data[CONFIG_BINDINGS_RECORDAUDIO]   = "Record audio";
	m_data[CONFIG_BINDINGS_EXIT]          = "Exit";

	m_data[CONFIG_MISC_TITLE]                  = "Misc";
	m_data[CONFIG_MISC_DEBUGMESSAGES]          = "Debug messages";
	m_data[CONFIG_MISC_DEBUGMESSAGES_DISABLED] = "Disabled";
	m_data[CONFIG_MISC_DEBUGMESSAGES_TOSTDOUT] = "To standard output";
	m_data[CONFIG_MISC_DEBUGMESSAGES_TOFILE]   = "To file";
	m_data[CONFIG_MISC_TOOLTIPS]               = "Tooltips";
	m_data[CONFIG_MISC_TOOLTIPS_DISABLED]      = "Disabled";
	m_data[CONFIG_MISC_TOOLTIPS_ENABLED]       = "Enabled";
	m_data[CONFIG_MISC_LANGUAGE]               = "Language file";
	m_data[CONFIG_MISC_NOLANGUAGESFOUND]       = "-- no language files found --";
	m_data[CONFIG_MISC_UISCALING]              = "UI scaling";

	m_data[CONFIG_PLUGINS_TITLE]       = "Plug-ins";
	m_data[CONFIG_PLUGINS_FOLDER]      = "Plug-ins folder";
	m_data[CONFIG_PLUGINS_SCANNING]    = "Scanning in progress. Please wait...";
	m_data[CONFIG_PLUGINS_SCAN]        = "Scan ({} found)";
	m_data[CONFIG_PLUGINS_INVALIDPATH] = "Invalid path.";

	m_data[CHANNELROUTING_TITLE] = "Channel Routing";
}

const char* LangMap::get(const std::string& key) const
{
	if (m_data.count(key) == 0)
		return m_default.c_str();
	return m_data.at(key).c_str();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

LangMapper::LangMapper()
{
	m_mapsPath = u::fs::getLangMapsPath();
}

/* -------------------------------------------------------------------------- */

void LangMapper::init()
{
	Mapper::init();
	u::log::print("[LangMapper::init] total langmaps found: {}\n", m_mapFiles.size());
}

/* -------------------------------------------------------------------------- */

int LangMapper::read(const std::string& file)
{
	std::optional<nl::json> res = Mapper::read(file);
	if (!res)
		return G_FILE_UNREADABLE;

	m_map.m_data = res.value().get<LangMap::Data>();

	return G_FILE_OK;
}

/* -------------------------------------------------------------------------- */

const char* LangMapper::get(const std::string& s) const
{
	return m_map.get(s);
}
} // namespace giada::v
