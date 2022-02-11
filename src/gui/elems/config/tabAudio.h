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

#ifndef GE_TAB_AUDIO_H
#define GE_TAB_AUDIO_H

#include "glue/config.h"
#include "gui/elems/basics/choice.h"
#include <FL/Fl_Group.H>

class geCheck;
class geInput;

namespace giada::v
{
class geButton;
class geTabAudio : public Fl_Group
{
public:
	struct geDeviceMenu : public geChoice
	{
		geDeviceMenu(int x, int y, int w, int h, const char* l, const std::vector<c::config::AudioDeviceData>&);
	};

	struct geChannelMenu : public geChoice
	{
		geChannelMenu(int x, int y, int w, int h, const char* l, const c::config::AudioDeviceData&);

		int getChannelsCount() const;
		int getChannelsStart() const;

		void rebuild(const c::config::AudioDeviceData&);

	private:
		static constexpr int STEREO_OFFSET = 1000;

		c::config::AudioDeviceData m_data;
	};

	geTabAudio(int x, int y, int w, int h);

	void save();

	geChoice*      soundsys;
	geChoice*      buffersize;
	geChoice*      samplerate;
	geDeviceMenu*  sounddevOut;
	geChannelMenu* channelsOut;
	geCheck*       limitOutput;
	geDeviceMenu*  sounddevIn;
	geCheck*       enableIn;
	geChannelMenu* channelsIn;
	geInput*       recTriggerLevel;
	geChoice*      rsmpQuality;

private:
	void invalidate();
	void fetch();
	void deactivateAll();
	void activateAll();

	c::config::AudioData m_data;

	int m_initialApi;
};
} // namespace giada::v

#endif
