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


#ifndef G_UTILS_VECTOR_H
#define G_UTILS_VECTOR_H


#include <vector>
#include <algorithm>
#include <functional>


namespace giada {
namespace u {
namespace vector 
{
template <typename T, typename F>
int indexOf(T& v, F&& func)
{
    static_assert(std::is_same<T, std::vector<typename T::value_type>>::value);

	auto it = std::find_if(v.begin(), v.end(), func);
	return it != v.end() ? std::distance(v.begin(), it) : -1;
}


/* -------------------------------------------------------------------------- */


template <typename T, typename F>
void removeIf(T& v, F&& func)
{
    static_assert(std::is_same<T, std::vector<typename T::value_type>>::value);

    v.erase(std::remove_if(v.begin(), v.end(), func), v.end());
}
}}};  // giada::u::vector::

#endif