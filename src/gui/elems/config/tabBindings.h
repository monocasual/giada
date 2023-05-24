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

#ifndef GE_CONFIG_TAB_BINDINGS_H
#define GE_CONFIG_TAB_BINDINGS_H

#include "deps/geompp/src/rect.hpp"
#include "gui/model.h"
#include <FL/Fl_Group.H>

namespace giada::v
{
class geCheck;
class geInput;
class geKeyBinder;
class geTabBindings : public Fl_Group
{
public:
	geTabBindings(geompp::Rect<int>, const Model&);

private:
	geKeyBinder* play;
	geKeyBinder* rewind;
	geKeyBinder* recordActions;
	geKeyBinder* recordInput;
	geKeyBinder* exit;
};
} // namespace giada::v

#endif
