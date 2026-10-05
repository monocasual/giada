/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/core/model/document.h"

namespace giada::m::model
{
void Document::load(const Patch& patch, float sampleRateRatio)
{
}

/* -------------------------------------------------------------------------- */

void Document::load(const Conf& conf)
{
}

/* -------------------------------------------------------------------------- */

void Document::store(Patch& patch) const
{
}

/* -------------------------------------------------------------------------- */

void Document::store(Conf& conf) const
{
}

/* -------------------------------------------------------------------------- */

#if G_DEBUG_MODE
void Document::debug() const
{
}
#endif
} // namespace giada::m::model
