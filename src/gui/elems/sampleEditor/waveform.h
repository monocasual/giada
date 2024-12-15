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

#include "core/types.h"
#include "deps/geompp/src/range.hpp"
#include <FL/Fl_Widget.H>
#include <utility>
#include <vector>

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
	geWaveform();

	void draw() override;

	void rebuild(const c::sampleEditor::Data&);
	void zoomIn();
	void zoomOut();
	void setStartRange(Frame);

private:
	class Data
	{
	public:
		using Peak  = std::pair<float, float>;
		using Peaks = std::vector<std::vector<Peak>>;

		Data() = default;
		Data(const m::Wave&, geompp::Range<Frame>, int width);

		const Peaks& get() const;
		double       getRatio() const;

	private:
		/* getPeak (1)
		Returns the Peak pair given a range of audio data to parse from Wave.
		It computes the highest/lowest value in that audio range. Used when
		ratio >= 1. */

		Peak getPeak(const m::Wave&, geompp::Range<double>, double steps, int channel) const;

		/* getPeak (2)
		Returs the Peak pair given a single frame of audio data to read from
		Wave. Used when ratio < 1. */

		Peak getPeak(const m::Wave&, Frame, int channel) const;

		Peaks  m_peaks;
		double m_ratio;
	};

	void invalidate();
	void drawWaveform();

	const c::sampleEditor::Data* m_sampleEditorData;
	geompp::Range<Frame>         m_range;
	Data                         m_data;
};
} // namespace giada::v

#endif
