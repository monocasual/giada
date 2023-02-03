/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef GD_MAINWINDOW_H
#define GD_MAINWINDOW_H

#include "core/conf.h"
#include "gui/dialogs/progress.h"
#include "window.h"

namespace giada::v
{
class geKeyboard;
class geMainIO;
class geMainMenu;
class geSequencer;
class geMainTransport;
class geMainTimer;
class gdMainWindow : public gdWindow
{
	class ScopedProgress;

public:
	gdMainWindow(geompp::Rect<int>, const char* title, int argc, char** argv, m::Conf&);
	~gdMainWindow();

	void refresh() override;
	void rebuild() override;

	/* clearKeyboard
	Resets Keyboard to initial state, with no columns. */

	void clearKeyboard();

	/* setTitle
	Sets a new window title, prefixed by "Giada - [title]". */

	void setTitle(const std::string&);

	[[nodiscard]] ScopedProgress getScopedProgress(const char* msg);

	geKeyboard*      keyboard;
	geSequencer*     sequencer;
	geMainMenu*      mainMenu;
	geMainIO*        mainIO;
	geMainTimer*     mainTimer;
	geMainTransport* mainTransport;

private:
	class ScopedProgress
	{
	public:
		ScopedProgress(gdProgress&, const char* msg);
		~ScopedProgress();

		void setProgress(float);

	private:
		gdProgress& m_progress;
	};

	m::Conf& m_conf;

	gdProgress m_progress;
};
} // namespace giada::v

#endif
