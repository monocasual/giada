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


#ifndef G_RENDER_MODEL_H
#define G_RENDER_MODEL_H


#include <memory>
#include <atomic>
#include "core/model/data.h"


namespace giada {
namespace m {
namespace model
{
/* getLayout
Returns a pointer to the current layout. Use this when you want to read the 
layout model or modify a specific atomic parameter. */
  
std::shared_ptr<Layout> getLayout();

Data& getData();

/* cloneLayout
Returns a copy of the current layout. Call this when you want to modify the 
current render layout model. */

std::shared_ptr<Layout> cloneLayout();

/* swapLayout
Atomically swaps current layout with the new one provided. */

void swapLayout(std::shared_ptr<Layout> l);

/* changed
Marks if the model has changed and requires UI update. */

extern std::atomic<bool> changed;

#ifndef NDEBUG

void debug();

#endif
}}} // giada::m::model::


#endif
