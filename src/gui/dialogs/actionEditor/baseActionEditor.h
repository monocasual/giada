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

#ifndef GD_BASE_ACTION_EDITOR_H
#define GD_BASE_ACTION_EDITOR_H

#include "core/conf.h"
#include "glue/actionEditor.h"
#include "gui/dialogs/window.h"
#include <functional>

namespace giada::m
{
class Channel;
struct Action;
} // namespace giada::m

namespace giada::v
{
class geGridTool;
class geImageButton;
class geSplitScroll;
class gdBaseActionEditor : public gdWindow
{
public:
	virtual ~gdBaseActionEditor();

	int  handle(int e) override;
	void draw() override;

	Pixel frameToPixel(Frame f) const;
	Frame pixelToFrame(Pixel p, Frame framesInBeat, bool snap = true) const;

	ID channelId;

	geGridTool* gridTool;

	Pixel fullWidth; // Full widgets width, i.e. scaled-down full sequencer
	Pixel loopWidth; // Loop width, i.e. scaled-down sequencer range

protected:
	static constexpr float MIN_RATIO  = 25.0f;
	static constexpr float MAX_RATIO  = 40000.0f;
	static constexpr float RATIO_STEP = 1.5f;

	gdBaseActionEditor(ID channelId, const m::Conf&);

	/* getMouseOverContent
	Returns mouse x-position relative to the viewport content. */

	int getMouseOverContent() const;

	void zoomIn();
	void zoomOut();

	/* computeWidth
	Computes total width, in pixel. */

	void computeWidth(Frame framesInSeq, Frame framesInLoop);

	/* prepareWindow
	Initializes window (favicon, limits, ...). */

	void prepareWindow();

	geImageButton* m_zoomInBtn;
	geImageButton* m_zoomOutBtn;
	geSplitScroll* m_splitScroll;

	c::actionEditor::Data m_data;

private:
	void refresh() override;

	/* zoomAbout
	Zooms and centers the viewport around the mouse cursor. Wants a function to 
	apply to the current ratio. */

	void zoomAbout(std::function<float()> f);

	Pixel currentFrameToPixel() const;

	float m_ratio;
};
} // namespace giada::v
#endif
