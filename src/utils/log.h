/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * log
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


#ifndef G_UTILS_LOG_H
#define G_UTILS_LOG_H

#include <cstdio>
#include <string>
#include <type_traits>
#include <utility>

#include "utils/fs.h"
#include "core/const.h"

namespace giada::u {

struct log {
  static inline FILE* f;
  static inline int   mode;
  static inline bool  stat;

  /* init
  Initializes logger. Mode defines where to write the output: LOG_MODE_STDOUT,
  LOG_MODE_FILE and LOG_MODE_MUTE. */
  static int init(int mode);

  static void close();

  // Use forwarding references && to avoid useless string copy
  static constexpr auto string_to_c_str = [] (auto&& s) {
    // Remove any reference and const-ness, since the function can
    // handle l-value and r-value, const or not.
    // Use std::remove_cvref instead, when sitching to C++20...
    if constexpr (std::is_same_v<std::remove_const_t<std::remove_reference_t<
                                   decltype(s)>>,
                                 std::string>)
      // If the argument is a std::string return an old-style C-string
      return s.c_str();
    else
      // Return the argument unchanged otherwise
      return s;
  };

  /** A variadic printf-like logging fonction

      Any `std::string` argument will be automatically transformed
      into a C-string.

      Use forwarding references to avoid useless string copy. */
  template <typename... Args>
  static void print(const char* format, Args&&... args) {
    if (mode == LOG_MODE_MUTE)
      return;

    if (mode == LOG_MODE_FILE && stat == true) {
      // Replace any std::string in the arguments by its C-string
      std::fprintf(f, format, string_to_c_str(std::forward<Args>(args))...);
#ifdef _WIN32
      fflush(f);
#endif
    }
    else
      std::printf(format, string_to_c_str(std::forward<Args>(args))...);
  }

};

}  // giada::u

#endif
