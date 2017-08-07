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
friend class geWaveform;

private:

	Fl_Group* createUpperBar();
	Fl_Group* createBottomBar(int x, int y, int h);

	Fl_Group* createPreviewBox(int x, int y, int h);
	Fl_Group* createOpTools(int x, int y, int h);
	Fl_Group* createInfoBox(int x, int y, int h);

	static void cb_reload    (Fl_Widget* w, void* p);
	static void cb_zoomIn    (Fl_Widget* w, void* p);
	static void cb_zoomOut   (Fl_Widget* w, void* p);
	static void cb_changeGrid(Fl_Widget* w, void* p);
	static void cb_enableSnap(Fl_Widget* w, void* p);
	static void cb_togglePreview(Fl_Widget* w, void* p);
	static void cb_rewindPreview(Fl_Widget* w, void* p);
	void __cb_reload();
	void __cb_zoomIn();
	void __cb_zoomOut();
	void __cb_changeGrid();
	void __cb_enableSnap();
	void __cb_togglePreview();
	void __cb_rewindPreview();

public:

	gdSampleEditor(SampleChannel* ch);
	~gdSampleEditor();

	void updateInfo();

	geChoice* grid;
	geCheck*  snap;
	geBox*    sep1;
	geButton* zoomIn;
	geButton* zoomOut;
	
	geWaveTools* waveTools;

	geVolumeTool* volumeTool;
	geBoostTool*  boostTool;
	gePanTool*    panTool;

	gePitchTool* pitchTool;

	geRangeTool* rangeTool;
	geButton*    reload;

	geButton* play;
	geButton* rewind;
	geCheck* loop;
	geBox* info;

	SampleChannel* ch;
};


#endif
