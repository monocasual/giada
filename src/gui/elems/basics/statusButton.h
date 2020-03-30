/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * geStatusButton
 * Simple geButton with a boolean 'status' parameter.
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


#ifndef GE_STATUS_BUTTON_H
#define GE_STATUS_BUTTON_H


#include "button.h"


class geStatusButton : public geButton
{
public:

	geStatusButton(int x, int y, int w, int h, const char** imgOff=nullptr,
    const char** imgOn=nullptr, const char** imgDisabled=nullptr, Fl_Color enabled = G_COLOR_GREY_4);

	void draw() override;

    bool getStatus() const;

    void setStatus(bool s);

private:

	bool m_status;
};


#endif
