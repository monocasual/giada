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

#include "gui/elems/basics/imageButton.h"
#include "core/const.h"
#include "gui/drawing.h"

namespace giada::v
{
geImageButton::geImageButton(int x, int y, int w, int h, const char* imgOff, const char* imgOn, const char* imgDisabled)
: geButton(x, y, w, h, nullptr)
, m_imgOff(imgOff != nullptr ? std::make_unique<Fl_SVG_Image>(nullptr, imgOff) : nullptr)
, m_imgOn(imgOn != nullptr ? std::make_unique<Fl_SVG_Image>(nullptr, imgOn) : nullptr)
, m_imgDisabled(imgDisabled != nullptr ? std::make_unique<Fl_SVG_Image>(nullptr, imgDisabled) : nullptr)
{
}

/* -------------------------------------------------------------------------- */

geImageButton::geImageButton(const char* imgOff, const char* imgOn, const char* imgDisabled)
: geImageButton(0, 0, 0, 0, imgOff, imgOn, imgDisabled)
{
}

/* -------------------------------------------------------------------------- */

void geImageButton::draw()
{
	if (active())
		draw(getValue() ? m_imgOn.get() : m_imgOff.get());
	else
		draw(m_imgDisabled.get());
}

/* -------------------------------------------------------------------------- */

void geImageButton::draw(Fl_SVG_Image* img)
{
	const geompp::Rect<int> bounds = getBounds();

	drawRect(bounds, G_COLOR_GREY_4); // draw border

	if (img != nullptr)
		drawImage(*img, bounds.reduced(1));
}
} // namespace giada::v