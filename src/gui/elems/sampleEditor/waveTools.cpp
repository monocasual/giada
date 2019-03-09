/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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


#include <cstdint>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Menu_Button.H>
#include "core/channels/sampleChannel.h"
#include "core/waveFx.h"
#include "glue/sampleEditor.h"
#include "core/const.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/dialogs/sampleEditor.h"
#include "waveform.h"
#include "waveTools.h"


namespace giada {
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
	const geWaveTools*      wavetools = static_cast<geWaveTools*>(w);
	const m::SampleChannel* ch        = static_cast<gdSampleEditor*>(wavetools->window())->ch;

	Menu selectedItem = (Menu) (intptr_t) v;

	int a = wavetools->waveform->getSelectionA();
	int b = wavetools->waveform->getSelectionB();

	switch (selectedItem) {
		case Menu::CUT:
			c::sampleEditor::cut(ch->id, a, b);
			break;		
		case Menu::COPY:
			c::sampleEditor::copy(ch->id, a, b);
			break;		
		case Menu::PASTE:
			c::sampleEditor::paste(ch->id, a);
			break;
		case Menu::TRIM:
			c::sampleEditor::trim(ch->id, a, b);
			break;
		case Menu::SILENCE:
			c::sampleEditor::silence(ch->id, a, b);
			break;	  
		case Menu::REVERSE:
			c::sampleEditor::reverse(ch->id, a, b);
			break;			
		case Menu::NORMALIZE:
			c::sampleEditor::normalizeHard(ch->id, a, b);
			break;	
		case Menu::FADE_IN:
			c::sampleEditor::fade(ch->id, a, b, m::wfx::FADE_IN);
			break;
		case Menu::FADE_OUT:
			c::sampleEditor::fade(ch->id, a, b, m::wfx::FADE_OUT);
			break;
		case Menu::SMOOTH_EDGES:
			c::sampleEditor::smoothEdges(ch->id, a, b);
			break;
		case Menu::SET_BEGIN_END:
			c::sampleEditor::setBeginEnd(ch->id, a, b);
			break;		
		case Menu::TO_NEW_CHANNEL:
			c::sampleEditor::toNewChannel(ch->id, a, b);
			break;
	}
}
}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


geWaveTools::geWaveTools(int x, int y, int w, int h)
: Fl_Scroll(x, y, w, h, nullptr)
{
	type(Fl_Scroll::HORIZONTAL_ALWAYS);
	hscrollbar.color(G_COLOR_GREY_2);
	hscrollbar.selection_color(G_COLOR_GREY_4);
	hscrollbar.labelcolor(G_COLOR_LIGHT_1);
	hscrollbar.slider(G_CUSTOM_BORDER_BOX);

	waveform = new v::geWaveform(x, y, w, h-24);
}



/* -------------------------------------------------------------------------- */


void geWaveTools::rebuild()
{
	waveform->rebuild();
}


/* -------------------------------------------------------------------------- */


void geWaveTools::refresh()
{
	const m::SampleChannel* ch = static_cast<gdSampleEditor*>(window())->ch;

	if (ch->isPreview())
		waveform->redraw();
}


/* -------------------------------------------------------------------------- */


void geWaveTools::resize(int x, int y, int w, int h)
{
	Fl_Widget::resize(x, y, w, h);

	if (this->w() == w || (this->w() != w && this->h() != h)) {   // vertical or both resize
		waveform->resize(x, y, waveform->w(), h-24);
		waveform->rebuild();
	}

	if (this->w() > waveform->w())
		waveform->stretchToWindow();

	int offset = waveform->x() + waveform->w() - this->w() - this->x();
	if (offset < 0)
		waveform->position(waveform->x()-offset, this->y());
}


/* -------------------------------------------------------------------------- */


int geWaveTools::handle(int e)
{
	switch (e) {
		case FL_MOUSEWHEEL: {
			waveform->setZoom(Fl::event_dy() == 1 ? geWaveform::Zoom::OUT : geWaveform::Zoom::IN);
			redraw();
			return 1;
		}
		case FL_PUSH: {
			if (Fl::event_button3()) {  // right button
				openMenu();
				return 1;
			}
			Fl::focus(waveform);
		}
		default:
			return Fl_Group::handle(e);
	}
}


/* -------------------------------------------------------------------------- */


void geWaveTools::openMenu()
{
	Fl_Menu_Item menu[] = {
		{"Cut",                 0, menuCallback_, (void*) Menu::CUT},
		{"Copy",                0, menuCallback_, (void*) Menu::COPY},
		{"Paste",               0, menuCallback_, (void*) Menu::PASTE},
		{"Trim",                0, menuCallback_, (void*) Menu::TRIM},
		{"Silence",             0, menuCallback_, (void*) Menu::SILENCE},
		{"Reverse",             0, menuCallback_, (void*) Menu::REVERSE},
		{"Normalize",           0, menuCallback_, (void*) Menu::NORMALIZE},
		{"Fade in",             0, menuCallback_, (void*) Menu::FADE_IN},
		{"Fade out",            0, menuCallback_, (void*) Menu::FADE_OUT},
		{"Smooth edges",        0, menuCallback_, (void*) Menu::SMOOTH_EDGES},
		{"Set begin/end here",  0, menuCallback_, (void*) Menu::SET_BEGIN_END},
		{"Copy to new channel", 0, menuCallback_, (void*) Menu::TO_NEW_CHANNEL},
		{0}
	};

	if (!waveform->isSelected()) {
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

}} // giada::v::