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


#include <jansson.h>
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
#include "core/channels/channelManager.h"
#include "core/channels/channel.h"
#include "core/channels/midiChannel.h"
#include "core/channels/sampleChannel.h"
#include "core/pluginManager.h"
#include "core/waveManager.h"
#include "core/const.h"
#include "core/types.h"
#include "core/midiEvent.h"
#include "core/recorderHandler.h"
#include "core/conf.h"
#include "core/mixer.h"
#include "patch.h"


extern giada::v::gdMainWindow* G_MainWin;


namespace giada {
namespace m {
namespace patch
{
namespace
{
/* jsonIs, jsonGet
Tiny wrappers around the old C-style macros provided by Jansson. This way we can
pass them as template parameters. */

bool jsonIsString(json_t* j) { return json_is_string(j); }
bool jsonIsInt   (json_t* j) { return json_is_integer(j); }
bool jsonIsFloat (json_t* j) { return json_is_real(j); }
bool jsonIsBool  (json_t* j) { return json_is_boolean(j); }
bool jsonIsArray (json_t* j) { return json_is_array(j); }
bool jsonIsObject(json_t* j) { return json_is_object(j); }

std::string jsonGetString(json_t* j) { return json_string_value(j); }
uint32_t    jsonGetInt   (json_t* j) { return json_integer_value(j); }
float       jsonGetFloat (json_t* j) { return json_real_value(j); }
bool        jsonGetBool  (json_t* j) { return json_boolean_value(j); }


/* -------------------------------------------------------------------------- */


template<typename F>
bool is_(json_t* j, F f)
{
	if (!f(j)) {
		gu_log("[patch::is_] malformed json!\n");
		json_decref(j);
		return false;
	}
	return true;
}


/* -------------------------------------------------------------------------- */


template<typename O, typename FC, typename FG>
O read_(json_t* j, const char* key, FC checker, FG getter, O def)
{
	json_t* jo = json_object_get(j, key);
	if (jo == nullptr) {
		gu_log("[patch::read_] key '%s' not found, using default value\n", key);
		return def;
	}
	if (!checker(jo)) {
		gu_log("[patch::read_] key '%s' is of the wrong type!\n", key);
		json_decref(j);
		return def;
	}
	return getter(jo);
}


/* -------------------------------------------------------------------------- */


std::string readString_(json_t* j, const char* key, const std::string& def="")
{
	return read_(j, key, jsonIsString, jsonGetString, def);
}

uint32_t readInt_(json_t* j, const char* key, uint32_t def=0)
{
	return read_(j, key, jsonIsInt, jsonGetInt, def);
}

float readFloat_(json_t* j, const char* key, float def=0.0f)
{
	return read_(j, key, jsonIsFloat, jsonGetFloat, def);
}

float readBool_(json_t* j, const char* key, bool def=false)
{
	return read_(j, key, jsonIsBool, jsonGetBool, def);
}


/* -------------------------------------------------------------------------- */


bool isArray_(json_t* j)  { return is_(j, jsonIsArray); };
bool isObject_(json_t* j) { return is_(j, jsonIsObject); };


/* -------------------------------------------------------------------------- */

/* sanitize
Internal sanity check. */

void sanitize()
{
#if 0
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
#endif
}


/* -------------------------------------------------------------------------- */

/* modernize
Makes sure an older patch is compatible with the current version. */

void modernize()
{
#if 0
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
#endif
}


/* -------------------------------------------------------------------------- */

/* setInvalid
Helper function used to return invalid status while reading. */

int setInvalid(json_t* jRoot)
{
	json_decref(jRoot);
	return G_PATCH_INVALID;
}


/* -------------------------------------------------------------------------- */


bool readCommons_(json_t* j)
{
	header = readString_(j, PATCH_KEY_HEADER);
	if (header == "")
		return false;

	name         = readString_(j, PATCH_KEY_NAME);
	version      = readString_(j, PATCH_KEY_VERSION);
	versionMajor = readInt_(j, PATCH_KEY_VERSION_MAJOR);
	versionMinor = readInt_(j, PATCH_KEY_VERSION_MINOR);
	versionPatch = readInt_(j, PATCH_KEY_VERSION_PATCH);
	samplerate   = readInt_(j, PATCH_KEY_SAMPLERATE);
	lastTakeId   = readInt_(j, PATCH_KEY_LAST_TAKE_ID);
	metronome    = readInt_(j, PATCH_KEY_METRONOME);

	model::onSwap(model::clock, [&](model::Clock& c)
	{
		c.bpm      = readFloat_(j, PATCH_KEY_BPM);
		c.bars     = readInt_(j, PATCH_KEY_BARS);
		c.beats    = readInt_(j, PATCH_KEY_BEATS);
		c.quantize = readInt_(j, PATCH_KEY_QUANTIZE);
	});

	model::onSwap(model::mixer, [&](model::Mixer& m)
	{
		m.inVol  = readFloat_(j, PATCH_KEY_MASTER_VOL_IN);
		m.outVol = readFloat_(j, PATCH_KEY_MASTER_VOL_OUT);
	});

	return true;
}


/* -------------------------------------------------------------------------- */

#ifdef WITH_VST

bool readPluginParams_(json_t* j, std::vector<float>& params)
{
	json_t* jps = json_object_get(j, PATCH_KEY_PLUGIN_PARAMS);
	if (!isArray_(jps))
		return false;

	size_t  i;
	json_t* jp;
	json_array_foreach(jps, i, jp)
		params.push_back(json_real_value(jp));

	return true;
}


bool readMidiInPluginParams_(json_t* j, std::vector<uint32_t>& params)
{
	json_t* jps = json_object_get(j, PATCH_KEY_PLUGIN_MIDI_IN_PARAMS);
	if (!isArray_(jps))
		return false;

	size_t  i;
	json_t* jp;
	json_array_foreach(jps, i, jp)
		params.push_back(json_integer_value(jp));

	return true;
}


/* -------------------------------------------------------------------------- */


bool readPlugins_(json_t* j)
{
	json_t* jps = json_object_get(j, PATCH_KEY_PLUGINS);
	if (!isArray_(jps))
		return false;

	size_t  i;
	json_t* jp;
	json_array_foreach(jps, i, jp) {
		
		if (!isObject_(jp))
			return false;

		Plugin p;
		p.id     = readInt_   (jp, PATCH_KEY_PLUGIN_ID);
		p.path   = readString_(jp, PATCH_KEY_PLUGIN_PATH);
		p.bypass = readBool_  (jp, PATCH_KEY_PLUGIN_BYPASS);

		if (!readPluginParams_(jp, p.params)) return false;
		if (!readMidiInPluginParams_(jp, p.midiInParams)) return false;

		model::plugins.push(std::move(pluginManager::makePlugin(p)));
	}
	return true;
}

#endif

/* -------------------------------------------------------------------------- */


bool readWaves_(json_t* j)
{
	json_t* jws = json_object_get(j, PATCH_KEY_WAVES);
	if (!isArray_(jws))
		return false;

	size_t  i;
	json_t* jw;
	json_array_foreach(jws, i, jw) {
		
		if (!isObject_(jw))
			return false;

		Wave w;
		w.id   = readInt_   (jw, PATCH_KEY_WAVE_ID);
		w.path = readString_(jw, PATCH_KEY_WAVE_PATH);

		model::waves.push(std::move(waveManager::createFromPatch(w)));
	}
	return true;
}


/* -------------------------------------------------------------------------- */


bool readActions_(json_t* j)
{
	json_t* jas = json_object_get(j, PATCH_KEY_ACTIONS);
	if (!isArray_(jas))
		return false;

	std::vector<Action> actions;
	size_t  i;
	json_t* ja;
	json_array_foreach(jas, i, ja) {
		
		if (!isObject_(ja))
			return false;

		Action a;
		a.id        = readInt_(ja, G_PATCH_KEY_ACTION_ID);
		a.channelId = readInt_(ja, G_PATCH_KEY_ACTION_CHANNEL);
		a.frame     = readInt_(ja, G_PATCH_KEY_ACTION_FRAME);
		a.event     = readInt_(ja, G_PATCH_KEY_ACTION_EVENT);
		a.prevId    = readInt_(ja, G_PATCH_KEY_ACTION_PREV);
		a.nextId    = readInt_(ja, G_PATCH_KEY_ACTION_NEXT);

		actions.push_back(a);
	}

	model::onSwap(model::actions, [&](model::Actions& a)
	{
		a.map = std::move(recorderHandler::makeActionsFromPatch(actions));
	});

	return true;
}


/* -------------------------------------------------------------------------- */


bool readChannelPlugins_(json_t* j, std::vector<ID>& pluginIds)
{
	json_t* jps = json_object_get(j, PATCH_KEY_CHANNEL_PLUGINS);
	if (!isArray_(jps))
		return false;

	size_t  i;
	json_t* jp;
	json_array_foreach(jps, i, jp)
		pluginIds.push_back(json_integer_value(jp));

	return true;
}


/* -------------------------------------------------------------------------- */


bool readChannels_(json_t* j)
{
	json_t* jcs = json_object_get(j, PATCH_KEY_CHANNELS);
	if (!isArray_(jcs))
		return false;

	size_t  i;
	json_t* jc;
	json_array_foreach(jcs, i, jc) {

		if (!isObject_(jc))
			return false;

		Channel c;
		c.id                = readInt_   (jc, PATCH_KEY_CHANNEL_ID);
		c.type              = static_cast<ChannelType>(readInt_(jc, PATCH_KEY_CHANNEL_TYPE));
		c.size              = readInt_   (jc, PATCH_KEY_CHANNEL_SIZE);
		c.name              = readString_(jc, PATCH_KEY_CHANNEL_NAME);
		c.columnId          = readInt_   (jc, PATCH_KEY_CHANNEL_COLUMN);
		c.key               = readInt_   (jc, PATCH_KEY_CHANNEL_KEY);
		c.mute              = readInt_   (jc, PATCH_KEY_CHANNEL_MUTE);
		c.solo              = readInt_   (jc, PATCH_KEY_CHANNEL_SOLO);
		c.volume            = readFloat_ (jc, PATCH_KEY_CHANNEL_VOLUME);
		c.pan               = readFloat_ (jc, PATCH_KEY_CHANNEL_PAN);
		c.hasActions        = readBool_  (jc, PATCH_KEY_CHANNEL_HAS_ACTIONS);
		c.midiIn            = readBool_  (jc, PATCH_KEY_CHANNEL_MIDI_IN);
		c.midiInVeloAsVol   = readBool_  (jc, PATCH_KEY_CHANNEL_MIDI_IN_VELO_AS_VOL);
		c.midiInKeyPress    = readInt_   (jc, PATCH_KEY_CHANNEL_MIDI_IN_KEYPRESS);
		c.midiInKeyRel      = readInt_   (jc, PATCH_KEY_CHANNEL_MIDI_IN_KEYREL);
		c.midiInKill        = readInt_   (jc, PATCH_KEY_CHANNEL_MIDI_IN_KILL);
		c.midiInArm         = readInt_   (jc, PATCH_KEY_CHANNEL_MIDI_IN_ARM);
		c.midiInVolume      = readInt_   (jc, PATCH_KEY_CHANNEL_MIDI_IN_VOLUME);
		c.midiInMute        = readInt_   (jc, PATCH_KEY_CHANNEL_MIDI_IN_MUTE);
		c.midiInSolo        = readInt_   (jc, PATCH_KEY_CHANNEL_MIDI_IN_SOLO);
		c.midiInFilter      = readInt_   (jc, PATCH_KEY_CHANNEL_MIDI_IN_FILTER);
		c.midiOutL          = readBool_  (jc, PATCH_KEY_CHANNEL_MIDI_OUT_L);
		c.midiOutLplaying   = readInt_   (jc, PATCH_KEY_CHANNEL_MIDI_OUT_L_PLAYING);
		c.midiOutLmute      = readInt_   (jc, PATCH_KEY_CHANNEL_MIDI_OUT_L_MUTE);
		c.midiOutLsolo      = readInt_   (jc, PATCH_KEY_CHANNEL_MIDI_OUT_L_SOLO);
		c.armed             = readBool_  (jc, PATCH_KEY_CHANNEL_ARMED);

		readChannelPlugins_(jc, c.pluginIds);

		if (static_cast<ChannelType>(c.type) == ChannelType::SAMPLE) {
			c.waveId            = readInt_  (jc, PATCH_KEY_CHANNEL_WAVE_ID);
			c.mode              = static_cast<ChannelMode>(readInt_(jc, PATCH_KEY_CHANNEL_MODE));
			c.begin             = readInt_  (jc, PATCH_KEY_CHANNEL_BEGIN);
			c.end               = readInt_  (jc, PATCH_KEY_CHANNEL_END);
			c.readActions       = readBool_ (jc, PATCH_KEY_CHANNEL_READ_ACTIONS);
			c.pitch             = readFloat_(jc, PATCH_KEY_CHANNEL_PITCH);
			c.inputMonitor      = readBool_ (jc, PATCH_KEY_CHANNEL_INPUT_MONITOR);
			c.midiInReadActions = readInt_  (jc, PATCH_KEY_CHANNEL_MIDI_IN_READ_ACTIONS);
			c.midiInPitch       = readInt_  (jc, PATCH_KEY_CHANNEL_MIDI_IN_PITCH);
		}
		else
		if (static_cast<ChannelType>(c.type) == ChannelType::MIDI) {
			c.midiOut     = readInt_(jc, PATCH_KEY_CHANNEL_MIDI_OUT);
			c.midiOutChan = readInt_(jc, PATCH_KEY_CHANNEL_MIDI_OUT_CHAN);
		}
		
		model::channels.push(channelManager::create(c, conf::buffersize));
	}
	
	return true;
}


/* -------------------------------------------------------------------------- */


#ifdef WITH_VST

void writePlugins_(json_t* j)
{
	model::PluginsLock pl(model::plugins);

	json_t* jps = json_array();

	for (const m::Plugin* p : model::plugins) {

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
		json_object_set_new(jc, PATCH_KEY_COLUMN_INDEX, json_integer(c.id));
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
	model::ActionsLock l(model::actions);

	json_t* jas = json_array();

	for (auto& kv : model::actions.get()->map) {
		for (m::Action& a : kv.second) {
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
	json_object_set_new(j, PATCH_KEY_ACTIONS, jas);
}


/* -------------------------------------------------------------------------- */


void writeWaves_(json_t* j)
{
	model::WavesLock l(model::waves);

	json_t* jws = json_array();

	for (const m::Wave* w : model::waves) {
		json_t* jw = json_object();
		json_object_set_new(jw, PATCH_KEY_WAVE_ID,   json_integer(w->id));
		json_object_set_new(jw, PATCH_KEY_WAVE_PATH, json_string(w->getPath().c_str()));
		json_array_append_new(jws, jw);
	}
	json_object_set_new(j, PATCH_KEY_WAVES, jws);
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
	json_object_set_new(j, PATCH_KEY_METRONOME,      json_integer(mixer::isMetronomeOn()));
}


/* -------------------------------------------------------------------------- */


void writeChannels_(json_t* j, bool project)
{
	model::ChannelsLock l(model::channels);

	json_t* jcs = json_array();

	for (m::Channel* c : model::channels) {

		if (c->type == ChannelType::MASTER)
			continue;

		json_t* jc = json_object();

		json_object_set_new(jc, PATCH_KEY_CHANNEL_ID,                 json_integer(c->id));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_TYPE,               json_integer(static_cast<int>(c->type)));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_SIZE,               json_integer(G_MainWin->keyboard->getChannel(c->id)->getSize()));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_NAME,               json_string(c->name.c_str()));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_COLUMN,             json_integer(c->columnId));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_MUTE,               json_integer(c->mute));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_SOLO,               json_integer(c->solo));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_VOLUME,             json_real(c->volume));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_PAN,                json_real(c->pan));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_ARMED,              json_boolean(c->armed));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_HAS_ACTIONS,        json_boolean(c->hasActions));
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
		json_object_set_new(jc, PATCH_KEY_CHANNEL_KEY,                json_integer(c->key));

		json_t* jplugins = json_array();
		for (ID pid : c->pluginIds)
			json_array_append_new(jplugins, json_integer(pid));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_PLUGINS, jplugins);

		if (c->type == ChannelType::SAMPLE) {
			SampleChannel* sc = static_cast<SampleChannel*>(c);

			std::string samplePath = sc->getSamplePath();
			if (project)
				samplePath = gu_basename(samplePath);

			json_object_set_new(jc, PATCH_KEY_CHANNEL_WAVE_ID,              json_integer(sc->waveId));
			json_object_set_new(jc, PATCH_KEY_CHANNEL_MODE,                 json_integer(static_cast<int>(sc->mode)));
			json_object_set_new(jc, PATCH_KEY_CHANNEL_BEGIN,                json_integer(sc->begin));
			json_object_set_new(jc, PATCH_KEY_CHANNEL_END,                  json_integer(sc->end));
			json_object_set_new(jc, PATCH_KEY_CHANNEL_READ_ACTIONS,         json_boolean(sc->readActions));
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

		json_array_append_new(jcs, jc);
	}
	json_object_set_new(j, PATCH_KEY_CHANNELS, jcs);
}
}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


