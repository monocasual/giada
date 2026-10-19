/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_RENDERING_SAMPLE_RENDERING_H
#define G_RENDERING_SAMPLE_RENDERING_H

#include "src/core/channels/channel.h"
#include "src/core/model/channel.h"
#include "src/core/rendering/renderInfo.h"
#include "src/scene.h"
#include "src/types.h"
#include <functional>

namespace mcl
{
class AudioBuffer;
}

namespace giada::m
{
class Wave;
class Resampler;
class Stretcher;
class PluginHost;
} // namespace giada::m

namespace giada::m::rendering
{
/* ReadResult
A ReadResult object is returned by the readWave() function below, containing the
number of frames used and generated from a buffer filling operation. The two
values are different only when pitch is != 1.0, where a chunk of audio in input
(used) might result in a longer or shorter portion of audio in output (generated).
'finished', used only by the Stretcher class, determines whether the stretch
process has really ended or not - a stretch operation might take more than one
callback, especially if the stretching factor is high. */

struct ReadResult
{
	Frame used      = 0;
	Frame generated = 0;
	bool  finished  = false;
};

void renderSampleChannel(
    const model::Channel&,
    const model::Channel::Parameters&,
    const model::Channel::Rendering&,
    const model::Wave*,
    RenderInfo,
    Scene,
    bool seqIsRunning);

/* renderSampleChannelInput
Copies input buffer to channel buffer: this enables the input monitoring. */

void renderSampleChannelInput(const Channel&, const mcl::AudioBuffer&);

ReadResult readWave(
    const model::Channel::Rendering&,
    const model::Wave&,
    const Sample&,
    Frame start,
    Frame offset);
} // namespace giada::m::rendering

#endif
