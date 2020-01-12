/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#ifndef GE_PITCH_TOOL_H
#define GE_PITCH_TOOL_H


#include <FL/Fl_Pack.H>


class geDial;
class geInput;
class geButton;
class geBox;


namespace giada {
namespace v 
{
class gePitchTool : public Fl_Pack
{
public:

	gePitchTool(ID channelId, int x, int y);

	void rebuild();

private:

	static void cb_setPitch      (Fl_Widget* w, void* p);
	static void cb_setPitchToBar (Fl_Widget* w, void* p);
	static void cb_setPitchToSong(Fl_Widget* w, void* p);
	static void cb_setPitchHalf  (Fl_Widget* w, void* p);
	static void cb_setPitchDouble(Fl_Widget* w, void* p);
	static void cb_resetPitch    (Fl_Widget* w, void* p);
	static void cb_setPitchNum   (Fl_Widget* w, void* p);
	void cb_setPitch();
	void cb_setPitchToBar();
	void cb_setPitchToSong();
	void cb_setPitchHalf();
	void cb_setPitchDouble();
	void cb_resetPitch();
	void cb_setPitchNum();

	ID m_channelId;

	geBox*    label;
	geDial*   dial;
	geInput*  input;
	geButton* pitchToBar;
	geButton* pitchToSong;
	geButton* pitchHalf;
	geButton* pitchDouble;
	geButton* pitchReset;
};
}} // giada::v::


#endif
