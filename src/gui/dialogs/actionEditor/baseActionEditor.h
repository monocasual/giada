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


#ifndef GD_BASE_ACTION_EDITOR_H
#define GD_BASE_ACTION_EDITOR_H


#include "core/types.h"
#include "gui/dialogs/window.h"


class geChoice;
class geButton;
class geScroll;


namespace giada {
namespace m
{
class Channel;
struct Action;
}
namespace v
{
class geGridTool;


class gdBaseActionEditor : public gdWindow
{
public:

	virtual ~gdBaseActionEditor();

	int handle(int e) override;

	Pixel frameToPixel(Frame f) const;
	Frame pixelToFrame(Pixel p, bool snap=true) const;
	int getActionType() const;
	std::vector<m::Action> getActions() const;

	geChoice*   actionType;
	geGridTool* gridTool;
	geButton*   zoomInBtn;
	geButton*   zoomOutBtn;
	geScroll*   viewport;       // widget container

	ID channelId;

	float ratio;
	Pixel fullWidth;     // Full widgets width, i.e. scaled-down full sequencer
	Pixel loopWidth; 	 // Loop width, i.e. scaled-down sequencer range

protected:

	static constexpr Pixel RESIZER_BAR_H = 20;
	static constexpr Pixel MIN_WIDGET_H  = 10;
	static constexpr float MIN_RATIO     = 25.0f;
	static constexpr float MAX_RATIO     = 40000.0f;

	std::vector<m::Action> m_actions;

	gdBaseActionEditor(ID channelId);

	void zoomIn();
	void zoomOut();
	static void cb_zoomIn(Fl_Widget* w, void* p);
	static void cb_zoomOut(Fl_Widget* w, void* p);
	
	/* computeWidth
	Computes total width, in pixel. */

	void computeWidth();

	void centerViewportIn();
	void centerViewportOut();

	void prepareWindow();
};
}} // giada::v::


#endif
