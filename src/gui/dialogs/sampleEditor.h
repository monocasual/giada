/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "core/conf.h"
#include "core/types.h"
#include "glue/sampleEditor.h"
#include "gui/dialogs/window.h"

class geCheck;

namespace giada::v
{
class geBox;
class geButton;
class geStatusButton;
class geChoice;
class gePack;
class geGroup;
class geWaveTools;
class geBoostTool;
class gePitchTool;
class geRangeTool;
class geShiftTool;
class gdSampleEditor : public gdWindow
{
	friend class geWaveform;

public:
	gdSampleEditor(ID channelId, m::Conf::Data&);
	~gdSampleEditor();

	void rebuild() override;
	void refresh() override;

	geChoice* grid;
	geCheck*  snap;
	geButton* zoomIn;
	geButton* zoomOut;

	geWaveTools* waveTools;

	gePitchTool* pitchTool;
	geRangeTool* rangeTool;
	geShiftTool* shiftTool;
	geButton*    reload;

	geStatusButton* play;
	geButton*       rewind;
	geCheck*        loop;
	geBox*          info;

private:
	void updateInfo();
	void togglePreview();

	ID                    m_channelId;
	c::sampleEditor::Data m_data;
	m::Conf::Data&        m_conf;
};
} // namespace giada::v

#endif
