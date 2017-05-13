/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


#ifndef GD_EDITOR_H
#define GD_EDITOR_H


#include "window.h"


class SampleChannel;
class geButton;
class geWaveTools;
class geVolumeTool;
class geBoostTool;
class gePanTool;
class gePitchTool;
class geRangeTool;
class geChoice;
class geCheck;
class geBox;
class geButton;


class gdSampleEditor : public gdWindow
{
private:

	static void cb_reload        (Fl_Widget *w, void *p);
	static void cb_zoomIn        (Fl_Widget *w, void *p);
	static void cb_zoomOut       (Fl_Widget *w, void *p);
	static void cb_changeGrid    (Fl_Widget *w, void *p);
	static void cb_enableSnap    (Fl_Widget *w, void *p);
	inline void __cb_reload();
	inline void __cb_zoomIn();
	inline void __cb_zoomOut();
	inline void __cb_changeGrid();
	inline void __cb_enableSnap();

public:

	gdSampleEditor(SampleChannel *ch);
	~gdSampleEditor();

	geWaveTools  *waveTools;
	geVolumeTool *volumeTool;
	geBoostTool  *boostTool;
	gePanTool    *panTool;
	gePitchTool  *pitchTool;
	geRangeTool  *rangeTool;
	geButton     *reload;

	geChoice     *grid;
	geCheck      *snap;
	geBox        *sep;
	geButton     *zoomIn;
	geButton     *zoomOut;

	SampleChannel *ch;
};


#endif
