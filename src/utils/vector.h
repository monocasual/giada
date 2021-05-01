/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
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

#ifndef G_UTILS_VECTOR_H
#define G_UTILS_VECTOR_H

#include <algorithm>
#include <functional>
#include <vector>

namespace giada::u::vector
{
template <typename T, typename P>
std::size_t indexOf(T& v, const P& p)
{
	return std::distance(v.begin(), std::find(v.begin(), v.end(), p));
}

/* -------------------------------------------------------------------------- */

template <typename T, typename F>
auto findIf(T& v, F&& func)
{
	return std::find_if(v.begin(), v.end(), func);
}

/* -------------------------------------------------------------------------- */

template <typename T, typename F>
bool has(T& v, F&& func)
{
	return findIf(v, func) != v.end();
}

/* -------------------------------------------------------------------------- */

template <typename T, typename F>
void removeIf(T& v, F&& func)
{
	v.erase(std::remove_if(v.begin(), v.end(), func), v.end());
}

template <typename T, typename V>
void remove(T& v, const V& o)
{
	v.erase(std::remove(v.begin(), v.end(), o), v.end());
}

/* -------------------------------------------------------------------------- */

template <typename T, typename I>
std::vector<T> cast(const I& i)
{
	return {i.begin(), i.end()};
}
} // namespace giada::u::vector

#endif
