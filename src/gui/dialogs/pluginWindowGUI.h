
/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_pluginWindowGUI
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

#ifndef GD_PLUGIN_WINDOW_GUI_H
#define GD_PLUGIN_WINDOW_GUI_H

#include "window.h"
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>

namespace giada::c::plugin
{
struct Plugin;
}

namespace giada::v
{
class gdPluginWindowGUI : public gdWindow
{
public:
	gdPluginWindowGUI(c::plugin::Plugin&);
	~gdPluginWindowGUI();

private:
	void createEditor();
	void showEditor();
	void closeEditor();
	void adjustSize();

	c::plugin::Plugin&                          m_plugin;
	std::unique_ptr<juce::AudioProcessorEditor> m_editor;
};
} // namespace giada::v

#endif