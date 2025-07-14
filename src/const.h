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
 * will be useful, but WITHOUT ANY WARRANTY without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------- */

#ifndef G_CONST_H
#define G_CONST_H

#include "src/version.h"

namespace giada
{
/* -- environment ----------------------------------------------------------- */
#if defined(_WIN32)
#define G_OS_WINDOWS
#elif defined(__APPLE__)
#define G_OS_MAC
#elif defined(__linux__)
#define G_OS_LINUX
#elif defined(__FreeBSD__)
#define G_OS_FREEBSD
#endif

#ifndef NDEBUG
#define G_DEBUG_MODE
#endif

#ifndef BUILD_DATE
#define BUILD_DATE __DATE__
#endif

/* -- version --------------------------------------------------------------- */
constexpr auto    G_APP_NAME = "Giada";
constexpr Version G_VERSION(1, 3, 0);
} // namespace giada

#endif
