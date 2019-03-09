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


#ifndef G_RENDER_DATA_H
#define G_RENDER_DATA_H


#include <memory>
#include <vector>
#include "core/types.h"
#include "core/recorder.h"
#include "core/wave.h"
#include "core/plugin.h"
#include "core/pluginHost.h"


namespace giada {
namespace m
{
class AudioBuffer;
class Channel;
namespace model
{
struct Layout
{
	Layout();
	Layout(const Layout& o);
	~Layout();

	void parse(Frame f);
	void render(AudioBuffer& out, const AudioBuffer& in, AudioBuffer& inToOut);

	Channel* getChannel(ID id);
	Plugin* getPlugin(ID pluginID, ID chanID);
	pluginHost::Stack* getPlugins(ID chanID);
	
	int   bars;
	int   beats;
	float bpm;
	int   quantize;
	
	recorder::ActionMap actions;
	std::vector<std::unique_ptr<Channel>> channels;
};


/* -------------------------------------------------------------------------- */


struct Data
{
	std::map<ID, std::shared_ptr<Wave>> waves;
#ifdef WITH_VST
	std::map<ID, pluginHost::Stack> plugins;
#endif
};
}}} // giada::m::model::

#endif
