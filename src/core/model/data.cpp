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
#include "core/channels/channel.h"
#include "core/channels/channelManager.h"
#include "core/model/data.h"
#include "core/clock.h"
#include "core/mixer.h"
#include "core/conf.h"
#include "core/audioBuffer.h"
#include "core/pluginHost.h"


namespace giada {
namespace m {
namespace model
{
Layout::Layout()
: bars    (G_DEFAULT_BARS),
  beats   (G_DEFAULT_BEATS),
  bpm     (G_DEFAULT_BPM),
  quantize(G_DEFAULT_QUANTIZE)
{
}


/* -------------------------------------------------------------------------- */


Layout::Layout(const Layout& o)
: bars    (o.bars),
  beats   (o.beats),
  bpm     (o.bpm),
  quantize(o.quantize),
  actions (o.actions)
{
printf("Layout: %p\n", (void*)this);

	for (const std::unique_ptr<Channel>& c : o.channels)
		channels.push_back(c != nullptr ? channelManager::create(*c) : nullptr);

	/* Needs to update all pointers of prev and next actions with addresses 
	coming from the new 'actions' map.  */

	recorder::updateMapPointers(actions);
}


/* -------------------------------------------------------------------------- */


Layout::~Layout()
{
printf("~Layout: %p\n", (void*)this);
}


/* -------------------------------------------------------------------------- */


void Layout::parse(Frame f)
{
	mixer::FrameEvents fe = {
		.frameLocal   = f,
		.frameGlobal  = clock::getCurrentFrame(),
		.doQuantize   = clock::getQuantize() == 0 || !clock::quantoHasPassed(),
		.onBar        = clock::isOnBar(),
		.onFirstBeat  = clock::isOnFirstBeat(),
		.quantoPassed = clock::quantoHasPassed(),
		.actions      = recorder::getActionsOnFrame(clock::getCurrentFrame()),
	};

	for (std::unique_ptr<Channel>& channel : channels)
		channel->parseEvents(fe);   
}


/* -------------------------------------------------------------------------- */


void Layout::render(AudioBuffer& out, const AudioBuffer& in, AudioBuffer& inToOut)
{
	/* Master channels are processed at the end, when the buffers have already 
	been filled. */

	for (std::unique_ptr<Channel>& channel : channels) {
		if (channel == nullptr ||
			channel->id == mixer::MASTER_OUT_CHANNEL_ID ||
			channel->id == mixer::MASTER_IN_CHANNEL_ID)
			continue;
		channel->render(out, in, inToOut, mixer::isChannelAudible(channel.get()), 
			clock::isRunning());
	}

	channels[mixer::MASTER_OUT_CHANNEL_ID]->render(out, in, inToOut, true, true);
	channels[mixer::MASTER_IN_CHANNEL_ID]->render(out, in, inToOut, true, true);
}


/* -------------------------------------------------------------------------- */


pluginHost::Stack* Layout::getPlugins(ID chanID)
{
	return getChannel(chanID) == nullptr ? nullptr : &getChannel(chanID)->plugins;
}


/* -------------------------------------------------------------------------- */


Plugin* Layout::getPlugin(ID pluginID, ID chanID)
{
	Channel* c = getChannel(chanID);
	for (std::shared_ptr<Plugin>& p : c->plugins)
		if (p->id == pluginID)
			return p.get();
	return nullptr;
}


/* -------------------------------------------------------------------------- */


Channel* Layout::getChannel(ID id)
{
	for (std::unique_ptr<Channel>& channel : channels)
		if (channel->id == id)
			return channel.get();
	return nullptr;
}


}}} // giada::m::model::
