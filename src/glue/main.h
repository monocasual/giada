/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_MAIN_H
#define G_MAIN_H

#include "src/core/model/mixer.h"
#include "src/core/model/modelView.h"
#include "src/core/model/parameters.h"
#include "src/core/model/sequencer.h"
#include "src/core/types.h"
#include "src/glue/channel.h"
#include "src/scene.h"
#include "src/types.h"

/* giada::c::main
Functions to interact with the tools in the main window. */

namespace giada::c::main
{
class Timer
{
public:
	Timer(const m::model::ModelView&);

	float         bpm;
	TimeSignature timeSignature;
	int           quantize;
	bool          isUsingJack;
	bool          isRecordingInput;
};

class IO
{
public:
	IO(const m::model::ModelView&);

	float masterOutVol;
	float masterInVol;
	bool  masterOutHasPlugins;
	bool  masterInHasPlugins;
	bool  inToOut;

	Peak getMasterOutPeak() const;
	Peak getMasterInPeak() const;
	bool isKernelReady() const;

private:
	const m::model::Mixer::Parameters& m_params;
};

class Sequencer
{
public:
	Sequencer(const m::model::ModelView&);

	bool          isFreeModeInputRec;
	TimeSignature timeSignature;
	Frame         recMaxLength;

	/* Values that change while the Document stays the same: they are read live
	from the model's Parameters. */

	bool  shouldBlink() const;
	int   getCurrentBeat() const;
	Frame getRecPosition() const;

private:
	bool                        m_isWaiting;
	const m::model::Parameters& m_params;
};

class Transport
{
public:
	Transport(const m::model::ModelView&);

	bool           isRunning;
	bool           isRecordingActions;
	bool           isRecordingInput;
	bool           isMetronomeOn;
	RecTriggerMode recTriggerMode;
	InputRecMode   inputRecMode;

	bool canRecordInput() const;
	bool canEnableFreeInputRec() const;
	bool canEnableRecOnSignal() const;

private:
	m::model::ModelView m_view;
};

class Scenes
{
public:
	Scenes(const m::model::ModelView&);

	SceneArray<bool> activeScenes;

	Scene       getCurrentScene() const;
	Scene       getNextScene() const;
	SceneStatus getStatus() const;

private:
	const m::model::Sequencer::Parameters& m_params;
};

class Main
{
public:
	Main(const m::model::ModelView&);

	std::vector<channel::Track> tracks;
	Timer                       timer;
	IO                          io;
	Transport                   transport;
	Sequencer                   sequencer;
	Scenes                      scenes;
};

/* get*
Returns viewModel objects filled with data. */

Main getMain(const m::model::ModelView&);

void setTimeSignature(TimeSignature);
void quantize(int val);
void clearAllSamples(bool allscenes);
void clearAllActions(bool allscenes);
void copyCurrentScene(Scene dst);

/* setInToOut
Enables the "hear what you playing" feature. */

void setInToOut(bool v);

void   toggleRecOnSignal();
void   toggleFreeInputRec();
void   toggleMetronome();
void   setMasterInVolume(float v, Thread t);
void   setMasterOutVolume(float v, Thread t);
void   setBpm(float);
void   multiplyBeats();
void   divideBeats();
void   goToBeat(int);
void   startSequencer();
void   stopSequencer();
void   toggleSequencer();
void   rewindSequencer();
void   stopActionRecording();
void   toggleActionRecording();
void   stopInputRecording();
void   toggleInputRecording();
void   setScene(Scene, bool forced);
double getCpuLoad();

#if G_DEBUG_MODE
void printDebugInfo();
#endif

/* closeProject
Resets Giada to init state. If resetGui also refresh all widgets. */

void closeProject();

void quitGiada();
} // namespace giada::c::main

#endif
