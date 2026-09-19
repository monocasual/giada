/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef GE_WAVEFORM_H
#define GE_WAVEFORM_H

#include "src/deps/geompp/src/range.hpp"
#include "src/types.h"
#include <FL/Fl_Widget.H>
#include <utility>
#include <vector>

namespace mcl
{
class AudioBuffer;
}

namespace giada::m
{
class Wave;
}

namespace giada::c::sampleEditor
{
struct Data;
}

namespace giada::v
{
class geWaveform : public Fl_Widget
{
public:
	geWaveform(const c::sampleEditor::Data&);

	void draw() override;

	geompp::Range<Frame> getMoveableRange() const;

	void rebuild();
	void zoomIn();
	void zoomOut();

	/* moveToFrame
	Moves the visible range to start at the given absolute frame. The value is
	clamped to the valid range of start frames, so the visible window can never
	run past the beginning or the end of the buffer. */

	void moveToFrame(Frame);

	/* moveToFraction
	Moves the visible range given a normalized scrollbar position in [0.0, 1.0],
	where 0.0 places the range at the very start of the buffer and 1.0 at the
	very end. */

	void moveToFraction(double);

private:
	class Data
	{
	public:
		struct Peak
		{
			float up;
			float down;
		};

		using Peaks = std::vector<std::vector<Peak>>;

		Data() = default;
		Data(const mcl::AudioBuffer&, geompp::Range<Frame>, int width);

		const Peaks& get() const;
		double       getRatio() const;

	private:
		/* getPeak (1)
		Returns the Peak pair given a range of audio data to parse from the audio
		buffer. It computes the highest/lowest value in that audio range. Used when
		ratio >= 1. */

		Peak getPeak(const mcl::AudioBuffer&, geompp::Range<double>, double steps, int channel) const;

		/* getPeak (2)
		Returs the Peak pair given a single frame of audio data to read from
		the audio buffer. Used when ratio < 1. */

		Peak getPeak(const mcl::AudioBuffer&, Frame, int channel) const;

		Peaks  m_peaks;
		double m_ratio;
	};

	void invalidate();
	void drawWaveform();

	const c::sampleEditor::Data& m_sampleEditorData;
	geompp::Range<Frame>         m_range;
	Data                         m_data;
};
} // namespace giada::v

#endif
