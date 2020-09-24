/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#include <string>
#include "utils/gui.h"
#include "core/const.h"
#include "core/types.h"


namespace giada {
namespace m {
namespace conf
{
struct Conf
{
	int  logMode         = LOG_MODE_MUTE;
	int  soundSystem     = G_DEFAULT_SOUNDSYS;
	int  soundDeviceOut  = G_DEFAULT_SOUNDDEV_OUT;
	int  soundDeviceIn   = G_DEFAULT_SOUNDDEV_IN;
	int  channelsOut     = 0;
	int  channelsInCount = 0;
	int  channelsInStart = 0;
	int  samplerate      = G_DEFAULT_SAMPLERATE;
	int  buffersize      = G_DEFAULT_BUFSIZE;
	bool limitOutput     = false;
	int  rsmpQuality     = 0;

	int         midiSystem  = 0;
	int         midiPortOut = G_DEFAULT_MIDI_PORT_OUT;
	int         midiPortIn  = G_DEFAULT_MIDI_PORT_IN;
	std::string midiMapPath = "";
	std::string lastFileMap = "";
	int         midiSync    = MIDI_SYNC_NONE;
	float       midiTCfps   = 25.0f;

	bool chansStopOnSeqHalt         = false;
	bool treatRecsAsLoops           = false;
	bool inputMonitorDefaultOn      = false;
	bool overdubProtectionDefaultOn = false;

	std::string pluginPath;
	std::string patchPath;
	std::string samplePath;

	int mainWindowX = u::gui::centerWindowX(G_MIN_GUI_WIDTH);
	int mainWindowY = u::gui::centerWindowY(G_MIN_GUI_HEIGHT);
	int mainWindowW = G_MIN_GUI_WIDTH;
	int mainWindowH = G_MIN_GUI_HEIGHT;

	int         browserX = u::gui::centerWindowX(G_DEFAULT_SUBWINDOW_W);
	int         browserY = u::gui::centerWindowY(G_DEFAULT_SUBWINDOW_H); 
	int         browserW = G_DEFAULT_SUBWINDOW_W; 
	int         browserH = G_DEFAULT_SUBWINDOW_H;
	int         browserPosition;
	int         browserLastValue;
	std::string browserLastPath;

	int actionEditorY       = u::gui::centerWindowY(G_DEFAULT_SUBWINDOW_H);
	int actionEditorX       = u::gui::centerWindowX(G_DEFAULT_SUBWINDOW_W);
	int actionEditorW       = G_DEFAULT_SUBWINDOW_W; 
	int actionEditorH       = G_DEFAULT_SUBWINDOW_H; 
	int actionEditorZoom    = 100;
	int actionEditorGridVal = 0;
	int actionEditorGridOn  = false;

	int sampleEditorX;
	int sampleEditorY;
	int sampleEditorW = G_DEFAULT_SUBWINDOW_W;
	int sampleEditorH = G_DEFAULT_SUBWINDOW_H;
	int sampleEditorGridVal = 0;
	int sampleEditorGridOn  = false;

	int midiInputX; 
	int midiInputY; 
	int midiInputW = G_DEFAULT_SUBWINDOW_W; 
	int midiInputH = G_DEFAULT_SUBWINDOW_H;

	int pianoRollY = -1;
	int pianoRollH = 422;

	int sampleActionEditorH = 40; 
	int velocityEditorH     = 40; 
	int envelopeEditorH     = 40; 

	int pluginListX;
	int pluginListY;

	RecTriggerMode recTriggerMode  = RecTriggerMode::NORMAL;
	float          recTriggerLevel = G_DEFAULT_REC_TRIGGER_LEVEL;

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

#ifdef WITH_VST

	int pluginChooserX; 
	int pluginChooserY;
	int pluginChooserW   = G_DEFAULT_SUBWINDOW_W; 
	int pluginChooserH   = G_DEFAULT_SUBWINDOW_H;
	int pluginSortMethod = 0;

#endif
};


/* -------------------------------------------------------------------------- */


extern Conf conf;


/* -------------------------------------------------------------------------- */


void init();
bool read();
bool write();
}}} // giada::m::conf::

#endif
