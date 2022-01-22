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

#include "keyGrabber.h"
#include "config.h"
#include "core/conf.h"
#include "core/model/model.h"
#include "glue/io.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "gui/elems/mainWindow/keyboard/channelButton.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "mainWindow.h"
#include "utils/gui.h"
#include "utils/log.h"
#include "utils/string.h"
#include <cassert>

extern giada::v::gdMainWindow* mainWin;

namespace giada
{
namespace v
{
gdKeyGrabber::gdKeyGrabber(const c::channel::Data& d)
: gdWindow(300, 126, "Key configuration")
, m_data(d)
{
	begin();
	m_text   = new geBox(8, 8, 284, 80, "");
	m_clear  = new geButton(w() - 88, m_text->y() + m_text->h() + 8, 80, 20, "Clear");
	m_cancel = new geButton(m_clear->x() - 88, m_clear->y(), 80, 20, "Close");
	end();

	m_clear->callback(cb_clear, (void*)this);
	m_cancel->callback(cb_cancel, (void*)this);

	updateText(m_data.key);

	u::gui::setFavicon(this);
	set_modal();
	show();
}

/* -------------------------------------------------------------------------- */

void gdKeyGrabber::rebuild()
{
	updateText(m_data.key);
}

/* -------------------------------------------------------------------------- */

void gdKeyGrabber::cb_clear(Fl_Widget* /*w*/, void* p) { ((gdKeyGrabber*)p)->cb_clear(); }
void gdKeyGrabber::cb_cancel(Fl_Widget* /*w*/, void* p) { ((gdKeyGrabber*)p)->cb_cancel(); }

/* -------------------------------------------------------------------------- */

void gdKeyGrabber::cb_cancel()
{
	do_callback();
}

/* -------------------------------------------------------------------------- */

void gdKeyGrabber::cb_clear()
{
	updateText(0);
	setButtonLabel(0);
}

/* -------------------------------------------------------------------------- */

void gdKeyGrabber::setButtonLabel(int key)
{
	c::io::channel_setKey(m_data.id, key);
}

/* -------------------------------------------------------------------------- */

void gdKeyGrabber::updateText(int key)
{
	std::string tmp = "Press a key.\n\nCurrent binding: ";
	if (key != 0)
		tmp += static_cast<wchar_t>(key);
	else
		tmp += "[none]";
	m_text->copy_label(tmp.c_str());
}

/* -------------------------------------------------------------------------- */

int gdKeyGrabber::handle(int e)
{
	int ret = Fl_Group::handle(e);
	switch (e)
	{
	case FL_KEYUP:
	{
		int x = Fl::event_key();
		if (strlen(Fl::event_text()) != 0 && x != FL_BackSpace && x != FL_Enter && x != FL_Delete && x != FL_Tab && x != FL_End && x != ' ')
		{
			u::log::print("set key '%c' (%d) for channel ID=%d\n", x, x, m_data.id);
			setButtonLabel(x);
			updateText(x);
			break;
		}
		else
			u::log::print("invalid key\n");
	}
	}
	return (ret);
}

} // namespace v
} // namespace giada
