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
#include "utils/json.h"
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
#include "core/clock.h"
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
void sanitize_()
{
	namespace um = u::math;
	samplerate = um::bound(samplerate, 0, G_DEFAULT_SAMPLERATE);
}


void sanitize_(Channel& c)
{
	namespace um = u::math;
	c.size        = um::bound(c.size, G_GUI_CHANNEL_H_1, G_GUI_CHANNEL_H_4);
	c.volume      = um::bound(c.volume, 0.0f, G_DEFAULT_VOL);
	c.pan         = um::bound(c.pan, 0.0f, 1.0f);
	c.pitch       = um::bound(c.pitch, 0.1f, G_MAX_PITCH);
	c.midiOutChan = um::bound(c.midiOutChan, 0, G_MAX_MIDI_CHANS - 1);
	
#if 0
	namespace um = u::math;

	bpm          = um::bound(bpm, G_MIN_BPM, G_MAX_BPM, G_DEFAULT_BPM);
	bars         = um::bound(bars, 1, G_MAX_BARS, G_DEFAULT_BARS);
	beats        = um::bound(beats, 1, G_MAX_BEATS, G_DEFAULT_BEATS);
	quantize     = um::bound(quantize, 0, G_MAX_QUANTIZE, G_DEFAULT_QUANTIZE);
	masterVolIn  = um::bound(masterVolIn, 0.0f, 1.0f, G_DEFAULT_VOL);
	masterVolOut = um::bound(masterVolOut, 0.0f, 1.0f, G_DEFAULT_VOL);

#endif
}

/*
void sanitize_(Column& c)
{
	namespace um = u::math;
	c.index = um::bound(c.index, 0, col.index);
	c.width = um::bound(c.width, G_MIN_COLUMN_WIDTH, c.width);
}
*/

/* -------------------------------------------------------------------------- */


/* modernize
Makes sure an older patch is compatible with the current version. */

#if 0
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
#endif


/* -------------------------------------------------------------------------- */


void readCommons_(json_t* j)
{
	namespace uj = u::json;

	name         = uj::readString(j, PATCH_KEY_NAME);
	samplerate   = uj::readInt(j, PATCH_KEY_SAMPLERATE);
	lastTakeId   = uj::readInt(j, PATCH_KEY_LAST_TAKE_ID);
	metronome    = uj::readBool(j, PATCH_KEY_METRONOME);

	clock::setBpm     (uj::readFloat(j, PATCH_KEY_BPM));
	clock::setBeats   (uj::readInt(j, PATCH_KEY_BEATS), uj::readInt(j, PATCH_KEY_BARS));
	clock::setQuantize(uj::readInt(j, PATCH_KEY_QUANTIZE));

	/*
	model::onSwap(model::mixer, [&](model::Mixer& m)
	{
	});*/
}


/* -------------------------------------------------------------------------- */


void readColumns_(json_t* j)
{

}


/* -------------------------------------------------------------------------- */

#ifdef WITH_VST

void readPluginParams_(json_t* j, std::vector<float>& params)
{
	json_t* jps = json_object_get(j, PATCH_KEY_PLUGIN_PARAMS);
	if (jps == nullptr)
		return;

	size_t  i;
	json_t* jp;
	json_array_foreach(jps, i, jp)
		params.push_back(json_real_value(jp));
}


void readMidiInPluginParams_(json_t* j, std::vector<uint32_t>& params)
{
	json_t* jps = json_object_get(j, PATCH_KEY_PLUGIN_MIDI_IN_PARAMS);
	if (jps == nullptr)
		return;

	size_t  i;
	json_t* jp;
	json_array_foreach(jps, i, jp)
		params.push_back(json_integer_value(jp));
}


/* -------------------------------------------------------------------------- */


