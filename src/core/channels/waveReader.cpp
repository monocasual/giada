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


#include <memory>
#include <cassert>
#include <algorithm>
#include "core/const.h"
#include "core/model/model.h"
#include "core/audioBuffer.h"
#include "core/wave.h"
#include "utils/log.h"
#include "waveReader.h"


namespace giada::m
{
WaveReader::WaveReader()
: wave      (nullptr)
, m_srcState(nullptr)
{
	allocateSrc();
}


/* -------------------------------------------------------------------------- */


WaveReader::WaveReader(const WaveReader& o)
: wave      (o.wave)
, m_srcState(nullptr)
{
	allocateSrc();
}


/* -------------------------------------------------------------------------- */


WaveReader::WaveReader(WaveReader&& o)
: wave      (o.wave)
, m_srcState(nullptr)
{
	moveSrc(&o.m_srcState);
}
    

/* -------------------------------------------------------------------------- */

WaveReader& WaveReader::operator=(const WaveReader& o)
{
	if (this == &o) return *this;
	wave = o.wave;
	allocateSrc();
	return *this;
}


WaveReader& WaveReader::operator=(WaveReader&& o)
{
	if (this == &o) return *this;
	wave = o.wave;
	moveSrc(&o.m_srcState);
	return *this;
}


/* -------------------------------------------------------------------------- */


WaveReader::~WaveReader()
{
	if (m_srcState != nullptr)
		src_delete(m_srcState);    
}


/* -------------------------------------------------------------------------- */


WaveReader::Result WaveReader::fill(AudioBuffer& out, Frame start, Frame max, Frame offset, float pitch) const
{
	assert(wave != nullptr);
	assert(start >= 0);
	assert(max <= wave->getSize());
	assert(offset < out.countFrames());

	if (pitch == 1.0) return fillCopy(out, start, max, offset);
	else              return fillResampled(out, start, max, offset, pitch);
}


/* -------------------------------------------------------------------------- */


WaveReader::Result WaveReader::fillResampled(AudioBuffer& dest, Frame start, Frame max, Frame offset, float pitch) const
{
    SRC_DATA srcData;
	
	srcData.data_in       = wave->getFrame(start);        // Source data
	srcData.input_frames  = max - start;                  // How many readable frames in Wave
	srcData.data_out      = dest[offset];                 // Destination (processed data)
	srcData.output_frames = dest.countFrames() - offset;  // How many writable frames in dest
	srcData.end_of_input  = false;
	srcData.src_ratio     = 1 / pitch;

	src_process(m_srcState, &srcData);

	return {
	    static_cast<Frame>(srcData.input_frames_used),
        static_cast<Frame>(srcData.output_frames_gen)
	};
}


/* -------------------------------------------------------------------------- */


WaveReader::Result WaveReader::fillCopy(AudioBuffer& dest, Frame start, Frame max, Frame offset) const
{
	Frame used = dest.countFrames() - offset;
	if (used > max - start)
		used = max - start;

	dest.copyData(wave->getFrame(start), used, G_MAX_IO_CHANS, offset);

	return {used, used};
}


/* -------------------------------------------------------------------------- */


void WaveReader::allocateSrc()
{
	m_srcState = src_new(SRC_LINEAR, G_MAX_IO_CHANS, nullptr);
	if (m_srcState == nullptr) {
		u::log::print("[WaveReader] unable to allocate memory for SRC_STATE!\n");
		throw std::bad_alloc();
	}
}


/* -------------------------------------------------------------------------- */

void WaveReader::moveSrc(SRC_STATE** other)
{
	if (m_srcState != nullptr)
		src_delete(m_srcState);
	m_srcState = *other;
	*other = nullptr;
}
} // giada::m::
