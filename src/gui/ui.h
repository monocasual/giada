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

#ifndef G_V_UI_H
#define G_V_UI_H

#include "core/conf.h"
#include "core/patch.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dispatcher.h"
#include "gui/langMapper.h"
#include "gui/model.h"
#include "gui/updater.h"
#include <memory>
#include <string>

namespace giada::v
{
class Ui final
{
public:
	static ID getPluginWindowId(ID pluginId);

	Ui();

	/* shouldBlink
	Return whether is time to blink something or not. This is used to make 
	widgets blink. */

	bool shouldBlink() const;

	/* getScaling
	Returns the current value for UI scaling. */

	float getScaling() const;

	/* getI18Text
	Returns the localized string, given a langmap key. */

	const char* getI18Text(const std::string&) const;

	/* getLangMapFilesFound
	Returns a list of all langmaps found in the system. */

	const std::vector<std::string>& getLangMapFilesFound() const;

	/* load
	Reads UI information from a Patch when a new project has been loaded. */

	void load(const m::Patch&);

	void init(int argc, char** argv, const m::Conf&, const std::string& patchName, bool isAudioReady);
	void reset();
	void run();
	void shutdown(m::Conf&);
	void stopUpdater();
	void startUpdater();
	bool pumpEvent(const Updater::Event&);

	/* refresh
	Repaints dynamic GUI elements. */

	void refresh();

	/* rebuild
	Rebuilds the UI from scratch. Used when the model has changed. */

	void rebuild();

	/* [rebuild|refresh]SubWindow 
	Rebuilds or refreshes subwindow with ID 'wid' if it exists, i.e. if it's open. */

	void rebuildSubWindow(ID wid);
	void refreshSubWindow(ID wid);

	/* getSubwindow
	Returns a pointer to an open subwindow, otherwise nullptr. */

	v::gdWindow* getSubwindow(ID wid);

	/* openSubWindow
	Opens a new sub-window as a child of the main window. */

	void openSubWindow(v::gdWindow* child);

	/* closeSubWindow
	Closes a sun-window currently attached to the main one. */

	void closeSubWindow(ID wid);

	/* closeAllSubwindows
	Closes all subwindows attached to the main one. */

	void closeAllSubwindows();

	/* setMainWindowTitle 
	Changes Main Window title. */

	void setMainWindowTitle(const std::string&);

	std::unique_ptr<gdMainWindow> mainWindow;
	Dispatcher                    dispatcher;
	Model                         model;

private:
	static constexpr int BLINK_RATE = G_GUI_FPS / 2;

	static void juceDispatchLoop(void*);

	/* [start|stop]JuceDispatchLoop
	Starts and stops the JUCE dispatch loop from its MessageManager component.
	This is needed for plugin-ins to wake up their UI editor and let it react
	to UI events. */

	void startJuceDispatchLoop();
	void stopJuceDispatchLoop();

	/* rebuildStaticWidgets
    Updates attributes of static widgets, i.e. those elements that don't get
    automatically refreshed during the UI update loop. Useful when loading a new 
    patch. */

	void rebuildStaticWidgets();

	LangMapper m_langMapper;
	Updater    m_updater;
	int        m_blinker;
};
} // namespace giada::v

#endif