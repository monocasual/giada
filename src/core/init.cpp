/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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
#include "core/engine.h"
#include "gui/dialogs/warnings.h"
#include "gui/ui.h"
#include "gui/updater.h"
#include "utils/log.h"
#include "utils/ver.h"
#ifdef WITH_TESTS
#define CATCH_CONFIG_RUNNER
#include "tests/actionRecorder.cpp"
#include "tests/midiLighter.cpp"
#include "tests/samplePlayer.cpp"
#include "tests/utils.cpp"
#include "tests/wave.cpp"
#include "tests/waveFx.cpp"
#include "tests/waveManager.cpp"
#include "tests/waveReader.cpp"
#include <catch2/catch.hpp>
#include <string>
#include <vector>
#endif
#include <FL/Fl.H>

extern giada::m::Engine g_engine;
extern giada::v::Ui     g_ui;

namespace giada::m::init
{
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

void printBuildInfo()
{
	u::log::print("[init] Giada %s\n", G_VERSION_STR);
	u::log::print("[init] Build date: " BUILD_DATE "\n");
#ifdef G_DEBUG_MODE
	u::log::print("[init] Debug build\n");
#else
	u::log::print("[init] Release build\n");
#endif
	u::log::print("[init] Dependencies:\n");
	u::log::print("[init]   FLTK - %d.%d.%d\n", FL_MAJOR_VERSION, FL_MINOR_VERSION, FL_PATCH_VERSION);
	u::log::print("[init]   RtAudio - %s\n", u::ver::getRtAudioVersion());
	u::log::print("[init]   RtMidi - %s\n", u::ver::getRtMidiVersion());
	u::log::print("[init]   Libsamplerate\n"); // TODO - print version
	u::log::print("[init]   Libsndfile - %s\n", u::ver::getLibsndfileVersion());
	u::log::print("[init]   JSON for modern C++ - %d.%d.%d\n",
	    NLOHMANN_JSON_VERSION_MAJOR, NLOHMANN_JSON_VERSION_MINOR, NLOHMANN_JSON_VERSION_PATCH);
#ifdef WITH_VST
	u::log::print("[init]   JUCE - %d.%d.%d\n", JUCE_MAJOR_VERSION, JUCE_MINOR_VERSION, JUCE_BUILDNUMBER);
#endif
	KernelAudio::logCompiledAPIs();
}

/* -------------------------------------------------------------------------- */

void startup(int argc, char** argv)
{
#ifdef WITH_VST
	juce::initialiseJuce_GUI();
#endif
	g_engine.init();
	g_ui.init(argc, argv, g_engine);

	if (!g_engine.kernelAudio.isReady())
		v::gdAlert(g_ui.langMapper.get(v::LangMap::MESSAGE_INIT_WRONGSYSTEM));
}

/* -------------------------------------------------------------------------- */

int run()
{
	Fl::lock(); // Enable multithreading in FLTK
	return Fl::run();
}

/* -------------------------------------------------------------------------- */

void closeMainWindow()
{
	if (!v::gdConfirmWin(g_ui.langMapper.get(v::LangMap::COMMON_WARNING), g_ui.langMapper.get(v::LangMap::MESSAGE_INIT_QUITGIADA)))
		return;
	shutdown();
}

/* -------------------------------------------------------------------------- */

void shutdown()
{
	g_ui.shutdown();
	g_engine.shutdown();
#ifdef WITH_VST
	juce::shutdownJuce_GUI();
#endif
	u::log::print("[init] Giada %s closed\n\n", G_VERSION_STR);
}
} // namespace giada::m::init
