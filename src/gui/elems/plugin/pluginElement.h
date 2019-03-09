/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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


#ifndef GE_PLUGIN_ELEMENT_H
#define GE_PLUGIN_ELEMENT_H


#include <FL/Fl_Pack.H>


class geChoice;
class geButton;


namespace giada {
namespace v
{
class gdPluginList;
class gePluginElement : public Fl_Pack
{
public:

	gePluginElement(const m::Plugin& p, ID chanID, int x, int y, int w);

	ID getPluginID() const;

	geButton* button;
	geChoice* program;
	geButton* bypass;
	geButton* shiftUp;
	geButton* shiftDown;
	geButton* remove;

private:

	static void cb_removePlugin(Fl_Widget* v, void* p);
	static void cb_openPluginWindow(Fl_Widget* v, void* p);
	static void cb_setBypass(Fl_Widget* v, void* p);
	static void cb_shiftUp(Fl_Widget* v, void* p);
	static void cb_shiftDown(Fl_Widget* v, void* p);
	static void cb_setProgram(Fl_Widget* v, void* p);
	void cb_removePlugin();
	void cb_openPluginWindow();
	void cb_setBypass();
	void cb_shiftUp();
	void cb_shiftDown();
	void cb_setProgram();

	const m::Plugin& m_plugin;
	ID m_chanID;
};
}} // giada::v::


#endif

#endif // #ifdef WITH_VST
