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

#ifndef GE_PLUGIN_ELEMENT_H
#define GE_PLUGIN_ELEMENT_H

#include "glue/plugin.h"

#include "gui/elems/basics/flex.h"

namespace giada::v
{
class geChoice;
class geTextButton;
class geImageButton;
class gePluginElement : public geFlex
{
public:
	gePluginElement(int x, int y, int w, int h, c::plugin::Plugin);

	ID               getPluginId() const;
	const m::Plugin& getPluginRef() const;

	geTextButton*  button;
	geChoice*      program;
	geTextButton*  bypass;
	geImageButton* shiftUpBtn;
	geImageButton* shiftDownBtn;
	geImageButton* remove;

private:
	void openPluginWindow();
	void removePlugin();
	void shiftUp();
	void shiftDown();

	c::plugin::Plugin m_plugin;
};
} // namespace giada::v

#endif