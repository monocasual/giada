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
Model::Channel::Channel(ID id, ID groupId, std::size_t columnIndex)
: Container(id)
, groupId(groupId)
, columnIndex(columnIndex)
{
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Model::Column::Column(int width)
: width(width)
{
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Model::Column& Model::Columns::getColumnByIndex(std::size_t index)
{
	return getByIndex(index);
}

/* -------------------------------------------------------------------------- */

Model::Column& Model::Columns::getColumnByChannelId(ID channelId)
{
	return *u::vector::findIfSafe(*this, [channelId](auto& col)
	    { return col.findById(channelId) != nullptr; });
}

/* -------------------------------------------------------------------------- */

void Model::Columns::addColumn()
{
	add({G_DEFAULT_COLUMN_WIDTH});
}

/* -------------------------------------------------------------------------- */

void Model::Columns::removeColumn(int columnIndex)
{
	removeByIndex(columnIndex);
}

/* -------------------------------------------------------------------------- */

void Model::Columns::moveChannel(ID channelId, std::size_t newColumnIndex, int newPosition)
{
	Column& sourceColumn = getColumnByChannelId(channelId);

	if (sourceColumn.index == newColumnIndex) // If in same column
	{
		sourceColumn.moveById(channelId, newPosition);
	}
	else
	{
		Channel channel      = sourceColumn.getById(channelId);
		Column& targetColumn = getColumnByIndex(newColumnIndex);
		sourceColumn.removeById(channelId);
		targetColumn.insert(std::move(channel), newPosition);
	}
}

/* -------------------------------------------------------------------------- */

void Model::Columns::addChannel(ID channelId, std::size_t columnIndex, ID groupId)
{
	Channel channel = {channelId, groupId, columnIndex};
	Column& column  = getByIndex(columnIndex);

	if (groupId == 0)
		column.add(std::move(channel));
	else
		column.getById(groupId).add(std::move(channel));
}

/* -------------------------------------------------------------------------- */

void Model::Columns::removeChannelFromColumn(ID channelId)
{
	for (Column& column : *this) // Brute force!
	{
		column.removeById(channelId);
		for (Channel& channel : column)
			channel.removeById(channelId);
	}
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
		for (const v::Model::Channel& channel : column)
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
		std::size_t columnIndex = 0;
		Column      column{pcolumn.width};
		for (ID channelId : pcolumn.channels)
			column.add({channelId, /* TODO - group persistence*/ 0, columnIndex++});
		columns.add(std::move(column));
	}

	projectName = patch.name;
}

/* -------------------------------------------------------------------------- */

void Model::reset()
{
	columns.clear();

	/* Add 6 empty columns as initial layout. */

	columns.addColumn();
	columns.addColumn();
	columns.addColumn();
	columns.addColumn();
	columns.addColumn();
	columns.addColumn();
}
} // namespace giada::v
