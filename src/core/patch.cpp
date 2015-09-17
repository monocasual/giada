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


int Patch::write(const char *file, const char *name, bool isProject)
{
	FILE *fp = fopen(file, "w");
	if (fp == NULL) {
    gLog("[patch::write] unable to read file\n");
		return 0;
  }

  jRoot = json_object();
  
  json_object_set_new(jRoot, "header",         json_string(header));
  json_object_set_new(jRoot, "version",        json_string(version));
  json_object_set_new(jRoot, "version_float",  json_real(versionFloat));
  json_object_set_new(jRoot, "name",           json_string(name));
  json_object_set_new(jRoot, "bpm",            json_integer(bpm));
  json_object_set_new(jRoot, "bars",           json_integer(bars));
  json_object_set_new(jRoot, "beats",          json_integer(beats));
  json_object_set_new(jRoot, "quantize",       json_integer(quantize));
  json_object_set_new(jRoot, "master_vol_in",  json_integer(masterVolIn));
  json_object_set_new(jRoot, "master_vol_out", json_integer(masterVolOut));
	json_object_set_new(jRoot, "metronome",      json_integer(metronome));
	json_object_set_new(jRoot, "last_take_id",   json_integer(lastTakeId));
	json_object_set_new(jRoot, "samplerate",     json_integer(samplerate)); // original samplerate when the patch was saved

  /* columns */
  
  json_t *jColumns = json_array();
  for (unsigned i=0; i<columns.size; i++) {
    json_t   *jColumn = json_object();
    column_t  column  = columns.at(i); 
    json_object_set_new(jColumn, "index", json_integer(column.index));
    json_object_set_new(jColumn, "width", json_integer(column.width));
    
    /* channels' pointers */
    
    json_t *jChannels = json_array();
    for (unsigned k=0; k<columns.at(i).channels.size; k++) {
      json_array_append_new(jChannels, json_integer(column.channels.at(k)));
    }
    json_object_set_new(jColumn, "channels", jChannels);
    json_array_append_new(jColumns, jColumn);
  }
  json_object_set_new(jRoot, "columns", jColumns);
  
  /* channels */
  
  json_t *jChannels = json_array();
  for (unsigned i=0; i<channels.size; i++) {
    json_t    *jChannel = json_object();
    channel_t  channel  = channels.at(i);
    json_object_set_new(jChannel, "type",                 json_integer(channel.type));
    json_object_set_new(jChannel, "index",                json_integer(channel.index));
    json_object_set_new(jChannel, "column",               json_integer(channel.column));
    json_object_set_new(jChannel, "mute",                 json_integer(channel.mute));
    json_object_set_new(jChannel, "mute_s",               json_integer(channel.mute_s));
    json_object_set_new(jChannel, "solo",                 json_integer(channel.solo));
    json_object_set_new(jChannel, "volume",               json_real(channel.volume));
    json_object_set_new(jChannel, "pan_left",             json_real(channel.panLeft));
    json_object_set_new(jChannel, "pan_left",             json_real(channel.panRight));
    json_object_set_new(jChannel, "midi_in",              json_boolean(channel.midiIn));
    json_object_set_new(jChannel, "midi_in_keypress",     json_integer(channel.midiInKeyPress));
    json_object_set_new(jChannel, "midi_in_keyrel",       json_integer(channel.midiInKeyRel));
    json_object_set_new(jChannel, "midi_in_kill",         json_integer(channel.midiInKill));
    json_object_set_new(jChannel, "midi_in_volume",       json_integer(channel.midiInVolume));
    json_object_set_new(jChannel, "midi_in_mute",         json_integer(channel.midiInMute));
    json_object_set_new(jChannel, "midi_in_solo",         json_integer(channel.midiInSolo));
    json_object_set_new(jChannel, "midi_out_l",           json_boolean(channel.midiOutL));
    json_object_set_new(jChannel, "midi_out_l_playing",   json_integer(channel.midiOutLplaying));
    json_object_set_new(jChannel, "midi_out_l_mute",      json_integer(channel.midiOutLmute));
    json_object_set_new(jChannel, "midi_out_l_solo",      json_integer(channel.midiOutLsolo));
    json_object_set_new(jChannel, "sample_path",          json_string(channel.samplePath));
    json_object_set_new(jChannel, "key",                  json_integer(channel.key));
    json_object_set_new(jChannel, "mode",                 json_integer(channel.mode));
    json_object_set_new(jChannel, "begin",                json_integer(channel.begin));
    json_object_set_new(jChannel, "end",                  json_integer(channel.end));
    json_object_set_new(jChannel, "boost",                json_real(channel.boost));
    json_object_set_new(jChannel, "rec_active",           json_integer(channel.recActive));
    json_object_set_new(jChannel, "pitch",                json_real(channel.pitch));
    json_object_set_new(jChannel, "midi_in_read_actions", json_integer(channel.midiInReadActions));
    json_object_set_new(jChannel, "midi_in_pitch",        json_integer(channel.midiInPitch));
    json_object_set_new(jChannel, "midi_out",             json_integer(channel.midiOut));
    json_object_set_new(jChannel, "midi_out_chan",        json_integer(channel.midiOutChan));
    json_array_append_new(jChannels, jChannel);
    
    /* actions */
    
    json_t *jActions = json_array();
    for (unsigned k=0; k<channel.actions.size; k++) {
      json_t   *jAction = json_object();
      action_t  action  = channel.actions.at(k);
      json_object_set_new(jAction, "type",    json_integer(action.type));
      json_object_set_new(jAction, "frame",   json_integer(action.frame));
      json_object_set_new(jAction, "f_value", json_real(action.fValue));
      json_object_set_new(jAction, "i_value", json_integer(action.iValue));
      json_array_append_new(jActions, jAction);
    }
    json_object_set_new(jChannel, "actions", jActions);

#ifdef WITH_VST

    /* plugins */
    
    json_t *jPlugins = json_array();
    for (unsigned j=0; j<channel.plugins.size; j++) {
      json_t   *jPlugin = json_object();
      plugin_t  plugin  = channel.plugins.at(j);
      json_object_set_new(jPlugin, "path",     json_string(plugin.path));
      json_object_set_new(jPlugin, "bypass",   json_boolean(plugin.bypass));
      json_array_append_new(jPlugins, jPlugin);
      
      /* plugin params */
      
      json_t *jPluginParams = json_array();
      for (unsigned z=0; z<plugin.params.size; z++) {
        json_array_append_new(jPluginParams, json_real(plugin.params.at(z)));
      }
      json_object_set_new(jPlugin, "params", jPluginParams);
    }
    json_object_set_new(jChannel, "plugins", jPlugins);
    
#endif
  }
  json_object_set_new(jRoot, "channels", jChannels);
  
  char *out = json_dumps(jRoot, 0);
  fputs(out, fp);
  fclose(fp);
  free(out);
}
