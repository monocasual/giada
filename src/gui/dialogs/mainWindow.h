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

#ifndef GD_MAINWINDOW_H
#define GD_MAINWINDOW_H

#include "src/gui/dialogs/progress.h"
#include "src/gui/dialogs/window.h"
#include <string>

namespace giada::v
{
class geKeyboard;
class geMainInput;
class geMainOutput;
class geMainMenu;
class geSequencer;
class geMainTransport;
class geMainTimer;
class geScenes;
class gdMainWindow : public gdWindow
{
	class ScopedProgress;

public:
	gdMainWindow(geompp::Rect<int>, const char* title);

	void refresh() override;
	void rebuild() override;

	/* clearKeyboard
	Resets Keyboard to initial state, with no tracks. */

	void clearKeyboard();

	/* setTitle
	Sets a new window title, prefixed by "Giada - [title]". */

	void setTitle(const std::string&);

	[[nodiscard]] ScopedProgress getScopedProgress(const char* msg, std::function<void()> onCancel = nullptr);

	geKeyboard*      keyboard;
	geSequencer*     sequencer;
	geMainMenu*      mainMenu;
	geMainTimer*     mainTimer;
	geMainTransport* mainTransport;
	geMainInput*     mainInput;
	geMainOutput*    mainOutput;
	geScenes*        scenes;

private:
	class ScopedProgress
	{
	public:
		ScopedProgress(gdProgress&, const char* msg, std::function<void()> onCancel);
		~ScopedProgress();

		void setProgress(float);

	private:
		gdProgress& m_progress;
	};

	void resize(int x, int y, int w, int h) override;

	gdProgress m_progress;
};
} // namespace giada::v

#endif
