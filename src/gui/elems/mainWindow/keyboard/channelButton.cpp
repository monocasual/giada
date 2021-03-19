/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#include "channelButton.h"
#include "core/channels/channel.h"
#include "core/const.h"
#include "core/model/model.h"
#include "core/recorder.h"
#include "glue/channel.h"
#include "utils/string.h"
#include <FL/fl_draw.H>

namespace giada
{
namespace v
{
geChannelButton::geChannelButton(int x, int y, int w, int h, const c::channel::Data& d)
: geButton(x, y, w, h)
, m_channel(d)
{
}

/* -------------------------------------------------------------------------- */

void geChannelButton::refresh()
{
	switch (m_channel.getPlayStatus())
	{
	case ChannelStatus::OFF:
	case ChannelStatus::EMPTY:
		setDefaultMode();
		break;
	case ChannelStatus::PLAY:
		setPlayMode();
		break;
	case ChannelStatus::ENDING:
		setEndingMode();
		break;
	default:
		break;
	}
	switch (m_channel.getRecStatus())
	{
	case ChannelStatus::ENDING:
		setEndingMode();
		break;
	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

void geChannelButton::draw()
{
	geButton::draw();

	if (m_channel.key == 0)
		return;

	/* draw background */

	fl_rectf(x() + 1, y() + 1, 18, h() - 2, bgColor0);

	/* draw m_key */

	fl_color(G_COLOR_LIGHT_2);
	fl_font(FL_HELVETICA, 11);
	fl_draw(std::string(1, static_cast<wchar_t>(m_channel.key)).c_str(), x(), y(), 18, h(), FL_ALIGN_CENTER);
}

/* -------------------------------------------------------------------------- */

void geChannelButton::setInputRecordMode()
{
	bgColor0 = G_COLOR_RED;
}

/* -------------------------------------------------------------------------- */

void geChannelButton::setActionRecordMode()
{
	bgColor0 = G_COLOR_BLUE;
	txtColor = G_COLOR_LIGHT_2;
}

/* -------------------------------------------------------------------------- */

void geChannelButton::setDefaultMode(const char* l)
{
	bgColor0 = G_COLOR_GREY_2;
	bdColor  = G_COLOR_GREY_4;
	txtColor = G_COLOR_LIGHT_2;
	if (l)
		label(l);
}

/* -------------------------------------------------------------------------- */

void geChannelButton::setPlayMode()
{
	bgColor0 = G_COLOR_LIGHT_1;
	bdColor  = G_COLOR_LIGHT_1;
	txtColor = G_COLOR_GREY_1;
}

/* -------------------------------------------------------------------------- */

void geChannelButton::setEndingMode()
{
	bgColor0 = G_COLOR_GREY_4;
}
} // namespace v
} // namespace giada
