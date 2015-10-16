/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * patch
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2015 Giovanni A. Zuliani | Monocasual
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


#include <stdint.h>
#include "../utils/log.h"
#include "../utils/utils.h"
#include "../gui/dialogs/gd_mainWindow.h"
#include "../gui/elems/ge_keyboard.h"
#include "patch.h"
#include "const.h"
#include "init.h"
#include "recorder.h"
#include "conf.h"
#include "pluginHost.h"
#include "wave.h"
#include "mixer.h"
#include "channel.h"


extern Mixer 		     G_Mixer;
extern Conf 		     G_Conf;
#ifdef WITH_VST
extern PluginHost    G_PluginHost;
#endif
extern gdMainWindow *mainWin;


int Patch::write(const char *file)
{
  jRoot = json_object();

  writeCommons(jRoot);
  writeColumns(jRoot, &columns);
  writeChannels(jRoot, &channels);
#ifdef WITH_VST
  writePlugins(jRoot, &masterInPlugins, PATCH_KEY_MASTER_IN_PLUGINS);
  writePlugins(jRoot, &masterOutPlugins, PATCH_KEY_MASTER_OUT_PLUGINS);
#endif

  if (json_dump_file(jRoot, file, JSON_INDENT(2)) != 0) {
    gLog("[Patch::write] unable to write patch file!\n");
    return 0;
  }
  return 1;
}


/* -------------------------------------------------------------------------- */


int Patch::read(const char *file)
{
  jRoot = json_load_file(file, 0, &jError);
  if (!jRoot) {
    gLog("[Patch::read] unable to read patch file! Error on line %d: %s\n", jError.line, jError.text);
    return 0;
  }
  if (!checkObject(jRoot, "root element"))
    return 0;

  readCommons(jRoot);
  readColumns(jRoot);

  json_decref(jRoot);
  return 1;
}

/* -------------------------------------------------------------------------- */

#ifdef WITH_VST

void Patch::writePlugins(json_t *jContainer, gVector<plugin_t> *plugins, const char *key)
{
  json_t *jPlugins = json_array();
  for (unsigned j=0; j<plugins->size; j++) {
    json_t   *jPlugin = json_object();
    plugin_t  plugin  = plugins->at(j);
    json_object_set_new(jPlugin, PATCH_KEY_PLUGIN_PATH,     json_string(plugin.path.c_str()));
    json_object_set_new(jPlugin, PATCH_KEY_PLUGIN_BYPASS,   json_boolean(plugin.bypass));
    json_array_append_new(jPlugins, jPlugin);

    /* plugin params */

    json_t *jPluginParams = json_array();
    for (unsigned z=0; z<plugin.params.size; z++) {
      json_array_append_new(jPluginParams, json_real(plugin.params.at(z)));
    }
    json_object_set_new(jPlugin, PATCH_KEY_PLUGIN_PARAMS, jPluginParams);
  }
  json_object_set_new(jContainer, key, jPlugins);
}

#endif


/* -------------------------------------------------------------------------- */


void Patch::writeColumns(json_t *jContainer, gVector<column_t> *columns)
{
  json_t *jColumns = json_array();
  for (unsigned i=0; i<columns->size; i++) {
    json_t   *jColumn = json_object();
    column_t  column  = columns->at(i);
    json_object_set_new(jColumn, PATCH_KEY_COLUMN_INDEX, json_integer(column.index));
    json_object_set_new(jColumn, PATCH_KEY_COLUMN_WIDTH, json_integer(column.width));

    /* channels' pointers */

    json_t *jChannels = json_array();
    for (unsigned k=0; k<column.channels.size; k++) {
      json_array_append_new(jChannels, json_integer(column.channels.at(k)));
    }
    json_object_set_new(jColumn, PATCH_KEY_COLUMN_CHANNELS, jChannels);
    json_array_append_new(jColumns, jColumn);
  }
  json_object_set_new(jContainer, PATCH_KEY_COLUMNS, jColumns);
}


