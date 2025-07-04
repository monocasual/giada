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

#include "src/gui/elems/mainWindow/keyboard/keyboard.h"
#include "src/glue/channel.h"
#include "src/glue/io.h"
#include "src/gui/const.h"
#include "src/gui/drawing.h"
#include "src/gui/elems/basics/dial.h"
#include "src/gui/elems/basics/menu.h"
#include "src/gui/elems/basics/resizerBar.h"
#include "src/gui/elems/basics/textButton.h"
#include "src/gui/elems/mainWindow/keyboard/channelButton.h"
#include "src/gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "src/gui/elems/mainWindow/keyboard/track.h"
#include "src/gui/elems/midiActivity.h"
#include "src/gui/ui.h"
#include "src/utils/fs.h"
#include "src/utils/gui.h"
#include "src/utils/log.h"
#include "src/utils/string.h"
#include <cassert>

extern giada::v::Ui* g_ui;

namespace giada::v
{
namespace
{
enum class Menu
{
	ADD_TRACK = 0
};
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

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
	const geTrack* track = m_keyboard.getTrackAtCursor(Fl::event_x());
	if (track == nullptr)
		return;

	const geChannel* channel = track->getChannelAtCursor(Fl::event_y());
	if (channel == nullptr || channel->isGroup())
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

	assert(m_keyboard.m_tracks.size() > 0);

	const geTrack* firstTrack = m_keyboard.m_tracks[0];
	const geTrack* lastTrack  = m_keyboard.m_tracks.back();

	const int minx = firstTrack->x();
	const int maxx = lastTrack->x() + lastTrack->w() - m_placeholder->w();
	const int miny = firstTrack->y();

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

	const geTrack* track = m_keyboard.getTrackAtCursor(Fl::event_x());
	if (track == nullptr)
	{
		m_channelId = -1;
		m_xoffset   = 0;
		m_keyboard.rebuild(); // Just cleanup the UI
		return;
	}

	const std::size_t targetTrackIndex = track->index;
	const int         targetPosition   = getPositionForCursor(track, Fl::event_y());

	c::channel::moveChannel(m_channelId, targetTrackIndex, targetPosition);

