/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#include <cstddef>
#include <algorithm>
#include <functional>
#include <vector>

namespace giada::u::vector
{
template <typename T, typename P>
std::size_t indexOf(const T& v, const P& p)
{
	return std::distance(std::cbegin(v), std::find(std::cbegin(v), std::cend(v), p));
}

/* -------------------------------------------------------------------------- */

template <typename T, typename F>
auto findIf(const T& v, F&& func)
{
	return std::find_if(std::cbegin(v), std::cend(v), func);
}

/* -------------------------------------------------------------------------- */

template <typename T, typename F>
bool has(const T& v, F&& func)
{
	return findIf(v, func) != std::cend(v);
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

/* -------------------------------------------------------------------------- */

template <typename Vector, typename Default>
auto atOr(const Vector& v, int index, Default d)
{
	return index >= 0 && static_cast<size_t>(index) < v.size() ? v[index] : d;
}
} // namespace giada::u::vector

#endif
