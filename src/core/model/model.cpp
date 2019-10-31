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
#include "core/model/model.h"
#ifndef NDEBUG
#include "core/channels/channel.h"
#include "core/channels/sampleChannel.h"
#include "core/channels/channelManager.h"
#endif


namespace giada {
namespace m {
namespace model
{
RCUList<Clock>    clock(std::make_unique<Clock>());
RCUList<Mixer>    mixer(std::make_unique<Mixer>());
RCUList<Kernel>   kernel(std::make_unique<Kernel>());
RCUList<Recorder> recorder(std::make_unique<Recorder>());
RCUList<Actions>  actions(std::make_unique<Actions>());
RCUList<Channel>  channels;
RCUList<Wave>     waves;
#ifdef WITH_VST
RCUList<Plugin>   plugins;
#endif


Actions::Actions(const Actions& o) : map(o.map)
{
	/* Needs to update all pointers of prev and next actions with addresses 
	coming from the new 'actions' map.  */

	recorder::updateMapPointers(map);
}


#ifndef NDEBUG

void debug()
{
	ChannelsLock chl(channels);
	ClockLock    cl(clock);
	WavesLock    wl(waves);
	ActionsLock  al(actions);
#ifdef WITH_VST
	PluginsLock  pl(plugins);
#endif

	puts("======== SYSTEM STATUS ========");
	
	puts("model::channels");

	int i = 0;
	for (const Channel* c : channels) {
		printf("    %d) %p - ID=%d name='%s' columnID=%d\n", i++, (void*)c, c->id, c->name.c_str(), c->columnId);
		if (c->hasData())
			printf("        wave: ID=%d\n", static_cast<const SampleChannel*>(c)->waveId);
#ifdef WITH_VST
		if (c->pluginIds.size() > 0) {
			puts("        plugins:");
			for (ID id : c->pluginIds)
				printf("            ID=%d\n", id);
		}
#endif
	}

	puts("model::waves");

	i = 0;
	for (const Wave* w : waves) 
		printf("    %d) %p - ID=%d name='%s'\n", i++, (void*)w, w->id, w->getPath().c_str());
		
#ifdef WITH_VST
	puts("model::plugins");

	i = 0;
	for (const Plugin* p : plugins) {
		if (p->valid)
			printf("    %d) %p - ID=%d name='%s'\n", i++, (void*)p, p->id, p->getName().c_str());
		else
			printf("    %d) %p - ID=%d INVALID\n", i++, (void*)p, p->id); 
	}
#endif

	puts("model::clock");

	printf("    clock.status   = %d\n", static_cast<int>(clock.get()->status));
	printf("    clock.bars     = %d\n", clock.get()->bars);
	printf("    clock.beats    = %d\n", clock.get()->beats);
	printf("    clock.bpm      = %f\n", clock.get()->bpm);
	printf("    clock.quantize = %d\n", clock.get()->quantize);

	puts("model::actions");

	for (auto& kv : actions.get()->map) {
		printf("    frame: %d\n", kv.first);
		for (const Action& a : kv.second)
			printf("        (%p) - ID=%d, frame=%d, channel=%d, value=0x%X, prevId=%d, prev=%p, nextId=%d, next=%p\n", 
				(void*) &a, a.id, a.frame, a.channelId, a.event.getRaw(), a.prevId, (void*) a.prev, a.nextId, (void*) a.next);	
	}
	
	puts("===============================");
}

#endif
}}} // giada::m::model::
