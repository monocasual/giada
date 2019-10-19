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
geColumn::geColumn(int X, int Y, int W, int H, ID id)
: Fl_Pack(X, Y, W, H), 
  id     (id)
{
	end();

	type(Fl_Pack::VERTICAL);
	spacing(G_GUI_INNER_MARGIN);

	init();
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


geChannel* geColumn::addChannel(ID channelId, ChannelType t, int size)
{
	geChannel* gch = nullptr;

	if (t == ChannelType::SAMPLE)
		gch = new geSampleChannel(0, 0, w(), size, channelId);
	else
		gch = new geMidiChannel(0, 0, w(), size, channelId);

	add(gch);
	gch->redraw();      // fix corruption
	return gch;
}


/* -------------------------------------------------------------------------- */


void geColumn::cb_addChannel()
{
	gu_log("[geColumn::cb_addChannel] id = %d\n", id);

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
		c::channel::addChannel(id, ChannelType::SAMPLE, G_GUI_CHANNEL_H_1);
	else
		c::channel::addChannel(id, ChannelType::MIDI, G_GUI_CHANNEL_H_1);
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


void geColumn::init()
{
	clear();

	m_addChannelBtn = new geButton(0, 0, w(), G_GUI_UNIT, "Add new channel");
	m_addChannelBtn->callback(cb_addChannel, (void*)this);

	add(m_addChannelBtn);
}


/* -------------------------------------------------------------------------- */


void geColumn::forEachChannel(std::function<void(geChannel* c)> f) const
{
	for (int i=1; i<children(); i++) // Skip "add channel" button
		f(static_cast<geChannel*>(child(i)));
}
}} // giada::v::
