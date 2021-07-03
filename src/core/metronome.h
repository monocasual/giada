/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#ifndef G_METRONOME_H
#define G_METRONOME_H

#include "core/types.h"

namespace mcl
{
class AudioBuffer;
}
namespace giada::m
{
class Metronome
{
public:
	enum class Click
	{
		BEAT,
		BAR
	};

	void render(mcl::AudioBuffer& outBuf);
	void trigger(Click c, Frame o);

	bool running = false;

private:
	static constexpr Frame CLICK_SIZE = 38;

	static constexpr float beat[CLICK_SIZE] = {
	    0.059033f, 0.117240f, 0.173807f, 0.227943f, 0.278890f, 0.325936f,
	    0.368423f, 0.405755f, 0.437413f, 0.462951f, 0.482013f, 0.494333f,
	    0.499738f, 0.498153f, 0.489598f, 0.474195f, 0.452159f, 0.423798f,
	    0.389509f, 0.349771f, 0.289883f, 0.230617f, 0.173194f, 0.118739f,
	    0.068260f, 0.022631f, -0.017423f, -0.051339f, -0.078721f, -0.099345f,
	    -0.113163f, -0.120295f, -0.121028f, -0.115804f, -0.105209f, -0.089954f,
	    -0.070862f, -0.048844f};

	static constexpr float bar[CLICK_SIZE] = {
	    0.175860f, 0.341914f, 0.488904f, 0.608633f, 0.694426f, 0.741500f,
	    0.747229f, 0.711293f, 0.635697f, 0.524656f, 0.384362f, 0.222636f,
	    0.048496f, -0.128348f, -0.298035f, -0.451105f, -0.579021f, -0.674653f,
	    -0.732667f, -0.749830f, -0.688924f, -0.594091f, -0.474481f, -0.340160f,
	    -0.201360f, -0.067752f, 0.052194f, 0.151746f, 0.226280f, 0.273493f,
	    0.293425f, 0.288307f, 0.262252f, 0.220811f, 0.170435f, 0.117887f,
	    0.069639f, 0.031320f};

	Frame m_tracker   = 0;
	Frame m_offset    = 0;
	bool  m_rendering = false;
	Click m_click     = Click::BEAT;
};
} // namespace giada::m

#endif
