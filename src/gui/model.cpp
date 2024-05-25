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
Model::Column::Column(int index, int width)
: index(index)
, width(width)
{
}

/* -------------------------------------------------------------------------- */

int Model::Column::getChannelIndex(ID channelId) const
{
	return static_cast<int>(u::vector::indexOf(m_channels, channelId));
}

/* -------------------------------------------------------------------------- */

std::vector<ID> Model::Column::getChannels() const
{
	return m_channels;
}

/* -------------------------------------------------------------------------- */

void Model::Column::addChannel(ID channelId, int position)
{
	if (position == -1)
		m_channels.push_back(channelId);
	else
		m_channels.insert(m_channels.begin() + position, channelId);
}

/* -------------------------------------------------------------------------- */

void Model::Column::removeChannel(ID channelId)
{
	u::vector::remove(m_channels, channelId);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

const std::vector<Model::Column>& Model::Columns::getAll() const
{

	return m_columns;
}

/* -------------------------------------------------------------------------- */

Model::Column& Model::Columns::getColumnByIndex(int index)
{
	assert(index < static_cast<int>(m_columns.size()));

	return m_columns.at(index);
}

/* -------------------------------------------------------------------------- */

Model::Column& Model::Columns::getColumnByChannelId(ID channelId)
{
	const auto p = [channelId](auto& col)
	{
		return u::vector::has(col.getChannels(), [channelId](ID otherId)
		    { return channelId == otherId; });
	};
	return *u::vector::findIfSafe(m_columns, p);
}

/* -------------------------------------------------------------------------- */

void Model::Columns::addDefaultColumn()
{
	const int index = static_cast<int>(m_columns.size());
	const int width = G_DEFAULT_COLUMN_WIDTH;

	addColumn({index, width});
}

/* -------------------------------------------------------------------------- */

void Model::Columns::addColumn(Column&& column)
{
	m_columns.push_back(std::move(column));
}

/* -------------------------------------------------------------------------- */

void Model::Columns::removeColumn(int columnIndex)
{
	m_columns.erase(m_columns.begin() + columnIndex);
}

/* -------------------------------------------------------------------------- */

void Model::Columns::moveChannel(ID channelId, int columnIndex, int newPosition)
{
	const Column& column = getColumnByChannelId(channelId);

	if (column.index == columnIndex) // If in same column
	{
		const int oldPosition = column.getChannelIndex(channelId);
		if (newPosition >= oldPosition) // If moved below, readjust index
			newPosition -= 1;
	}

	removeChannelFromColumn(channelId);
	addChannelToColumn(channelId, columnIndex, newPosition);
}

/* -------------------------------------------------------------------------- */

void Model::Columns::addChannelToColumn(ID channelId, int columnIndex, int position)
{
	getColumnByIndex(columnIndex).addChannel(channelId, position);
}

/* -------------------------------------------------------------------------- */

void Model::Columns::addChannelToGroup(ID channelId, ID groupId)
{
	getColumnByChannelId(groupId).addChannel(channelId, /*position=*/-1);
}

/* -------------------------------------------------------------------------- */

void Model::Columns::removeChannelFromColumn(ID channelId)
{
	for (Column& column : m_columns) // Brute force!
		column.removeChannel(channelId);
}

/* -------------------------------------------------------------------------- */

void Model::Columns::clear()
{
	m_columns.clear();
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

	for (const Column& column : columns.getAll())
	{
		m::Patch::Column pcolumn;
		pcolumn.width = column.width;
		for (ID channelId : column.getChannels())
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
	for (int i = 0; const m::Patch::Column& pcolumn : patch.columns)
	{
		Column column(i++, pcolumn.width);
		for (ID channelId : pcolumn.channels)
			column.addChannel(channelId, /*position=*/-1);
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
