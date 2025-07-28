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

#ifndef G_CONF_H
#define G_CONF_H

#include "src/core/const.h"
#include "src/core/resampler.h"
#include "src/core/types.h"
#include "src/deps/geompp/src/rect.hpp"
#include "src/deps/rtaudio/RtAudio.h"
#include "src/gui/const.h"
#include "src/types.h"
#include <FL/Enumerations.H>
#include <RtMidi.h>
#include <set>
#include <string>
#include <vector>

namespace giada::m
{
struct Conf final
{
	bool               valid            = false;
	int                logMode          = LOG_MODE_MUTE;
	bool               showTooltips     = true;
	std::string        langMap          = "";
	RtAudio::Api       soundSystem      = G_DEFAULT_SOUNDSYS;
	int                soundDeviceOut   = G_DEFAULT_SOUNDDEV_OUT;
	int                soundDeviceIn    = G_DEFAULT_SOUNDDEV_IN;
	int                channelsOutCount = G_MAX_IO_CHANS;
	int                channelsOutStart = 0;
	int                channelsInCount  = 1;
	int                channelsInStart  = 0;
	int                samplerate       = G_DEFAULT_SAMPLERATE;
	int                buffersize       = G_DEFAULT_BUFSIZE;
	bool               limitOutput      = false;
	Resampler::Quality rsmpQuality      = Resampler::Quality::SINC_BEST;

	RtMidi::Api           midiSystem = G_DEFAULT_MIDI_API;
	std::set<std::size_t> midiDevicesOut;
	std::set<std::size_t> midiDevicesIn;
	std::string           midiMapPath = "";
	int                   midiSync    = G_MIDI_SYNC_NONE;
	float                 midiTCfps   = 25.0f;

	bool chansStopOnSeqHalt         = false;
	bool treatRecsAsLoops           = false;
	bool inputMonitorDefaultOn      = false;
	bool overdubProtectionDefaultOn = false;

	std::string pluginPath;
	std::string patchPath;
	std::string samplePath;

	geompp::Rect<int> mainWindowBounds = {-1, -1, G_MIN_GUI_WIDTH, G_MIN_GUI_HEIGHT};

	geompp::Rect<int> settingsBounds = {-1, -1, G_DEFAULT_SUBWINDOW_W, G_DEFAULT_SUBWINDOW_W};

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

	geompp::Rect<int> midiInputBounds      = {-1, -1, G_DEFAULT_SUBWINDOW_W, G_DEFAULT_SUBWINDOW_W};
	geompp::Rect<int> pluginListBounds     = {-1, -1, 468, 204};
	geompp::Rect<int> channelRoutingBounds = {-1, -1, 260, 176};

	RecTriggerMode recTriggerMode  = RecTriggerMode::NORMAL;
	float          recTriggerLevel = G_DEFAULT_REC_TRIGGER_LEVEL;
	InputRecMode   inputRecMode    = InputRecMode::FREE;

	bool     midiInEnabled    = false;
	int      midiInFilter     = -1;
	uint32_t midiInRewind     = 0x0;
	uint32_t midiInStartStop  = 0x0;
	uint32_t midiInActionRec  = 0x0;
	uint32_t midiInInputRec   = 0x0;
	uint32_t midiInMetronome  = 0x0;
	uint32_t midiInVolumeIn   = 0x0;
	uint32_t midiInVolumeOut  = 0x0;
	uint32_t midiInBeatDouble = 0x0;
	uint32_t midiInBeatHalf   = 0x0;

	geompp::Rect<int> pluginChooserBounds = {-1, -1, G_DEFAULT_SUBWINDOW_W, G_DEFAULT_SUBWINDOW_W};
	PluginSortMode    pluginSortMode      = {PluginSortMethod::NAME, PluginSortDir::ASC};

	int keyBindPlay          = ' ';
	int keyBindRewind        = FL_BackSpace;
	int keyBindRecordActions = FL_Enter;
	int keyBindRecordInput   = FL_End;
	int keyBindExit          = FL_Escape;

	float uiScaling = G_DEFAULT_UI_SCALING;
};
} // namespace giada::m

#endif
