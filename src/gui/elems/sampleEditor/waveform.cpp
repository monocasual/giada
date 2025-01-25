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
#include "src/glue/sampleEditor.h"
#include "src/gui/const.h"
#include "src/gui/drawing.h"
#include "src/utils/log.h"
#include "src/utils/math.h"

namespace giada::v
{
geWaveform::Data::Data(const m::Wave& wave, geompp::Range<Frame> rangeAudio, int widthScreen)
{
	const mcl::AudioBuffer& audioBuffer = wave.getBuffer();

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
			double startFrame = rangeAudio.a;
			for (int column = 0; column < widthScreen; column++)
			{
				m_peaks[channel][column] = getPeak(wave, {startFrame, startFrame + m_ratio}, m_ratio, channel);
				startFrame += m_ratio;
			}
		}
		else
		{
			const double columnsPerFrame = widthScreen / static_cast<double>(rangeAudio.getLength());
			double       column          = 0;
			for (int frame = rangeAudio.a; frame < rangeAudio.b; frame++)
			{
				m_peaks[channel][column] = getPeak(wave, frame, channel);
				column += columnsPerFrame;
			}
		}
	}
}

/* -------------------------------------------------------------------------- */

geWaveform::Data::Peak geWaveform::Data::getPeak(const m::Wave& wave, geompp::Range<double> range, double steps, int channel) const
{
	const mcl::AudioBuffer& audioBuffer = wave.getBuffer();
	float                   peakUp      = 0.0;
	float                   peakDown    = 0.0;

	for (double frameD = range.a; frameD < std::floor(range.a + steps); frameD++)
	{
		const Frame frame = std::min(static_cast<Frame>(frameD), audioBuffer.countFrames() - 1);
		const float value = audioBuffer[frame][channel];
		peakUp            = std::max(value, peakUp);
		peakDown          = std::min(value, peakDown);
	}

	return {peakUp, peakDown};
}

/* -------------------------------------------------------------------------- */

geWaveform::Data::Peak geWaveform::Data::getPeak(const m::Wave& wave, Frame frame, int channel) const
{
	const mcl::AudioBuffer& audioBuffer = wave.getBuffer();
	const float             value       = audioBuffer[frame][channel];
	const float             peakUp      = value >= 0.0f ? value : 0.0f;
	const float             peakDown    = value < 0.0f ? value : 0.0f;

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

geWaveform::geWaveform()
: Fl_Widget(0, 0, 0, 0)
, m_sampleEditorData(nullptr)
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
	const auto windowF = static_cast<Frame>(m_data.getRatio() * w());
	const auto maxF    = m_sampleEditorData->getWaveRef().getBuffer().countFrames();
	const auto hidden  = m_sampleEditorData->getWaveRef().getBuffer().countFrames() - m_range.getLength();
	if (hidden == 0)
		return {0, maxF};
	else
		return {0, maxF - windowF};
}

/* -------------------------------------------------------------------------- */

void geWaveform::rebuild(const c::sampleEditor::Data& data)
{
	m_sampleEditorData = &data;
	m_range            = {0, m_sampleEditorData->getWaveRef().getBuffer().countFrames()};
	invalidate();
}

/* -------------------------------------------------------------------------- */

void geWaveform::zoomIn()
{
	m_range.b = std::max(m_range.b / 2, 1);

	invalidate();
	redraw();
}

/* -------------------------------------------------------------------------- */

void geWaveform::zoomOut()
{
	m_range.b = std::min(m_range.b * 2, m_sampleEditorData->getWaveRef().getBuffer().countFrames());

	invalidate();
	redraw();
}

/* -------------------------------------------------------------------------- */

void geWaveform::moveRange(Frame start)
{
	// const auto wFrames   = static_cast<Frame>(m_data.getRatio() * w());
	const auto maxFrames = m_sampleEditorData->getWaveRef().getBuffer().countFrames();

	// if (start >= maxFrames)
	//	puts("!");

	// TODO - m_range.move(start);

	printf("[%d, %d) . max=%d\n", m_range.a, m_range.b, maxFrames);

	invalidate();
	redraw();
}

/* -------------------------------------------------------------------------- */

void geWaveform::move(double p)
{
	const auto wFrames   = static_cast<Frame>(m_data.getRatio() * w());
	const auto maxFrames = m_sampleEditorData->getWaveRef().getBuffer().countFrames();
	const auto hidden    = m_sampleEditorData->getWaveRef().getBuffer().countFrames() - m_range.getLength();

	if (hidden == 0)
		return;

	const auto pMap = u::math::map(p, 1.0, maxFrames - wFrames);
	printf("%d - wFrames=%d maxFrames=%d hidden=%d\n", pMap, wFrames, maxFrames, hidden);

	// TODO - m_range.move(pMap);

	invalidate();
	redraw();
	// std::cout << p << "\n";
}

/* -------------------------------------------------------------------------- */

void geWaveform::invalidate()
{
	G_DEBUG("Invalidate waveform data");

	assert(w() > 0);

	m_data = {m_sampleEditorData->getWaveRef(), m_range, w() - 2}; // -2: take care of 1px border
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

		for (int column = x() + 1; const auto& peaks : channel) // x() + 1: take care of 1px border
		{
			if (column > bounds.xw - 2) // Don't overflow paint
				break;

			const float peakUpAudio    = peaks.first;
			const float peakDownAudio  = peaks.second;
			const int   peakUpScreen   = u::math::map(peakUpAudio, 0.0f, 1.0f, zero, channelY);
			const int   peakDownScreen = u::math::map(peakDownAudio, 0.0f, -1.0f, zero, channelY + channelHeight);

			fl_line(column, zero, column, peakUpScreen);
			fl_line(column, zero, column, peakDownScreen);

			column++;
		}

		numChannel++;
	}
}
} // namespace giada::v