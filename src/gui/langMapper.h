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

#ifndef G_LANGMAPPER_H
#define G_LANGMAPPER_H

#include "mapper.h"
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace giada::v
{
class LangMap
{
public:
	friend class LangMapper;

	using Data = std::unordered_map<std::string, std::string>;

	static constexpr auto COMMON_OK         = "common_ok";
	static constexpr auto COMMON_CANCEL     = "common_cancel";
	static constexpr auto COMMON_YES        = "common_yes";
	static constexpr auto COMMON_NO         = "common_no";
	static constexpr auto COMMON_OFF        = "common_off";
	static constexpr auto COMMON_SELECT     = "common_select";
	static constexpr auto COMMON_LOAD       = "common_load";
	static constexpr auto COMMON_SAVE       = "common_save";
	static constexpr auto COMMON_WARNING    = "common_warning";
	static constexpr auto COMMON_CLOSE      = "common_close";
	static constexpr auto COMMON_ADD        = "common_add";
	static constexpr auto COMMON_CLEAR      = "common_clear";
	static constexpr auto COMMON_RELOAD     = "common_reload";
	static constexpr auto COMMON_RESET      = "common_reset";
	static constexpr auto COMMON_ZOOMIN     = "common_zoomIn";
	static constexpr auto COMMON_ZOOMOUT    = "common_zoomOut";
	static constexpr auto COMMON_GRIDRES    = "common_gridRes";
	static constexpr auto COMMON_SNAPTOGRID = "common_snapToGrid";
	static constexpr auto COMMON_BIND       = "common_bind";
	static constexpr auto COMMON_LEARN      = "common_learn";
	static constexpr auto COMMON_NOTSET     = "common_notSet";
	static constexpr auto COMMON_NONE       = "common_none";
	static constexpr auto COMMON_APPLY      = "common_apply";

	static constexpr auto MESSAGE_MAIN_FREEALLSAMPLES           = "message_main_freeAllSamples";
	static constexpr auto MESSAGE_MAIN_CLEARALLACTIONS          = "message_main_clearAllActions";
	static constexpr auto MESSAGE_MAIN_CLEARALLVOLUMEACTIONS    = "message_main_clearAllVolumeActions";
	static constexpr auto MESSAGE_MAIN_CLEARALLSTARTSTOPACTIONS = "message_main_clearAllStartStopActions";
	static constexpr auto MESSAGE_MAIN_CLOSEPROJECT             = "message_main_closeProject";

	static constexpr auto MESSAGE_INIT_WRONGSYSTEM = "message_init_wrongSystem";
	static constexpr auto MESSAGE_INIT_QUITGIADA   = "message_init_quitGiada";

	static constexpr auto MESSAGE_CHANNEL_MULTICHANNOTSUPPORTED = "message_channel_multiChanNotSupported";
	static constexpr auto MESSAGE_CHANNEL_CANTREADSAMPLE        = "message_channel_cantReadSample";
	static constexpr auto MESSAGE_CHANNEL_PATHTOOLONG           = "message_channel_pathTooLong";
	static constexpr auto MESSAGE_CHANNEL_NOFILESPECIFIED       = "message_channel_noFileSpecified";
	static constexpr auto MESSAGE_CHANNEL_LOADINGSAMPLES        = "message_channel_loadingSamples";
	static constexpr auto MESSAGE_CHANNEL_LOADINGSAMPLESERROR   = "message_channel_loadingSamplesError";
	static constexpr auto MESSAGE_CHANNEL_DELETE                = "message_channel_delete";
	static constexpr auto MESSAGE_CHANNEL_FREE                  = "message_channel_free";

	static constexpr auto MESSAGE_STORAGE_PATCHUNREADABLE     = "message_storage_patchUnreadable";
	static constexpr auto MESSAGE_STORAGE_PATCHINVALID        = "message_storage_patchInvalid";
	static constexpr auto MESSAGE_STORAGE_PATCHUNSUPPORTED    = "message_storage_patchUnsupported";
	static constexpr auto MESSAGE_STORAGE_PROJECTEXISTS       = "message_storage_projectExists";
	static constexpr auto MESSAGE_STORAGE_LOADINGPROJECT      = "message_storage_loadingProject";
	static constexpr auto MESSAGE_STORAGE_LOADINGSAMPLE       = "message_storage_loadingSample";
	static constexpr auto MESSAGE_STORAGE_SAVINGPROJECT       = "message_storage_savingProject";
	static constexpr auto MESSAGE_STORAGE_SAVINGPROJECTERROR  = "message_storage_savingProjectError";
	static constexpr auto MESSAGE_STORAGE_CHOOSEFILENAME      = "message_storage_chooseFileName";
	static constexpr auto MESSAGE_STORAGE_FILEHASINVALIDCHARS = "message_storage_fileHasInvalidChars";
	static constexpr auto MESSAGE_STORAGE_FILEEXISTS          = "message_storage_fileExists";
	static constexpr auto MESSAGE_STORAGE_SAVINGFILEERROR     = "message_storage_savingFileError";

	static constexpr auto MAIN_MENU_FILE                 = "main_menu_file";
	static constexpr auto MAIN_MENU_FILE_OPENPROJECT     = "main_menu_file_openProject";
	static constexpr auto MAIN_MENU_FILE_SAVEPROJECT     = "main_menu_file_saveProject";
	static constexpr auto MAIN_MENU_FILE_CLOSEPROJECT    = "main_menu_file_closeProject";
	static constexpr auto MAIN_MENU_FILE_DEBUGSTATS      = "main_menu_file_debugStats";
	static constexpr auto MAIN_MENU_FILE_QUIT            = "main_menu_file_quit";
	static constexpr auto MAIN_MENU_EDIT                 = "main_menu_edit";
	static constexpr auto MAIN_MENU_EDIT_FREEALLSAMPLES  = "main_menu_edit_freeAllSamples";
	static constexpr auto MAIN_MENU_EDIT_CLEARALLACTIONS = "main_menu_edit_clearAllActions";
	static constexpr auto MAIN_MENU_EDIT_SETUPMIDIINPUT  = "main_menu_edit_setupMidiInput";
	static constexpr auto MAIN_MENU_CONFIG               = "main_menu_config";
	static constexpr auto MAIN_MENU_ABOUT                = "main_menu_about";

	static constexpr auto MAIN_IO_LABEL_OUTMETER        = "main_IO_label_outMeter";
	static constexpr auto MAIN_IO_LABEL_INMETER         = "main_IO_label_inMeter";
	static constexpr auto MAIN_IO_LABEL_OUTVOL          = "main_IO_label_outVol";
	static constexpr auto MAIN_IO_LABEL_INVOL           = "main_IO_label_inVol";
	static constexpr auto MAIN_IO_LABEL_INTOOUT         = "main_IO_label_inToOut";
	static constexpr auto MAIN_IO_LABEL_FXOUT           = "main_IO_label_fxOut";
	static constexpr auto MAIN_IO_LABEL_FXIN            = "main_IO_label_fxIn";
	static constexpr auto MAIN_IO_LABEL_MIDIACTIVITY    = "main_IO_label_midiActivity";
	static constexpr auto MAIN_IO_LABEL_MIDIINACTIVITY  = "main_IO_label_midiInActivity";
	static constexpr auto MAIN_IO_LABEL_MIDIOUTACTIVITY = "main_IO_label_midiOutActivity";

	static constexpr auto MAIN_TIMER_LABEL_BPM        = "main_mainTimer_label_bpm";
	static constexpr auto MAIN_TIMER_LABEL_METER      = "main_mainTimer_label_meter";
	static constexpr auto MAIN_TIMER_LABEL_QUANTIZER  = "main_mainTimer_label_quantizer";
	static constexpr auto MAIN_TIMER_LABEL_MULTIPLIER = "main_mainTimer_label_multiplier";
	static constexpr auto MAIN_TIMER_LABEL_DIVIDER    = "main_mainTimer_label_divider";

	static constexpr auto MAIN_SEQUENCER_LABEL = "main_sequencer_label";

	static constexpr auto MAIN_TRANSPORT_LABEL_REWIND         = "main_transport_label_rewind";
	static constexpr auto MAIN_TRANSPORT_LABEL_PLAY           = "main_transport_label_play";
	static constexpr auto MAIN_TRANSPORT_LABEL_RECTRIGGERMODE = "main_transport_label_recTriggerMode";
	static constexpr auto MAIN_TRANSPORT_LABEL_RECACTIONS     = "main_transport_label_recActions";
	static constexpr auto MAIN_TRANSPORT_LABEL_RECINPUT       = "main_transport_label_recInput";
	static constexpr auto MAIN_TRANSPORT_LABEL_RECINPUTMODE   = "main_transport_label_recInputMode";
	static constexpr auto MAIN_TRANSPORT_LABEL_METRONOME      = "main_transport_label_metronome";

	static constexpr auto MAIN_TRACK_HELP = "main_track_help";

	static constexpr auto MAIN_TRACK_BUTTON_ADDSAMPLECHANNEL = "main_track_button_addSampleChannel";
	static constexpr auto MAIN_TRACK_BUTTON_ADDMIDICHANNEL   = "main_track_button_addMidiChannel";
	static constexpr auto MAIN_TRACK_BUTTON_REMOVE_TRACK     = "main_track_button_removeTrack";
	static constexpr auto MAIN_TRACK_BUTTON_ADD_TRACK        = "main_track_button_addTrack";

	static constexpr auto MAIN_CHANNEL_NOSAMPLE                  = "main_channel_noSample";
	static constexpr auto MAIN_CHANNEL_DEFAULTGROUPNAME          = "main_channel_defaultGroupName";
	static constexpr auto MAIN_CHANNEL_SAMPLENOTFOUND            = "main_channel_sampleNotFound";
	static constexpr auto MAIN_CHANNEL_LABEL_PLAY                = "main_channel_label_play";
	static constexpr auto MAIN_CHANNEL_LABEL_ARM                 = "main_channel_label_arm";
	static constexpr auto MAIN_CHANNEL_LABEL_STATUS              = "main_channel_label_status";
	static constexpr auto MAIN_CHANNEL_LABEL_READACTIONS         = "main_channel_label_readActions";
	static constexpr auto MAIN_CHANNEL_LABEL_MODEBOX             = "main_channel_label_modeBox";
	static constexpr auto MAIN_CHANNEL_LABEL_MUTE                = "main_channel_label_mute";
	static constexpr auto MAIN_CHANNEL_LABEL_SOLO                = "main_channel_label_solo";
	static constexpr auto MAIN_CHANNEL_LABEL_FX                  = "main_channel_label_fx";
	static constexpr auto MAIN_CHANNEL_LABEL_VOLUME              = "main_channel_label_volume";
	static constexpr auto MAIN_CHANNEL_LABEL_MIDIACTIVITY        = "main_channel_label_midiActivity";
	static constexpr auto MAIN_CHANNEL_LABEL_MIDIACTIVITY_INONLY = "main_channel_label_midiActivity_inOnly";

	static constexpr auto MAIN_CHANNEL_MENU_INPUTMONITOR           = "main_channel_menu_inputMonitor";
	static constexpr auto MAIN_CHANNEL_MENU_OVERDUBPROTECTION      = "main_channel_menu_overdubProtection";
	static constexpr auto MAIN_CHANNEL_MENU_LOADSAMPLE             = "main_channel_menu_loadSample";
	static constexpr auto MAIN_CHANNEL_MENU_EXPORTSAMPLE           = "main_channel_menu_exportSample";
	static constexpr auto MAIN_CHANNEL_MENU_KEYBOARDINPUT          = "main_channel_menu_keyboardInput";
	static constexpr auto MAIN_CHANNEL_MENU_MIDIINPUT              = "main_channel_menu_midiInput";
	static constexpr auto MAIN_CHANNEL_MENU_MIDIOUTPUT             = "main_channel_menu_midiOutput";
	static constexpr auto MAIN_CHANNEL_MENU_EDITROUTING            = "main_channel_menu_editRouting";
	static constexpr auto MAIN_CHANNEL_MENU_EDITSAMPLE             = "main_channel_menu_editSample";
	static constexpr auto MAIN_CHANNEL_MENU_EDITACTIONS            = "main_channel_menu_editActions";
	static constexpr auto MAIN_CHANNEL_MENU_CLEARACTIONS           = "main_channel_menu_clearActions";
	static constexpr auto MAIN_CHANNEL_MENU_CLEARACTIONS_ALL       = "main_channel_menu_clearActions_all";
	static constexpr auto MAIN_CHANNEL_MENU_CLEARACTIONS_VOLUME    = "main_channel_menu_clearActions_volume";
	static constexpr auto MAIN_CHANNEL_MENU_CLEARACTIONS_STARTSTOP = "main_channel_menu_clearActions_startStop";
	static constexpr auto MAIN_CHANNEL_MENU_RENAME                 = "main_channel_menu_rename";
	static constexpr auto MAIN_CHANNEL_MENU_CLONE                  = "main_channel_menu_clone";
	static constexpr auto MAIN_CHANNEL_MENU_FREE                   = "main_channel_menu_free";
	static constexpr auto MAIN_CHANNEL_MENU_DELETE                 = "main_channel_menu_delete";

	static constexpr auto MISSINGASSETS_INTRO      = "missingAssets_intro";
	static constexpr auto MISSINGASSETS_AUDIOFILES = "missingAssets_audioFiles";
	static constexpr auto MISSINGASSETS_PLUGINS    = "missingAssets_plugIns";

	static constexpr auto PLUGINCHOOSER_TITLE               = "pluginChooser_title";
	static constexpr auto PLUGINCHOOSER_HEADER              = "pluginChooser_header";
	static constexpr auto PLUGINCHOOSER_SORTBY              = "pluginChooser_sortBy";
	static constexpr auto PLUGINCHOOSER_SORTBY_NAME         = "pluginChooser_sortBy_name";
	static constexpr auto PLUGINCHOOSER_SORTBY_CATEGORY     = "pluginChooser_sortBy_category";
	static constexpr auto PLUGINCHOOSER_SORTBY_MANUFACTURER = "pluginChooser_sortBy_manufacturer";
	static constexpr auto PLUGINCHOOSER_SORTBY_FORMAT       = "pluginChooser_sortBy_format";

	static constexpr auto PLUGINLIST_TITLE_MASTEROUT = "pluginList_title_masterOut";
	static constexpr auto PLUGINLIST_TITLE_MASTERIN  = "pluginList_title_masterIn";
	static constexpr auto PLUGINLIST_TITLE_CHANNEL   = "pluginList_title_channel";
	static constexpr auto PLUGINLIST_ADDPLUGIN       = "pluginList_addPlugin";
	static constexpr auto PLUGINLIST_NOPROGRAMS      = "pluginList_noPrograms";

	static constexpr auto CHANNELNAME_TITLE = "channelName_title";

	static constexpr auto KEYGRABBER_TITLE = "keyGrabber_title";
	static constexpr auto KEYGRABBER_BODY  = "keyGrabber_body";

	static constexpr auto SAMPLEEDITOR_TITLE                = "sampleEditor_title";
	static constexpr auto SAMPLEEDITOR_RELOAD               = "sampleEditor_reload";
	static constexpr auto SAMPLEEDITOR_LOOP                 = "sampleEditor_loop";
	static constexpr auto SAMPLEEDITOR_INFO                 = "sampleEditor_info";
	static constexpr auto SAMPLEEDITOR_PAN                  = "sampleEditor_pan";
	static constexpr auto SAMPLEEDITOR_PITCH                = "sampleEditor_pitch";
	static constexpr auto SAMPLEEDITOR_PITCH_TOBAR          = "sampleEditor_pitch_toBar";
	static constexpr auto SAMPLEEDITOR_PITCH_TOSONG         = "sampleEditor_pitch_toSong";
	static constexpr auto SAMPLEEDITOR_RANGE                = "sampleEditor_range";
	static constexpr auto SAMPLEEDITOR_SHIFT                = "sampleEditor_shift";
	static constexpr auto SAMPLEEDITOR_VOLUME               = "sampleEditor_volume";
	static constexpr auto SAMPLEEDITOR_TOOLS_CUT            = "sampleEditor_tools_cut";
	static constexpr auto SAMPLEEDITOR_TOOLS_COPY           = "sampleEditor_tools_copy";
	static constexpr auto SAMPLEEDITOR_TOOLS_PASTE          = "sampleEditor_tools_paste";
	static constexpr auto SAMPLEEDITOR_TOOLS_TRIM           = "sampleEditor_tools_trim";
	static constexpr auto SAMPLEEDITOR_TOOLS_SILENCE        = "sampleEditor_tools_silence";
	static constexpr auto SAMPLEEDITOR_TOOLS_REVERSE        = "sampleEditor_tools_reverse";
	static constexpr auto SAMPLEEDITOR_TOOLS_NORMALIZE      = "sampleEditor_tools_normalize";
	static constexpr auto SAMPLEEDITOR_TOOLS_FADE_IN        = "sampleEditor_tools_fadeIn";
	static constexpr auto SAMPLEEDITOR_TOOLS_FADE_OUT       = "sampleEditor_tools_fadeOut";
	static constexpr auto SAMPLEEDITOR_TOOLS_SMOOTH_EDGES   = "sampleEditor_tools_smoothEdgdes";
	static constexpr auto SAMPLEEDITOR_TOOLS_SET_BEGIN_END  = "sampleEditor_tools_setBeginEnd";
	static constexpr auto SAMPLEEDITOR_TOOLS_TO_NEW_CHANNEL = "sampleEditor_tools_toNewChannel";

	static constexpr auto ACTIONEDITOR_TITLE            = "actionEditor_title";
	static constexpr auto ACTIONEDITOR_VOLUME           = "actionEditor_volume";
	static constexpr auto ACTIONEDITOR_KEYPRESS         = "actionEditor_keyPress";
	static constexpr auto ACTIONEDITOR_KEYRELEASE       = "actionEditor_keyRelease";
	static constexpr auto ACTIONEDITOR_STOPSAMPLE       = "actionEditor_stopSample";
	static constexpr auto ACTIONEDITOR_STARTSTOP        = "actionEditor_startStop";
	static constexpr auto ACTIONEDITOR_VELOCITY         = "actionEditor_velocity";
	static constexpr auto ACTIONEDITOR_LABEL_ACTIONTYPE = "actionEditor_label_actionType";

	static constexpr auto BROWSER_SHOWHIDDENFILES = "browser_showHiddenFiles";
	static constexpr auto BROWSER_OPENPROJECT     = "browser_openProject";
	static constexpr auto BROWSER_SAVEPROJECT     = "browser_saveProject";
	static constexpr auto BROWSER_OPENSAMPLE      = "browser_openSample";
	static constexpr auto BROWSER_SAVESAMPLE      = "browser_saveSample";
	static constexpr auto BROWSER_OPENPLUGINSDIR  = "browser_openPluginsDir";

	static constexpr auto MIDIINPUT_MASTER_TITLE           = "midiInput_master_title";
	static constexpr auto MIDIINPUT_MASTER_ENABLE          = "midiInput_master_enable";
	static constexpr auto MIDIINPUT_MASTER_LEARN_REWIND    = "midiInput_master_learn_rewind";
	static constexpr auto MIDIINPUT_MASTER_LEARN_PLAYSTOP  = "midiInput_master_learn_playStop";
	static constexpr auto MIDIINPUT_MASTER_LEARN_ACTIONREC = "midiInput_master_learn_actionRec";
	static constexpr auto MIDIINPUT_MASTER_LEARN_INPUTREC  = "midiInput_master_learn_inputRec";
	static constexpr auto MIDIINPUT_MASTER_LEARN_METRONOME = "midiInput_master_learn_metronome";
	static constexpr auto MIDIINPUT_MASTER_LEARN_INVOLUME  = "midiInput_master_learn_inVolume";
	static constexpr auto MIDIINPUT_MASTER_LEARN_OUTVOLUME = "midiInput_master_learn_outVolume";
	static constexpr auto MIDIINPUT_MASTER_LEARN_SEQDOUBLE = "midiInput_master_learn_seqDouble";
	static constexpr auto MIDIINPUT_MASTER_LEARN_SEQHALF   = "midiInput_master_learn_seqHalf";

	static constexpr auto MIDIINPUT_CHANNEL_TITLE             = "midiInput_channel_title";
	static constexpr auto MIDIINPUT_CHANNEL_ENABLE            = "midiInput_channel_enable";
	static constexpr auto MIDIINPUT_CHANNEL_VELOCITYDRIVESVOL = "midiInput_channel_learn_velocityDrivesVol";
	static constexpr auto MIDIINPUT_CHANNEL_LEARN_CHANNEL     = "midiInput_channel_learn_channel";
	static constexpr auto MIDIINPUT_CHANNEL_LEARN_KEYPRESS    = "midiInput_channel_learn_keyPress";
	static constexpr auto MIDIINPUT_CHANNEL_LEARN_KEYREL      = "midiInput_channel_learn_keyRel";
	static constexpr auto MIDIINPUT_CHANNEL_LEARN_KEYKILL     = "midiInput_channel_learn_keyKill";
	static constexpr auto MIDIINPUT_CHANNEL_LEARN_ARM         = "midiInput_channel_learn_arm";
	static constexpr auto MIDIINPUT_CHANNEL_LEARN_MUTE        = "midiInput_channel_learn_mute";
	static constexpr auto MIDIINPUT_CHANNEL_LEARN_SOLO        = "midiInput_channel_learn_solo";
	static constexpr auto MIDIINPUT_CHANNEL_LEARN_VOLUME      = "midiInput_channel_learn_volume";
	static constexpr auto MIDIINPUT_CHANNEL_LEARN_PITCH       = "midiInput_channel_learn_pitch";
	static constexpr auto MIDIINPUT_CHANNEL_LEARN_READACTIONS = "midiInput_channel_learn_readActions";

	static constexpr auto MIDIOUTPUT_CHANNEL_TITLE            = "midiOutput_channel_title";
	static constexpr auto MIDIOUTPUT_CHANNEL_ENABLE           = "midiOutput_channel_enable";
	static constexpr auto MIDIOUTPUT_CHANNEL_ENABLE_LIGHTNING = "midiOutput_channel_enableLightning";
	static constexpr auto MIDIOUTPUT_CHANNEL_LEARN_PLAYING    = "midiOutput_channel_learn_playing";
	static constexpr auto MIDIOUTPUT_CHANNEL_LEARN_MUTE       = "midiOutput_channel_learn_mute";
	static constexpr auto MIDIOUTPUT_CHANNEL_LEARN_SOLO       = "midiOutput_channel_learn_solo";

	static constexpr auto ABOUT_TITLE    = "about_title";
	static constexpr auto ABOUT_BODY     = "about_body";
	static constexpr auto ABOUT_BODY_VST = "about_body_vst";

	static constexpr auto CONFIG_TITLE        = "config_title";
	static constexpr auto CONFIG_RESTARTGIADA = "config_restartGiada";

	static constexpr auto CONFIG_AUDIO_TITLE                 = "config_audio_title";
	static constexpr auto CONFIG_AUDIO_SYSTEM                = "config_audio_system";
	static constexpr auto CONFIG_AUDIO_BUFFERSIZE            = "config_audio_bufferSize";
	static constexpr auto CONFIG_AUDIO_SAMPLERATE            = "config_audio_sampleRate";
	static constexpr auto CONFIG_AUDIO_OUTPUTDEVICE          = "config_audio_outputDevice";
	static constexpr auto CONFIG_AUDIO_OUTPUTCHANNELS        = "config_audio_outputChannels";
	static constexpr auto CONFIG_AUDIO_LIMITOUTPUT           = "config_audio_limitOutput";
	static constexpr auto CONFIG_AUDIO_INPUTDEVICE           = "config_audio_inputDevice";
	static constexpr auto CONFIG_AUDIO_INPUTCHANNELS         = "config_audio_inputChannels";
	static constexpr auto CONFIG_AUDIO_RECTHRESHOLD          = "config_audio_recThreshold";
	static constexpr auto CONFIG_AUDIO_ENABLEINPUT           = "config_audio_enableInput";
	static constexpr auto CONFIG_AUDIO_RESAMPLING            = "config_audio_reseampling";
	static constexpr auto CONFIG_AUDIO_RESAMPLING_SINCBEST   = "config_audio_reseampling_sincBest";
	static constexpr auto CONFIG_AUDIO_RESAMPLING_SINCMEDIUM = "config_audio_reseampling_sincMedium";
	static constexpr auto CONFIG_AUDIO_RESAMPLING_SINCBASIC  = "config_audio_reseampling_sincBasic";
	static constexpr auto CONFIG_AUDIO_RESAMPLING_ZEROORDER  = "config_audio_reseampling_zeroOrder";
	static constexpr auto CONFIG_AUDIO_RESAMPLING_LINEAR     = "config_audio_reseampling_linear";
	static constexpr auto CONFIG_AUDIO_NODEVICESFOUND        = "config_audio_noDevicesFound";

	static constexpr auto CONFIG_MIDI_TITLE           = "config_midi_title";
	static constexpr auto CONFIG_MIDI_SYSTEM          = "config_midi_system";
	static constexpr auto CONFIG_MIDI_OUTPUTPORT      = "config_midi_outputPort";
	static constexpr auto CONFIG_MIDI_INPUTPORT       = "config_midi_inputPort";
	static constexpr auto CONFIG_MIDI_NOPORTSFOUND    = "config_midi_noPortsFound";
	static constexpr auto CONFIG_MIDI_OUTPUTMIDIMAP   = "config_midi_outputMidiMap";
	static constexpr auto CONFIG_MIDI_NOMIDIMAPSFOUND = "config_midi_noMidiMapsFound";
	static constexpr auto CONFIG_MIDI_SYNC            = "config_midi_sync";
	static constexpr auto CONFIG_MIDI_LABEL_ENABLEOUT = "config_midi_label_enableOut";
	static constexpr auto CONFIG_MIDI_LABEL_ENABLEIN  = "config_midi_label_enableIn";
	static constexpr auto CONFIG_MIDI_LABEL_WRONGMIDI = "config_midi_label_wrongMidi";

	static constexpr auto CONFIG_BEHAVIORS_TITLE                      = "config_behaviors_title";
	static constexpr auto CONFIG_BEHAVIORS_CHANSSTOPONSEQHALT         = "config_behaviors_chansStopOnSeqHalt";
	static constexpr auto CONFIG_BEHAVIORS_TREATRECSASLOOPS           = "config_behaviors_treatRecsAsLoops";
	static constexpr auto CONFIG_BEHAVIORS_INPUTMONITORDEFAULTON      = "config_behaviors_inputMonitorDefaultOn";
	static constexpr auto CONFIG_BEHAVIORS_OVERDUBPROTECTIONDEFAULTON = "config_behaviors_overdubProtectionDefaultOn";

	static constexpr auto CONFIG_BINDINGS_TITLE         = "config_bindings_title";
	static constexpr auto CONFIG_BINDINGS_PLAY          = "config_bindings_play";
	static constexpr auto CONFIG_BINDINGS_REWIND        = "config_bindings_rewind";
	static constexpr auto CONFIG_BINDINGS_RECORDACTIONS = "config_bindings_recordActions";
	static constexpr auto CONFIG_BINDINGS_RECORDAUDIO   = "config_bindings_recordAudio";
	static constexpr auto CONFIG_BINDINGS_EXIT          = "config_bindings_exit";

	static constexpr auto CONFIG_MISC_TITLE                  = "config_misc_title";
	static constexpr auto CONFIG_MISC_DEBUGMESSAGES          = "config_misc_debugMessages";
	static constexpr auto CONFIG_MISC_DEBUGMESSAGES_DISABLED = "config_misc_debugMessages_disabled";
	static constexpr auto CONFIG_MISC_DEBUGMESSAGES_TOSTDOUT = "config_misc_debugMessages_toStdOut";
	static constexpr auto CONFIG_MISC_DEBUGMESSAGES_TOFILE   = "config_misc_debugMessages_toFile";
	static constexpr auto CONFIG_MISC_TOOLTIPS               = "config_misc_tooltips";
	static constexpr auto CONFIG_MISC_TOOLTIPS_DISABLED      = "config_misc_tooltips_disabled";
	static constexpr auto CONFIG_MISC_TOOLTIPS_ENABLED       = "config_misc_tooltips_enabled";
	static constexpr auto CONFIG_MISC_LANGUAGE               = "config_misc_language";
	static constexpr auto CONFIG_MISC_NOLANGUAGESFOUND       = "config_misc_noLanguagesFound";
	static constexpr auto CONFIG_MISC_UISCALING              = "config_misc_uiScaling";

	static constexpr auto CONFIG_PLUGINS_TITLE       = "config_plugins_title";
	static constexpr auto CONFIG_PLUGINS_FOLDER      = "config_plugins_folder";
	static constexpr auto CONFIG_PLUGINS_SCANNING    = "config_plugins_scanning";
	static constexpr auto CONFIG_PLUGINS_SCAN        = "config_plugins_scan";
	static constexpr auto CONFIG_PLUGINS_INVALIDPATH = "config_plugins_invalidPath";

	static constexpr auto CHANNELROUTING_TITLE = "channelRouting_title";

	LangMap();

	const char* get(const std::string&) const;

private:
	Data        m_data;
	std::string m_default;
};

/* -------------------------------------------------------------------------- */

class LangMapper final : public Mapper
{
public:
	LangMapper();

	/* get
	Gets a key from the currently loaded langmap file. */

	const char* get(const std::string&) const;

	/* init
	Parses the langmap folders and find the available langmaps. */

	void init();

	/* read
	Reads a langmap from file 'file' and sets it as the current one. */

	int read(const std::string& file);

private:
	/* maps
	The current langmap selected and loaded. It might be the default one (En_US)
	if no langmaps have been found. */

	LangMap m_map;
};
} // namespace giada::v

#endif
