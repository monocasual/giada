/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "glue/sampleEditor.h"
#include "core/engine.h"
#include "core/mixer.h"
#include "core/wave.h"
#include "core/waveFx.h"
#include "glue/channel.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/dialogs/warnings.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/choice.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/basics/group.h"
#include "gui/elems/basics/imageButton.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/basics/pack.h"
#include "gui/elems/basics/textButton.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "gui/elems/sampleEditor/pitchTool.h"
#include "gui/elems/sampleEditor/rangeTool.h"
#include "gui/elems/sampleEditor/shiftTool.h"
#include "gui/elems/sampleEditor/waveTools.h"
#include "gui/elems/sampleEditor/waveform.h"
#include "gui/graphics.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include "utils/string.h"
#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <cassert>
#include <cmath>
#include <fmt/core.h>

#ifdef G_OS_WINDOWS
#undef IN
#undef OUT
#endif

extern giada::v::Ui* g_ui;

namespace giada::v
{
gdSampleEditor::gdSampleEditor(ID channelId, const Model& model)
: gdWindow(u::gui::getCenterWinBounds(model.sampleEditorBounds), g_ui->getI18Text(LangMap::SAMPLEEDITOR_TITLE), WID_SAMPLE_EDITOR)
, m_channelId(channelId)
{
	geFlex* container = new geFlex(getContentBounds().reduced({G_GUI_OUTER_MARGIN}), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		geFlex* top = new geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN);
		{
			reload  = new geTextButton(g_ui->getI18Text(LangMap::SAMPLEEDITOR_RELOAD));
			grid    = new geChoice();
			snap    = new geCheck(0, 0, 0, 0, g_ui->getI18Text(LangMap::COMMON_SNAPTOGRID));
			zoomOut = new geImageButton(graphics::minusOff, graphics::minusOn);
			zoomIn  = new geImageButton(graphics::plusOff, graphics::plusOn);
			top->addWidget(reload, 70);
			top->addWidget(grid, 50);
			top->addWidget(snap, 12);
			top->addWidget(new geBox());
			top->addWidget(zoomOut, G_GUI_UNIT);
			top->addWidget(zoomIn, G_GUI_UNIT);
			top->end();
		}

		waveTools = new geWaveTools(0, 0, 0, 0, model.sampleEditorGridOn, model.sampleEditorGridVal);
		waveTools->rebuild(c::sampleEditor::getData(m_channelId)); // TODO - crappy temporary workaround for WaveTools

		geFlex* bottom = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			geFlex* controls = new geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN, {21, 0, 22, 0});
			{
				rewind = new geImageButton(graphics::rewindOff, graphics::rewindOn);
				play   = new geImageButton(graphics::playOff, graphics::playOn);
				loop   = new geCheck(0, 0, 0, 0, g_ui->getI18Text(LangMap::SAMPLEEDITOR_LOOP));
				controls->addWidget(rewind, 25);
				controls->addWidget(play, 25);
				controls->addWidget(loop, -1);
				controls->end();
			}

			geFlex* tools = new geFlex(Direction::VERTICAL, G_GUI_INNER_MARGIN);
			{
				pitchTool = new gePitchTool(m_data);
				rangeTool = new geRangeTool(m_data);
				shiftTool = new geShiftTool(m_data);
				tools->addWidget(pitchTool, G_GUI_UNIT);
				tools->addWidget(rangeTool, G_GUI_UNIT);
				tools->addWidget(shiftTool, G_GUI_UNIT);
				tools->end();
			}

			info = new geBox();

			bottom->addWidget(controls, 120);
			bottom->addWidget(tools, 420);
			bottom->addWidget(info);
			bottom->end();
		}

		container->addWidget(top, G_GUI_UNIT);
		container->addWidget(waveTools);
		container->addWidget(bottom, 68);
		container->end();
	}

	add(container);
	resizable(container);

	reload->onClick = [this]()
	{
		c::sampleEditor::reload(m_data.channelId);
		redraw();
	};

	grid->addItem("1", 1);
	grid->addItem("2", 2);
	grid->addItem("3", 3);
	grid->addItem("4", 4);
	grid->addItem("6", 6);
	grid->addItem("8", 8);
	grid->addItem("16", 16);
	grid->addItem("32", 32);
	grid->addItem("64", 64);
	grid->copy_tooltip(g_ui->getI18Text(LangMap::COMMON_GRIDRES));
	grid->showItem(model.sampleEditorGridVal);
	grid->onChange = [this](ID)
	{
		/* TODO - redraw grid if != (off) */
		waveTools->waveform->setGridLevel(grid->getSelectedId());
	};

	snap->value(model.sampleEditorGridOn);
	snap->copy_tooltip(g_ui->getI18Text(LangMap::COMMON_SNAPTOGRID));
	snap->onChange = [this](bool val)
	{
		waveTools->waveform->setSnap(val);
	};

	zoomOut->copy_tooltip(g_ui->getI18Text(LangMap::COMMON_ZOOMOUT));
	zoomOut->onClick = [this]()
	{
		waveTools->waveform->setZoom(geWaveform::Zoom::OUT);
		waveTools->redraw();
	};

	zoomIn->copy_tooltip(g_ui->getI18Text(LangMap::COMMON_ZOOMIN));
	zoomIn->onClick = [this]()
	{
		waveTools->waveform->setZoom(geWaveform::Zoom::IN);
		waveTools->redraw();
	};

	play->setToggleable(true);
	play->onClick = []()
	{ c::sampleEditor::togglePreview(); };

	rewind->onClick = [this]()
	{
		c::sampleEditor::setPreviewTracker(m_data.begin);
	};

	loop->onChange = [](bool shouldLoop)
	{ c::sampleEditor::setLoop(shouldLoop); };

	info->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_TOP);

	size_range(720, 480);
	set_non_modal();
	rebuild();
	show();
}

/* -------------------------------------------------------------------------- */

gdSampleEditor::~gdSampleEditor()
{
	g_ui->model.sampleEditorBounds  = getBounds();
	g_ui->model.sampleEditorGridVal = grid->getSelectedId();
	g_ui->model.sampleEditorGridOn  = snap->value();

	c::sampleEditor::stopPreview();
	c::sampleEditor::cleanupPreview();
}

/* -------------------------------------------------------------------------- */

void gdSampleEditor::rebuild()
{
	c::sampleEditor::preparePreview(m_channelId);

	m_data = c::sampleEditor::getData(m_channelId);
	waveTools->rebuild(m_data);
	pitchTool->rebuild(m_data);
	rangeTool->rebuild(m_data);
	shiftTool->rebuild(m_data);

	updateInfo();

	if (m_data.isLogical) // Logical samples (aka takes) cannot be reloaded.
		reload->deactivate();
}

/* -------------------------------------------------------------------------- */

void gdSampleEditor::refresh()
{
	waveTools->refresh();
	play->setValue(m_data.a_getPreviewStatus() == ChannelStatus::PLAY);
}

/* -------------------------------------------------------------------------- */

void gdSampleEditor::updateInfo()
{
	std::string infoText = fmt::format(fmt::runtime(g_ui->getI18Text(LangMap::SAMPLEEDITOR_INFO)),
	    m_data.wavePath, m_data.waveSize, m_data.waveDuration,
	    m_data.waveBits != 0 ? std::to_string(m_data.waveBits) : "?", m_data.waveRate);

	info->copy_label(infoText.c_str());
}
} // namespace giada::v
