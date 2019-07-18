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
RCUList<Plugin>   plugins;


Actions::Actions(const Actions& o) : map(o.map)
{
	/* Needs to update all pointers of prev and next actions with addresses 
	coming from the new 'actions' map.  */

	recorder::updateMapPointers(map);
}


#ifndef NDEBUG

void debug()
{
	ChannelsLock cl(channels);
	WavesLock    wl(waves);
	PluginsLock  pl(plugins);

	puts("======== SYSTEM STATUS ========");
	
	puts("model::channel");

	int i = 0;
	for (const Channel* c : channels) {
		printf("    %d) %p - ID=%d name=%s\n", i++, (void*)c, c->id, c->name.c_str());
		if (c->hasData())
			printf("        wave: %d\n", static_cast<const SampleChannel*>(c)->waveId);
		if (c->pluginIds.size() > 0) {
			puts("        plugins:");
			for (ID id : c->pluginIds)
				printf("            ID=%d\n", id);
		}

	}

	puts("model::waves");

	i = 0;
	for (const Wave* w : waves) 
		printf("    %d) %p - ID=%d name=%s\n", i++, (void*)w, w->id, w->getPath().c_str());
		
	puts("model::plugins");

	i = 0;
	for (const Plugin* p : plugins) {
		printf("    %d) %p - ID=%d name=%s\n", i++, (void*)p, p->id, p->getName().c_str());
	}
	
	puts("===============================");
}

#endif
}}} // giada::m::model::
