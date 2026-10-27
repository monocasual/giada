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

#include "src/glue/main.h"
#include "src/core/engine.h"
#include "src/core/init.h"
#include "src/core/kernelMidi.h"
#include "src/core/mixer.h"
#include "src/core/model/modelView.h"
#include "src/gui/dialogs/warnings.h"
#include "src/gui/elems/mainWindow/keyboard/keyboard.h"
#include "src/gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "src/gui/elems/mainWindow/mainInput.h"
#include "src/gui/elems/mainWindow/mainOutput.h"
#include "src/gui/elems/mainWindow/mainTimer.h"
#include "src/gui/ui.h"
#include "src/utils/gui.h"
#include "src/utils/string.h"

extern giada::v::Ui*     g_ui;
extern giada::m::Engine* g_engine;

namespace giada::c::main
{
Timer::Timer(const m::model::ModelView& view)
: bpm(view.getDocument().sequencer.getBpm())
, timeSignature(view.getDocument().sequencer.getTimeSignature())
, quantize(view.getDocument().sequencer.quantize)
, isUsingJack(view.getDocument().kernelAudio.api == RtAudio::Api::UNIX_JACK)
, isRecordingInput(view.getDocument().mixer.isRecordingInput)
{
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

IO::IO(const m::model::ModelView& view)
: m_params(view.getParameters().mixer)
{
	const m::model::Channel& masterOut = view.getDocument().tracks.getChannel(MASTER_OUT_CHANNEL_ID);
	const m::model::Channel& masterIn  = view.getDocument().tracks.getChannel(MASTER_IN_CHANNEL_ID);

	masterOutVol        = masterOut.volume;
	masterOutHasPlugins = !masterOut.plugins.empty();
	masterInVol         = masterIn.volume;
	masterInHasPlugins  = !masterIn.plugins.empty();
	inToOut             = view.getDocument().mixer.inToOut;
}

/* -------------------------------------------------------------------------- */

Peak IO::getMasterOutPeak() const { return m_params.getPeakOut(); }
Peak IO::getMasterInPeak() const { return m_params.getPeakIn(); }

/* -------------------------------------------------------------------------- */

bool IO::isKernelReady() const { return g_engine->isAudioReady(); }

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Scenes::Scenes(const m::model::ModelView& view)
: m_params(view.getParameters().sequencer)
{
	const m::model::Document& document = view.getDocument();

	for (const std::size_t sceneIndex : utils::container::range(G_MAX_NUM_SCENES))
	{
		const Scene scene{sceneIndex};
		activeScenes[sceneIndex] = document.actions.hasActions(scene) || document.tracks.hasSampleChannelsWithWaves(scene);
	}
}

Scene Scenes::getCurrentScene() const { return m_params.getCurrentScene(); }

/* -------------------------------------------------------------------------- */

Scene Scenes::getNextScene() const { return m_params.getNextScene(); }

/* -------------------------------------------------------------------------- */

SceneStatus Scenes::getStatus() const { return m_params.getSceneStatus(); }

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Main::Main(const m::model::ModelView& view)
: tracks(channel::getTracks(view.getDocument(), view.getAssets(), view.getParameters().sequencer.getCurrentScene()))
, timer(view)
, io(view)
, transport(view)
, sequencer(view)
, scenes(view)
{
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Sequencer::Sequencer(const m::model::ModelView& view)
: isFreeModeInputRec(view.getDocument().mixer.isRecordingInput && view.getDocument().mixer.inputRecMode == InputRecMode::FREE)
, timeSignature(view.getDocument().sequencer.getTimeSignature())
, recMaxLength(view.getAssets().mixerRendering != nullptr ? view.getAssets().mixerRendering->getRecBuffer().countFrames() : 0)
, m_isWaiting(view.getDocument().sequencer.status == SeqStatus::WAITING)
, m_params(view.getParameters())
{
}

/* -------------------------------------------------------------------------- */

bool Sequencer::shouldBlink() const
{
	return g_ui->shouldBlink() && (m_isWaiting || isFreeModeInputRec);
}

int   Sequencer::getCurrentBeat() const { return m_params.sequencer.getCurrentBeat(); }
Frame Sequencer::getRecPosition() const { return m_params.mixer.getInputTracker(); }

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Transport::Transport(const m::model::ModelView& view)
: isRunning(view.getDocument().sequencer.isRunning())
, isRecordingActions(view.getDocument().mixer.isRecordingActions)
, isRecordingInput(view.getDocument().mixer.isRecordingInput)
, isMetronomeOn(view.getDocument().sequencer.metronome)
, recTriggerMode(view.getDocument().mixer.recTriggerMode)
, inputRecMode(view.getDocument().mixer.inputRecMode)
, m_view(view)
{
}

/* -------------------------------------------------------------------------- */

bool Transport::canRecordInput() const
{
	const m::model::Document& document = m_view.getDocument();
	const Scene               scene    = m_view.getParameters().sequencer.getCurrentScene();

	return document.kernelAudio.deviceIn.id != 0 && document.tracks.hasInputRecordableChannels(scene);
}

/* -------------------------------------------------------------------------- */

bool Transport::canEnableFreeInputRec() const
{
	const Scene scene = m_view.getParameters().sequencer.getCurrentScene();

	return !m_view.getDocument().tracks.hasSampleChannelsWithWaves(scene);
}

/* -------------------------------------------------------------------------- */

bool Transport::canEnableRecOnSignal() const
{
	return !m_view.getDocument().sequencer.isRunning();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Main getMain()
{
	return Main(g_engine->getModelView());
}

/* -------------------------------------------------------------------------- */

void setTimeSignature(TimeSignature t)
{
	g_engine->getMainApi().setTimeSignature(t);
}

/* -------------------------------------------------------------------------- */

void quantize(int val)
{
	g_engine->getMainApi().setQuantize(val);
}

/* -------------------------------------------------------------------------- */

void clearAllSamples(bool allScenes)
{
	if (!v::gdConfirmWin(g_ui->getI18Text(v::LangMap::COMMON_WARNING),
	        g_ui->getI18Text(v::LangMap::MESSAGE_MAIN_FREEALLSAMPLES)))
		return;

	g_ui->closeSubWindow(WID_SAMPLE_EDITOR);
	g_engine->getChannelsApi().freeAllSampleChannels(allScenes);
}

/* -------------------------------------------------------------------------- */

void clearAllActions(bool allScenes)
{
	if (!v::gdConfirmWin(g_ui->getI18Text(v::LangMap::COMMON_WARNING),
	        g_ui->getI18Text(v::LangMap::MESSAGE_MAIN_CLEARALLACTIONS)))
		return;

	g_ui->closeSubWindow(WID_ACTION_EDITOR);
	g_engine->getChannelsApi().clearAllActions({}, allScenes);
}

/* -------------------------------------------------------------------------- */

void copyCurrentScene(Scene dst)
{
	g_engine->getMainApi().copyCurrentScene(dst);
}

/* -------------------------------------------------------------------------- */

void setInToOut(bool v)
{
	g_engine->getMainApi().setInToOut(v);
}

/* -------------------------------------------------------------------------- */

void toggleRecOnSignal()
{
	g_engine->getMainApi().toggleRecOnSignal();
}

/* -------------------------------------------------------------------------- */

void toggleFreeInputRec()
{
	g_engine->getMainApi().toggleFreeInputRec();
}

/* -------------------------------------------------------------------------- */

void toggleMetronome()
{
	g_engine->getMainApi().toggleMetronome();
}

/* -------------------------------------------------------------------------- */

void setMasterInVolume(float v, Thread t)
{
	g_engine->getMainApi().setMasterInVolume(v);

	if (t != Thread::MAIN)
		g_ui->pumpEvent([v]()
		{ g_ui->mainWindow->mainInput->setInVol(v); });
}

void setMasterOutVolume(float v, Thread t)
{
	g_engine->getMainApi().setMasterOutVolume(v);

	if (t != Thread::MAIN)
		g_ui->pumpEvent([v]()
		{ g_ui->mainWindow->mainOutput->setOutVol(v); });
}

/* -------------------------------------------------------------------------- */

void setBpm(float v) { g_engine->getMainApi().setBpm(v); }

/* -------------------------------------------------------------------------- */

void multiplyBeats() { g_engine->getMainApi().multiplyBeats(); }
void divideBeats() { g_engine->getMainApi().divideBeats(); }

/* -------------------------------------------------------------------------- */

void goToBeat(int beat) { g_engine->getMainApi().goToBeat(beat); }

/* -------------------------------------------------------------------------- */

void startSequencer() { g_engine->getMainApi().startSequencer(); }
void stopSequencer() { g_engine->getMainApi().stopSequencer(); }
void toggleSequencer() { g_engine->getMainApi().toggleSequencer(); }
void rewindSequencer() { g_engine->getMainApi().rewindSequencer(); }

/* -------------------------------------------------------------------------- */

void stopActionRecording() { g_engine->getMainApi().stopActionRecording(); }
void stopInputRecording() { g_engine->getMainApi().stopInputRecording(); }
void toggleActionRecording() { g_engine->getMainApi().toggleActionRecording(); }
void toggleInputRecording() { g_engine->getMainApi().toggleInputRecording(); }

/* -------------------------------------------------------------------------- */

void setScene(Scene scene, bool forced) { g_engine->getMainApi().setScene(scene, forced); }

/* -------------------------------------------------------------------------- */

double getCpuLoad() { return g_engine->getMainApi().getCpuLoad(); }

/* -------------------------------------------------------------------------- */

#if G_DEBUG_MODE

void printDebugInfo()
{
	g_engine->debug();
}

#endif

/* -------------------------------------------------------------------------- */

void closeProject()
{
	if (!v::gdConfirmWin(g_ui->getI18Text(v::LangMap::COMMON_WARNING),
	        g_ui->getI18Text(v::LangMap::MESSAGE_MAIN_CLOSEPROJECT)))
		return;

	g_ui->stopUpdater();
	g_ui->closeAllSubwindows(); // Close VST editors, if any, before deleting their processors
	g_engine->suspend();
	g_engine->reset();
	g_ui->reset();
	g_engine->resume();
	g_ui->startUpdater();
}

/* -------------------------------------------------------------------------- */

void quitGiada()
{
	if (!v::gdConfirmWin(g_ui->getI18Text(v::LangMap::COMMON_WARNING),
	        g_ui->getI18Text(v::LangMap::MESSAGE_INIT_QUITGIADA)))
		return;

	m::init::shutdown();
}
} // namespace giada::c::main