/* -------------------------------------------------------------------------- */


void Patch::writeActions(json_t *jContainer, gVector<action_t> *actions)
{
  json_t *jActions = json_array();
  for (unsigned k=0; k<actions->size; k++) {
    json_t   *jAction = json_object();
    action_t  action  = actions->at(k);
    json_object_set_new(jAction, PATCH_KEY_ACTION_TYPE,    json_integer(action.type));
    json_object_set_new(jAction, PATCH_KEY_ACTION_FRAME,   json_integer(action.frame));
    json_object_set_new(jAction, PATCH_KEY_ACTION_F_VALUE, json_real(action.fValue));
    json_object_set_new(jAction, PATCH_KEY_ACTION_I_VALUE, json_integer(action.iValue));
    json_array_append_new(jActions, jAction);
  }
  json_object_set_new(jContainer, PATCH_KEY_CHANNEL_ACTIONS, jActions);
}


/* -------------------------------------------------------------------------- */


void Patch::writeCommons(json_t *jContainer)
{
  json_object_set_new(jContainer, PATCH_KEY_HEADER,         json_string(header.c_str()));
  json_object_set_new(jContainer, PATCH_KEY_VERSION,        json_string(version.c_str()));
  json_object_set_new(jContainer, PATCH_KEY_VERSION_FLOAT,  json_real(versionFloat));
  json_object_set_new(jContainer, PATCH_KEY_NAME,           json_string(name.c_str()));
  json_object_set_new(jContainer, PATCH_KEY_BPM,            json_integer(bpm));
  json_object_set_new(jContainer, PATCH_KEY_BARS,           json_integer(bars));
  json_object_set_new(jContainer, PATCH_KEY_BEATS,          json_integer(beats));
  json_object_set_new(jContainer, PATCH_KEY_QUANTIZE,       json_integer(quantize));
  json_object_set_new(jContainer, PATCH_KEY_MASTER_VOL_IN,  json_real(masterVolIn));
  json_object_set_new(jContainer, PATCH_KEY_MASTER_VOL_OUT, json_real(masterVolOut));
	json_object_set_new(jContainer, PATCH_KEY_METRONOME,      json_integer(metronome));
	json_object_set_new(jContainer, PATCH_KEY_LAST_TAKE_ID,   json_integer(lastTakeId));
	json_object_set_new(jContainer, PATCH_KEY_SAMPLERATE,     json_integer(samplerate));
}


/* -------------------------------------------------------------------------- */


void Patch::writeChannels(json_t *jContainer, gVector<channel_t> *channels)
{
  json_t *jChannels = json_array();
  for (unsigned i=0; i<channels->size; i++) {
    json_t    *jChannel = json_object();
    channel_t  channel  = channels->at(i);
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_TYPE,                 json_integer(channel.type));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_INDEX,                json_integer(channel.index));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_COLUMN,               json_integer(channel.column));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_MUTE,                 json_integer(channel.mute));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_MUTE_S,               json_integer(channel.mute_s));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_SOLO,                 json_integer(channel.solo));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_VOLUME,               json_real(channel.volume));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_PAN_LEFT,             json_real(channel.panLeft));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_PAN_RIGHT,            json_real(channel.panRight));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_MIDI_IN,              json_boolean(channel.midiIn));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_MIDI_IN_KEYPRESS,     json_integer(channel.midiInKeyPress));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_MIDI_IN_KEYREL,       json_integer(channel.midiInKeyRel));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_MIDI_IN_KILL,         json_integer(channel.midiInKill));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_MIDI_IN_VOLUME,       json_integer(channel.midiInVolume));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_MIDI_IN_MUTE,         json_integer(channel.midiInMute));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_MIDI_IN_SOLO,         json_integer(channel.midiInSolo));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_MIDI_OUT_L,           json_boolean(channel.midiOutL));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_MIDI_OUT_L_PLAYING,   json_integer(channel.midiOutLplaying));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_MIDI_OUT_L_MUTE,      json_integer(channel.midiOutLmute));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_MIDI_OUT_L_SOLO,      json_integer(channel.midiOutLsolo));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_SAMPLE_PATH,          json_string(channel.samplePath.c_str()));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_KEY,                  json_integer(channel.key));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_MODE,                 json_integer(channel.mode));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_BEGIN,                json_integer(channel.begin));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_END,                  json_integer(channel.end));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_BOOST,                json_real(channel.boost));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_REC_ACTIVE,           json_integer(channel.recActive));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_PITCH,                json_real(channel.pitch));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_MIDI_IN_READ_ACTIONS, json_integer(channel.midiInReadActions));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_MIDI_IN_PITCH,        json_integer(channel.midiInPitch));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_MIDI_OUT,             json_integer(channel.midiOut));
    json_object_set_new(jChannel, PATCH_KEY_CHANNEL_MIDI_OUT_CHAN,        json_integer(channel.midiOutChan));
    json_array_append_new(jChannels, jChannel);

    writeActions(jChannel, &channel.actions);

