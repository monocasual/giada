/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_modeBox
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

#ifndef GE_CHANNEL_MODE_H
#define GE_CHANNEL_MODE_H

#include <FL/Fl_Menu_Button.H>

namespace giada
{
namespace v
{
class geChannelMode : public Fl_Menu_Button
{
  public:
	geChannelMode(int x, int y, int w, int h, c::channel::Data& d);

	void draw() override;

  private:
	static void cb_changeMode(Fl_Widget* /*w*/, void* p);
	void        cb_changeMode(int mode);

	c::channel::Data& m_channel;
};
} // namespace v
} // namespace giada

#endif