std::string name;
std::string header;
std::string version;
int         versionMajor;
int         versionMinor;
int         versionPatch;
int         samplerate;
int         lastTakeId;
int         metronome;


/* -------------------------------------------------------------------------- */


void init()
{
	lastTakeId = 0;
	samplerate = G_DEFAULT_SAMPLERATE;
}


/* -------------------------------------------------------------------------- */


bool write(const std::string& name, const std::string& file, bool isProject)
{
	json_t* j = json_object();

	writeCommons_(j, name);
	writeColumns_(j);
	writeChannels_(j, isProject);
	writeActions_(j);
	writeWaves_(j);
#ifdef WITH_VST
	writePlugins_(j);
#endif

	if (json_dump_file(j, file.c_str(), JSON_COMPACT) != 0) {
		gu_log("[patch::write] unable to write patch file!\n");
		return false;
	}
	return true;
}


/* -------------------------------------------------------------------------- */


int read(const std::string& file)
{
	json_error_t jerr;
	json_t* j = json_load_file(file.c_str(), 0, &jerr);
	if (j == nullptr) {
		gu_log("[patch::read] unable to read patch file! Error on line %d: %s\n",
			jerr.line, jerr.text);
		return G_PATCH_UNREADABLE;
	}
/*
	if (!storager::checkObject(j, "root element"))
		return PATCH_INVALID;
*/
	init();

	/* TODO json_decref also when PATCH_INVALID */

#ifdef WITH_VST
	if (!readPlugins_(j)) return setInvalid(j);
#endif
	if (!readCommons_(j)) return setInvalid(j);
	if (!readWaves_(j)) return setInvalid(j);
	if (!readActions_(j)) return setInvalid(j);
	if (!readChannels_(j)) return setInvalid(j);

	json_decref(j);

	sanitize();
	modernize();

	return G_PATCH_READ_OK;
}
}}}; // giada::m::patch::
