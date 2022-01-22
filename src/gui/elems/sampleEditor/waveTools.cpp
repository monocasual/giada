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

#include "waveTools.h"
#include "core/const.h"
#include "core/model/model.h"
#include "core/waveFx.h"
#include "glue/sampleEditor.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/elems/basics/boxtypes.h"
#include "utils/gui.h"
#include "waveform.h"
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Menu_Item.H>
#include <cstdint>

namespace giada
{
namespace v
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
	SET_BEGIN_END,
	TO_NEW_CHANNEL
};

/* -------------------------------------------------------------------------- */

void menuCallback_(Fl_Widget* w, void* v)
{
	const geWaveTools* wt = static_cast<geWaveTools*>(w);

	ID   channelId    = wt->getChannelData().channelId;
	Menu selectedItem = (Menu)(intptr_t)v;

	Frame a = wt->waveform->getSelectionA();
	Frame b = wt->waveform->getSelectionB();

	switch (selectedItem)
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
	case Menu::SET_BEGIN_END:
		c::sampleEditor::setBeginEnd(channelId, a, b);
		break;
	case Menu::TO_NEW_CHANNEL:
		c::sampleEditor::toNewChannel(channelId, a, b);
		break;
	}
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geWaveTools::geWaveTools(int x, int y, int w, int h, bool gridEnabled, int gridVal)
: Fl_Scroll(x, y, w, h, nullptr)
, m_data(nullptr)
{
	type(Fl_Scroll::HORIZONTAL_ALWAYS);
	hscrollbar.color(G_COLOR_GREY_2);
	hscrollbar.selection_color(G_COLOR_GREY_4);
	hscrollbar.labelcolor(G_COLOR_LIGHT_1);
	hscrollbar.slider(G_CUSTOM_BORDER_BOX);

	waveform = new v::geWaveform(x, y, w, h - 24, gridEnabled, gridVal);
}

/* -------------------------------------------------------------------------- */

void geWaveTools::rebuild(const c::sampleEditor::Data& d)
{
	m_data = &d;
	waveform->rebuild(d);
}

/* -------------------------------------------------------------------------- */

void geWaveTools::refresh()
{
	if (m_data->a_getPreviewStatus() == ChannelStatus::PLAY)
		waveform->redraw();
}

/* -------------------------------------------------------------------------- */

void geWaveTools::resize(int x, int y, int w, int h)
{
	Fl_Widget::resize(x, y, w, h);

	if (this->w() == w || (this->w() != w && this->h() != h))
	{ // vertical or both resize
		waveform->resize(x, y, waveform->w(), h - 24);
		waveform->rebuild(*m_data);
	}

	if (this->w() > waveform->w())
		waveform->stretchToWindow();

	int offset = waveform->x() + waveform->w() - this->w() - this->x();
	if (offset < 0)
		waveform->position(waveform->x() - offset, this->y());
}

/* -------------------------------------------------------------------------- */

int geWaveTools::handle(int e)
{
	switch (e)
	{
	case FL_MOUSEWHEEL:
	{
		waveform->setZoom(Fl::event_dy() == 1 ? geWaveform::Zoom::OUT : geWaveform::Zoom::IN);
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
		Fl::focus(waveform);
		return Fl_Group::handle(e);
	}
	default:
		return Fl_Group::handle(e);
	}
}

/* -------------------------------------------------------------------------- */

void geWaveTools::openMenu()
{
	Fl_Menu_Item menu[] = {
	    u::gui::makeMenuItem("Cut", menuCallback_, (void*)Menu::CUT),
	    u::gui::makeMenuItem("Copy", menuCallback_, (void*)Menu::COPY),
	    u::gui::makeMenuItem("Paste", menuCallback_, (void*)Menu::PASTE),
	    u::gui::makeMenuItem("Trim", menuCallback_, (void*)Menu::TRIM),
	    u::gui::makeMenuItem("Silence", menuCallback_, (void*)Menu::SILENCE),
	    u::gui::makeMenuItem("Reverse", menuCallback_, (void*)Menu::REVERSE),
	    u::gui::makeMenuItem("Normalize", menuCallback_, (void*)Menu::NORMALIZE),
	    u::gui::makeMenuItem("Fade in", menuCallback_, (void*)Menu::FADE_IN),
	    u::gui::makeMenuItem("Fade out", menuCallback_, (void*)Menu::FADE_OUT),
	    u::gui::makeMenuItem("Smooth edges", menuCallback_, (void*)Menu::SMOOTH_EDGES),
	    u::gui::makeMenuItem("Set begin/end here", menuCallback_, (void*)Menu::SET_BEGIN_END),
	    u::gui::makeMenuItem("Copy to new channel", menuCallback_, (void*)Menu::TO_NEW_CHANNEL),
	    {}};

	if (!waveform->isSelected())
	{
		menu[(int)Menu::CUT].deactivate();
		menu[(int)Menu::COPY].deactivate();
		menu[(int)Menu::TRIM].deactivate();
		menu[(int)Menu::SILENCE].deactivate();
		menu[(int)Menu::REVERSE].deactivate();
		menu[(int)Menu::NORMALIZE].deactivate();
		menu[(int)Menu::FADE_IN].deactivate();
		menu[(int)Menu::FADE_OUT].deactivate();
		menu[(int)Menu::SMOOTH_EDGES].deactivate();
		menu[(int)Menu::SET_BEGIN_END].deactivate();
		menu[(int)Menu::TO_NEW_CHANNEL].deactivate();
	}

	Fl_Menu_Button b(0, 0, 100, 50);
	b.box(G_CUSTOM_BORDER_BOX);
	b.textsize(G_GUI_FONT_SIZE_BASE);
	b.textcolor(G_COLOR_LIGHT_2);
	b.color(G_COLOR_GREY_2);

	const Fl_Menu_Item* m = menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, &b);
	if (m != nullptr)
		m->do_callback(this, m->user_data());

	return;
}

} // namespace v
} // namespace giada
