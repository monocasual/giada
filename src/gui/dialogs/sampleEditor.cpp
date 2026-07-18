/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/glue/sampleEditor.h"
#include "src/deps/geompp/src/range.hpp"
#include "src/glue/channel.h"
#include "src/gui/dialogs/sampleEditor.h"
#include "src/gui/dialogs/warnings.h"
#include "src/gui/elems/basics/box.h"
#include "src/gui/elems/basics/check.h"
#include "src/gui/elems/basics/choice.h"
#include "src/gui/elems/basics/dial.h"
#include "src/gui/elems/basics/group.h"
#include "src/gui/elems/basics/imageButton.h"
#include "src/gui/elems/basics/input.h"
#include "src/gui/elems/basics/pack.h"
#include "src/gui/elems/basics/textButton.h"
#include "src/gui/elems/mainWindow/keyboard/channel.h"
#include "src/gui/elems/sampleEditor/pitchTool.h"
#include "src/gui/elems/sampleEditor/rangeTool.h"
#include "src/gui/elems/sampleEditor/shiftTool.h"
#include "src/gui/elems/sampleEditor/waveTools.h"
#include "src/gui/elems/sampleEditor/waveform.h"
#include "src/gui/graphics.h"
#include "src/gui/ui.h"
#include "src/utils/gui.h"
#include "src/utils/string.h"
#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <cassert>
#include <cmath>
#include <fmt/core.h>

#if G_OS_WINDOWS
#undef IN
#undef OUT
#endif

extern giada::v::Ui* g_ui;

namespace giada::v
{
gdSampleEditor::gdSampleEditor(ID channelId, const Model& model)
: gdWindow(u::gui::getCenterWinBounds(model.sampleEditorBounds), g_ui->getI18Text(LangMap::SAMPLEEDITOR_TITLE), WID_SAMPLE_EDITOR)
, m_channelId(channelId)
, m_data(c::sampleEditor::getData(channelId))
{
	geFlex* container = new geFlex(getContentBounds().reduced({G_GUI_OUTER_MARGIN}), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		geFlex* top = new geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN);
		{
			m_reload  = new geTextButton(g_ui->getI18Text(LangMap::SAMPLEEDITOR_RELOAD));
			m_grid    = new geChoice();
			m_snap    = new geCheck(0, 0, 0, 0, g_ui->getI18Text(LangMap::COMMON_SNAPTOGRID));
			m_zoomOut = new geImageButton(graphics::minusOff, graphics::minusOn);
			m_zoomIn  = new geImageButton(graphics::plusOff, graphics::plusOn);
			top->addWidget(m_reload, 70);
			top->addWidget(new geBox(), G_GUI_INNER_MARGIN);
			top->addWidget(m_grid, 50);
			top->addWidget(m_snap, 12);
			top->addWidget(new geBox());
			top->addWidget(m_zoomOut, G_GUI_UNIT);
			top->addWidget(m_zoomIn, G_GUI_UNIT);
			top->end();
		}

		m_waveTools = new geWaveTools(0, 0, 0, 0, model.sampleEditorGridOn, model.sampleEditorGridVal);
		m_waveTools->rebuild(c::sampleEditor::getData(m_channelId)); // TODO - crappy temporary workaround for WaveTools

		geFlex* bottom = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			geFlex* controls = new geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN, {21, 0, 22, 0});
			{
				m_rewind = new geImageButton(graphics::rewindOff, graphics::rewindOn);
				m_play   = new geImageButton(graphics::playOff, graphics::playOn);
				m_loop   = new geCheck(0, 0, 0, 0, g_ui->getI18Text(LangMap::SAMPLEEDITOR_LOOP));
				controls->addWidget(m_rewind, 25);
				controls->addWidget(m_play, 25);
				controls->addWidget(m_loop, -1);
				controls->end();
			}

			geFlex* tools = new geFlex(Direction::VERTICAL, G_GUI_INNER_MARGIN);
			{
				m_pitchTool = new gePitchTool(m_data);
				m_rangeTool = new geRangeTool(m_data);
				m_shiftTool = new geShiftTool(m_data);
				tools->addWidget(m_pitchTool, G_GUI_UNIT * 2);
				tools->addWidget(m_rangeTool, G_GUI_UNIT);
				tools->addWidget(m_shiftTool, G_GUI_UNIT);
				tools->end();
			}

			m_info = new geBox();

