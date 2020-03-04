/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * utils
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#include "../src/core/channels/channel_NEW.h"
#include "../src/core/channels/samplePlayer.h"
#include "../src/core/audioBuffer.h"
#include "../src/core/types.h"
#include "../src/core/wave.h"
#include "../src/core/mixer.h"
#include <catch.hpp>


TEST_CASE("SamplePlayer")
{
    using namespace giada;
    using namespace giada::m;

    AudioBuffer buffer;
    buffer.alloc(/*size=*/30, /*channels=*/1);

    Wave w(0);
    w.alloc(/*size=*/10, /*channels=*/1, 44100, 16, "");

    ChannelState chs;
    Channel_NEW  ch(chs);

    SamplePlayerState sps;
    SamplePlayer      sp(sps, ch);

    sp.loadWave(&w);

	SECTION("Test wave loading")
	{
        REQUIRE(sp.shift == 0);
        REQUIRE(sp.begin == 0);
        REQUIRE(sp.end == w.getSize() - 1);

        sp.loadWave(nullptr);

        REQUIRE(sp.shift == 0);
        REQUIRE(sp.begin == 0);
        REQUIRE(sp.end == 0);
    }

	SECTION("Test rendering")
	{
        SECTION("One-shot mode")
        {   
            /* Wave is smaller than audiobuffer, so in SINGLE_BASIC mode it should
            reset on end. */

            sp.mode = SamplePlayer::Mode::SINGLE_BASIC;
            sp.render(buffer);

            REQUIRE(sps.tracker.load() == 0);
        }

        SECTION("Loop mode")
        {   
            /* Test rendering in loop mode. Tracker should loop. */

            sp.mode = SamplePlayer::Mode::LOOP_REPEAT;
            sp.render(buffer);
            // TODO
            //REQUIRE(sps.tracker.load() == 10);
        }
    }

	SECTION("Events")
	{
        SECTION("On bar")
        {
            mixer::FrameEvents fe = {
                .frameLocal   = 2,
                .frameGlobal  = 12,
                .doQuantize   = false,
                .onBar        = true,
                .onFirstBeat  = false,
                .quantoPassed = false,
                .actions      = nullptr,
            };

            SECTION("Channel playing, LOOP_REPEAT mode");
            {
                chs.status.store(ChannelStatus::PLAY);
                sp.mode = SamplePlayer::Mode::LOOP_REPEAT;

                sp.parse(fe);
                sp.render(buffer);

                REQUIRE(sps.rewinding == true);
                REQUIRE(sps.offset    == fe.frameLocal);
            }

            SECTION("Channel waiting, LOOP_ONCE_BAR mode");
            {
                chs.status.store(ChannelStatus::WAIT);
                sp.mode = SamplePlayer::Mode::LOOP_ONCE_BAR;

                sp.parse(fe);
                sp.render(buffer);

                REQUIRE(sps.offset == fe.frameLocal);
            }
        }        
    }
}
