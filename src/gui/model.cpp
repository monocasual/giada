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

#include "gui/model.h"
#include "core/const.h"
#include "core/patch.h"
#include "utils/vector.h"

namespace giada::v
{
int Model::Track::getChannelIndex(ID channelId) const
{
	return static_cast<int>(u::vector::indexOf(channels, channelId));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

const std::vector<Model::Track>& Model::Tracks::getAll() const
{

	return m_tracks;
}

/* -------------------------------------------------------------------------- */

Model::Track& Model::Tracks::getTrackByIndex(int index)
{
	assert(index < static_cast<int>(m_columns.size()));

	return m_tracks.at(index);
}

/* -------------------------------------------------------------------------- */

Model::Track& Model::Tracks::getTrackByChannelId(ID channelId)
{
	const auto p = [channelId](auto& col)
	{
		return u::vector::has(col.channels, [channelId](ID otherId)
		{ return channelId == otherId; });
	};
	return *u::vector::findIfSafe(m_tracks, p);
}

/* -------------------------------------------------------------------------- */

void Model::Tracks::addDefaultTrack()
{
	const int index = static_cast<int>(m_tracks.size());
	const int width = G_DEFAULT_COLUMN_WIDTH;

	addTrack({index, width});
}

/* -------------------------------------------------------------------------- */

void Model::Tracks::addTrack(Track&& track)
{
	m_tracks.push_back(std::move(track));
}

/* -------------------------------------------------------------------------- */

void Model::Tracks::removeTrack(int trackIndex)
{
	m_tracks.erase(m_tracks.begin() + trackIndex);
}

/* -------------------------------------------------------------------------- */

void Model::Tracks::moveChannel(ID channelId, int trackIndex, int newPosition)
{
	const Track& track = getTrackByChannelId(channelId);

	if (track.index == trackIndex) // If in same track
	{
		const int oldPosition = track.getChannelIndex(channelId);
		if (newPosition >= oldPosition) // If moved below, readjust index
			newPosition -= 1;
	}

	removeChannelFromTrack(channelId);
	addChannelToTrack(channelId, trackIndex, newPosition);
}

/* -------------------------------------------------------------------------- */

void Model::Tracks::addChannelToTrack(ID channelId, int trackIndex, int position)
{
	std::vector<ID>& channels = getTrackByIndex(trackIndex).channels;
	if (position == -1)
		channels.push_back(channelId);
	else
		channels.insert(channels.begin() + position, channelId);
}

/* -------------------------------------------------------------------------- */

void Model::Tracks::removeChannelFromTrack(ID channelId)
{
	for (Track& track : m_tracks) // Brute force!
		u::vector::remove(track.channels, channelId);
}

/* -------------------------------------------------------------------------- */

void Model::Tracks::clear()
{
	m_tracks.clear();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Model::Model()
{
	reset();
}

/* -------------------------------------------------------------------------- */

void Model::store(m::Conf& conf) const
{
	conf.logMode      = logMode;
	conf.showTooltips = showTooltips;
	conf.langMap      = langMap;
	conf.pluginPath   = pluginPath;
	conf.patchPath    = patchPath;
	conf.samplePath   = samplePath;

	conf.mainWindowBounds = mainWindowBounds;

	conf.browserBounds    = browserBounds;
	conf.browserPosition  = browserPosition;
	conf.browserLastValue = browserLastValue;
	conf.browserLastPath  = browserLastPath;

	conf.actionEditorBounds     = actionEditorBounds;
	conf.actionEditorZoom       = actionEditorZoom;
	conf.actionEditorSplitH     = actionEditorSplitH;
	conf.actionEditorGridVal    = actionEditorGridVal;
	conf.actionEditorGridOn     = actionEditorGridOn;
	conf.actionEditorPianoRollY = actionEditorPianoRollY;

	conf.sampleEditorBounds  = sampleEditorBounds;
	conf.sampleEditorGridVal = sampleEditorGridVal;
	conf.sampleEditorGridOn  = sampleEditorGridOn;

	conf.midiInputBounds = midiInputBounds;

	conf.pluginListBounds = pluginListBounds;

	conf.pluginChooserBounds = pluginChooserBounds;
	conf.pluginSortMode      = pluginChooserSortMode;

	conf.keyBindPlay          = keyBindPlay;
	conf.keyBindRewind        = keyBindRewind;
	conf.keyBindRecordActions = keyBindRecordActions;
	conf.keyBindRecordInput   = keyBindRecordInput;
	conf.keyBindExit          = keyBindExit;

	conf.uiScaling = uiScaling;
}

/* -------------------------------------------------------------------------- */

void Model::store(m::Patch& patch) const
{
	patch.name = projectName;

	for (const Track& track : tracks.getAll())
	{
		m::Patch::Track ptrack;
		ptrack.width = track.width;
		for (ID channelId : track.channels)
			ptrack.channels.push_back(channelId);
		patch.tracks.push_back(ptrack);
	}
}

/* -------------------------------------------------------------------------- */

void Model::load(const m::Conf& conf)
{
	logMode          = conf.logMode;
	showTooltips     = conf.showTooltips;
	langMap          = conf.langMap;
	pluginPath       = conf.pluginPath;
	patchPath        = conf.patchPath;
	samplePath       = conf.samplePath;
	mainWindowBounds = conf.mainWindowBounds;

	browserBounds    = conf.browserBounds;
	browserPosition  = conf.browserPosition;
	browserLastValue = conf.browserLastValue;
	browserLastPath  = conf.browserLastPath;

	actionEditorBounds     = conf.actionEditorBounds;
	actionEditorZoom       = conf.actionEditorZoom;
	actionEditorSplitH     = conf.actionEditorSplitH;
	actionEditorGridVal    = conf.actionEditorGridVal;
	actionEditorGridOn     = conf.actionEditorGridOn;
	actionEditorPianoRollY = conf.actionEditorPianoRollY;

	sampleEditorBounds  = conf.sampleEditorBounds;
	sampleEditorGridVal = conf.sampleEditorGridVal;
	sampleEditorGridOn  = conf.sampleEditorGridOn;

	midiInputBounds  = conf.midiInputBounds;
	pluginListBounds = conf.pluginListBounds;

	pluginChooserBounds   = conf.pluginChooserBounds;
	pluginChooserSortMode = conf.pluginSortMode;

	keyBindPlay          = conf.keyBindPlay;
	keyBindRewind        = conf.keyBindRewind;
	keyBindRecordActions = conf.keyBindRecordActions;
	keyBindRecordInput   = conf.keyBindRecordInput;
	keyBindExit          = conf.keyBindExit;

	uiScaling = conf.uiScaling;
}

/* -------------------------------------------------------------------------- */

void Model::load(const m::Patch& patch)
{
	tracks.clear();
	for (int i = 0; const m::Patch::Track& ptrack : patch.tracks)
	{
		Track track{.index = i++, .width = ptrack.width};
		for (ID channelId : ptrack.channels)
			track.channels.push_back(channelId);
		tracks.addTrack(std::move(track));
	}

	projectName = patch.name;
}

/* -------------------------------------------------------------------------- */

void Model::reset()
{
	tracks.clear();

	/* Add 6 empty tracks as initial layout. */

	tracks.addDefaultTrack();
	tracks.addDefaultTrack();
	tracks.addDefaultTrack();
	tracks.addDefaultTrack();
	tracks.addDefaultTrack();
	tracks.addDefaultTrack();
}
} // namespace giada::v
