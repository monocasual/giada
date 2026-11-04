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

#include "src/core/engine.h"
#include "src/gui/ui.h"
#include "src/glue/storage.h"
#include "src/utils/fs.h"
#if G_OS_WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>
#endif

giada::m::Engine* g_engine = nullptr;
giada::v::Ui*     g_ui     = nullptr;

int main(int argc, char** argv)
{
	using namespace giada;

	if (int ret = m::init::tests(argc, argv); ret != -1)
		return ret;

	auto enginePtr = std::make_unique<m::Engine>();
	auto uiPtr     = std::make_unique<v::Ui>();

	g_engine = enginePtr.get();
	g_ui     = uiPtr.get();

	m::init::startup();
	if (argc > 1)
	{
		std::string argument = argv[1];
#if G_OS_WINDOWS
		// Explorer supplies Unicode filenames; the narrow CRT argv can lose them.
		int count = 0;
		if (auto** args = CommandLineToArgvW(GetCommandLineW(), &count))
		{
			if (count > 1)
				argument = juce::String(args[1]).toStdString();
			LocalFree(args);
		}
#endif
		if (const std::string project = u::fs::getProjectOpenPath(argument); !project.empty())
			c::storage::loadProjectPath(project);
	}
	m::init::run();

	return 0;
}
