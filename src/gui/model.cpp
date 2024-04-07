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
		for (const v::Model::Channel& channel : column.channels.getAll())
			pcolumn.channels.push_back(channel.id);
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
		int    columnIndex = 0;
		Column column{.width = pcolumn.width};
		for (ID channelId : pcolumn.channels)
			column.channels.add({channelId, columnIndex++});
		columns.add(std::move(column));
	}

	projectName = patch.name;
}

/* -------------------------------------------------------------------------- */

Model::Column& Model::getColumnByIndex(int index)
{
	return columns.getByIndex(index);
}

/* -------------------------------------------------------------------------- */

Model::Column& Model::getColumnByChannelId(ID channelId)
{
	return *u::vector::findIfSafe(columns.getAll(), [channelId](auto& col)
	    { return col.channels.findById(channelId) != nullptr; });
}

/* -------------------------------------------------------------------------- */

void Model::addColumn()
{
	columns.add({G_DEFAULT_COLUMN_WIDTH});
}

/* -------------------------------------------------------------------------- */

void Model::removeColumn(int columnIndex)
{
	columns.removeByIndex(columnIndex);
}

/* -------------------------------------------------------------------------- */

void Model::moveChannel(ID channelId, int newColumnIndex, int newPosition)
{
	Column& sourceColumn = getColumnByChannelId(channelId);

	if (sourceColumn.index == newColumnIndex) // If in same column
	{
		sourceColumn.channels.moveById(channelId, newPosition);
	}
	else
	{
		Channel channel      = sourceColumn.channels.getById(channelId);
		Column& targetColumn = getColumnByIndex(newColumnIndex);
		sourceColumn.channels.removeById(channelId);
		targetColumn.channels.insert(std::move(channel), newPosition);
	}
}

/* -------------------------------------------------------------------------- */

void Model::addChannelToColumn(ID channelId, int columnIndex, int position)
{
	Column& column = getColumnByIndex(columnIndex);
	if (position == -1)
		column.channels.add({channelId, columnIndex});
	else
		column.channels.insert({channelId, columnIndex}, position);
}

/* -------------------------------------------------------------------------- */

void Model::addChannelToGroup(ID channelId, ID groupId, int position)
{
	Column&  column = getColumnByChannelId(groupId);
	Channel& group  = column.channels.getById(groupId);
	if (position == -1)
		group.channels.add({channelId, column.index});
	else
		group.channels.insert({channelId, column.index}, position);
}

/* -------------------------------------------------------------------------- */

void Model::removeChannelFromColumn(ID channelId)
{
	for (Column& column : columns.getAll()) // Brute force!
	{
		column.channels.removeById(channelId);
		for (Channel& channel : column.channels.getAll())
			channel.channels.removeById(channelId);
	}
}

/* -------------------------------------------------------------------------- */

void Model::reset()
{
	columns.clear();

	/* Add 6 empty columns as initial layout. */

	addColumn();
	addColumn();
	addColumn();
	addColumn();
	addColumn();
	addColumn();
}
} // namespace giada::v
