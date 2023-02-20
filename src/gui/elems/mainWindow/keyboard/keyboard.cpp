/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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
#include "deps/geompp/src/rect.hpp"
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
#include "utils/log.h"
#include "utils/string.h"
#include "utils/vector.h"
#include <FL/fl_draw.H>
#include <cassert>

extern giada::v::Ui g_ui;

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

	const ID         targetColumnId = column->id;
	const geChannel* targetChannel  = column->getChannelAtCursor(Fl::event_y());
	const int        targetPosition = targetChannel == nullptr ? 0 : targetChannel->getData().position + 1;

	c::channel::moveChannel(m_channelId, targetColumnId, targetPosition);

	m_channelId = -1;
	m_xoffset   = 0;
	m_keyboard.remove(m_placeholder);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geKeyboard::geKeyboard()
: geScroll(Fl_Scroll::BOTH_ALWAYS)
, m_channelDragger(*this)
, m_addColumnBtn(nullptr)
{
	autoscroll = true;

	init();
	rebuild();
}

/* -------------------------------------------------------------------------- */

ID geKeyboard::getChannelColumnId(ID channelId) const
{
	return getChannel(channelId)->getColumnId();
}

/* -------------------------------------------------------------------------- */

void geKeyboard::init()
{
	m_columnId = m::IdManager();

	deleteAllColumns();

	/* Add 6 empty columns as initial layout. */

	g_ui.model.columns.clear();
	g_ui.model.columns.push_back({1, G_DEFAULT_COLUMN_WIDTH});
	g_ui.model.columns.push_back({2, G_DEFAULT_COLUMN_WIDTH});
	g_ui.model.columns.push_back({3, G_DEFAULT_COLUMN_WIDTH});
	g_ui.model.columns.push_back({4, G_DEFAULT_COLUMN_WIDTH});
	g_ui.model.columns.push_back({5, G_DEFAULT_COLUMN_WIDTH});
	g_ui.model.columns.push_back({6, G_DEFAULT_COLUMN_WIDTH});
}

/* -------------------------------------------------------------------------- */

void geKeyboard::rebuild()
{
	/* Wipe out all columns and add them according to the current layout in model. */

	deleteAllColumns();

	for (const Model::Column& c : g_ui.model.columns)
		addColumn(c.width, c.id);

	for (const c::channel::Data& ch : c::channel::getChannels())
		getColumn(ch.columnId)->addChannel(ch);

	redraw();
}

/* -------------------------------------------------------------------------- */

void geKeyboard::deleteColumn(ID id)
{
	u::vector::removeIf(g_ui.model.columns, [=](const Model::Column& c) { return c.id == id; });
	rebuild();
}

/* -------------------------------------------------------------------------- */

void geKeyboard::deleteAllColumns()
{
	Fl_Scroll::clear();
	m_columns.clear();

	m_addColumnBtn          = new geTextButton(8, y(), 200, 20, "Add new column");
	m_addColumnBtn->onClick = [this]() {
		addColumn();
		storeLayout();
	};

	add(m_addColumnBtn);
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
		if (Fl::event_button3())
		{
			m_channelDragger.begin();
			return 1;
		}
		return geScroll::handle(e);

	case FL_DRAG:
		if (Fl::event_button3() && m_channelDragger.isDragging())
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
		g_ui.dispatcher.dispatchKey(e);
		return 1;

	case FL_DND_ENTER: // return(1) for these events to 'accept' dnd
	case FL_DND_DRAG:
	case FL_DND_RELEASE:
		return 1;

	case FL_PASTE: // handle actual drop (paste) operation
		const geColumn* c = getColumnAtCursor(Fl::event_x());
		if (c != nullptr)
			c::channel::addAndLoadChannels(c->id, getDroppedFilePaths());
		return 1;
	}

	return geScroll::handle(e); // Assume the buttons won't handle the Keyboard events
}

/* -------------------------------------------------------------------------- */

void geKeyboard::draw()
{
	geScroll::draw();

	/* Paint columns background. Use a clip to draw only what's visible. */

	fl_push_clip(
	    x(),
	    y(),
	    w() - scrollbar_size() - (G_GUI_OUTER_MARGIN * 2),
	    h() - scrollbar_size() - (G_GUI_OUTER_MARGIN * 2));

	for (const geColumn* c : m_columns)
	{
		geompp::Rect bounds(c->x(), c->y() + c->h(), c->w(), h() + yposition());
		drawRectf(bounds, G_COLOR_GREY_1_5);
		drawRect(bounds, G_COLOR_GREY_2);
	}

	draw_children();

	fl_pop_clip();
}

/* -------------------------------------------------------------------------- */

void geKeyboard::addColumn(int width, ID id)
{
	int colx = x() - xposition(); // Mind the x-scroll offset with xposition()

	/* If this is not the first column... */

	if (m_columns.size() > 0)
		colx = m_columns.back()->x() + m_columns.back()->w() + COLUMN_GAP;

	/* Generate new index. If not passed in. */

	m_columnId.set(id);

	/* Add a new column + a new resizer bar. */

	geResizerBar* bar    = new geResizerBar(colx + width, y(), COLUMN_GAP, h(), G_MIN_COLUMN_WIDTH, geResizerBar::Direction::HORIZONTAL);
	geColumn*     column = new geColumn(colx, y(), width, G_GUI_UNIT, m_columnId.generate(id), bar);

	/* Store the column width in the model when the resizer bar is released. */

	bar->onRelease = [this](const Fl_Widget& /*w*/) { storeLayout(); };

	add(column);
	add(bar);
	m_columns.push_back(column);

	/* And then shift the "add column" button on the rightmost edge. */

	m_addColumnBtn->position(colx + width + COLUMN_GAP, y());

	redraw();
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

geColumn* geKeyboard::getColumn(ID id)
{
	for (geColumn* c : m_columns)
		if (c->id == id)
			return c;
	assert(false);
	return nullptr;
}

geColumn* geKeyboard::getColumnAtCursor(Pixel px)
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

void geKeyboard::storeLayout()
{
	g_ui.model.columns.clear();
	for (const geColumn* c : m_columns)
		g_ui.model.columns.push_back({c->id, c->w()});
}
} // namespace giada::v