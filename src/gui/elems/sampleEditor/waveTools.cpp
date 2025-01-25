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

#include "src/gui/elems/sampleEditor/waveTools.h"
#include "src/glue/sampleEditor.h"
#include "src/gui/const.h"
#include "src/gui/dialogs/sampleEditor.h"
#include "src/gui/elems/basics/boxtypes.h"
#include "src/gui/elems/basics/menu.h"
#include "src/gui/elems/basics/scrollbar.h"
#include "src/gui/elems/sampleEditor/waveform.h"
#include "src/gui/elems/sampleEditor/waveform_DEPR_.h"
#include "src/gui/ui.h"
#include "src/utils/gui.h"
#include <cstdint>

extern giada::v::Ui* g_ui;

namespace giada::v
{
namespace
{
enum class Menu
{
	CUT = 0,
	COPY,
	PASTE,
	TRIM,
	SILENCE,
	REVERSE,
	NORMALIZE,
	FADE_IN,
	FADE_OUT,
	SMOOTH_EDGES,
	SET_RANGE,
	TO_NEW_CHANNEL
};
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geWaveTools::geWaveTools(int x, int y, int w, int h, bool gridEnabled, int gridVal)
: geFlex(Direction::VERTICAL, G_GUI_OUTER_MARGIN)
, m_data(nullptr)
{
	m_waveform  = new v::geWaveform();
	m_scrollbar = new geScrollbar();
	addWidget(m_waveform);
	addWidget(m_scrollbar, 20);
	end();

	m_scrollbar->type(FL_HORIZONTAL);
	// m_scrollbar->bounds(0.0, 100.0);
	//  m_scrollbar->step(0.01);
	m_scrollbar->slider_size(1);
	m_scrollbar->onScroll = [this](double value)
	{
		// m_waveform->move(value / 100.0);
		m_waveform->moveRange(value);
	};
}

/* -------------------------------------------------------------------------- */

void geWaveTools::rebuild(const c::sampleEditor::Data& d)
{
	m_data = &d;
	m_waveform->rebuild(d);

	/*
	    m_scrollbar->type(FL_HORIZONTAL);
	    m_scrollbar->bounds(0, m_waveform->getRange().b);
	    m_scrollbar->step(1);
	    m_scrollbar->value(0.0);
	    m_scrollbar->slider_size(0.1);*/
}

/* -------------------------------------------------------------------------- */

void geWaveTools::refresh()
{
}

/* -------------------------------------------------------------------------- */

void geWaveTools::zoomIn()
{
	m_waveform->zoomIn();
	m_scrollbar->slider_size(m_scrollbar->slider_size() / 2);
	// TODO - m_scrollbar->bounds(m_waveform->getMoveableRange());
}

/* -------------------------------------------------------------------------- */

void geWaveTools::zoomOut()
{
	m_waveform->zoomOut();
	m_scrollbar->slider_size(m_scrollbar->slider_size() * 2);
	// TODO - m_scrollbar->bounds(m_waveform->getMoveableRange());
}

/* -------------------------------------------------------------------------- */

int geWaveTools::handle(int e)
{
	switch (e)
	{
	case FL_MOUSEWHEEL:
	{
		/* Zoom with mousewheel (or two fingers gesture on the mac trackpad) only
		works if Ctrl or Cmd are pressed. */
		if (!Fl::event_command())
			return geFlex::handle(e);
		// waveform_DEPR_->setZoom(Fl::event_dy() <= 0 ? geWaveform_DEPR_::Zoom::IN : geWaveform_DEPR_::Zoom::OUT);
		redraw();
		return 1;
	}
	case FL_PUSH:
	{
		if (Fl::event_button3()) // right button
		{
			openMenu();
			return 1;
		}
		// Fl::focus(waveform_DEPR_);
		return geFlex::handle(e);
	}
	default:
		return geFlex::handle(e);
	}
}

/* -------------------------------------------------------------------------- */

void geWaveTools::openMenu()
{
	geMenu menu;

	menu.addItem((ID)Menu::CUT, g_ui->getI18Text(LangMap::SAMPLEEDITOR_TOOLS_CUT));
	menu.addItem((ID)Menu::COPY, g_ui->getI18Text(LangMap::SAMPLEEDITOR_TOOLS_COPY));
	menu.addItem((ID)Menu::PASTE, g_ui->getI18Text(LangMap::SAMPLEEDITOR_TOOLS_PASTE));
	menu.addItem((ID)Menu::TRIM, g_ui->getI18Text(LangMap::SAMPLEEDITOR_TOOLS_TRIM));
	menu.addItem((ID)Menu::SILENCE, g_ui->getI18Text(LangMap::SAMPLEEDITOR_TOOLS_SILENCE));
	menu.addItem((ID)Menu::REVERSE, g_ui->getI18Text(LangMap::SAMPLEEDITOR_TOOLS_REVERSE));
	menu.addItem((ID)Menu::NORMALIZE, g_ui->getI18Text(LangMap::SAMPLEEDITOR_TOOLS_NORMALIZE));
	menu.addItem((ID)Menu::FADE_IN, g_ui->getI18Text(LangMap::SAMPLEEDITOR_TOOLS_FADE_IN));
	menu.addItem((ID)Menu::FADE_OUT, g_ui->getI18Text(LangMap::SAMPLEEDITOR_TOOLS_FADE_OUT));
	menu.addItem((ID)Menu::SMOOTH_EDGES, g_ui->getI18Text(LangMap::SAMPLEEDITOR_TOOLS_SMOOTH_EDGES));
	menu.addItem((ID)Menu::SET_RANGE, g_ui->getI18Text(LangMap::SAMPLEEDITOR_TOOLS_SET_RANGE));
	menu.addItem((ID)Menu::TO_NEW_CHANNEL, g_ui->getI18Text(LangMap::SAMPLEEDITOR_TOOLS_TO_NEW_CHANNEL));

#if 0
	if (!waveform_DEPR_->isSelected())
	{
		menu.setEnabled((ID)Menu::CUT, false);
		menu.setEnabled((ID)Menu::COPY, false);
		menu.setEnabled((ID)Menu::TRIM, false);
		menu.setEnabled((ID)Menu::SILENCE, false);
		menu.setEnabled((ID)Menu::REVERSE, false);
		menu.setEnabled((ID)Menu::NORMALIZE, false);
		menu.setEnabled((ID)Menu::FADE_IN, false);
		menu.setEnabled((ID)Menu::FADE_OUT, false);
		menu.setEnabled((ID)Menu::SMOOTH_EDGES, false);
		menu.setEnabled((ID)Menu::SET_RANGE, false);
		menu.setEnabled((ID)Menu::TO_NEW_CHANNEL, false);
	}

	menu.onSelect = [channelId = m_data->channelId,
	                    a      = waveform_DEPR_->getSelectionA(),
	                    b      = waveform_DEPR_->getSelectionB()](ID id)
	{
		switch (static_cast<Menu>(id))
		{
		case Menu::CUT:
			c::sampleEditor::cut(channelId, a, b);
			break;
		case Menu::COPY:
			c::sampleEditor::copy(channelId, a, b);
			break;
		case Menu::PASTE:
			c::sampleEditor::paste(channelId, a);
			break;
		case Menu::TRIM:
			c::sampleEditor::trim(channelId, a, b);
			break;
		case Menu::SILENCE:
			c::sampleEditor::silence(channelId, a, b);
			break;
		case Menu::REVERSE:
			c::sampleEditor::reverse(channelId, a, b);
			break;
		case Menu::NORMALIZE:
			c::sampleEditor::normalize(channelId, a, b);
			break;
		case Menu::FADE_IN:
			c::sampleEditor::fade(channelId, a, b, m::wfx::Fade::IN);
			break;
		case Menu::FADE_OUT:
			c::sampleEditor::fade(channelId, a, b, m::wfx::Fade::OUT);
			break;
		case Menu::SMOOTH_EDGES:
			c::sampleEditor::smoothEdges(channelId, a, b);
			break;
		case Menu::SET_RANGE:
			c::sampleEditor::setRange(channelId, {a, b});
			break;
		case Menu::TO_NEW_CHANNEL:
			c::sampleEditor::toNewChannel(channelId, a, b);
			break;
		}
	};
#endif
	menu.popup();
}
} // namespace giada::v