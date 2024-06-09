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
Model::Column::Column(int width)
: width(width)
{
}

/* -------------------------------------------------------------------------- */

int Model::Column::getChannelIndex(ID channelId) const
{
	const Channel* ch = deepFindById<Channel>(channelId);
	assert(ch != nullptr);
	return ch->index;
}

/* -------------------------------------------------------------------------- */

const std::vector<ID>& Model::Column::getChannels() const
{
	return m_channelIds;
}

/* -------------------------------------------------------------------------- */

void Model::Column::addChannel(ID channelId, int position, ID groupId)
{
	if (position == -1)
		position = size();

	if (groupId == 0)
	{
		insert({mcl::ID(channelId)}, position);
	}
	else
	{
		// TODO position not used yet when grouping
		getById(groupId).add({mcl::ID(channelId)});
	}

	rebuildIds();
}

/* -------------------------------------------------------------------------- */

void Model::Column::removeChannel(ID channelId)
{
	deepRemoveById<Channel>({mcl::ID(channelId)});
	rebuildIds();
}

/* -------------------------------------------------------------------------- */

void Model::Column::rebuildIds()
{
	m_channelIds.clear();
	for (const Channel& ch : *this)
	{
		m_channelIds.push_back(ch.id);
		for (const Channel& child : ch)
			m_channelIds.push_back(child.id);
	}
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Model::Column& Model::Columns::getColumnByIndex(int index)
{
	return getByIndex(index);
}

/* -------------------------------------------------------------------------- */

Model::Column& Model::Columns::getColumnByChannelId(ID channelId)
{
	return getIf([channelId](const Column& column)
	    { return column.contains(channelId); });
}

/* -------------------------------------------------------------------------- */

void Model::Columns::addDefaultColumn()
{
	addColumn({G_DEFAULT_COLUMN_WIDTH});
}

/* -------------------------------------------------------------------------- */

void Model::Columns::addColumn(Column&& column)
{
	add(std::move(column));
}

/* -------------------------------------------------------------------------- */

void Model::Columns::removeColumn(int columnIndex)
{
	removeByIndex(columnIndex);
}

/* -------------------------------------------------------------------------- */

void Model::Columns::moveChannel(ID channelId, int columnIndex, int newPosition)
{
	const Column&        column      = getColumnByChannelId(channelId);
	std::vector<mcl::ID> childrenIds = column.getById(channelId).getItemIds();

	if (column.index == columnIndex) // If in same column
	{
		const int oldPosition = column.getChannelIndex(channelId);
		if (newPosition >= oldPosition) // If moved below, readjust index
			newPosition -= 1;
	}

	removeChannel(channelId);
	addChannelToColumn(channelId, columnIndex, newPosition);

	// Add also children channels, if any
	for (const mcl::ID childId : childrenIds)
		addChannelToGroup(childId, channelId);
}

/* -------------------------------------------------------------------------- */

void Model::Columns::addChannelToColumn(ID channelId, int columnIndex, int position)
{
	getColumnByIndex(columnIndex).addChannel(channelId, position, /*groupId=*/0);
}

/* -------------------------------------------------------------------------- */

void Model::Columns::addChannelToGroup(ID channelId, ID groupId)
{
	getColumnByChannelId(groupId).addChannel(channelId, /*position=*/-1, groupId);
}

/* -------------------------------------------------------------------------- */

void Model::Columns::removeChannel(ID channelId)
{
	for (Column& column : *this) // Brute force!
		column.removeChannel(channelId);
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

	for (const Column& column : columns)
	{
		m::Patch::Column pcolumn;
		pcolumn.width = column.width;
		for (const ID channelId : column.getChannels())
			pcolumn.channels.push_back(channelId);
		patch.columns.push_back(pcolumn);
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
	columns.clear();
	for (const m::Patch::Column& pcolumn : patch.columns)
	{
		Column column(pcolumn.width);
		for (ID channelId : pcolumn.channels)
			column.addChannel(channelId, /*position=*/-1, /*groupId=*/0);
		columns.addColumn(std::move(column));
	}

	projectName = patch.name;
}

/* -------------------------------------------------------------------------- */

void Model::reset()
{
	columns.clear();

	/* Add 6 empty columns as initial layout. */

	columns.addDefaultColumn();
	columns.addDefaultColumn();
	columns.addDefaultColumn();
	columns.addDefaultColumn();
	columns.addDefaultColumn();
	columns.addDefaultColumn();
}
} // namespace giada::v
