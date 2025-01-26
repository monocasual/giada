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

#include "gui/model.h"
#include "core/const.h"
#include "core/patch.h"
#include "utils/vector.h"

namespace giada::v
{
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
	projectName = patch.name;
}
} // namespace giada::v
