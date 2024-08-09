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

#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "glue/channel.h"
#include "glue/io.h"
#include "gui/dialogs/warnings.h"
#include "gui/dispatcher.h"
#include "gui/drawing.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/basics/resizerBar.h"
#include "gui/elems/basics/textButton.h"
#include "gui/elems/mainWindow/keyboard/channelButton.h"
#include "gui/elems/mainWindow/keyboard/column.h"
#include "gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "gui/elems/midiActivity.h"
#include "gui/ui.h"
#include "utils/fs.h"
#include "utils/gui.h"
#include "utils/log.h"
#include "utils/string.h"
#include "utils/vector.h"
#include <FL/fl_draw.H>
#include <cassert>

extern giada::v::Ui* g_ui;

namespace giada::v
{
geKeyboard::ChannelDragger::ChannelDragger(geKeyboard& k)
: m_keyboard(k)
, m_channelId(-1)
, m_xoffset(0)
{
}

/* -------------------------------------------------------------------------- */

bool geKeyboard::ChannelDragger::isDragging() const
{
	return m_channelId != -1;
}

/* -------------------------------------------------------------------------- */

void geKeyboard::ChannelDragger::begin()
{
	const geColumn* column = m_keyboard.getColumnAtCursor(Fl::event_x());
	if (column == nullptr)
		return;

	const geChannel* channel = column->getChannelAtCursor(Fl::event_y());
	if (channel == nullptr)
		return;

	m_channelId   = channel->getData().id;
	m_xoffset     = channel->x() - Fl::event_x();
	m_placeholder = new Fl_Box(m_xoffset + Fl::event_x(), Fl::event_y(), channel->w(), channel->h());
	m_placeholder->image(toImage(*channel));
	m_keyboard.add(m_placeholder);
	m_keyboard.redraw();
}

/* -------------------------------------------------------------------------- */

void geKeyboard::ChannelDragger::drag()
{
	if (!isDragging())
		return;

	assert(m_keyboard.m_columns.size() > 0);

	const geColumn* firstColumn = m_keyboard.m_columns[0];
	const geColumn* lastColumn  = m_keyboard.m_columns.back();

	const int minx = firstColumn->x();
	const int maxx = lastColumn->x() + lastColumn->w() - m_placeholder->w();
	const int miny = firstColumn->y();

	// Explicit type std::min/max<int> to fix MINMAX macro hell on Windows
	const int newx = std::min<int>(std::max<int>(minx, m_xoffset + Fl::event_x()), maxx);
	const int newy = std::max<int>(miny, Fl::event_y());

	m_placeholder->position(newx, newy);
	m_keyboard.redraw();
}

/* -------------------------------------------------------------------------- */

void geKeyboard::ChannelDragger::end()
{
	if (!isDragging())
		return;

	const geColumn* column = m_keyboard.getColumnAtCursor(Fl::event_x());
	if (column == nullptr)
	{
		m_channelId = -1;
		m_xoffset   = 0;
		m_keyboard.rebuild(); // Just cleanup the UI
		return;
	}

	const int targetColumnIndex = column->index;
	const int targetPosition    = getPositionForCursor(column, Fl::event_y());

	c::channel::moveChannel(m_channelId, targetColumnIndex, targetPosition);

	m_channelId = -1;
	m_xoffset   = 0;
	m_keyboard.remove(m_placeholder);
}

/* -------------------------------------------------------------------------- */

int geKeyboard::ChannelDragger::getPositionForCursor(const geColumn* column, Pixel y) const
{
	const geChannel* lastChannel = column->getLastChannel();
	if (lastChannel == nullptr) // Column is empty
		return 0;

	const geChannel* targetChannel = column->getChannelAtCursor(y);
	if (targetChannel != nullptr)
		return targetChannel->getData().position + 1;

	/* Channel not found, case 1: the cursor could be above the first channel. 
	Channel not found, case 2: the cursor could be below the last channel, over
	the empty space at the bottom of the column. */

	const geChannel* firstChannel = column->getFirstChannel();
	if (y < firstChannel->y())
		return 0;
	return lastChannel->getData().position + 1;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geKeyboard::geKeyboard()
: geScroll(Fl_Scroll::BOTH)
, m_channelDragger(*this)
{
	autoscroll = true;

	init();
	rebuild();
}

/* -------------------------------------------------------------------------- */

int geKeyboard::getChannelColumnIndex(ID channelId) const
{
	return getChannel(channelId)->getColumnIndex();
}

/* -------------------------------------------------------------------------- */

size_t geKeyboard::countColumns() const
{
	return m_columns.size();
}

/* -------------------------------------------------------------------------- */

void geKeyboard::init()
{
	deleteAllColumns();
}

/* -------------------------------------------------------------------------- */

void geKeyboard::rebuild()
{
	deleteAllColumns();
	for (const c::channel::Track& c : c::channel::getTracks())
		addColumn(c);
	redraw();
}

/* -------------------------------------------------------------------------- */

void geKeyboard::deleteColumn(int index)
{
	c::channel::deleteTrack(index);
}

/* -------------------------------------------------------------------------- */

void geKeyboard::deleteAllColumns()
{
	Fl_Scroll::clear();
	m_columns.clear();
}

/* -------------------------------------------------------------------------- */

void geKeyboard::setChannelVolume(ID channelId, float v)
{
	getChannel(channelId)->vol->value(v);
}

/* -------------------------------------------------------------------------- */

void geKeyboard::notifyMidiIn(ID channelId)
{
	getChannel(channelId)->midiActivity->in->lit();
}

void geKeyboard::notifyMidiOut(ID channelId)
{
	getChannel(channelId)->midiActivity->out->lit();
}

/* -------------------------------------------------------------------------- */

void geKeyboard::refresh()
{
	if (m_channelDragger.isDragging())
		return;
	for (geColumn* c : m_columns)
		c->refresh();
}

/* -------------------------------------------------------------------------- */

int geKeyboard::handle(int e)
{
	switch (e)
	{
	case FL_PUSH:
		if (Fl::event_button1() && Fl::event_ctrl())
		{
			m_channelDragger.begin();
			return 1;
		}
		if (Fl::event_button3())
		{
			openColumnMenu();
			return 1;
		}

		return geScroll::handle(e);

	case FL_DRAG:
		if (Fl::event_button1() && Fl::event_ctrl() && m_channelDragger.isDragging())
			m_channelDragger.drag();
		return geScroll::handle(e); // Let geScroll process this event anyway, to enable autoscroll

	case FL_RELEASE:
		if (m_channelDragger.isDragging())
			m_channelDragger.end();
		return 1;

	case FL_FOCUS:
	case FL_UNFOCUS:
		return 1;

	case FL_SHORTCUT: // In case widget that isn't ours has focus
	case FL_KEYDOWN:  // Keyboard key pushed
	case FL_KEYUP:    // Keyboard key released
		g_ui->dispatcher.dispatchKey(e);
		return 1;

	case FL_DND_ENTER: // return(1) for these events to 'accept' dnd
	case FL_DND_DRAG:
	case FL_DND_RELEASE:
		return 1;

	case FL_PASTE: // handle actual drop (paste) operation
		const geColumn* c = getColumnAtCursor(Fl::event_x());
		if (c != nullptr)
			c::channel::addAndLoadChannels(c->index, getDroppedFilePaths());
		return 1;
	}

	return geScroll::handle(e); // Assume the buttons won't handle the Keyboard events
}

/* -------------------------------------------------------------------------- */

void geKeyboard::draw()
{
	geScroll::draw();

	/* Paint columns background. Use a clip to draw only what's visible. */

	const geompp::Rect<int> viewportBounds = getViewportBounds();

	fl_push_clip(viewportBounds.x, viewportBounds.y, viewportBounds.w, viewportBounds.h);

	for (const geColumn* c : m_columns)
	{
		const geompp::Rect background = getColumnBackround(*c);

		drawRectf(background, G_COLOR_GREY_1_5);
		drawRect(background, G_COLOR_GREY_2);
		if (background.h >= c->endMargin)
		{
			const std::string text = u::gui::truncate(g_ui->getI18Text(LangMap::MAIN_COLUMN_HELP), c->w() - G_GUI_UNIT);
			drawText(text, background, FL_HELVETICA, G_GUI_FONT_SIZE_BASE, G_COLOR_GREY_3);
		}
	}

	draw_children();

	fl_pop_clip();
}

/* -------------------------------------------------------------------------- */

geompp::Rect<int> geKeyboard::getColumnBackround(const geColumn& c) const
{
	const geompp::Rect  columnBounds(c.x(), c.y(), c.w(), c.h() - c.endMargin);
	const geompp::Rect  viewportBounds = getViewportBounds();
	const geompp::Range columnRange    = columnBounds.getHeightAsRange();
	const geompp::Range thisRange      = viewportBounds.getHeightAsRange();

	/* Column is empty (c.h() == 0) or away from the viewport: just return a 
	full-size background. */

	if (c.h() == 0 || !thisRange.intersects(columnRange))
		return viewportBounds.withX(c.x()).withW(c.w());

	const auto [_, r2] = thisRange.getDifference(columnRange);

	if (!r2.isValid())
		return {};
	return columnBounds.withVerticalRange(r2);
}

/* -------------------------------------------------------------------------- */

void geKeyboard::addColumn()
{
	c::channel::addTrack();
}

/* -------------------------------------------------------------------------- */

geColumn& geKeyboard::addColumn(const c::channel::Track& columnModel)
{
	int colx = x() - xposition(); // Mind the x-scroll offset with xposition()

	/* If this is not the first column... */

	if (m_columns.size() > 0)
		colx = m_columns.back()->x() + m_columns.back()->w() + COLUMN_GAP;

	/* Add a new column + a new resizer bar. */

	const int viewportH = getViewportBounds().h;

	geResizerBar* bar    = new geResizerBar(colx + columnModel.width, y(), COLUMN_GAP, viewportH, G_MIN_COLUMN_WIDTH, Direction::HORIZONTAL);
	geColumn*     column = new geColumn(colx, y(), columnModel.width, 0, columnModel.index, bar);

	/* Store the column width in the model when the resizer bar is released. */

	bar->onRelease = [](const Fl_Widget& w) {
		const geColumn& column = static_cast<const geColumn&>(w);
		c::channel::setTrackWidth(column.index, column.w());
	};

	add(column);
	add(bar);
	m_columns.push_back(column);

	/* Fill column with channels. */

	for (const c::channel::Data& ch : columnModel.channels)
		column->addChannel(ch);

	redraw();

	return *column;
}

/* -------------------------------------------------------------------------- */

void geKeyboard::forEachChannel(std::function<void(geChannel& c)> f) const
{
	for (geColumn* column : m_columns)
		column->forEachChannel(f);
}

void geKeyboard::forEachColumn(std::function<void(const geColumn& c)> f) const
{
	for (geColumn* column : m_columns)
		f(*column);
}

/* -------------------------------------------------------------------------- */

geColumn* geKeyboard::getColumnAtCursor(Pixel px) const
{
	for (geColumn* c : m_columns)
		if (geompp::Range(c->x(), c->x() + c->w()).contains(px))
			return c;
	return nullptr;
}

/* -------------------------------------------------------------------------- */

const geChannel* geKeyboard::getChannel(ID channelId) const
{
	for (geColumn* column : m_columns)
	{
		geChannel* c = column->getChannel(channelId);
		if (c != nullptr)
			return c;
	}
	assert(false);
	return nullptr;
}

geChannel* geKeyboard::getChannel(ID channelId)
{
	return const_cast<geChannel*>(const_cast<const geKeyboard*>(this)->getChannel(channelId));
}

/* -------------------------------------------------------------------------- */

std::vector<std::string> geKeyboard::getDroppedFilePaths() const
{
	std::vector<std::string> paths = u::string::split(Fl::event_text(), "\n");
	for (std::string& p : paths)
		p = u::fs::stripFileUrl(p);
	return paths;
}

/* -------------------------------------------------------------------------- */

void geKeyboard::openColumnMenu() const
{
	const geColumn* column = getColumnAtCursor(Fl::event_x());
	if (column == nullptr || column->getChannelAtCursor(Fl::event_y()) != nullptr)
		return;
	column->showAddChannelMenu();
}
} // namespace giada::v