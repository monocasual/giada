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

#include "gui/ui.h"
#include "core/const.h"
#include "core/engine.h"
#include "core/recorder.h"
#include "gui/elems/mainWindow/keyboard/column.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/elems/mainWindow/mainIO.h"
#include "gui/elems/mainWindow/mainTimer.h"
#include "gui/updater.h"
#include "utils/gui.h"
#include "utils/log.h"
#ifdef WITH_VST
#include <FL/Fl.H>
#if defined(G_OS_LINUX) || defined(G_OS_FREEBSD)
#include <X11/Xlib.h> // For XInitThreads
#endif
#endif

namespace giada::v
{
Ui::Ui(m::Recorder& recorder)
: m_updater(*this)
, m_blinker(0)
{
	dispatcher.onEventOccured = [&recorder]() {
		recorder.startActionRecOnCallback();
	};
}

/* -------------------------------------------------------------------------- */

bool Ui::shouldBlink() const
{
	return m_blinker > 6; // TODO magic numbers
}

/* -------------------------------------------------------------------------- */

void Ui::load(const m::Patch::Data& patch)
{
	reset();
	mainWindow->keyboard->layout.clear();
	for (const m::Patch::Column& col : patch.columns)
		mainWindow->keyboard->layout.push_back({col.id, col.width});
	mainWindow->keyboard->rebuild();
	setMainWindowTitle(patch.name);
}

/* -------------------------------------------------------------------------- */

void Ui::store(const std::string patchName, m::Patch::Data& patch)
{
	patch.columns.clear();
	mainWindow->keyboard->forEachColumn([&](const geColumn& c) {
		patch.columns.push_back({c.id, c.w()});
	});
	setMainWindowTitle(patchName);
}

/* -------------------------------------------------------------------------- */

void Ui::init(int argc, char** argv, m::Engine& engine)
{
	/* This is of paramount importance on Linux with VST enabled, otherwise many
	plug-ins go nuts and crash hard. It seems that some plug-ins on our Juce-based
	PluginHost use Xlib concurrently. */

#if (defined(G_OS_LINUX) || defined(G_OS_FREEBSD)) && defined(WITH_VST)
	XInitThreads();
#endif

	mainWindow = std::make_unique<gdMainWindow>(G_MIN_GUI_WIDTH, G_MIN_GUI_HEIGHT, "", argc, argv, engine.conf.data);
	mainWindow->resize(engine.conf.data.mainWindowX, engine.conf.data.mainWindowY, engine.conf.data.mainWindowW,
	    engine.conf.data.mainWindowH);

	setMainWindowTitle(engine.patch.data.name == "" ? G_DEFAULT_PATCH_NAME : engine.patch.data.name);

	m_updater.init(engine.model);

	if (engine.kernelAudio.isReady())
		rebuildStaticWidgets();
}

/* -------------------------------------------------------------------------- */

void Ui::reset()
{
	setMainWindowTitle(G_DEFAULT_PATCH_NAME);
	rebuildStaticWidgets();
	closeAllSubwindows();
	mainWindow->clearKeyboard();
	mainWindow->rebuild();
}

/* -------------------------------------------------------------------------- */

void Ui::shutdown()
{
	mainWindow.reset();
	m_updater.close();

	u::log::print("[ui] All windows closed\n");
}

/* -------------------------------------------------------------------------- */

void Ui::refresh()
{
	/* Update dynamic elements inside main window: in and out meters, beat meter
	and each channel. */

	mainWindow->refresh();

	/* Compute timer for blinker. */

	m_blinker = (m_blinker + 1) % 12; // TODO magic numbers

	/* Refresh Sample Editor and Action Editor for dynamic playhead. */

	refreshSubWindow(WID_SAMPLE_EDITOR);
	refreshSubWindow(WID_ACTION_EDITOR);
}

/* -------------------------------------------------------------------------- */

void Ui::rebuild()
{
	mainWindow->rebuild();
	rebuildSubWindow(WID_FX_LIST);
	rebuildSubWindow(WID_SAMPLE_EDITOR);
	rebuildSubWindow(WID_ACTION_EDITOR);
}

/* -------------------------------------------------------------------------- */

void Ui::rebuildSubWindow(int wid)
{
	v::gdWindow* w = getSubwindow(*mainWindow.get(), wid);
	if (w != nullptr) // If its open
		w->rebuild();
}

/* -------------------------------------------------------------------------- */

void Ui::refreshSubWindow(int wid)
{
	v::gdWindow* w = getSubwindow(*mainWindow.get(), wid);
	if (w != nullptr) // If its open
		w->refresh();
}

/* -------------------------------------------------------------------------- */

v::gdWindow* Ui::getSubwindow(v::gdWindow& parent, int wid)
{
	return parent.hasWindow(wid) ? parent.getChild(wid) : nullptr;
}

/* -------------------------------------------------------------------------- */

void Ui::openSubWindow(v::gdWindow& parent, v::gdWindow* child, int wid)
{
	if (parent.hasWindow(wid))
	{
		u::log::print("[GU] parent has subwindow with id=%d, deleting\n", wid);
		parent.delSubWindow(wid);
	}
	child->setId(wid);
	parent.addSubWindow(child);
}

/* -------------------------------------------------------------------------- */

void Ui::closeSubWindow(int wid)
{
	mainWindow->delSubWindow(wid);
}

/* -------------------------------------------------------------------------- */

void Ui::closeAllSubwindows()
{
	mainWindow->delSubWindow(WID_ACTION_EDITOR);
	mainWindow->delSubWindow(WID_SAMPLE_EDITOR);
	mainWindow->delSubWindow(WID_FX_LIST);
	mainWindow->delSubWindow(WID_FX);
}

/* -------------------------------------------------------------------------- */

void Ui::setMainWindowTitle(const std::string& s)
{
	std::string out = std::string(G_APP_NAME) + " - " + s;
	mainWindow->copy_label(out.c_str());
}

/* -------------------------------------------------------------------------- */

#ifdef WITH_VST

void Ui::startJuceDispatchLoop()
{
	Fl::add_timeout(G_GUI_REFRESH_RATE, juceDispatchLoop);
}

void Ui::stopJuceDispatchLoop()
{
	Fl::remove_timeout(juceDispatchLoop);
}

#endif

/* -------------------------------------------------------------------------- */

void Ui::rebuildStaticWidgets()
{
	mainWindow->mainIO->rebuild();
	mainWindow->mainTimer->rebuild();
}

/* -------------------------------------------------------------------------- */

#ifdef WITH_VST

void Ui::juceDispatchLoop(void*)
{
	juce::MessageManager* mm = juce::MessageManager::getInstanceWithoutCreating();
	assert(mm != nullptr);
	mm->runDispatchLoopUntil(1);
	Fl::add_timeout(G_GUI_REFRESH_RATE, juceDispatchLoop);
}

#endif
} // namespace giada::v
