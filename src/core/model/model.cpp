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


#include "core/model/model.h"
#ifndef NDEBUG
#include "core/channels/channel.h"
#endif


namespace giada {
namespace m {
namespace model
{
std::atomic<bool> changed(false);
std::shared_ptr<Layout> layout_ = std::make_shared<Layout>();
Data data_;


/* -------------------------------------------------------------------------- */


std::shared_ptr<Layout> getLayout()
{
    return std::atomic_load(&layout_);
}


Data& getData()
{
    return data_;
}


/* -------------------------------------------------------------------------- */


std::shared_ptr<Layout> cloneLayout()
{
    return std::make_shared<Layout>(*layout_);
}


/* -------------------------------------------------------------------------- */


void swapLayout(std::shared_ptr<Layout> newLayout)
{
    std::shared_ptr<Layout> oldLayout = std::atomic_load(&layout_);
    while (!std::atomic_compare_exchange_weak(&layout_, &oldLayout, newLayout));
    changed.store(true);
}


/* -------------------------------------------------------------------------- */


#ifndef NDEBUG

void debug()
{
	puts("======== SYSTEM STATUS ========");
	
	puts("model::Data");
	
	printf("  waves=%ld\n", data_.waves.size());
	for (const auto& kv : data_.waves) 
		printf("    [chanId=%d] %p - %s\n", kv.first, (void*)kv.second.get(), kv.second->getPath().c_str());
	
	printf("  plugins=%ld\n", data_.plugins.size());
	for (const auto& kv : data_.plugins) {
		printf("    [chanId=%d]\n", kv.first);
		for (const auto& p : kv.second)
			printf("      id=%d - %p %s\n", p->id, (void*)p.get(), p->getName().c_str());
	}

	puts("");

	puts("model::Layout");

	printf("  channels=%ld\n", layout_->channels.size());
	for (const auto& ch : layout_->channels) {
		printf("    id=%d - %p\n", ch->id, (void*)ch.get());
		printf("      plugins=%ld\n", ch->plugins.size());
		for (const auto& p : ch->plugins)
			printf("        id=%d - %p %s\n", p->id, (void*)p.get(), p->getName().c_str());
	}

	printf("  bars=%d\n",     layout_->bars);
	printf("  beats=%d\n",    layout_->beats);
	printf("  bpm=%f\n",      layout_->bpm);
	printf("  quantize=%d\n", layout_->quantize);
	
	puts("===============================");
}

#endif
}}} // giada::m::model::
