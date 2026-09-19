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

#include "src/gui/elems/sampleEditor/waveform.h"
#include "src/core/wave.h"
#include "src/deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include "src/deps/mcl-utils/src/math.hpp"
#include "src/glue/sampleEditor.h"
#include "src/gui/const.h"
#include "src/gui/drawing.h"
#include "src/utils/log.h"

namespace math = mcl::utils::math;

namespace giada::v
{
namespace
{
/* BORDER_PX
With of the border surrounding the viewport. */

constexpr int BORDER_PX = 1;
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geWaveform::Data::Data(const mcl::AudioBuffer& audioBuffer, geompp::Range<Frame> rangeAudio, int widthScreen)
{
	/* ratio
	Defines the ratio between the audio content and the screen width. If < 1.0,
	we are digging between individual samples. If >= 1.0, it also tells how many
	audio frames we should consider as a 'chunk' to use for a single column in
	screen space. */

	m_ratio = rangeAudio.getLength() / static_cast<double>(widthScreen);

	m_peaks.resize(audioBuffer.countChannels(), {});

	for (int channel = 0; channel < audioBuffer.countChannels(); channel++)
	{
		m_peaks[channel].resize(widthScreen, {0.0f, 0.0f});

		if (m_ratio >= 1)
		{
			for (int column = 0; column < widthScreen; column++)
			{
				const double startFrame  = rangeAudio.getA() + (column * m_ratio);
				m_peaks[channel][column] = getPeak(audioBuffer, {startFrame, startFrame + m_ratio}, m_ratio, channel);
			}
		}
		else
		{
			const double columnsPerFrame = widthScreen / static_cast<double>(rangeAudio.getLength());
			for (int frame = rangeAudio.getA(); frame < rangeAudio.getB(); frame++)
			{
				const int column         = static_cast<int>((frame - rangeAudio.getA()) * columnsPerFrame);
				m_peaks[channel][column] = getPeak(audioBuffer, frame, channel);
			}
		}
	}
}

/* -------------------------------------------------------------------------- */

geWaveform::Data::Peak geWaveform::Data::getPeak(const mcl::AudioBuffer& audioBuffer,
    geompp::Range<double> range, double steps, int channel) const
{
	float peakUp   = 0.0;
	float peakDown = 0.0;

	for (double frameD = range.getA(); frameD < std::floor(range.getA() + steps); frameD++)
	{
		const Frame frame = std::min(static_cast<Frame>(frameD), static_cast<Frame>(range.getB()) - 1);
		const float value = audioBuffer.at(frame, channel);
		peakUp            = std::max(value, peakUp);
		peakDown          = std::min(value, peakDown);
	}

	return {peakUp, peakDown};
}

/* -------------------------------------------------------------------------- */

geWaveform::Data::Peak geWaveform::Data::getPeak(const mcl::AudioBuffer& audioBuffer,
    Frame frame, int channel) const
{
	const float value    = audioBuffer.at(frame, channel);
	const float peakUp   = value >= 0.0f ? value : 0.0f;
	const float peakDown = value < 0.0f ? value : 0.0f;

	return {peakUp, peakDown};
}

/* -------------------------------------------------------------------------- */

const geWaveform::Data::Peaks& geWaveform::Data::get() const
{
	return m_peaks;
}

/* -------------------------------------------------------------------------- */

double geWaveform::Data::getRatio() const
{
	return m_ratio;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geWaveform::geWaveform(const c::sampleEditor::Data& data)
: Fl_Widget(0, 0, 0, 0)
, m_sampleEditorData(data)
{
}

/* -------------------------------------------------------------------------- */

void geWaveform::draw()
{
	const geompp::Rect<int> bounds = {x(), y(), w(), h()};

	v::drawRectf(bounds, G_COLOR_GREY_2); // Blank canvas
	v::drawRect(bounds, G_COLOR_GREY_4);  // Border

	if (m_data.get().size() == 0)
	{
		drawText("No audio data.", bounds, FL_HELVETICA, G_GUI_FONT_SIZE_BASE, G_COLOR_GREY_4);
		return;
	}

	drawWaveform();
}

/* -------------------------------------------------------------------------- */

geompp::Range<Frame> geWaveform::getMoveableRange() const
{
	const auto maxF   = m_sampleEditorData.getAudioBufferRef().countFrames();
	const auto hidden = maxF - m_range.getLength();

	if (hidden == 0)
		return {0, maxF};
	else
		return {0, hidden};
}

/* -------------------------------------------------------------------------- */

void geWaveform::rebuild()
{
	m_range = {0, m_sampleEditorData.getAudioBufferRef().countFrames()};
	invalidate();
}

/* -------------------------------------------------------------------------- */

void geWaveform::zoomIn()
{
	const Frame maxFrames = m_sampleEditorData.getAudioBufferRef().countFrames();
	const Frame newLength = std::max(m_range.getLength() / 2, 1);

	/* Zooming changes the window length, not its start: re-anchor 'a' first
	(clamped so the window never overflows the buffer), then resize. */

	m_range.move(std::min(m_range.getA(), maxFrames - newLength));
	m_range.setLength(newLength);

	invalidate();
	redraw();
}

/* -------------------------------------------------------------------------- */

void geWaveform::zoomOut()
{
	const Frame maxFrames = m_sampleEditorData.getAudioBufferRef().countFrames();
	const Frame newLength = std::min(m_range.getLength() * 2, maxFrames);

	/* Zooming changes the window length, not its start: re-anchor 'a' first
	(clamped so the window never overflows the buffer), then resize. */

	m_range.move(std::min(m_range.getA(), maxFrames - newLength));
	m_range.setLength(newLength);

	invalidate();
	redraw();
}

/* -------------------------------------------------------------------------- */

void geWaveform::moveToFrame(Frame start)
{
	const auto maxFrames = m_sampleEditorData.getAudioBufferRef().countFrames();
	const auto hidden    = maxFrames - m_range.getLength();

	if (hidden == 0)
		return;

	m_range.move(std::clamp(start, 0, hidden));

	invalidate();
	redraw();
}

/* -------------------------------------------------------------------------- */

void geWaveform::moveToFraction(double p)
{
	const auto maxFrames = m_sampleEditorData.getAudioBufferRef().countFrames();
	const auto hidden    = maxFrames - m_range.getLength();

	if (hidden == 0)
		return;

	const Frame newStart = math::map(p, 1.0, hidden);
	m_range.move(newStart);

	invalidate();
	redraw();
}

/* -------------------------------------------------------------------------- */

void geWaveform::invalidate()
{
	G_DEBUG("Invalidate waveform data");

	assert(w() > 0);

	m_data = {m_sampleEditorData.getAudioBufferRef(), m_range, w() - (BORDER_PX * 2)};
}

/* -------------------------------------------------------------------------- */

void geWaveform::drawWaveform()
{
	const geompp::Rect<int> bounds        = {x(), y(), w(), h()};
	const int               channelHeight = h() / m_data.get().size();

	fl_color(G_COLOR_BLACK);

	for (int numChannel = 0; const auto& channel : m_data.get())
	{
		const int channelY = (channelHeight * numChannel) + y();
		const int zero     = channelY + (channelHeight / 2);

		drawLine(bounds.reduced({1}).getWidthAsLine().withY(zero), G_COLOR_BLACK); // Draw zero line

		for (int column = x() + BORDER_PX; const auto& peaks : channel)
		{
			if (column > bounds.xw - 2) // Don't overflow paint
				break;

			const float peakUpAudio    = std::clamp(peaks.up, 0.0f, 1.0f);    // Clamp to avoid nasty bleedings
			const float peakDownAudio  = std::clamp(peaks.down, -1.0f, 0.0f); // Clamp to avoid nasty bleedings
			const int   peakUpScreen   = math::map(peakUpAudio, 0.0f, 1.0f, zero, channelY);
			const int   peakDownScreen = math::map(peakDownAudio, 0.0f, -1.0f, zero, channelY + channelHeight);

			fl_line(column, peakDownScreen, column, peakUpScreen);

			column++;
		}

		numChannel++;
	}
}
} // namespace giada::v
