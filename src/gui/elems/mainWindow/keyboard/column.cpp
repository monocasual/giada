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


#include <cassert>
#include <FL/fl_draw.H>
#include <FL/Fl_Menu_Button.H>
#include "core/channels/sampleChannel.h"
#include "core/channels/midiChannel.h"
#include "glue/channel.h"
#include "utils/log.h"
#include "utils/fs.h"
#include "utils/string.h"
#include "gui/dialogs/warnings.h"
#include "gui/elems/basics/boxtypes.h"
#include "keyboard.h"
#include "sampleChannel.h"
#include "midiChannel.h"
#include "column.h"


namespace giada {
namespace v
{
geColumn::geColumn(int X, int Y, int W, int H, int index)
: Fl_Pack(X, Y, W, H), 
  m_index(index)
{
	type(Fl_Pack::VERTICAL);
	spacing(G_GUI_INNER_MARGIN);

	m_addChannelBtn = new geButton(0, 0, w(), G_GUI_UNIT, "Add new channel");
	m_addChannelBtn->callback(cb_addChannel, (void*)this);

	end();
}


/* -------------------------------------------------------------------------- */


int geColumn::handle(int e)
{
	switch (e) {
		case FL_RELEASE: {
			if (Fl::event_button() == FL_RIGHT_MOUSE) {
				cb_addChannel();
				return 1;
			}
		}
		case FL_DND_ENTER:           	// return(1) for these events to 'accept' dnd
		case FL_DND_DRAG:
		case FL_DND_RELEASE: {
			return 1;
		}
		case FL_PASTE: {              // handle actual drop (paste) operation
			/* TODO - add only the first element for now. Need new c::channel::
			function that takes a list of paths in input...*/
			std::vector<std::string> paths = u::string::split(Fl::event_text(), "\n");
			c::channel::addAndLoadChannel(m_index, G_GUI_CHANNEL_H_1, gu_stripFileUrl(paths[0])); 
			return 1;
		}
	}

	/* Return fl_Group::handle only if none of the cases above are fired. That
	is because you don't want to propagate a dnd drop to all the sub widgets. */

	return Fl_Group::handle(e);
}


/* -------------------------------------------------------------------------- */


void geColumn::refresh()
{
	for (int i=1; i<children(); i++) {  // Skip "add channel" button
		geChannel* c = dynamic_cast<geChannel*>(child(i));
		if (c != nullptr)
			c->refresh();
	}
}


/* -------------------------------------------------------------------------- */


void geColumn::cb_addChannel(Fl_Widget* v, void* p) { ((geColumn*)p)->cb_addChannel(); }


/* -------------------------------------------------------------------------- */


void geColumn::computeHeight()
{
	int totalH = 0;
	for (int i=0; i<children(); i++)
		totalH += child(i)->h() + G_GUI_INNER_MARGIN;
	resize(x(), y(), w(), totalH + 66); // evil space for drag n drop
}


/* -------------------------------------------------------------------------- */


geChannel* geColumn::addChannel(ID channelId, ChannelType t, int size)
{
	geChannel* gch = nullptr;

	if (t == ChannelType::SAMPLE)
		gch = new geSampleChannel(0, 0, w(), size, channelId);
	else
		gch = new geMidiChannel(0, 0, w(), size, channelId);

	add(gch);
	computeHeight();
	gch->redraw();      // fix corruption
	return gch;
}


/* -------------------------------------------------------------------------- */


void geColumn::cb_addChannel()
{
	gu_log("[geColumn::cb_addChannel] m_index = %d\n", m_index);

	Fl_Menu_Item rclick_menu[] = {
		{"Sample channel"},
		{"MIDI channel"},
		{0}
	};

	Fl_Menu_Button b(0, 0, 100, 50);
	b.box(G_CUSTOM_BORDER_BOX);
	b.textsize(G_GUI_FONT_SIZE_BASE);
	b.textcolor(G_COLOR_LIGHT_2);
	b.color(G_COLOR_GREY_2);

	const Fl_Menu_Item* m = rclick_menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, &b);
	if (m == nullptr) return;

	if (strcmp(m->label(), "Sample channel") == 0)
		c::channel::addChannel(m_index, ChannelType::SAMPLE, G_GUI_CHANNEL_H_1);
	else
		c::channel::addChannel(m_index, ChannelType::MIDI, G_GUI_CHANNEL_H_1);
}


/* -------------------------------------------------------------------------- */


geChannel* geColumn::getChannel(ID chanID) const
{
	for (int i=1; i<children(); i++) { // Skip "add channel" button
		geChannel* gch = static_cast<geChannel*>(child(i));
		if (gch->channelId == chanID)
			return gch;
	}
	return nullptr;
}


/* -------------------------------------------------------------------------- */


void geColumn::forEachChannel(std::function<void(geChannel* c)> f) const
{
	for (int i=1; i<children(); i++) // Skip "add channel" button
		f(static_cast<geChannel*>(child(i)));
}

/* -------------------------------------------------------------------------- */


int geColumn::getIndex() const { return m_index; }

}} // giada::v::
