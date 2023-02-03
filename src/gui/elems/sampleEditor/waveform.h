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

#ifndef GE_WAVEFORM_H
#define GE_WAVEFORM_H

#include "core/const.h"
#include "core/types.h"
#include <FL/Fl_Widget.H>
#include <vector>

namespace giada::c::sampleEditor
{
struct Data;
}

namespace giada::v
{
class geWaveform : public Fl_Widget
{
public:
#ifdef G_OS_WINDOWS
/* Fuck... */
#undef IN
#undef OUT
#endif
	enum class Zoom
	{
		IN,
		OUT
	};

	geWaveform(int x, int y, int w, int h, bool gridEnabled, int gridVal);

	void draw() override;
	int  handle(int e) override;

	/* isSelected
	Tells whether a portion of the waveform has been selected. */

	bool isSelected() const;

	int getSelectionA() const;
	int getSelectionB() const;

	bool getSnap() const;
	int  getSize() const;

	/* recalcPoints
	Recomputes m_chanStart, m_chanEnd, ... */

	void recalcPoints();

	/* zoom
	Type == 1 : zoom out, type == -1: zoom in */

	void setZoom(Zoom z);

	/* strecthToWindow
	Shrinks or enlarge the waveform to match parent's width (gWaveTools) */

	void stretchToWindow();

	/* rebuild
	Redraws the waveform. */

	void rebuild(const c::sampleEditor::Data& d);

	/* setGridLevel
	Sets a new frequency level for the grid. 0 means disabled. */

	void setGridLevel(int l);

	void setSnap(bool v);

	/* clearSelection
	Removes any active selection. */

	void clearSelection();

	/* setWaveId
	Call this when the Wave ID has changed (e.g. after a reload). */

	void setWaveId(ID /*id*/){/* TODO m_waveId = id;*/};

private:
	static const int FLAG_WIDTH  = 20;
	static const int FLAG_HEIGHT = 20;
	static const int BORDER      = 8; // window border <-> widget border
	static const int SNAPPING    = 16;

	/* selection
	Portion of the selected wave, in frames. */

	struct
	{
		int a;
		int b;
	} m_selection;

	/* data
	Real graphic stuff from the underlying waveform. */

	struct
	{
		std::vector<int> sup;  // upper part of the waveform
		std::vector<int> inf;  // lower part of the waveform
		int              size; // width of the waveform to draw (in pixel)
	} m_waveform;

	struct
	{
		bool             snap;
		int              level;
		std::vector<int> points;
	} m_grid;

	/* mouseOnStart/end
	Is mouse on start or end flag? */

	bool mouseOnStart() const;
	bool mouseOnEnd() const;

	/* mouseOnSelectionA/B
	As above, for the selection. */

	bool mouseOnSelectionA() const;
	bool mouseOnSelectionB() const;

	/* smaller
	Is the waveform smaller than the parent window? */

	bool smaller() const;

	int pixelToFrame(int p) const; // TODO - move these to utils::, will be needed in actionEditor
	int frameToPixel(int f) const; // TODO - move these to utils::, will be needed in actionEditor

	/* fixSelection
	Helper function which flattens the selection if it was made from right to left 
	(inverse selection). It also computes the absolute points. Call this one
	whenever the selection gesture is done. */

	void fixSelection();

	/* clearData
	Destroys any graphical buffer. */

	void clearData();

	/* snap
	Snaps a point at 'pos' pixel. */

	int snap(int pos);

	/* draw*
	Drawing functions. */

	void drawSelection();
	void drawWaveform(int from, int to);
	void drawGrid(int from, int to);
	void drawStartEndPoints();
	void drawPlayHead();

	void selectAll();

	/* alloc
	Allocates memory for the picture. It's smart enough not to reallocate if 
	datasize hasn't changed, but it can be forced otherwise. */

	int alloc(int datasize, bool force = false);

	const c::sampleEditor::Data* m_data;

	int   m_chanStart;
	bool  m_chanStartLit;
	int   m_chanEnd;
	bool  m_chanEndLit;
	bool  m_pushed;
	bool  m_dragged;
	bool  m_resizedA;
	bool  m_resizedB;
	float m_ratio;
	int   m_mouseX;
	int   m_mouseY;
};
} // namespace giada::v

#endif
