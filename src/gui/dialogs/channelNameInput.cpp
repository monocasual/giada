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


#include "glue/channel.h"
#include "utils/gui.h"
#include "core/channels/channel.h"
#include "core/model/model.h"
#include "core/const.h"
#include "core/conf.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/input.h"
#include "channelNameInput.h"



namespace giada {
namespace v 
{
gdChannelNameInput::gdChannelNameInput(ID channelId)
: gdWindow   (400, 64, "New channel name"),
  m_channelId(channelId)
{
	if (m::conf::nameX)
		resize(m::conf::nameX, m::conf::nameY, w(), h());

	set_modal();

	m_name   = new geInput(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN, w() - (G_GUI_OUTER_MARGIN * 2), G_GUI_UNIT);
	m_ok     = new geButton(w() - 70 - G_GUI_OUTER_MARGIN, m_name->y()+m_name->h() + G_GUI_OUTER_MARGIN, 70, G_GUI_UNIT, "Ok");
	m_cancel = new geButton(m_ok->x() - 70 - G_GUI_OUTER_MARGIN, m_ok->y(), 70, G_GUI_UNIT, "Cancel");
	end();

	m::model::onGet(m::model::channels, m_channelId, [&](m::Channel& c)
	{
		m_name->value(c.name.c_str());
	});

	m_ok->shortcut(FL_Enter);
	m_ok->callback(cb_update, (void*)this);

	m_cancel->callback(cb_cancel, (void*)this);

	u::gui::setFavicon(this);
	setId(WID_SAMPLE_NAME);
	show();
}


/* -------------------------------------------------------------------------- */


gdChannelNameInput::~gdChannelNameInput()
{
	m::conf::nameX = x();
	m::conf::nameY = y();
}


/* -------------------------------------------------------------------------- */


void gdChannelNameInput::cb_update(Fl_Widget* w, void* p) { ((gdChannelNameInput*)p)->cb_update(); }
void gdChannelNameInput::cb_cancel(Fl_Widget* w, void* p) { ((gdChannelNameInput*)p)->cb_cancel(); }


/* -------------------------------------------------------------------------- */


void gdChannelNameInput::cb_cancel()
{
	do_callback();
}


/* -------------------------------------------------------------------------- */


void gdChannelNameInput::cb_update()
{
	c::channel::setName(m_channelId, m_name->value());
	do_callback();
}

}} // giada::v::
