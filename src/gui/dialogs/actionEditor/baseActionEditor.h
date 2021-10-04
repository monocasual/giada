/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
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

#ifndef GD_BASE_ACTION_EDITOR_H
#define GD_BASE_ACTION_EDITOR_H

#include "core/conf.h"
#include "glue/actionEditor.h"
#include "gui/dialogs/window.h"
#include "gui/elems/actionEditor/gridTool.h"
#include "gui/elems/actionEditor/splitScroll.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/pack.h"

namespace giada::m
{
class Channel;
struct Action;
} // namespace giada::m

namespace giada::v
{
class gdBaseActionEditor : public gdWindow
{
public:
	virtual ~gdBaseActionEditor();

	int  handle(int e) override;
	void draw() override;

	Pixel frameToPixel(Frame f) const;
	Frame pixelToFrame(Pixel p, bool snap = true) const;

	ID channelId;

	geGridTool gridTool;
	geButton   zoomInBtn;
	geButton   zoomOutBtn;

	Pixel fullWidth; // Full widgets width, i.e. scaled-down full sequencer
	Pixel loopWidth; // Loop width, i.e. scaled-down sequencer range

protected:
	static constexpr float MIN_RATIO  = 25.0f;
	static constexpr float MAX_RATIO  = 40000.0f;
	static constexpr float RATIO_STEP = 1.5f;

	gdBaseActionEditor(ID channelId, m::Conf::Data&, Frame framesInBeat);

	/* getMouseOverContent
	Returns mouse x-position relative to the viewport content. */

	int getMouseOverContent() const;

	static void cb_zoomIn(Fl_Widget* w, void* p);
	static void cb_zoomOut(Fl_Widget* w, void* p);
	void        zoomIn();
	void        zoomOut();

	/* computeWidth
	Computes total width, in pixel. */

	void computeWidth(Frame framesInSeq, Frame framesInLoop);

	/* prepareWindow
	Initializes window (favicon, limits, ...). */

	void prepareWindow();

	gePack        m_barTop;
	geSplitScroll m_splitScroll;

	c::actionEditor::Data m_data;
	m::Conf::Data&        m_conf;

private:
	void refresh() override;

	/* zoomAbout
	Zooms and centers the viewport around the mouse cursor. Wants a function to 
	apply to the current ratio. */

	void zoomAbout(std::function<float()> f);

	Pixel currentFrameToPixel() const;

	Pixel m_playhead;
	float m_ratio;
};
} // namespace giada::v
#endif
