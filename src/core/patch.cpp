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


#include "utils/log.h"
#include "utils/string.h"
#include "utils/ver.h"
#include "utils/math.h"
#include "utils/fs.h"
#include "gui/elems/mainWindow/keyboard/column.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/dialogs/mainWindow.h"
#include "core/model/model.h"
#include "core/channels/channel.h"
#include "core/channels/midiChannel.h"
#include "core/channels/sampleChannel.h"
#include "const.h"
#include "types.h"
#include "storager.h"
#include "midiEvent.h"
#include "conf.h"
#include "mixer.h"
#include "patch.h"


extern giada::v::gdMainWindow* G_MainWin;


namespace giada {
namespace m {
namespace patch
{
namespace
{
/* sanitize
Internal sanity check. */

void sanitize()
{
	namespace um = u::math;

	bpm          = um::bound(bpm, G_MIN_BPM, G_MAX_BPM, G_DEFAULT_BPM);
	bars         = um::bound(bars, 1, G_MAX_BARS, G_DEFAULT_BARS);
	beats        = um::bound(beats, 1, G_MAX_BEATS, G_DEFAULT_BEATS);
	quantize     = um::bound(quantize, 0, G_MAX_QUANTIZE, G_DEFAULT_QUANTIZE);
	masterVolIn  = um::bound(masterVolIn, 0.0f, 1.0f, G_DEFAULT_VOL);
	masterVolOut = um::bound(masterVolOut, 0.0f, 1.0f, G_DEFAULT_VOL);
	samplerate   = samplerate <= 0 ? G_DEFAULT_SAMPLERATE : samplerate;

	for (column_t& col : columns) {
		col.index = col.index < 0 ? 0 : col.index;
		col.width = col.width < G_MIN_COLUMN_WIDTH ? G_MIN_COLUMN_WIDTH : col.width;
	}

	for (channel_t& ch : channels) {
		ch.size        = um::bound(ch.size, G_GUI_CHANNEL_H_1, G_GUI_CHANNEL_H_4, G_GUI_CHANNEL_H_1);
		ch.volume      = um::bound(ch.volume, 0.0f, 1.0f, G_DEFAULT_VOL);
		ch.pan         = um::bound(ch.pan, 0.0f, 1.0f, 1.0f);
		ch.pitch       = um::bound(ch.pitch, 0.1f, G_MAX_PITCH, G_DEFAULT_PITCH);
		ch.midiOutChan = um::bound(ch.midiOutChan, 0, G_MAX_MIDI_CHANS - 1, 0);
	}
}


/* -------------------------------------------------------------------------- */

/* modernize
Makes sure an older patch is compatible with the current version. */

void modernize()
{
	/* Starting from 0.15.0 actions are recorded on frames, not samples. */
	if (u::ver::isLess(versionMajor, versionMinor, versionPatch, 0, 15, 0)) {
		for (channel_t& ch : channels)
			for (action_t& a : ch.actions)
				a.frame /= 2;
	}

	/* Starting from 0.15.1 Channel Modes have different values. */
	if (u::ver::isLess(versionMajor, versionMinor, versionPatch, 0, 15, 1)) {
		for (channel_t& ch : channels) {
			if      (ch.mode == 0x04) ch.mode = static_cast<int>(ChannelMode::SINGLE_BASIC);
			else if (ch.mode == 0x08) ch.mode = static_cast<int>(ChannelMode::SINGLE_PRESS);
			else if (ch.mode == 0x10) ch.mode = static_cast<int>(ChannelMode::SINGLE_RETRIG);
			else if (ch.mode == 0x20) ch.mode = static_cast<int>(ChannelMode::LOOP_REPEAT);
			else if (ch.mode == 0x40) ch.mode = static_cast<int>(ChannelMode::SINGLE_ENDLESS);
			else if (ch.mode == 0x80) ch.mode = static_cast<int>(ChannelMode::LOOP_ONCE_BAR);
		}
	}
}


/* -------------------------------------------------------------------------- */

/* setInvalid
Helper function used to return invalid status while reading. */

int setInvalid(json_t* jRoot)
{
	json_decref(jRoot);
	return PATCH_INVALID;
}


/* -------------------------------------------------------------------------- */


bool readCommons(json_t* jContainer)
{
	if (!storager::setString(jContainer, PATCH_KEY_HEADER, header))  return 0;
	if (!storager::setString(jContainer, PATCH_KEY_VERSION, version)) return 0;
	if (!storager::setInt   (jContainer, PATCH_KEY_VERSION_MAJOR, versionMajor)) return 0;
	if (!storager::setInt   (jContainer, PATCH_KEY_VERSION_MINOR, versionMinor)) return 0;
	if (!storager::setInt   (jContainer, PATCH_KEY_VERSION_PATCH, versionPatch)) return 0;
	if (!storager::setString(jContainer, PATCH_KEY_NAME, name)) return 0;
	if (!storager::setFloat (jContainer, PATCH_KEY_BPM, bpm)) return 0;
	if (!storager::setInt   (jContainer, PATCH_KEY_BARS, bars)) return 0;
	if (!storager::setInt   (jContainer, PATCH_KEY_BEATS, beats)) return 0;
	if (!storager::setInt   (jContainer, PATCH_KEY_QUANTIZE, quantize)) return 0;
	if (!storager::setFloat (jContainer, PATCH_KEY_MASTER_VOL_IN, masterVolIn)) return 0;
	if (!storager::setFloat (jContainer, PATCH_KEY_MASTER_VOL_OUT, masterVolOut)) return 0;
	if (!storager::setInt   (jContainer, PATCH_KEY_METRONOME, metronome)) return 0;
	if (!storager::setInt   (jContainer, PATCH_KEY_LAST_TAKE_ID, lastTakeId)) return 0;
	if (!storager::setInt   (jContainer, PATCH_KEY_SAMPLERATE, samplerate)) return 0;
	return 1;
}



/* -------------------------------------------------------------------------- */

#ifdef WITH_VST

bool readPlugins(json_t* jContainer, std::vector<plugin_t>* container, const char* key)
{
	json_t* jPlugins = json_object_get(jContainer, key);
	if (!storager::checkArray(jPlugins, key))
		return 0;

	size_t pluginIndex;
	json_t* jPlugin;
	json_array_foreach(jPlugins, pluginIndex, jPlugin) {

		if (!storager::checkObject(jPlugin, "")) // TODO pass pluginIndex as string
			return 0;

		plugin_t plugin;
		if (!storager::setString(jPlugin, PATCH_KEY_PLUGIN_PATH,   plugin.path)) return 0;
		if (!storager::setBool  (jPlugin, PATCH_KEY_PLUGIN_BYPASS, plugin.bypass)) return 0;

		/* read plugin params */

		json_t* jParams = json_object_get(jPlugin, PATCH_KEY_PLUGIN_PARAMS);
		if (!storager::checkArray(jParams, PATCH_KEY_PLUGIN_PARAMS)) return 0;

		size_t paramIndex;
		json_t* jParam;
		json_array_foreach(jParams, paramIndex, jParam)
			plugin.params.push_back(json_real_value(jParam));

		/* read midiIn params (midi learning on plugins' parameters) */

		json_t* jMidiInParams = json_object_get(jPlugin, PATCH_KEY_PLUGIN_MIDI_IN_PARAMS);
		if (!storager::checkArray(jMidiInParams, PATCH_KEY_PLUGIN_MIDI_IN_PARAMS)) return 0;

		size_t midiInParamIndex;
		json_t* jMidiInParam;
		json_array_foreach(jMidiInParams, midiInParamIndex, jMidiInParam)
			plugin.midiInParams.push_back(json_integer_value(jMidiInParam));

		container->push_back(plugin);
	}
	return 1;
}

#endif

/* -------------------------------------------------------------------------- */


bool readActions(json_t* jContainer, channel_t* channel)
{
	json_t* jActions = json_object_get(jContainer, PATCH_KEY_CHANNEL_ACTIONS);
	if (!storager::checkArray(jActions, PATCH_KEY_CHANNEL_ACTIONS))
		return false;

	size_t actionIndex;
	json_t* jAction;
	json_array_foreach(jActions, actionIndex, jAction) {

		if (!storager::checkObject(jAction, "")) // TODO pass actionIndex as string
			return false;

		action_t action;

		/* TODO - temporary code for backward compatibility with old actions. 
		To be removed in 0.16.0. */
		if (u::ver::isLess(versionMajor, versionMinor, versionPatch, 0, 15, 3)) {

			action.id = -1;
			action.channel = channel->index;
			if (!storager::setInt   (jAction, "frame", action.frame)) return 0;			
			if (!storager::setUint32(jAction, "type",  action.event)) return 0;
			action.prev = -1;
			action.next = -1;

			if      (action.event == 0x01)   // KEY_PRESS
				action.event = MidiEvent(MidiEvent::NOTE_ON, 0, 0).getRaw();
			else if (action.event == 0x02)   // KEY_REL
				action.event = MidiEvent(MidiEvent::NOTE_OFF, 0, 0).getRaw();
			else if (action.event == 0x04)   // KEY_KILL
				action.event = MidiEvent(MidiEvent::NOTE_KILL, 0, 0).getRaw();
			else if (action.event == 0x20)   // VOLUME not supported, sorry :)
				continue;
			else if (action.event == 0x40)   // MIDI EVENT
				if (!storager::setUint32(jAction, "i_value", action.event)) return 0;
		}
		else {
			if (!storager::setInt   (jAction, G_PATCH_KEY_ACTION_ID,      action.id)) return 0;
			if (!storager::setInt   (jAction, G_PATCH_KEY_ACTION_CHANNEL, action.channel)) return 0;
			if (!storager::setInt   (jAction, G_PATCH_KEY_ACTION_FRAME,   action.frame)) return 0;
			if (!storager::setUint32(jAction, G_PATCH_KEY_ACTION_EVENT,   action.event)) return 0;
			if (!storager::setInt   (jAction, G_PATCH_KEY_ACTION_PREV,    action.prev)) return 0;
			if (!storager::setInt   (jAction, G_PATCH_KEY_ACTION_NEXT,    action.next)) return 0;
		}
		channel->actions.push_back(action);
	}
	return true;
}


/* -------------------------------------------------------------------------- */


bool readChannels(json_t* jContainer)
{
	json_t* jChannels = json_object_get(jContainer, PATCH_KEY_CHANNELS);
	if (!storager::checkArray(jChannels, PATCH_KEY_CHANNELS))
		return 0;

	size_t channelIndex;
	json_t* jChannel;
	json_array_foreach(jChannels, channelIndex, jChannel) {

		std::string channelIndexStr = "channel " + u::string::iToString(channelIndex);
		if (!storager::checkObject(jChannel, channelIndexStr.c_str()))
			return 0;

		channel_t channel;

		if (!storager::setInt   (jChannel, PATCH_KEY_CHANNEL_TYPE,                 channel.type)) return 0;
		if (!storager::setInt   (jChannel, PATCH_KEY_CHANNEL_INDEX,                channel.index)) return 0;
		if (!storager::setInt   (jChannel, PATCH_KEY_CHANNEL_SIZE,                 channel.size)) return 0;
		if (!storager::setString(jChannel, PATCH_KEY_CHANNEL_NAME,                 channel.name)) return 0;
		if (!storager::setInt   (jChannel, PATCH_KEY_CHANNEL_COLUMN,               channel.column)) return 0;
		if (!storager::setInt   (jChannel, PATCH_KEY_CHANNEL_MUTE,                 channel.mute)) return 0;
		if (!storager::setInt   (jChannel, PATCH_KEY_CHANNEL_SOLO,                 channel.solo)) return 0;
		if (!storager::setFloat (jChannel, PATCH_KEY_CHANNEL_VOLUME,               channel.volume)) return 0;
		if (!storager::setFloat (jChannel, PATCH_KEY_CHANNEL_PAN,                  channel.pan)) return 0;
		if (!storager::setBool  (jChannel, PATCH_KEY_CHANNEL_MIDI_IN,              channel.midiIn)) return 0;
		if (!storager::setBool  (jChannel, PATCH_KEY_CHANNEL_MIDI_IN_VELO_AS_VOL,  channel.midiInVeloAsVol)) return 0;
		if (!storager::setUint32(jChannel, PATCH_KEY_CHANNEL_MIDI_IN_KEYPRESS,     channel.midiInKeyPress)) return 0;
		if (!storager::setUint32(jChannel, PATCH_KEY_CHANNEL_MIDI_IN_KEYREL,       channel.midiInKeyRel)) return 0;
		if (!storager::setUint32(jChannel, PATCH_KEY_CHANNEL_MIDI_IN_KILL,         channel.midiInKill)) return 0;
		if (!storager::setUint32(jChannel, PATCH_KEY_CHANNEL_MIDI_IN_ARM,          channel.midiInArm)) return 0;
		if (!storager::setUint32(jChannel, PATCH_KEY_CHANNEL_MIDI_IN_VOLUME,       channel.midiInVolume)) return 0;
		if (!storager::setUint32(jChannel, PATCH_KEY_CHANNEL_MIDI_IN_MUTE,         channel.midiInMute)) return 0;
		if (!storager::setUint32(jChannel, PATCH_KEY_CHANNEL_MIDI_IN_SOLO,         channel.midiInSolo)) return 0;
		if (!storager::setInt   (jChannel, PATCH_KEY_CHANNEL_MIDI_IN_FILTER,       channel.midiInFilter)) return 0;
		if (!storager::setBool  (jChannel, PATCH_KEY_CHANNEL_MIDI_OUT_L,           channel.midiOutL)) return 0;
		if (!storager::setUint32(jChannel, PATCH_KEY_CHANNEL_MIDI_OUT_L_PLAYING,   channel.midiOutLplaying)) return 0;
		if (!storager::setUint32(jChannel, PATCH_KEY_CHANNEL_MIDI_OUT_L_MUTE,      channel.midiOutLmute)) return 0;
		if (!storager::setUint32(jChannel, PATCH_KEY_CHANNEL_MIDI_OUT_L_SOLO,      channel.midiOutLsolo)) return 0;
		if (!storager::setString(jChannel, PATCH_KEY_CHANNEL_SAMPLE_PATH,          channel.samplePath)) return 0;
		if (!storager::setInt   (jChannel, PATCH_KEY_CHANNEL_KEY,                  channel.key)) return 0;
		if (!storager::setInt   (jChannel, PATCH_KEY_CHANNEL_MODE,                 channel.mode)) return 0;
		if (!storager::setInt   (jChannel, PATCH_KEY_CHANNEL_BEGIN,                channel.begin)) return 0;
		if (!storager::setInt   (jChannel, PATCH_KEY_CHANNEL_END,                  channel.end)) return 0;
		if (!storager::setFloat (jChannel, PATCH_KEY_CHANNEL_BOOST,                channel.boost)) return 0;
		if (!storager::setInt   (jChannel, PATCH_KEY_CHANNEL_READ_ACTIONS,         channel.readActions)) return 0;
		if (!storager::setFloat (jChannel, PATCH_KEY_CHANNEL_PITCH,                channel.pitch)) return 0;
		if (!storager::setBool  (jChannel, PATCH_KEY_CHANNEL_INPUT_MONITOR,        channel.inputMonitor)) return 0;
		if (!storager::setUint32(jChannel, PATCH_KEY_CHANNEL_MIDI_IN_READ_ACTIONS, channel.midiInReadActions)) return 0;
		if (!storager::setUint32(jChannel, PATCH_KEY_CHANNEL_MIDI_IN_PITCH,        channel.midiInPitch)) return 0;
		if (!storager::setInt   (jChannel, PATCH_KEY_CHANNEL_MIDI_OUT,             channel.midiOut)) return 0;
		if (!storager::setInt   (jChannel, PATCH_KEY_CHANNEL_MIDI_OUT_CHAN,        channel.midiOutChan)) return 0;
		if (!storager::setBool  (jChannel, PATCH_KEY_CHANNEL_ARMED,                channel.armed)) return 0;

		readActions(jChannel, &channel);

#ifdef WITH_VST
		readPlugins(jChannel, &channel.plugins, PATCH_KEY_CHANNEL_PLUGINS);
#endif
		channels.push_back(channel);
	}
	return 1;
}


/* -------------------------------------------------------------------------- */


bool readColumns(json_t* jContainer)
{
	json_t* jColumns = json_object_get(jContainer, PATCH_KEY_COLUMNS);
	if (!storager::checkArray(jColumns, PATCH_KEY_COLUMNS))
		return 0;

	size_t columnIndex;
	json_t* jColumn;
	json_array_foreach(jColumns, columnIndex, jColumn) {

		std::string columnIndexStr = "column " + u::string::iToString(columnIndex);
		if (!storager::checkObject(jColumn, columnIndexStr.c_str()))
			return 0;

		column_t column;
		if (!storager::setInt(jColumn, PATCH_KEY_COLUMN_INDEX, column.index)) return 0;
		if (!storager::setInt(jColumn, PATCH_KEY_COLUMN_WIDTH, column.width)) return 0;

		columns.push_back(column);
	}
	return 1;
}


/* -------------------------------------------------------------------------- */

#ifdef WITH_VST

void writePlugins_(json_t* j)
{
	model::PluginsLock pl(model::plugins);
	
	json_t* jps = json_array();

	for (const Plugin* p : model::plugins) {

		/* Plugin. */

		json_t* jp = json_object();
		json_object_set_new(jp, PATCH_KEY_PLUGIN_ID,     json_integer(p->id));
		json_object_set_new(jp, PATCH_KEY_PLUGIN_PATH,   json_string(p->getUniqueId().c_str()));
		json_object_set_new(jp, PATCH_KEY_PLUGIN_BYPASS, json_boolean(p->isBypassed()));
		json_array_append_new(jps, jp);
		
		/* Plugin parameters. */

		json_t* jparams = json_array();	
		for (int k = 0; k < p->getNumParameters(); k++)
			json_array_append_new(jparams, json_real(p->getParameter(k)));
		json_object_set_new(jp, PATCH_KEY_PLUGIN_PARAMS, jparams);
		
		/* MidiIn params (midi learning on plugins' parameters). */

		json_t* jmidiparams = json_array();
		for (uint32_t param : p->midiInParams)
			json_array_append_new(jmidiparams, json_integer(param));
		json_object_set_new(jp, PATCH_KEY_PLUGIN_MIDI_IN_PARAMS, jmidiparams);
	}
	json_object_set_new(j, PATCH_KEY_PLUGINS, jps);
}

#endif


/* -------------------------------------------------------------------------- */


void writeColumns_(json_t* j)
{
	json_t* jcs = json_array();

	G_MainWin->keyboard->forEachColumn([&](const v::geColumn& c)
	{
		json_t* jc = json_object();
		json_object_set_new(jc, PATCH_KEY_COLUMN_INDEX, json_integer(c.getIndex()));
		json_object_set_new(jc, PATCH_KEY_COLUMN_WIDTH, json_integer(c.w()));
		
		json_t* jchans = json_array();
		c.forEachChannel([&](v::geChannel* c)
		{	
			json_array_append_new(jchans, json_integer(c->channelId));
		});
		json_object_set_new(jc, PATCH_KEY_COLUMN_CHANNELS, jchans);
		
		json_array_append_new(jcs, jc);

	});
	json_object_set_new(j, PATCH_KEY_COLUMNS, jcs);
}


/* -------------------------------------------------------------------------- */


void writeActions_(json_t* j)
{
	model::ActionsLock al(model::actions);
	
	json_t* jas = json_array();
	
	for (auto& kv : model::actions.get()->map) {
		for (Action& a : kv.second) {
			json_t* ja = json_object();
			json_object_set_new(ja, G_PATCH_KEY_ACTION_ID,      json_integer(a.id));
			json_object_set_new(ja, G_PATCH_KEY_ACTION_CHANNEL, json_integer(a.channelId));
			json_object_set_new(ja, G_PATCH_KEY_ACTION_FRAME,   json_integer(a.frame));
			json_object_set_new(ja, G_PATCH_KEY_ACTION_EVENT,   json_integer(a.event.getRaw()));
			json_object_set_new(ja, G_PATCH_KEY_ACTION_PREV,    json_integer(a.prevId));
			json_object_set_new(ja, G_PATCH_KEY_ACTION_NEXT,    json_integer(a.nextId));
			json_array_append_new(jas, ja);
		}
	}
	json_object_set_new(j, PATCH_KEY_CHANNEL_ACTIONS, jas);
}


/* -------------------------------------------------------------------------- */


void writeCommons_(json_t* j, const std::string& name)
{
	model::ClockLock cl(model::clock);
	model::MixerLock ml(model::mixer);
	
	json_object_set_new(j, PATCH_KEY_HEADER,         json_string("GIADAPTC"));
	json_object_set_new(j, PATCH_KEY_VERSION,        json_string(G_VERSION_STR));
	json_object_set_new(j, PATCH_KEY_VERSION_MAJOR,  json_integer(G_VERSION_MAJOR));
	json_object_set_new(j, PATCH_KEY_VERSION_MINOR,  json_integer(G_VERSION_MINOR));
	json_object_set_new(j, PATCH_KEY_VERSION_PATCH,  json_integer(G_VERSION_PATCH));
	json_object_set_new(j, PATCH_KEY_NAME,           json_string(name.c_str()));
	json_object_set_new(j, PATCH_KEY_BARS,           json_integer(model::clock.get()->bars));
	json_object_set_new(j, PATCH_KEY_BEATS,          json_integer(model::clock.get()->beats));
	json_object_set_new(j, PATCH_KEY_BPM,            json_real(model::clock.get()->bpm));
	json_object_set_new(j, PATCH_KEY_QUANTIZE,       json_integer(model::clock.get()->quantize));
	json_object_set_new(j, PATCH_KEY_MASTER_VOL_IN,  json_real(model::mixer.get()->inVol));
	json_object_set_new(j, PATCH_KEY_MASTER_VOL_OUT, json_real(model::mixer.get()->outVol));
	json_object_set_new(j, PATCH_KEY_LAST_TAKE_ID,   json_integer(lastTakeId));
	json_object_set_new(j, PATCH_KEY_SAMPLERATE,     json_integer(samplerate));

	/* TODO */
	json_object_set_new(j, PATCH_KEY_METRONOME,      json_integer(metronome));
}


/* -------------------------------------------------------------------------- */


void writeChannels_(json_t* j, bool project)
{
	model::ChannelsLock l(model::channels);
	
	json_t* jcs = json_array();
	
	for (Channel* c : model::channels) {
		
		if (c->type == ChannelType::MASTER)
			continue;

		json_t* jc = json_object();

		json_object_set_new(jc, PATCH_KEY_CHANNEL_TYPE,               json_integer(static_cast<int>(c->type)));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_INDEX,              json_integer(c->id)); // TODO -> CHANNEL_ID
		json_object_set_new(jc, PATCH_KEY_CHANNEL_SIZE,               json_integer(G_MainWin->keyboard->getChannel(c->id)->getSize()));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_NAME,               json_string(c->name.c_str()));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_COLUMN,             json_integer(c->columnIndex));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_MUTE,               json_integer(c->mute));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_SOLO,               json_integer(c->solo));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_VOLUME,             json_real(c->volume));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_PAN,                json_real(c->pan));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_ARMED,              json_boolean(c->armed));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_MIDI_IN,            json_boolean(c->midiIn.load()));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_MIDI_IN_KEYREL,     json_integer(c->midiInKeyRel.load()));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_MIDI_IN_KEYPRESS,   json_integer(c->midiInKeyPress.load()));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_MIDI_IN_KILL,       json_integer(c->midiInKill.load()));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_MIDI_IN_ARM,        json_integer(c->midiInArm.load()));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_MIDI_IN_VOLUME,     json_integer(c->midiInVolume.load()));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_MIDI_IN_MUTE,       json_integer(c->midiInMute.load()));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_MIDI_IN_SOLO,       json_integer(c->midiInSolo.load()));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_MIDI_IN_FILTER,     json_integer(c->midiInFilter.load()));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_MIDI_OUT_L,         json_boolean(c->midiOutL.load()));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_MIDI_OUT_L_PLAYING, json_integer(c->midiOutLplaying.load()));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_MIDI_OUT_L_MUTE,    json_integer(c->midiOutLmute.load()));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_MIDI_OUT_L_SOLO,    json_integer(c->midiOutLsolo.load()));
		
		json_t* jplugins = json_array();
		for (ID pid : c->pluginIds)
			json_array_append_new(jplugins, json_integer(pid));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_PLUGINS, jplugins);

		if (c->type == ChannelType::SAMPLE) {
			SampleChannel* sc = static_cast<SampleChannel*>(c);
			
			std::string samplePath = sc->getSamplePath();
			if (project)
				samplePath = gu_basename(samplePath); 

			json_object_set_new(jc, PATCH_KEY_CHANNEL_SAMPLE_PATH,          json_string(samplePath.c_str()));
			json_object_set_new(jc, PATCH_KEY_CHANNEL_KEY,                  json_integer(sc->key));
			json_object_set_new(jc, PATCH_KEY_CHANNEL_MODE,                 json_integer(static_cast<int>(sc->mode)));
			json_object_set_new(jc, PATCH_KEY_CHANNEL_BEGIN,                json_integer(sc->begin));
			json_object_set_new(jc, PATCH_KEY_CHANNEL_END,                  json_integer(sc->end));
			json_object_set_new(jc, PATCH_KEY_CHANNEL_READ_ACTIONS,         json_integer(sc->readActions));
			json_object_set_new(jc, PATCH_KEY_CHANNEL_PITCH,                json_real(sc->pitch));
			json_object_set_new(jc, PATCH_KEY_CHANNEL_INPUT_MONITOR,        json_boolean(sc->inputMonitor));
			json_object_set_new(jc, PATCH_KEY_CHANNEL_MIDI_IN_READ_ACTIONS, json_integer(sc->midiInReadActions.load()));
			json_object_set_new(jc, PATCH_KEY_CHANNEL_MIDI_IN_PITCH,        json_integer(sc->midiInPitch.load()));
		}
		else
		if (c->type == ChannelType::MIDI) {
			MidiChannel* mc = static_cast<MidiChannel*>(c);
			json_object_set_new(jc, PATCH_KEY_CHANNEL_MIDI_OUT,      json_integer(mc->midiOut));
			json_object_set_new(jc, PATCH_KEY_CHANNEL_MIDI_OUT_CHAN, json_integer(mc->midiOutChan));
		}

		//writeActions_(jc);
		json_array_append_new(jcs, jc);
	}
	json_object_set_new(j, PATCH_KEY_CHANNELS, jcs);
}
}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


