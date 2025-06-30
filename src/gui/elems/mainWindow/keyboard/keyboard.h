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

#ifndef GE_KEYBOARD_H
#define GE_KEYBOARD_H

#include "deps/geompp/src/rect.hpp"
#include "gui/elems/basics/scroll.h"
#include "gui/model.h"
#include <FL/Fl_Box.H>
#include <functional>
#include <memory>
#include <vector>

namespace giada::c::channel
{
struct Track;
}

namespace giada::v
{
class geResizerBar;
class geTrack;
class geChannel;
class geKeyboard : public geScroll
{
public:
	friend class ChannelDragger;

	geKeyboard();

	int  handle(int e) override;
	void draw() override;

	/* getChannelTrackIndex
	Given a channel ID, returns the index of the track it belongs to. */

	std::size_t getChannelTrackIndex(ID channelId) const;

	/* countTracks
	Returns the number of tracks in the viewport. */

	size_t countTracks() const;

	/* showMenu
	Displays the menu for adding/removing tracks. */

	void showMenu() const;

	/* rebuild
	Rebuilds this widget from scratch. Used when the model has changed. */

	void rebuild();

	/* refresh
	Refreshes each track's channel, called on each GUI cycle. */

	void refresh();

	/* addTrack
	Adds new track at the end of the stack. */

	void addTrack() const;

	/* deleteTrack
	Deletes track by index. */

	void deleteTrack(std::size_t);

	/* deleteAllTracks
	Deletes all tracks from the stack. */

	void deleteAllTracks();

	void setChannelVolume(ID channelId, float v);
	void notifyMidiIn(ID channelId);
	void notifyMidiOut(ID channelId);

	/* init
	Builds the default setup of empty tracks. */

	void init();

	void forEachChannel(std::function<void(geChannel& c)> f) const;
	void forEachTrack(std::function<void(const geTrack& c)> f) const;

private:
	static constexpr int TRACK_GAP = 20;

	class ChannelDragger
	{
	public:
		ChannelDragger(geKeyboard&);

		bool isDragging() const;

		void begin();
		void drag();
		void end();

	private:
		Fl_Box*     m_placeholder;
		geKeyboard& m_keyboard;
		ID          m_channelId;
		int         m_xoffset;

		/* getPositionForCursor
		Given a geTrack and a y-coordinate, returns the channel position for
		that point, taking empty spaces into account. */

		int getPositionForCursor(const geTrack*, Pixel y) const;
	};

	/* getTrackBackround
	Returns a geompp::Rect for the square to be drawn as track background. */

	geompp::Rect<int> getTrackBackround(const geTrack&) const;

	/* getDroppedFilePaths
	Returns a vector of audio file paths after a drag-n-drop from desktop
	event. */

	std::vector<std::string> getDroppedFilePaths() const;

	/* openMenu
	Opens the Track menu for the track under the cursor, or the generic one if
	no tracks hovered. */

	void openMenu() const;

	/* getTrackAtCursor
	Returns the track below the cursor. */

	geTrack* getTrackAtCursor(Pixel x) const;

	/* getChannel
	Given a channel ID returns the UI channel it belongs to. */

	geChannel*       getChannel(ID channelId);
	const geChannel* getChannel(ID channelId) const;

	/* addTrack
	Adds new track at the end of the stack. Pass Track::id != 0 when loading
	tracks from model. */

	geTrack& addTrack(const c::channel::Track&);

	ChannelDragger        m_channelDragger;
	std::vector<geTrack*> m_tracks;
};
} // namespace giada::v

#endif
