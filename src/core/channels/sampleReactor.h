/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_CHANNEL_SAMPLE_REACTOR_H
#define G_CHANNEL_SAMPLE_REACTOR_H

#include "core/conf.h"
#include "core/eventDispatcher.h"

namespace giada::m::model
{
class Model;
}

namespace giada::m
{
class Channel;
struct ChannelShared;

/* SampleReactor
Reacts to manual events sent to Sample Channels: key press, key release, 
sequencer stop, ... . */

class SampleReactor final
{
public:
	struct Event
	{
		int   type;
		Frame offset;
	};

	SampleReactor(Channel&, ID channelId);

	void react(Channel&, const EventDispatcher::Event&, bool chansStopOnSeqHalt, bool canQuantize) const;

private:
	void          onStopBySeq(Channel&, bool chansStopOnSeqHalt) const;
	void          release(Channel&) const;
	void          press(Channel&, int velocity, bool canQuantize) const;
	ChannelStatus pressWhilePlay(Channel&, SamplePlayerMode, bool isLoop, bool canQuantize) const;
	ChannelStatus pressWhileOff(Channel&, int velocity, bool isLoop, bool canQuantize) const;
	void          rewind(ChannelShared&, Frame localFrame) const;
	void          play(ChannelShared&, Frame localFrame) const;
	void          stop(ChannelShared&) const;
};

} // namespace giada::m

#endif
