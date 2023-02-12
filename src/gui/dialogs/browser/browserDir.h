/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef GD_BROWSER_DIR_H
#define GD_BROWSER_DIR_H

#include "browserBase.h"
#include "core/conf.h"

namespace giada::v
{
class gdBrowserDir : public gdBrowserBase
{
public:
	gdBrowserDir(const std::string& title, const std::string& path,
	    std::function<void(void*)> cb, const m::Conf&);

private:
	static void cb_load(Fl_Widget* /*w*/, void* p);
	static void cb_down(Fl_Widget* /*w*/, void* p);
	void        cb_load();
	void        cb_down();
};
} // namespace giada::v

#endif