#ifdef WITH_VST

    writePlugins(jChannel, &channel.plugins, PATCH_KEY_CHANNEL_PLUGINS);

#endif
  }
  json_object_set_new(jContainer, PATCH_KEY_CHANNELS, jChannels);
}


/* -------------------------------------------------------------------------- */


bool Patch::readCommons(json_t *jContainer)
{
  if (!setString(jContainer, PATCH_KEY_HEADER, header))  return 0;
  if (!setString(jContainer, PATCH_KEY_VERSION, version)) return 0;
  if (!setFloat (jContainer, PATCH_KEY_VERSION_FLOAT, versionFloat)) return 0;
  if (!setString(jContainer, PATCH_KEY_NAME, name)) return 0;
  if (!setInt   (jContainer, PATCH_KEY_BPM, bpm)) return 0;
  if (!setInt   (jContainer, PATCH_KEY_BARS, bars)) return 0;
  if (!setInt   (jContainer, PATCH_KEY_BEATS, beats)) return 0;
  if (!setInt   (jContainer, PATCH_KEY_QUANTIZE, quantize)) return 0;
  if (!setFloat (jContainer, PATCH_KEY_MASTER_VOL_IN, masterVolIn)) return 0;
  if (!setInt   (jContainer, PATCH_KEY_METRONOME, metronome)) return 0;
  if (!setInt   (jContainer, PATCH_KEY_LAST_TAKE_ID, lastTakeId)) return 0;
  if (!setInt   (jContainer, PATCH_KEY_SAMPLERATE, samplerate)) return 0;
  return 1;
}


/* -------------------------------------------------------------------------- */


bool Patch::readColumns(json_t *jContainer)
{
  json_t *jColumns = json_object_get(jContainer, PATCH_KEY_COLUMNS);
  if (!checkArray(jColumns, PATCH_KEY_COLUMNS))
    return 0;

  size_t columnIndex;
  json_t *jColumn;
  json_array_foreach(jColumns, columnIndex, jColumn) {

    if (!checkObject(jColumn, ""))
      return 0;

    column_t column;
    if (!setInt(jColumn, PATCH_KEY_COLUMN_INDEX, column.index)) return 0;
    if (!setInt(jColumn, PATCH_KEY_COLUMN_WIDTH, column.width)) return 0;

    /* read columns channels */

    json_t *jChannels = json_object_get(jColumn, PATCH_KEY_COLUMN_CHANNELS);
    if (!checkArray(jChannels, PATCH_KEY_COLUMN_CHANNELS)) return 0;

    size_t channelIndex;
    json_t *jChannel;
    json_array_foreach(jChannels, columnIndex, jChannel)
      column.channels.add(json_integer_value(jChannel));

    json_decref(jChannels);
    columns.add(column);
    json_decref(jColumn);
  }
  return 1;
}
