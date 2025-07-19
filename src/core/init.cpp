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

#ifdef __APPLE__
#include <pwd.h>
#endif
#include "src/core/confFactory.h"
#include "src/core/engine.h"
#include "src/gui/elems/mainWindow/keyboard/keyboard.h"
#include "src/gui/elems/mainWindow/mainInput.h"
#include "src/gui/elems/mainWindow/mainOutput.h"
#include "src/gui/ui.h"
#include "src/utils/log.h"
#include "src/utils/ver.h"
#ifdef WITH_TESTS
#define CATCH_CONFIG_RUNNER
#include "tests/actionRecorder.cpp"
#include "tests/channelFactory.cpp"
#include "tests/midiEvent.cpp"
#include "tests/midiLightning.cpp"
#include "tests/patch.cpp"
#include "tests/sampleRendering.cpp"
#include "tests/utils.cpp"
#include "tests/version.cpp"
#include "tests/wave.cpp"
#include "tests/waveFactory.cpp"
#include "tests/waveFx.cpp"
#include "tests/waveReading.cpp"
#include <catch2/catch.hpp>
#include <string>
#include <vector>
#endif
#include <FL/Fl.H>

extern giada::m::Engine* g_engine;
extern giada::v::Ui*     g_ui;

namespace giada::m::init
{
namespace
{
void printBuildInfo_()
{
	u::log::print("[init] Giada {}\n", G_VERSION.toString());
	u::log::print("[init] Build date: " BUILD_DATE "\n");
#if G_DEBUG_MODE
	u::log::print("[init] Debug build\n");
#else
	u::log::print("[init] Release build\n");
#endif
	u::log::print("[init] Dependencies:\n");
	u::log::print("[init]   FLTK - {}.{}.{}\n", FL_MAJOR_VERSION, FL_MINOR_VERSION, FL_PATCH_VERSION);
	u::log::print("[init]   RtAudio - {}\n", u::ver::getRtAudioVersion());
	u::log::print("[init]   RtMidi - {}\n", u::ver::getRtMidiVersion());
	u::log::print("[init]   Libsamplerate\n"); // TODO - print version
	u::log::print("[init]   Libsndfile - {}\n", u::ver::getLibsndfileVersion());
	u::log::print("[init]   JSON for modern C++ - {}.{}.{}\n",
	    NLOHMANN_JSON_VERSION_MAJOR, NLOHMANN_JSON_VERSION_MINOR, NLOHMANN_JSON_VERSION_PATCH);
	u::log::print("[init]   JUCE - {}.{}.{}\n", JUCE_MAJOR_VERSION, JUCE_MINOR_VERSION, JUCE_BUILDNUMBER);

	KernelAudio::logCompiledAPIs();
	KernelMidi::logCompiledAPIs();
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int tests(int argc, char** argv)
{
#ifdef WITH_TESTS
	std::vector<char*> args(argv, argv + argc);
	if (args.size() > 1 && strcmp(args[1], "--run-tests") == 0)
		return Catch::Session().run(args.size() - 1, &args[1]);
	else
		return -1;
#else
	(void)argc;
	(void)argv;
	return -1;
#endif
}

/* -------------------------------------------------------------------------- */

void startup()
{
	g_ui->dispatcher.onEventOccured = []()
	{
		g_engine->getMainApi().startActionRecOnCallback();
	};

	g_engine->onMidiReceived = []()
	{
		g_ui->pumpEvent([]
		{ g_ui->mainWindow->mainInput->setMidiInActivity(); });
	};

	g_engine->onMidiSent = []()
	{
		g_ui->pumpEvent([]
		{ g_ui->mainWindow->mainOutput->setMidiOutActivity(); });
	};

	g_engine->onMidiSentFromChannel = [](ID channelId)
	{
		g_ui->pumpEvent([channelId]()
		{ g_ui->mainWindow->keyboard->notifyMidiOut(channelId); });
	};

	g_engine->onModelSwap = [](model::SwapType type)
	{
		/* Rebuild or refresh the UI accoring to the swap type. Note: the onSwap
		callback might be performed by a non-main thread, which must talk to the
		UI (main thread) through the UI queue by pumping an event in it. */
		if (type == model::SwapType::NONE)
			return;
		g_ui->pumpEvent([type]()
		{ type == model::SwapType::HARD ? g_ui->rebuild() : g_ui->refresh(); });
	};

	Conf conf = confFactory::deserialize();

	if (!conf.valid)
		u::log::print("[init::startup] Can't read configuration file! Using default values\n");

	if (!u::log::init(conf.logMode))
		u::log::print("[init::startup] log init failed! Using default stdout\n");

	juce::initialiseJuce_GUI();
	g_engine->init(conf);
	g_ui->init(conf, G_DEFAULT_PATCH_NAME, g_engine->isAudioReady());

	printBuildInfo_();
}

/* -------------------------------------------------------------------------- */

void run()
{
	g_ui->run();
}

/* -------------------------------------------------------------------------- */

void shutdown()
{
	Conf conf;

	g_ui->shutdown(conf);
	g_engine->shutdown(conf);
	juce::shutdownJuce_GUI();

	if (!confFactory::serialize(conf))
		u::log::print("[init::shutdown] error while saving configuration file!\n");
	else
		u::log::print("[init::shutdown] configuration saved\n");

	u::log::print("[init] Giada {} closed\n\n", G_VERSION.toString());
	u::log::close();
}
} // namespace giada::m::init
