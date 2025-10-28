/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef GD_SAMPLE_EDITOR_H
#define GD_SAMPLE_EDITOR_H

#include "src/glue/sampleEditor.h"
#include "src/gui/dialogs/window.h"
#include "src/gui/model.h"

namespace giada::v
{
class geCheck;
class geBox;
class geTextButton;
class geImageButton;
class geChoice;
class geWaveTools;
class gePitchTool;
class geRangeTool;
class geShiftTool;
class gdSampleEditor : public gdWindow
{
	friend class geWaveform;

public:
	gdSampleEditor(ID channelId, const Model&);
	~gdSampleEditor();

	void rebuild() override;
	void refresh() override;

	geChoice*      grid;
	geCheck*       snap;
	geImageButton* zoomIn;
	geImageButton* zoomOut;

	geWaveTools* waveTools;

	gePitchTool*  pitchTool;
	geRangeTool*  rangeTool;
	geShiftTool*  shiftTool;
	geTextButton* reload;

	geImageButton* play;
	geImageButton* rewind;
	geCheck*       loop;
	geBox*         info;

private:
	void updateInfo();

	/* updateTitleWithScene
	Updates window title with the current scene being edited. */

	void updateTitleWithScene(std::size_t scene);

	ID                    m_channelId;
	c::sampleEditor::Data m_data;
};
} // namespace giada::v

#endif