void readPlugins_(json_t* j)
{
	namespace uj = u::json;

	json_t* jps = json_object_get(j, PATCH_KEY_PLUGINS);
	if (jps == nullptr)
		return;

	size_t  i;
	json_t* jp;
	json_array_foreach(jps, i, jp) {
		
		if (!uj::isObject(jp))
			continue;

		Plugin p;
		p.id     = uj::readInt   (jp, PATCH_KEY_PLUGIN_ID);
		p.path   = uj::readString(jp, PATCH_KEY_PLUGIN_PATH);
		p.bypass = uj::readBool  (jp, PATCH_KEY_PLUGIN_BYPASS);

		readPluginParams_(jp, p.params);
		readMidiInPluginParams_(jp, p.midiInParams);

		model::plugins.push(std::move(pluginManager::makePlugin(p)));
	}
}

#endif

/* -------------------------------------------------------------------------- */


void readWaves_(json_t* j, const std::string& basePath)
{
	namespace uj = u::json;

	json_t* jws = json_object_get(j, PATCH_KEY_WAVES);
	if (jws == nullptr)
		return;

	size_t  i;
	json_t* jw;
	json_array_foreach(jws, i, jw) {
		
		if (!uj::isObject(jw))
			continue;

		Wave w;
		w.id   = uj::readInt(jw, PATCH_KEY_WAVE_ID);
		w.path = basePath + uj::readString(jw, PATCH_KEY_WAVE_PATH);

		model::waves.push(std::move(waveManager::createFromPatch(w)));
	}
	return;
}


/* -------------------------------------------------------------------------- */


void readActions_(json_t* j)
{
	namespace uj = u::json;

	json_t* jas = json_object_get(j, PATCH_KEY_ACTIONS);
	if (jas == nullptr)
		return;

	std::vector<Action> actions;
	size_t  i;
	json_t* ja;
	json_array_foreach(jas, i, ja) {
		
		if (!uj::isObject(ja))
			continue;

		Action a;
		a.id        = uj::readInt(ja, G_PATCH_KEY_ACTION_ID);
		a.channelId = uj::readInt(ja, G_PATCH_KEY_ACTION_CHANNEL);
		a.frame     = uj::readInt(ja, G_PATCH_KEY_ACTION_FRAME);
		a.event     = uj::readInt(ja, G_PATCH_KEY_ACTION_EVENT);
		a.prevId    = uj::readInt(ja, G_PATCH_KEY_ACTION_PREV);
		a.nextId    = uj::readInt(ja, G_PATCH_KEY_ACTION_NEXT);

		actions.push_back(a);
	}

	model::onSwap(model::actions, [&](model::Actions& a)
	{
		a.map = std::move(recorderHandler::makeActionsFromPatch(actions));
	});
}


/* -------------------------------------------------------------------------- */


void readChannelPlugins_(json_t* j, std::vector<ID>& pluginIds)
{
	json_t* jps = json_object_get(j, PATCH_KEY_CHANNEL_PLUGINS);
	if (jps == nullptr)
		return;

	size_t  i;
	json_t* jp;
	json_array_foreach(jps, i, jp)
		pluginIds.push_back(json_integer_value(jp));
}


/* -------------------------------------------------------------------------- */