std::string header;
std::string version;
int    versionMajor;
int    versionMinor;
int    versionPatch;
std::string name;
float  bpm;
int    bars;
int    beats;
int    quantize;
float  masterVolIn;
float  masterVolOut;
int    metronome;
int    lastTakeId;
int    samplerate;   // original samplerate when the patch was saved

std::vector<column_t>  columns;
std::vector<channel_t> channels;

#ifdef WITH_VST
std::vector<plugin_t> masterInPlugins;
std::vector<plugin_t> masterOutPlugins;
#endif


/* -------------------------------------------------------------------------- */


void init()
{
	columns.clear();
	channels.clear();
#ifdef WITH_VST
	masterInPlugins.clear();
	masterOutPlugins.clear();
#endif
	lastTakeId = 0;
	samplerate = G_DEFAULT_SAMPLERATE;
}


/* -------------------------------------------------------------------------- */


int write(const std::string& name, const std::string& file, bool isProject)
{
	json_t* j = json_object();

	writeCommons_(j, name);
	writeColumns_(j);
	writeChannels_(j, isProject);
	writeActions_(j);
#ifdef WITH_VST
	writePlugins_(j);
#endif

	if (json_dump_file(j, file.c_str(), JSON_COMPACT) != 0) {
		gu_log("[patch::write] unable to write patch file!\n");
		return 0;
	}
	return 1;
}


/* -------------------------------------------------------------------------- */


int read(const std::string& file)
{
	json_error_t jError;
	json_t* jRoot = json_load_file(file.c_str(), 0, &jError);
	if (!jRoot) {
		gu_log("[patch::read] unable to read patch file! Error on line %d: %s\n", 
			jError.line, jError.text);
		return PATCH_UNREADABLE;
	}

	if (!storager::checkObject(jRoot, "root element"))
		return PATCH_INVALID;

	init();

	/* TODO json_decref also when PATCH_INVALID */

	if (!readCommons(jRoot))  return setInvalid(jRoot);
	if (!readColumns(jRoot))  return setInvalid(jRoot);
	if (!readChannels(jRoot)) return setInvalid(jRoot);
#ifdef WITH_VST
	if (!readPlugins(jRoot, &masterInPlugins, PATCH_KEY_MASTER_IN_PLUGINS))   return setInvalid(jRoot);
	if (!readPlugins(jRoot, &masterOutPlugins, PATCH_KEY_MASTER_OUT_PLUGINS)) return setInvalid(jRoot);
#endif

	json_decref(jRoot);

	sanitize();
	modernize();

	return PATCH_READ_OK;
}


}}}; // giada::m::patch::