			bottom->addWidget(controls, 120);
			bottom->addWidget(tools, 420);
			bottom->addWidget(m_info);
			bottom->end();
		}

		container->addWidget(top, G_GUI_UNIT);
		container->addWidget(m_waveTools);
		container->addWidget(bottom, 88);
		container->end();
	}

	add(container);
	resizable(container);

	m_reload->onClick = [this]()
	{
		c::sampleEditor::reload(m_data.channelId);
		redraw();
	};

	m_grid->addItem("1", 1);
	m_grid->addItem("2", 2);
	m_grid->addItem("3", 3);
	m_grid->addItem("4", 4);
	m_grid->addItem("6", 6);
	m_grid->addItem("8", 8);
	m_grid->addItem("16", 16);
	m_grid->addItem("32", 32);
	m_grid->addItem("64", 64);
	m_grid->copy_tooltip(g_ui->getI18Text(LangMap::COMMON_GRIDRES));
	m_grid->showItem(model.sampleEditorGridVal);
	m_grid->onChange = [this](int)
	{
		/* TODO - redraw grid if != (off) */
		m_waveTools->waveform->setGridLevel(m_grid->getSelectedId());
	};

	m_snap->value(model.sampleEditorGridOn);
	m_snap->copy_tooltip(g_ui->getI18Text(LangMap::COMMON_SNAPTOGRID));
	m_snap->onChange = [this](bool val)
	{
		m_waveTools->waveform->setSnap(val);
	};

	m_zoomOut->copy_tooltip(g_ui->getI18Text(LangMap::COMMON_ZOOMOUT));
	m_zoomOut->onClick = [this]()
	{
		m_waveTools->waveform->setZoom(geWaveform::Zoom::OUT);
		m_waveTools->redraw();
	};

	m_zoomIn->copy_tooltip(g_ui->getI18Text(LangMap::COMMON_ZOOMIN));
	m_zoomIn->onClick = [this]()
	{
		m_waveTools->waveform->setZoom(geWaveform::Zoom::IN);
		m_waveTools->redraw();
	};

	m_play->setToggleable(true);
	m_play->onClick = []()
	{ c::sampleEditor::togglePreview(); };

	m_rewind->onClick = [this]()
	{
		c::sampleEditor::setPreviewTracker(m_data.getSample().range.getA());
	};

	m_loop->onChange = [](bool shouldLoop)
	{ c::sampleEditor::setLoop(shouldLoop); };

	m_info->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_TOP);

	size_range(720, 480);
	set_non_modal();
	rebuild();
	show();
}

/* -------------------------------------------------------------------------- */

gdSampleEditor::~gdSampleEditor()
{
	g_ui->model.sampleEditorBounds  = getBounds();
	g_ui->model.sampleEditorGridVal = m_grid->getSelectedId();
	g_ui->model.sampleEditorGridOn  = m_snap->value();

	c::sampleEditor::cleanupPreview();
}

/* -------------------------------------------------------------------------- */

void gdSampleEditor::rebuild()
{
	c::sampleEditor::preparePreview(m_channelId);

	m_data = c::sampleEditor::getData(m_channelId);
	m_waveTools->rebuild(m_data);
	m_pitchTool->rebuild(m_data);
	m_rangeTool->rebuild(m_data);
	m_shiftTool->rebuild(m_data);

	updateInfo();
	updateTitleWithScene(m_data.scene);

	if (!m_data.isValid())
	{
		m_rangeTool->deactivate();
		m_pitchTool->deactivate();
		m_shiftTool->deactivate();
		m_reload->deactivate();
	}
	else
	{
		m_rangeTool->activate();
		m_pitchTool->activate();
		m_shiftTool->activate();
		m_reload->activate();
	}

	if (m_data.isLogical) // Logical samples (aka takes) cannot be reloaded.
		m_reload->deactivate();
}

/* -------------------------------------------------------------------------- */

void gdSampleEditor::refresh()
{
	m_waveTools->refresh();
	m_play->setValue(m_data.a_getPreviewStatus() == ChannelStatus::PLAY);
}

/* -------------------------------------------------------------------------- */

void gdSampleEditor::refreshPitch()
{
	m_pitchTool->refresh();
}

/* -------------------------------------------------------------------------- */

void gdSampleEditor::updateInfo()
{
	std::string infoText = fmt::format(fmt::runtime(g_ui->getI18Text(LangMap::SAMPLEEDITOR_INFO)),
	    m_data.wavePath, m_data.waveSize, m_data.waveDuration,
	    m_data.waveBits != 0 ? std::to_string(m_data.waveBits) : "?", m_data.waveRate);

	m_info->setLabel(infoText);
}

/* -------------------------------------------------------------------------- */

void gdSampleEditor::updateTitleWithScene(Scene scene)
{
	setTitle(fmt::format("{} - {} {}", g_ui->getI18Text(LangMap::ACTIONEDITOR_TITLE),
	    g_ui->getI18Text(LangMap::COMMON_SCENE), scene.getIndex() + 1));
}
} // namespace giada::v
