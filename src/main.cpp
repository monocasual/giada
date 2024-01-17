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

#include "core/engine.h"
#include "gui/ui.h"

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
	m::init::run();

	return 0;
}