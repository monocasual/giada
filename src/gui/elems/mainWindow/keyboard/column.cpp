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

#include "gui/elems/mainWindow/keyboard/column.h"
#include "deps/geompp/src/range.hpp"
#include "glue/channel.h"
#include "gui/dialogs/warnings.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/menu.h"
#include "gui/elems/basics/resizerBar.h"
#include "gui/elems/basics/textButton.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/elems/mainWindow/keyboard/midiChannel.h"
#include "gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "gui/ui.h"
#include "utils/fs.h"
#include "utils/gui.h"
#include "utils/log.h"
#include "utils/string.h"
#include <cassert>

extern giada::v::Ui* g_ui;

namespace giada::v
{
namespace
{
enum class Menu
{
	ADD_SAMPLE_CHANNEL = 0,
	ADD_MIDI_CHANNEL,
	REMOVE
};

/* -------------------------------------------------------------------------- */

geChannel* makeChannel_(const c::channel::Data& data)
{
	if (data.type == ChannelType::SAMPLE)
		return new geSampleChannel(0, 0, 0, 0, data);
	return new geMidiChannel(0, 0, 0, 0, data);
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geColumn::geColumn(int X, int Y, int W, int H, ID id, geResizerBar* b)
: geFlex(X, Y, W, H, Direction::VERTICAL)
, id(id)
, resizerBar(b)
{
	end();
	init();
}

/* -------------------------------------------------------------------------- */

void geColumn::refresh()
{
	for (geChannel* c : m_channels)
		c->refresh();
}

/* -------------------------------------------------------------------------- */

geChannel* geColumn::addChannel(c::channel::Data d)
{
	geChannel*    gch = makeChannel_(d);
	geResizerBar* bar = new geResizerBar(0, 0, 0, G_GUI_INNER_MARGIN, G_GUI_UNIT, geResizerBar::Direction::VERTICAL,
	    geResizerBar::Mode::MOVE);

	/* Update the column height while dragging the resizer bar. */

	bar->onDrag = [this](const Fl_Widget& /*w*/) {
		resizable(nullptr);
		size(this->w(), (child(children() - 1)->y() - y()) + G_GUI_INNER_MARGIN);
	};

	/* Store the channel height in model when the resizer bar is released. */

	bar->onRelease = [channelId = d.id, this](const Fl_Widget& w) {
		resizable(this);
		c::channel::setHeight(channelId, w.h());
	};

	m_channels.push_back(gch);

	addWidget(gch, d.height);
	addWidget(bar, G_GUI_INNER_MARGIN);
	size(w(), computeHeight());

	return gch;
}

/* -------------------------------------------------------------------------- */

void geColumn::showAddChannelMenu() const
{
	geMenu menu;

	menu.addItem((ID)Menu::ADD_SAMPLE_CHANNEL, g_ui->getI18Text(LangMap::MAIN_COLUMN_BUTTON_ADDSAMPLECHANNEL));
	menu.addItem((ID)Menu::ADD_MIDI_CHANNEL, g_ui->getI18Text(LangMap::MAIN_COLUMN_BUTTON_ADDMIDICHANNEL));
	menu.addItem((ID)Menu::REMOVE, g_ui->getI18Text(LangMap::MAIN_COLUMN_BUTTON_REMOVE));

	if (countChannels() > 0)
		menu.setEnabled((ID)Menu::REMOVE, false);

	menu.onSelect = [this](ID menuId) {
		switch (static_cast<Menu>(menuId))
		{
		case Menu::ADD_SAMPLE_CHANNEL:
			c::channel::addChannel(id, ChannelType::SAMPLE);
			break;
		case Menu::ADD_MIDI_CHANNEL:
			c::channel::addChannel(id, ChannelType::MIDI);
			break;
		case Menu::REMOVE:
			static_cast<geKeyboard*>(parent())->deleteColumn(id);
			break;
		}
	};

	menu.popup();
}

/* -------------------------------------------------------------------------- */

geChannel* geColumn::getChannel(ID channelId) const
{
	for (geChannel* c : m_channels)
		if (c->getData().id == channelId)
			return c;
	return nullptr;
}

/* -------------------------------------------------------------------------- */

geChannel* geColumn::getChannelAtCursor(Pixel y) const
{
	if (m_channels.empty())
		return nullptr;

	for (geChannel* c : m_channels)
		if (geompp::Range(c->y(), c->y() + c->h()).contains(y))
			return c;

	return nullptr;
}

/* -------------------------------------------------------------------------- */

geChannel* geColumn::getFirstChannel() const
{
	return m_channels.empty() ? nullptr : m_channels.front();
}

geChannel* geColumn::getLastChannel() const
{
	return m_channels.empty() ? nullptr : m_channels.back();
}

/* -------------------------------------------------------------------------- */

void geColumn::init()
{
	clear();
	m_channels.clear();

	m_addChannelBtn          = new geTextButton(g_ui->getI18Text(LangMap::MAIN_COLUMN_BUTTON));
	m_addChannelBtn->onClick = [this]() { showAddChannelMenu(); };

	addWidget(m_addChannelBtn, G_GUI_UNIT);
	addWidget(new geBox(), G_GUI_INNER_MARGIN); // Spacer between 'Edit column' button and channels
	end();
}

/* -------------------------------------------------------------------------- */

void geColumn::forEachChannel(std::function<void(geChannel& c)> f) const
{
	for (geChannel* c : m_channels)
		f(*c);
}

/* -------------------------------------------------------------------------- */

int geColumn::countChannels() const
{
	return m_channels.size();
}

/* -------------------------------------------------------------------------- */

int geColumn::computeHeight() const
{
	const auto last   = child(children() - 1);
	const int  height = (last->y() + last->h()) - y();
	return height;
}
} // namespace giada::v