
/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_pluginWindowGUI
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

#ifdef WITH_VST

#ifndef GD_PLUGIN_WINDOW_GUI_H
#define GD_PLUGIN_WINDOW_GUI_H

#include "deps/juce-config.h"
#include "window.h"
#include <FL/Fl.H>
#include <FL/Fl_Window.H>

namespace giada
{
namespace c
{
namespace plugin
{
struct Plugin;
}
} // namespace c
namespace v
{
class gdPluginWindowGUI : public gdWindow
{
  public:
	gdPluginWindowGUI(c::plugin::Plugin&);
	~gdPluginWindowGUI();

  private:
	static void cb_close(Fl_Widget* /*w*/, void* p);
	static void cb_refresh(void* data);
	void        cb_close();
	void        cb_refresh();

	void openEditor(void* parent);
	void closeEditor();

	c::plugin::Plugin& m_plugin;

	juce::AudioProcessorEditor* m_ui;
};
} // namespace v
} // namespace giada

#endif // include guard

#endif // #ifdef WITH_VST