void readChannels_(json_t* j)
{
	namespace uj = u::json;

	json_t* jcs = json_object_get(j, PATCH_KEY_CHANNELS);
	if (jcs == nullptr)
		return;

	size_t  i;
	json_t* jc;
	json_array_foreach(jcs, i, jc) {

		if (!uj::isObject(jc))
			continue;

		Channel c;
		c.id     = uj::readInt  (jc, PATCH_KEY_CHANNEL_ID);
		c.type   = static_cast<ChannelType>(uj::readInt(jc, PATCH_KEY_CHANNEL_TYPE));
		c.volume = uj::readFloat(jc, PATCH_KEY_CHANNEL_VOLUME);
		
		if (c.type != ChannelType::MASTER) {
			c.size              = uj::readInt   (jc, PATCH_KEY_CHANNEL_SIZE);
			c.name              = uj::readString(jc, PATCH_KEY_CHANNEL_NAME);
			c.columnId          = uj::readInt   (jc, PATCH_KEY_CHANNEL_COLUMN);
			c.key               = uj::readInt   (jc, PATCH_KEY_CHANNEL_KEY);
			c.mute              = uj::readInt   (jc, PATCH_KEY_CHANNEL_MUTE);
			c.solo              = uj::readInt   (jc, PATCH_KEY_CHANNEL_SOLO);
			c.pan               = uj::readFloat (jc, PATCH_KEY_CHANNEL_PAN);
			c.hasActions        = uj::readBool  (jc, PATCH_KEY_CHANNEL_HAS_ACTIONS);
			c.midiIn            = uj::readBool  (jc, PATCH_KEY_CHANNEL_MIDI_IN);
			c.midiInKeyPress    = uj::readInt   (jc, PATCH_KEY_CHANNEL_MIDI_IN_KEYPRESS);
			c.midiInKeyRel      = uj::readInt   (jc, PATCH_KEY_CHANNEL_MIDI_IN_KEYREL);
			c.midiInKill        = uj::readInt   (jc, PATCH_KEY_CHANNEL_MIDI_IN_KILL);
			c.midiInArm         = uj::readInt   (jc, PATCH_KEY_CHANNEL_MIDI_IN_ARM);
			c.midiInVolume      = uj::readInt   (jc, PATCH_KEY_CHANNEL_MIDI_IN_VOLUME);
			c.midiInMute        = uj::readInt   (jc, PATCH_KEY_CHANNEL_MIDI_IN_MUTE);
			c.midiInSolo        = uj::readInt   (jc, PATCH_KEY_CHANNEL_MIDI_IN_SOLO);
			c.midiInFilter      = uj::readInt   (jc, PATCH_KEY_CHANNEL_MIDI_IN_FILTER);
			c.midiOutL          = uj::readBool  (jc, PATCH_KEY_CHANNEL_MIDI_OUT_L);
			c.midiOutLplaying   = uj::readInt   (jc, PATCH_KEY_CHANNEL_MIDI_OUT_L_PLAYING);
			c.midiOutLmute      = uj::readInt   (jc, PATCH_KEY_CHANNEL_MIDI_OUT_L_MUTE);
			c.midiOutLsolo      = uj::readInt   (jc, PATCH_KEY_CHANNEL_MIDI_OUT_L_SOLO);
			c.armed             = uj::readBool  (jc, PATCH_KEY_CHANNEL_ARMED);
		}

#ifdef WITH_VST
		readChannelPlugins_(jc, c.pluginIds);
#endif

		if (c.type == ChannelType::SAMPLE) {
			c.waveId            = uj::readInt  (jc, PATCH_KEY_CHANNEL_WAVE_ID);
			c.mode              = static_cast<ChannelMode>(uj::readInt(jc, PATCH_KEY_CHANNEL_MODE));
			c.begin             = uj::readInt  (jc, PATCH_KEY_CHANNEL_BEGIN);
			c.end               = uj::readInt  (jc, PATCH_KEY_CHANNEL_END);
			c.readActions       = uj::readBool (jc, PATCH_KEY_CHANNEL_READ_ACTIONS);
			c.pitch             = uj::readFloat(jc, PATCH_KEY_CHANNEL_PITCH);
			c.inputMonitor      = uj::readBool (jc, PATCH_KEY_CHANNEL_INPUT_MONITOR);
			c.midiInVeloAsVol   = uj::readBool (jc, PATCH_KEY_CHANNEL_MIDI_IN_VELO_AS_VOL);
			c.midiInReadActions = uj::readInt  (jc, PATCH_KEY_CHANNEL_MIDI_IN_READ_ACTIONS);
			c.midiInPitch       = uj::readInt  (jc, PATCH_KEY_CHANNEL_MIDI_IN_PITCH);
		}
		else
		if (c.type == ChannelType::MIDI) {
			c.midiOut     = uj::readInt(jc, PATCH_KEY_CHANNEL_MIDI_OUT);
			c.midiOutChan = uj::readInt(jc, PATCH_KEY_CHANNEL_MIDI_OUT_CHAN);
		}

		sanitize_(c);
		
		if (c.type == ChannelType::MASTER || c.type == ChannelType::PREVIEW) {
			if (c.id == mixer::MASTER_OUT_CHANNEL_ID)
				model::onSwap(model::channels, mixer::MASTER_OUT_CHANNEL_ID, [&](m::Channel& ch) { ch.load(c); });
			else
			if (c.id == mixer::MASTER_IN_CHANNEL_ID)
				model::onSwap(model::channels, mixer::MASTER_IN_CHANNEL_ID, [&](m::Channel& ch) { ch.load(c); });
		}
		else
			model::channels.push(channelManager::create(c, conf::buffersize));
	}
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
		json_object_set_new(jc, PATCH_KEY_COLUMN_ID, json_integer(c.id));
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


void writeWaves_(json_t* j, bool isProject)
{
	model::WavesLock l(model::waves);

	json_t* jws = json_array();

	for (const m::Wave* w : model::waves) {
	
		std::string path = isProject ? gu_basename(w->getPath()) : w->getPath();

		json_t* jw = json_object();
		json_object_set_new(jw, PATCH_KEY_WAVE_ID,   json_integer(w->id));
		json_object_set_new(jw, PATCH_KEY_WAVE_PATH, json_string(path.c_str()));
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
	json_object_set_new(j, PATCH_KEY_VERSION_MAJOR,  json_integer(G_VERSION_MAJOR));
	json_object_set_new(j, PATCH_KEY_VERSION_MINOR,  json_integer(G_VERSION_MINOR));
	json_object_set_new(j, PATCH_KEY_VERSION_PATCH,  json_integer(G_VERSION_PATCH));
	json_object_set_new(j, PATCH_KEY_NAME,           json_string(name.c_str()));
	json_object_set_new(j, PATCH_KEY_BARS,           json_integer(model::clock.get()->bars));
	json_object_set_new(j, PATCH_KEY_BEATS,          json_integer(model::clock.get()->beats));
	json_object_set_new(j, PATCH_KEY_BPM,            json_real(model::clock.get()->bpm));
	json_object_set_new(j, PATCH_KEY_QUANTIZE,       json_integer(model::clock.get()->quantize));
	json_object_set_new(j, PATCH_KEY_LAST_TAKE_ID,   json_integer(lastTakeId));
	json_object_set_new(j, PATCH_KEY_SAMPLERATE,     json_integer(samplerate));
	json_object_set_new(j, PATCH_KEY_METRONOME,      json_boolean(mixer::isMetronomeOn()));
}


/* -------------------------------------------------------------------------- */


void writeChannels_(json_t* j)
{
	model::ChannelsLock l(model::channels);

	json_t* jcs = json_array();

	for (m::Channel* c : model::channels) {

		json_t* jc = json_object();

		json_object_set_new(jc, PATCH_KEY_CHANNEL_ID,     json_integer(c->id));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_TYPE,   json_integer(static_cast<int>(c->type)));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_VOLUME, json_real(c->volume));

		if (c->type != ChannelType::MASTER) {
			json_object_set_new(jc, PATCH_KEY_CHANNEL_SIZE,               json_integer(G_MainWin->keyboard->getChannel(c->id)->getSize()));
			json_object_set_new(jc, PATCH_KEY_CHANNEL_NAME,               json_string(c->name.c_str()));
			json_object_set_new(jc, PATCH_KEY_CHANNEL_COLUMN,             json_integer(c->columnId));
			json_object_set_new(jc, PATCH_KEY_CHANNEL_MUTE,               json_integer(c->mute));
			json_object_set_new(jc, PATCH_KEY_CHANNEL_SOLO,               json_integer(c->solo));
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
		}	

#ifdef WITH_VST
		json_t* jplugins = json_array();
		for (ID pid : c->pluginIds)
			json_array_append_new(jplugins, json_integer(pid));
		json_object_set_new(jc, PATCH_KEY_CHANNEL_PLUGINS, jplugins);
#endif

		if (c->type == ChannelType::SAMPLE) {
			SampleChannel* sc = static_cast<SampleChannel*>(c);

			json_object_set_new(jc, PATCH_KEY_CHANNEL_WAVE_ID,              json_integer(sc->waveId));
			json_object_set_new(jc, PATCH_KEY_CHANNEL_MODE,                 json_integer(static_cast<int>(sc->mode)));
			json_object_set_new(jc, PATCH_KEY_CHANNEL_BEGIN,                json_integer(sc->begin));
			json_object_set_new(jc, PATCH_KEY_CHANNEL_END,                  json_integer(sc->end));
			json_object_set_new(jc, PATCH_KEY_CHANNEL_READ_ACTIONS,         json_boolean(sc->readActions));
			json_object_set_new(jc, PATCH_KEY_CHANNEL_PITCH,                json_real(sc->pitch));
			json_object_set_new(jc, PATCH_KEY_CHANNEL_INPUT_MONITOR,        json_boolean(sc->inputMonitor));
			json_object_set_new(jc, PATCH_KEY_CHANNEL_MIDI_IN_VELO_AS_VOL,  json_boolean(sc->midiInVeloAsVol));
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
int         samplerate;
int         lastTakeId;
bool        metronome;


/* -------------------------------------------------------------------------- */


bool Version::operator ==(const Version& o) const
{
	return major == o.major && minor == o.minor && patch == o.patch;	
}


bool Version::operator <(const Version& o) const
{
	if (major < o.major) return true;
	if (minor < o.minor) return true;
	if (patch < o.patch) return true;
	return false;
}


/* -------------------------------------------------------------------------- */


void init()
{
	lastTakeId = 0;
	samplerate = G_DEFAULT_SAMPLERATE;
}


/* -------------------------------------------------------------------------- */


int verify(const std::string& file)
{
	namespace uj = u::json;

	json_t* j = uj::load(file);
	if (j == nullptr)
		return G_PATCH_UNREADABLE;

	if (uj::readString(j, PATCH_KEY_HEADER) != "GIADAPTC")
		return G_PATCH_INVALID;
	
	Version version = {
		static_cast<int>(uj::readInt(j, PATCH_KEY_VERSION_MAJOR)),
		static_cast<int>(uj::readInt(j, PATCH_KEY_VERSION_MINOR)),
		static_cast<int>(uj::readInt(j, PATCH_KEY_VERSION_PATCH))
	};
	if (version < Version{0, 16, 0})
		return G_PATCH_UNSUPPORTED;
	
	return G_PATCH_OK;
}


/* -------------------------------------------------------------------------- */


bool write(const std::string& name, const std::string& file, bool isProject)
{
	json_t* j = json_object();

	writeCommons_(j, name);
	writeColumns_(j);
	writeChannels_(j);
	writeActions_(j);
	writeWaves_(j, isProject);
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


int read(const std::string& file, const std::string& basePath)
{
	namespace uj = u::json;

	json_t* j = uj::load(file);
	if (j == nullptr) 
		return G_PATCH_UNREADABLE;
	
	init();
	readCommons_(j);
#ifdef WITH_VST
	readPlugins_(j);
#endif
	readWaves_(j, basePath);
	readActions_(j);
	readChannels_(j);

	json_decref(j);

	sanitize_();

	return G_PATCH_OK;
}
}}}; // giada::m::patch::
