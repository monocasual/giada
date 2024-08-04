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

#ifndef G_V_MODEL_H
#define G_V_MODEL_H

#include "core/conf.h"
#include "core/const.h"
#include "core/plugins/pluginManager.h"
#include "deps/geompp/src/rect.hpp"
#include <FL/Enumerations.H>
#include <string>
#include <vector>

namespace giada::v
{
struct Model
{
	struct Track
	{
		int getChannelIndex(ID) const;

		int             index;
		int             width;
		std::vector<ID> channels = {};
	};

	struct Tracks
	{
		const std::vector<Track>& getAll() const;

		Track& getTrackByIndex(int);
		Track& getTrackByChannelId(ID);
		void   addDefaultTrack();
		void   addTrack(Track&&);
		void   removeTrack(int trackIndex);
		void   moveChannel(ID channelId, int trackIndex, int newPosition);
		void   addChannelToTrack(ID channelId, int trackIndex, int position = -1);
		void   removeChannelFromTrack(ID channelId);
		void   clear();

	private:
		std::vector<Track> m_tracks;
	};

	Model();

	void store(m::Conf&) const;
	void store(m::Patch&) const;

	void load(const m::Conf&);
	void load(const m::Patch&);

	/* reset
	Resets the Model to the latest state loaded from m::Conf. Call this when you
	load a new patch and you want to reset the column layout. */

	void reset();

	int         logMode      = LOG_MODE_MUTE;
	bool        showTooltips = true;
	std::string langMap      = "";
	std::string pluginPath   = "";
	std::string patchPath    = "";
	std::string samplePath   = "";
	std::string projectName  = "";

	geompp::Rect<int> mainWindowBounds = {-1, -1, G_MIN_GUI_WIDTH, G_MIN_GUI_HEIGHT};

	geompp::Rect<int> browserBounds = {-1, -1, G_DEFAULT_SUBWINDOW_W, G_DEFAULT_SUBWINDOW_W};
	int               browserPosition;
	int               browserLastValue;
	std::string       browserLastPath;

	geompp::Rect<int> actionEditorBounds     = {-1, -1, G_DEFAULT_SUBWINDOW_W, G_DEFAULT_SUBWINDOW_W};
	int               actionEditorZoom       = G_DEFAULT_ZOOM_RATIO;
	int               actionEditorSplitH     = -1;
	int               actionEditorGridVal    = 0;
	int               actionEditorGridOn     = false;
	int               actionEditorPianoRollY = -1;

	geompp::Rect<int> sampleEditorBounds  = {-1, -1, G_DEFAULT_SUBWINDOW_W, G_DEFAULT_SUBWINDOW_W};
	int               sampleEditorGridVal = 1;
	int               sampleEditorGridOn  = false;

	geompp::Rect<int> midiInputBounds  = {-1, -1, G_DEFAULT_SUBWINDOW_W, G_DEFAULT_SUBWINDOW_W};
	geompp::Rect<int> pluginListBounds = {-1, -1, 468, 204};

	geompp::Rect<int>          pluginChooserBounds   = {-1, -1, G_DEFAULT_SUBWINDOW_W, G_DEFAULT_SUBWINDOW_W};
	m::PluginManager::SortMode pluginChooserSortMode = {m::PluginManager::SortMethod::NAME, m::PluginManager::SortDir::ASC};

	int keyBindPlay          = ' ';
	int keyBindRewind        = FL_BackSpace;
	int keyBindRecordActions = FL_Enter;
	int keyBindRecordInput   = FL_End;
	int keyBindExit          = FL_Escape;

	float uiScaling = G_DEFAULT_UI_SCALING;

	Tracks tracks;
};
} // namespace giada::v

#endif