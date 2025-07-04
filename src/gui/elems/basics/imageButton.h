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

#ifndef GE_IMAGE_BUTTON_H
#define GE_IMAGE_BUTTON_H

#include "src/gui/elems/basics/button.h"
#include <FL/Fl_SVG_Image.H>
#include <memory>

namespace giada::v
{
class geImageButton : public geButton
{
public:
	geImageButton(int x, int y, int w, int h, const char* imgOff, const char* imgOn, const char* imgDisabled = nullptr);
	geImageButton(const char* imgOff, const char* imgOn, const char* imgDisabled = nullptr);

	void draw() override;

protected:
	void draw(Fl_SVG_Image*);

	std::unique_ptr<Fl_SVG_Image> m_imgOff;
	std::unique_ptr<Fl_SVG_Image> m_imgOn;
	std::unique_ptr<Fl_SVG_Image> m_imgDisabled;
};
} // namespace giada::v

#endif
