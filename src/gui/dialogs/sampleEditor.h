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


#ifndef GD_EDITOR_H
#define GD_EDITOR_H


#include "core/types.h"
#include "glue/sampleEditor.h"
#include "window.h"


class geButton;
class geCheck;
class geBox;
class geButton;
class geStatusButton;


namespace giada {
namespace m
{
class Wave;
}
namespace v 
{
class geChoice;
class geVolumeTool;
class geWaveTools;
class geBoostTool;
class gePanTool;
class gePitchTool;
class geRangeTool;
class geShiftTool;
class gdSampleEditor : public gdWindow
{
friend class geWaveform;

public:

	gdSampleEditor(ID channelId);
	~gdSampleEditor();

	void rebuild() override;
	void refresh() override;

	geChoice* grid;
	geCheck*  snap;
	geBox*    sep1;
	geButton* zoomIn;
	geButton* zoomOut;
	
	geWaveTools* waveTools;

	geVolumeTool* volumeTool;
	gePanTool*    panTool;

	gePitchTool* pitchTool;

	geRangeTool* rangeTool;
	geShiftTool* shiftTool;
	geButton*    reload;

	geStatusButton* play;
	geButton*       rewind;
	geCheck*        loop;
	geBox*          info;


private:

	Fl_Group* createUpperBar();
	Fl_Group* createBottomBar(int x, int y, int h);

	Fl_Group* createPreviewBox(int x, int y, int h);
	Fl_Group* createOpTools(int x, int y, int h);
	Fl_Group* createInfoBox(int x, int y, int h);

	static void cb_reload    (Fl_Widget* /*w*/, void* p);
	static void cb_zoomIn    (Fl_Widget* /*w*/, void* p);
	static void cb_zoomOut   (Fl_Widget* /*w*/, void* p);
	static void cb_changeGrid(Fl_Widget* /*w*/, void* p);
	static void cb_enableSnap(Fl_Widget* /*w*/, void* p);
	static void cb_togglePreview(Fl_Widget* /*w*/, void* p);
	static void cb_rewindPreview(Fl_Widget* /*w*/, void* p);
	void cb_reload();
	void cb_zoomIn();
	void cb_zoomOut();
	void cb_changeGrid();
	void cb_enableSnap();
	void cb_togglePreview();
	void cb_rewindPreview();

	void updateInfo();

	ID m_channelId;

	c::sampleEditor::Data m_data;
};
}} // giada::v::


#endif
