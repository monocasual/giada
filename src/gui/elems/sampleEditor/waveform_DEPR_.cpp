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

#include "src/gui/elems/sampleEditor/waveform_DEPR_.h"
#include "src/core/wave.h"
#include "src/glue/channel.h"
#include "src/glue/sampleEditor.h"
#include "src/gui/const.h"
#include "src/gui/dialogs/sampleEditor.h"
#include "src/gui/drawing.h"
#include "src/gui/elems/basics/boxtypes.h"
#include "src/gui/elems/sampleEditor/waveTools.h"
#include "src/utils/log.h"
#include <FL/Fl_Menu_Button.H>
#include <FL/fl_draw.H>
#include <cassert>
#include <cmath>

namespace giada::v
{
geWaveform_DEPR_::geWaveform_DEPR_(int x, int y, int w, int h, bool gridEnabled, int gridVal)
: Fl_Widget(x, y, w, h, nullptr)
, m_selection{}
, m_data(nullptr)
, m_chanStart(0)
, m_chanStartLit(false)
, m_chanEnd(0)
, m_chanEndLit(false)
, m_pushed(false)
, m_dragged(false)
, m_resizedA(false)
, m_resizedB(false)
, m_ratio(0.0f)
{
	m_waveform.size = w;

	m_grid.snap  = gridEnabled;
	m_grid.level = gridVal;
}

/* -------------------------------------------------------------------------- */

void geWaveform_DEPR_::clearData()
{
	m_waveform.sup.clear();
	m_waveform.inf.clear();
	m_waveform.size = 0;
	m_grid.points.clear();
}

/* -------------------------------------------------------------------------- */

int geWaveform_DEPR_::alloc(int datasize, bool force)
{
	if (!m_data->isValid())
		return 0;

	const m::Wave& wave = *m_data->sample.wave;

	m_ratio = wave.getBuffer().countFrames() / (float)datasize;

	/* Limit 1:1 drawing (to avoid sub-frame drawing) by keeping m_ratio >= 1. */

	if (m_ratio < 1)
	{
		datasize = wave.getBuffer().countFrames();
		m_ratio  = 1;
	}

	if (datasize == m_waveform.size && !force)
		return 0;

	clearData();

	m_waveform.size = datasize;
	m_waveform.sup.resize(m_waveform.size);
	m_waveform.inf.resize(m_waveform.size);

	u::log::print("[geWaveform::alloc] {} pixels, {} m_ratio\n", m_waveform.size, m_ratio);

	int offset = h() / 2;
	int zero   = y() + offset; // center, zero amplitude (-inf dB)

	/* Frid frequency: store a grid point every 'gridFreq' frame (if grid is
	enabled). TODO - this will cause round off errors, since gridFreq is integer. */

	int gridFreq = m_grid.level != 0 ? wave.getBuffer().countFrames() / m_grid.level : 0;

	/* Resampling the waveform, hardcore way. Many thanks to
	http://fourier.eng.hmc.edu/e161/lectures/resize/node3.html */

	for (int i = 0; i < m_waveform.size; i++)
	{

		/* Scan the original waveform in chunks [pc, pn]. */

		int pc = i * m_ratio;       // current point TODO - int until we switch to uint32_t for Wave size...
		int pn = (i + 1) * m_ratio; // next point    TODO - int until we switch to uint32_t for Wave size...

		float peaksup = 0.0f;
		float peakinf = 0.0f;

		for (int k = pc; k < pn; k++)
		{ // TODO - int until we switch to uint32_t for Wave size...

			if (k >= wave.getBuffer().countFrames())
				continue;

			/* Compute average of stereo signal. */

			float  avg   = 0.0f;
			float* frame = wave.getBuffer()[k];
			for (int j = 0; j < wave.getBuffer().countChannels(); j++)
				avg += frame[j];
			avg /= wave.getBuffer().countChannels();

			/* Find peaks (greater and lower). */

			if (avg > peaksup)
				peaksup = avg;
			else if (avg <= peakinf)
				peakinf = avg;

			/* Fill up grid vector. */

			if (gridFreq != 0 && (int)k % gridFreq == 0 && k != 0)
				m_grid.points.push_back(k);
		}

		m_waveform.sup[i] = zero - (peaksup * offset);
		m_waveform.inf[i] = zero - (peakinf * offset);

		// avoid window overflow

		if (m_waveform.sup[i] < y())
			m_waveform.sup[i] = y();
		if (m_waveform.inf[i] > y() + h() - 1)
			m_waveform.inf[i] = y() + h() - 1;
	}

	recalcPoints();
	return 1;
}

/* -------------------------------------------------------------------------- */

void geWaveform_DEPR_::recalcPoints()
{
	m_chanStart = m_data->sample.range.a;
	m_chanEnd   = m_data->sample.range.b;
}

/* -------------------------------------------------------------------------- */

void geWaveform_DEPR_::drawSelection()
{
	if (!isSelected())
		return;

	int a = frameToPixel(m_selection.a) + x();
	int b = frameToPixel(m_selection.b) + x();

	if (a < 0)
		a = 0;
	if (b >= w() + BORDER)
		b = w() + BORDER;

	if (a < b)
		fl_rectf(a, y(), b - a, h(), G_COLOR_GREY_4);
	else
		fl_rectf(b, y(), a - b, h(), G_COLOR_GREY_4);
}

/* -------------------------------------------------------------------------- */

void geWaveform_DEPR_::drawWaveform(int from, int to)
{
	int zero = y() + (h() / 2); // zero amplitude (-inf dB)

	fl_color(G_COLOR_BLACK);
	for (int i = from; i < to; i++)
	{
		if (i >= m_waveform.size)
			break;
		fl_line(i + x(), zero, i + x(), m_waveform.sup[i]);
		fl_line(i + x(), zero, i + x(), m_waveform.inf[i]);
	}
}

/* -------------------------------------------------------------------------- */

void geWaveform_DEPR_::drawGrid(int from, int to)
{
	fl_color(G_COLOR_GREY_3);
	fl_line_style(FL_DASH, 1, nullptr);

	for (int pf : m_grid.points)
	{
		int pp = frameToPixel(pf);
		if (pp > from && pp < to)
			fl_line(pp + x(), y(), pp + x(), y() + h());
	}

	fl_line_style(FL_SOLID, 0, nullptr);
}

/* -------------------------------------------------------------------------- */

void geWaveform_DEPR_::drawStartEndPoints()
{
	/* print m_chanStart */

	int lineX = frameToPixel(m_chanStart) + x();

	if (m_chanStartLit)
		fl_color(G_COLOR_LIGHT_2);
	else
		fl_color(G_COLOR_LIGHT_1);

	/* vertical line */

	fl_line(lineX, y() + 1, lineX, y() + h() - 2);

	/* print flag and avoid overflow */

	if (lineX + FLAG_WIDTH > w() + x() - 2)
		fl_rectf(lineX, y() + h() - FLAG_HEIGHT - 1, w() - lineX + x() - 1, FLAG_HEIGHT);
	else
		fl_rectf(lineX, y() + h() - FLAG_HEIGHT - 1, FLAG_WIDTH, FLAG_HEIGHT);

	/* print m_chanEnd */

	lineX = frameToPixel(m_chanEnd) + x() - 1;
	if (m_chanEndLit)
		fl_color(G_COLOR_LIGHT_2);
	else
		fl_color(G_COLOR_LIGHT_1);

	/* vertical line */

	fl_line(lineX, y() + 1, lineX, y() + h() - 2);

	if (lineX - FLAG_WIDTH < x())
		fl_rectf(x() + 1, y() + 1, lineX - x(), FLAG_HEIGHT);
	else
		fl_rectf(lineX - FLAG_WIDTH, y() + 1, FLAG_WIDTH, FLAG_HEIGHT);
}

/* -------------------------------------------------------------------------- */

void geWaveform_DEPR_::drawPlayHead()
{
	int p = frameToPixel(m_data->a_getPreviewTracker()) + x();
	fl_color(G_COLOR_LIGHT_2);
	fl_line(p, y() + 1, p, y() + h() - 2);
}

/* -------------------------------------------------------------------------- */

void geWaveform_DEPR_::draw()
{
	assert(m_waveform.sup.size() > 0);
	assert(m_waveform.inf.size() > 0);

	fl_rectf(x(), y(), w(), h(), G_COLOR_GREY_2); // blank canvas

	if (m_data->isValid())
	{
		/* Draw things from 'from' (offset driven by the scrollbar) to 'to' (width of
		parent window). We don't draw the entire waveform, only the visible part. */

		int from = abs(x() - parent()->x());
		int to   = from + parent()->w();
		if (x() + w() < parent()->w())
			to = x() + w() - BORDER;

		drawSelection();
		drawWaveform(from, to);
		drawGrid(from, to);
		drawPlayHead();

		fl_rect(x(), y(), w(), h(), G_COLOR_GREY_4); // border box

		drawStartEndPoints();
	}
	else
	{
		drawText("No audio data.", {x(), y(), w(), h()}, FL_HELVETICA, G_GUI_FONT_SIZE_BASE, G_COLOR_GREY_4);
		fl_rect(x(), y(), w(), h(), G_COLOR_GREY_4); // border box
	}
}

/* -------------------------------------------------------------------------- */

int geWaveform_DEPR_::handle(int e)
{
	if (!m_data->isValid())
		return 0;

	const m::Wave& wave = *m_data->sample.wave;

	m_mouseX = pixelToFrame(Fl::event_x() - x());
	m_mouseY = pixelToFrame(Fl::event_y() - y());

	switch (e)
	{
	case FL_FOCUS:
	case FL_UNFOCUS:
		return 1; // enables receiving keyboard events

	case FL_SHORTCUT:
	case FL_KEYDOWN:
	{
		if (Fl::event_key() == ' ')
			c::sampleEditor::togglePreview();
		else if (Fl::event_key() == FL_BackSpace)
			c::sampleEditor::setPreviewTracker(m_data->sample.range.a);
		return 1;
	}

	case FL_PUSH:
	{
		m_pushed = true;

		if (!mouseOnEnd() && !mouseOnStart())
		{
			if (Fl::event_button3()) // let the parent (waveTools) handle this
				return 0;
			if (mouseOnSelectionA())
				m_resizedA = true;
			else if (mouseOnSelectionB())
				m_resizedB = true;
			else
			{
				m_dragged     = true;
				m_selection.a = m_mouseX;
				m_selection.b = m_mouseX;
			}
		}
		return 1;
	}

	case FL_RELEASE:
	{
		if (!m_chanStartLit && !m_chanEndLit)
			c::sampleEditor::setPreviewTracker(m_mouseX);

		/* If selection has been done (m_dragged or resized), make sure that point A
		    is always lower than B. */

		if (m_dragged || m_resizedA || m_resizedB)
			fixSelection();

		/* Handle begin/end markers interaction. */

		if (m_chanStartLit || m_chanEndLit)
			c::sampleEditor::setRange(m_data->channelId, {m_chanStart, m_chanEnd});

		m_pushed   = false;
		m_dragged  = false;
		m_resizedA = false;
		m_resizedB = false;

		redraw();
		return 1;
	}

	case FL_ENTER:
	{ // enables FL_DRAG
		return 1;
	}

	case FL_LEAVE:
	{
		if (m_chanStartLit || m_chanEndLit)
		{
			m_chanStartLit = false;
			m_chanEndLit   = false;
			redraw();
		}
		return 1;
	}

	case FL_MOVE:
	{

		if (mouseOnStart())
		{
			m_chanStartLit = true;
			redraw();
		}
		else if (m_chanStartLit)
		{
			m_chanStartLit = false;
			redraw();
		}

		if (mouseOnEnd())
		{
			m_chanEndLit = true;
			redraw();
		}
		else if (m_chanEndLit)
		{
			m_chanEndLit = false;
			redraw();
		}

		if (mouseOnSelectionA() && isSelected())
			fl_cursor(FL_CURSOR_WE, FL_WHITE, FL_BLACK);
		else if (mouseOnSelectionB() && isSelected())
			fl_cursor(FL_CURSOR_WE, FL_WHITE, FL_BLACK);
		else
			fl_cursor(FL_CURSOR_DEFAULT, FL_WHITE, FL_BLACK);

		return 1;
	}

	case FL_DRAG:
	{

		/* here the mouse is on the m_chanStart tool */

		if (m_chanStartLit && m_pushed)
		{
			m_chanStart = snap(m_mouseX);

			if (m_chanStart < 0)
				m_chanStart = 0;
			else if (m_chanStart >= m_chanEnd)
				m_chanStart = m_chanEnd - 2;

			redraw();
		}
		else if (m_chanEndLit && m_pushed)
		{

			m_chanEnd = snap(m_mouseX);

			if (m_chanEnd > wave.getBuffer().countFrames())
				m_chanEnd = wave.getBuffer().countFrames();
			else if (m_chanEnd <= m_chanStart)
				m_chanEnd = m_chanStart + 2;

			redraw();
		}

		/* Here the mouse is on the waveform, i.e. a new selection has started. */

		else if (m_dragged)
		{
			m_selection.b = snap(m_mouseX);
			redraw();
		}

		/* here the mouse is on a selection boundary i.e. resize */

		else if (m_resizedA || m_resizedB)
		{
			int pos = snap(m_mouseX);
			m_resizedA ? m_selection.a = pos : m_selection.b = pos;
			redraw();
		}

		return 1;
	}

	default:
		return Fl_Widget::handle(e);
	}
}

/* -------------------------------------------------------------------------- */

int geWaveform_DEPR_::snap(int pos)
{
	// TODO use math::quantize
	if (!m_grid.snap)
		return pos;
	for (int pf : m_grid.points)
	{
		if (pos >= pf - pixelToFrame(SNAPPING) &&
		    pos <= pf + pixelToFrame(SNAPPING))
		{
			return pf;
		}
	}
	return pos;
}

/* -------------------------------------------------------------------------- */

bool geWaveform_DEPR_::mouseOnStart() const
{
	int mouseXp    = frameToPixel(m_mouseX);
	int mouseYp    = frameToPixel(m_mouseY);
	int chanStartP = frameToPixel(m_chanStart);
	return mouseXp - (FLAG_WIDTH / 2) > chanStartP - BORDER &&
	       mouseXp - (FLAG_WIDTH / 2) <= chanStartP - BORDER + FLAG_WIDTH &&
	       mouseYp > h() - FLAG_HEIGHT;
}

/* -------------------------------------------------------------------------- */

bool geWaveform_DEPR_::mouseOnEnd() const
{
	int mouseXp  = frameToPixel(m_mouseX);
	int mouseYp  = frameToPixel(m_mouseY);
	int chanEndP = frameToPixel(m_chanEnd);
	return mouseXp - (FLAG_WIDTH / 2) >= chanEndP - BORDER - FLAG_WIDTH &&
	       mouseXp - (FLAG_WIDTH / 2) <= chanEndP - BORDER &&
	       mouseYp <= FLAG_HEIGHT + 1;
}

/* -------------------------------------------------------------------------- */

bool geWaveform_DEPR_::mouseOnSelectionA() const
{
	int mouseXp = frameToPixel(m_mouseX);
	int selAp   = frameToPixel(m_selection.a);
	return mouseXp >= selAp - (FLAG_WIDTH / 2) && mouseXp <= selAp + (FLAG_WIDTH / 2);
}

bool geWaveform_DEPR_::mouseOnSelectionB() const
{
	int mouseXp = frameToPixel(m_mouseX);
	int selBp   = frameToPixel(m_selection.b);
	return mouseXp >= selBp - (FLAG_WIDTH / 2) && mouseXp <= selBp + (FLAG_WIDTH / 2);
}

/* -------------------------------------------------------------------------- */

int geWaveform_DEPR_::pixelToFrame(int p) const
{
	if (p <= 0)
		return 0;
	if (p > m_waveform.size)
		return m_data->waveSize;
	return p * m_ratio;
}

/* -------------------------------------------------------------------------- */

int geWaveform_DEPR_::frameToPixel(int p) const
{
	return ceil(p / m_ratio);
}

/* -------------------------------------------------------------------------- */

void geWaveform_DEPR_::fixSelection()
{
	if (m_selection.a > m_selection.b) // inverted m_selection
		std::swap(m_selection.a, m_selection.b);

	c::sampleEditor::setPreviewTracker(m_selection.a);
}

/* -------------------------------------------------------------------------- */

void geWaveform_DEPR_::clearSelection()
{
	m_selection.a = 0;
	m_selection.b = 0;
}

/* -------------------------------------------------------------------------- */

#if G_OS_WINDOWS
#undef IN
#endif

void geWaveform_DEPR_::setZoom(Zoom z)
{
	if (!alloc(z == Zoom::IN ? m_waveform.size * G_GUI_ZOOM_FACTOR : m_waveform.size / G_GUI_ZOOM_FACTOR))
		return;

	size(m_waveform.size, h());

	/* Zoom to cursor. */

	int newX = -frameToPixel(m_mouseX) + Fl::event_x();
	if (newX > BORDER)
		newX = BORDER;
	position(newX, y());

	/* Avoid overflow when zooming out with scrollbar like that:

	    |----------[scrollbar]|

	Offset vs smaller:

	    |[wave------------| offset > 0  smaller = false
	    |[wave----]       | offset < 0, smaller = true
	    |-------------]   | offset < 0, smaller = false  */

	int parentW = parent()->w();
	int thisW   = x() + w() - BORDER; // visible width, not full width

	if (thisW < parentW)
		position(x() + parentW - thisW, y());
	if (smaller())
		stretchToWindow();

	redraw();
}

/* -------------------------------------------------------------------------- */

void geWaveform_DEPR_::stretchToWindow()
{
	int s = parent()->w();
	alloc(s);
	position(BORDER, y());
	size(s, h());
}

/* -------------------------------------------------------------------------- */

void geWaveform_DEPR_::rebuild(const c::sampleEditor::Data& d)
{
	m_data = &d;
	clearSelection();
	alloc(m_waveform.size, /*force=*/true);
	redraw();
}

/* -------------------------------------------------------------------------- */

bool geWaveform_DEPR_::smaller() const
{
	return w() < parent()->w();
}

/* -------------------------------------------------------------------------- */

void geWaveform_DEPR_::setGridLevel(int l)
{
	m_grid.points.clear();
	m_grid.level = l;
	alloc(m_waveform.size, true); // force alloc
	redraw();
}

/* -------------------------------------------------------------------------- */

bool geWaveform_DEPR_::isSelected() const
{
	return m_selection.a != m_selection.b;
}

/* -------------------------------------------------------------------------- */

void geWaveform_DEPR_::setSnap(bool v) { m_grid.snap = v; }
bool geWaveform_DEPR_::getSnap() const { return m_grid.snap; }
int  geWaveform_DEPR_::getSize() const { return m_waveform.size; }

/* -------------------------------------------------------------------------- */

int geWaveform_DEPR_::getSelectionA() const { return m_selection.a; }
int geWaveform_DEPR_::getSelectionB() const { return m_selection.b; }

void geWaveform_DEPR_::selectAll()
{
	m_selection.a = 0;
	m_selection.b = m_data->waveSize - 1;
	redraw();
}
} // namespace giada::v