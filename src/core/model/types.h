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

#ifndef G_MODEL_TYPES_H
#define G_MODEL_TYPES_H

#include "core/model/document.h"
#include "deps/mcl-atomic-swapper/src/atomic-swapper.hpp"

namespace giada::m::model
{
/* DocumentLock
Alias for a REALTIME scoped lock provided by the Swapper class. Use this in the
real-time thread to lock the Document. */

using AtomicSwapper = mcl::AtomicSwapper<Document, /*size=*/6>;
using DocumentLock  = AtomicSwapper::RtLock;

/* SwapType
Type of Document change.
    Hard: the structure has changed (e.g. add a new channel);
    Soft: a property has changed (e.g. change volume);
    None: something has changed but we don't care.
Used by model listeners to determine the type of change that occurred in the
Document. */

enum class SwapType
{
	HARD,
	SOFT,
	NONE
};
} // namespace giada::m::model

#endif