	m_channelId = -1;
	m_xoffset   = 0;
	m_keyboard.remove(m_placeholder);
}

/* -------------------------------------------------------------------------- */

int geKeyboard::ChannelDragger::getPositionForCursor(const geTrack* track, Pixel y) const
{
	const geChannel* lastChannel = track->getLastChannel();
	if (lastChannel == nullptr) // Track is empty
		return 0;

	const geChannel* targetChannel = track->getChannelAtCursor(y);
	if (targetChannel != nullptr)
		return static_cast<int>(targetChannel->getData().channelIndex + 1);

	/* Channel not found, case 1: the cursor could be above the first channel.
	Channel not found, case 2: the cursor could be below the last channel, over
	the empty space at the bottom of the track. */

	const geChannel* firstChannel = track->getFirstChannel();
	if (y < firstChannel->y())
		return 0;
	return static_cast<int>(lastChannel->getData().channelIndex + 1);
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

std::size_t geKeyboard::getChannelTrackIndex(ID channelId) const
{
	return getChannel(channelId)->getTrackIndex();
}

/* -------------------------------------------------------------------------- */

size_t geKeyboard::countTracks() const
{
	return m_tracks.size();
}

/* -------------------------------------------------------------------------- */

void geKeyboard::showMenu() const
{
	geMenu menu;

	menu.addItem((ID)Menu::ADD_TRACK, g_ui->getI18Text(LangMap::MAIN_TRACK_BUTTON_ADD_TRACK));

	menu.onSelect = [this](ID menuId)
	{
		switch (static_cast<Menu>(menuId))
		{
		case Menu::ADD_TRACK:
			addTrack();
			break;
		}
	};

	menu.popup();
}

/* -------------------------------------------------------------------------- */

void geKeyboard::init()
{
	deleteAllTracks();
}

/* -------------------------------------------------------------------------- */

void geKeyboard::rebuild()
{
	deleteAllTracks();
	for (const c::channel::Track& c : c::channel::getTracks())
		addTrack(c);
	redraw();
}

/* -------------------------------------------------------------------------- */

void geKeyboard::deleteTrack(std::size_t index)
{
	c::channel::deleteTrack(index);
}

/* -------------------------------------------------------------------------- */

void geKeyboard::deleteAllTracks()
{
	Fl_Scroll::clear();
	m_tracks.clear();
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
	for (geTrack* c : m_tracks)
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
			openMenu();
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
		const geTrack* c = getTrackAtCursor(Fl::event_x());
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

	/* Paint tracks background. Use a clip to draw only what's visible. */

	const geompp::Rect<int> viewportBounds = getViewportBounds();

	fl_push_clip(viewportBounds.x, viewportBounds.y, viewportBounds.w, viewportBounds.h);

	for (const geTrack* c : m_tracks)
	{
		const geompp::Rect background = getTrackBackround(*c);

		drawRectf(background, G_COLOR_GREY_1_5);
		drawRect(background, G_COLOR_GREY_2);
		if (background.h >= c->endMargin)
		{
			const std::string text = u::gui::truncate(g_ui->getI18Text(LangMap::MAIN_TRACK_HELP), c->w() - G_GUI_UNIT);
			drawText(text, background, FL_HELVETICA, G_GUI_FONT_SIZE_BASE, G_COLOR_GREY_3);
		}
	}

	draw_children();

	fl_pop_clip();
}

/* -------------------------------------------------------------------------- */

geompp::Rect<int> geKeyboard::getTrackBackround(const geTrack& c) const
{
	const geompp::Rect  trackBounds(c.x(), c.y(), c.w(), c.h() - c.endMargin);
	const geompp::Rect  viewportBounds = getViewportBounds();
	const geompp::Range trackRange     = trackBounds.getHeightAsRange();
	const geompp::Range thisRange      = viewportBounds.getHeightAsRange();

	/* Track is empty (c.h() == 0) or away from the viewport: just return a
	full-size background. */

	if (c.h() == 0 || !thisRange.intersects(trackRange))
		return viewportBounds.withX(c.x()).withW(c.w());

	const auto [_, r2] = thisRange.getDifference(trackRange);

	if (!r2.isValid())
		return {};
	return trackBounds.withVerticalRange(r2);
}

/* -------------------------------------------------------------------------- */

void geKeyboard::addTrack() const
{
	c::channel::addTrack();
}

/* -------------------------------------------------------------------------- */

geTrack& geKeyboard::addTrack(const c::channel::Track& trackModel)
{
	int colx = x() - xposition(); // Mind the x-scroll offset with xposition()

	/* If this is not the first track... */

	if (m_tracks.size() > 0)
		colx = m_tracks.back()->x() + m_tracks.back()->w() + TRACK_GAP;

	/* Add a new track + a new resizer bar. */

	const int viewportH = getViewportBounds().h;

	geResizerBar* bar   = new geResizerBar(colx + trackModel.width, y(), TRACK_GAP, viewportH, G_MIN_TRACK_WIDTH, Direction::HORIZONTAL);
	geTrack*      track = new geTrack(colx, y(), trackModel.width, 0, trackModel.index, bar);

	/* Store the track width in the model when the resizer bar is released. */

	bar->onRelease = [](const Fl_Widget& w)
	{
		const geTrack& track = static_cast<const geTrack&>(w);
		c::channel::setTrackWidth(track.index, track.w());
	};

	add(track);
	add(bar);
	m_tracks.push_back(track);

	/* Fill track with channels. */

	for (const c::channel::Data& ch : trackModel.channels)
		track->addChannel(ch);

	redraw();

	return *track;
}

/* -------------------------------------------------------------------------- */

void geKeyboard::forEachChannel(std::function<void(geChannel& c)> f) const
{
	for (geTrack* track : m_tracks)
		track->forEachChannel(f);
}

void geKeyboard::forEachTrack(std::function<void(const geTrack& c)> f) const
{
	for (geTrack* track : m_tracks)
		f(*track);
}

/* -------------------------------------------------------------------------- */

geTrack* geKeyboard::getTrackAtCursor(Pixel px) const
{
	for (geTrack* c : m_tracks)
		if (geompp::Range(c->x(), c->x() + c->w()).contains(px))
			return c;
	return nullptr;
}

/* -------------------------------------------------------------------------- */

const geChannel* geKeyboard::getChannel(ID channelId) const
{
	for (geTrack* track : m_tracks)
	{
		geChannel* c = track->getChannel(channelId);
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

void geKeyboard::openMenu() const
{
	const geTrack* track = getTrackAtCursor(Fl::event_x());
	if (track == nullptr) // No track hovered: show Keyboard menu
	{
		showMenu();
		return;
	}
	if (track->getChannelAtCursor(Fl::event_y()) == nullptr) // No channel hovered: show track menu
		track->showMenu();
}
} // namespace giada::v
