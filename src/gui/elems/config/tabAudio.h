/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/deps/geompp/src/rect.hpp"
#include "src/glue/config.h"
#include "src/gui/elems/basics/choice.h"
#include <FL/Fl_Group.H>

namespace giada::v
{
class geCheck;
class geInput;
class geTextButton;
class geTabAudio : public Fl_Group
{
public:
	struct geDeviceMenu : public geChoice
	{
		geDeviceMenu();

		void rebuild(const std::vector<c::config::AudioDeviceData>&);
	};

	struct geChannelMenu : public geChoice
	{
		geChannelMenu();

		int getChannelsCount() const;
		int getChannelsStart() const;

		void rebuild(const c::config::AudioDeviceData&);

	private:
		static constexpr int STEREO_OFFSET = 1000;
	};

	geTabAudio(geompp::Rect<int>);

private:
	void rebuild(const c::config::AudioData&);
	void refreshDevOutProperties();
	void refreshChannelOutProperties();
	void refreshDevInProperties();
	void deactivateAll();
	void activateAll();

	c::config::AudioData m_data;

	geChoice*      m_api;
	geChoice*      m_bufferSize;
	geChoice*      m_sampleRate;
	geDeviceMenu*  m_sounddevOut;
	geChannelMenu* m_channelsOut;
	geInput*       m_numOutChannels;
	geCheck*       m_limitOutput;
	geDeviceMenu*  m_sounddevIn;
	geCheck*       m_enableIn;
	geChannelMenu* m_channelsIn;
	geInput*       m_recTriggerLevel;
	geChoice*      m_rsmpQuality;
	geTextButton*  m_applyBtn;
};
} // namespace giada::v

#endif
