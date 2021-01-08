/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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


#include <FL/Fl.H>
#include "core/init.h"
#include "gui/dialogs/mainWindow.h"
#ifdef WITH_TESTS
	#define CATCH_CONFIG_RUNNER
	#include <vector>
	#include <string>
	#include <catch2/catch.hpp>
	#include "tests/audioBuffer.cpp"
	#include "tests/recorder.cpp"
	#include "tests/utils.cpp"
	#include "tests/wave.cpp"
	#include "tests/waveFx.cpp"
	#include "tests/waveManager.cpp"
#endif


class giada::v::gdMainWindow* G_MainWin = nullptr;


int main(int argc, char** argv)
{
#ifdef WITH_TESTS
	std::vector<char*> args(argv, argv + argc);
	if (args.size() > 1 && strcmp(args[1], "--run-tests") == 0)
		return Catch::Session().run(args.size() - 1, &args[1]);
#endif

	giada::m::init::startup(argc, argv);

	Fl::lock();  // Enable multithreading in FLTK
	int ret = Fl::run();

	giada::m::init::shutdown();

	return ret;
}