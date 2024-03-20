/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_CONTAINER_H
#define G_CONTAINER_H

#include "core/types.h"
#include "utils/vector.h"

namespace giada::u
{
template <typename T>
concept HasId = requires(T) { T::id; };

template <HasId T>
class Container
{
public:
	virtual ~Container() = default;

	const int size() const
	{
		return m_items.size();
	}

	/* get (1)
	Returns a const reference of item with given ID. Assumes that the item is
	present in the container (raises assertion otherwise). */

	virtual const T& get(ID id) const
	{
		const T* item = find(id);
		assert(item != nullptr);
		return *item;
	}

	/* get (2)
	Alternate version of get() (1) that returns a non-const reference. */

	virtual T& get(ID id)
	{
		return const_cast<T&>(std::as_const(*this).get(id));
	}

	/* getAll (1)
	Returns a const reference to the underlyling vector of items. */

	const std::vector<T>& getAll() const
	{
		return m_items;
	}

	/* getAll (2)
	Alternate version of getAll() (1) that returns a non-const reference. */

	std::vector<T>& getAll()
	{
		return m_items;
	}

	/* find (1)
	Finds an element with the given ID. Unlike get(), the item might not be 
	present. If so, returns nullptr. */

	const T* find(ID id) const
	{
		for (const T& item : m_items)
			if (item.id == id)
				return &item;
		return nullptr;
	}

	/* find (2)
	Alternate version of find() (1) that returns a non-const reference. */

	T* find(ID id)
	{
		return const_cast<T*>(std::as_const(*this).find(id));
	}

	/* anyOf
    Returns true if any item satisfies the callback 'f'. */

	bool anyOf(std::function<bool(const T&)> f) const
	{
		return std::any_of(m_items.begin(), m_items.end(), f);
	}

	/* getIndex
	Return the index of the item with the given ID. Assumes that the item is
	present in the container (raises assertion otherwise). */

	const int getIndex(ID id) const
	{
		int i = 0;
		for (const T& item : m_items)
		{
			if (item.id == id)
				return i;
			i++;
		}
		assert(false);
		return -1;
	}

	/* getLast (1)
	Return a const reference to the last item in the container. Raises an
	assertion if the container is empty. */

	const T& getLast() const
	{
		assert(m_items.size() > 0);
		return m_items.back();
	}

	/* getLast (2)
	Alternate version of getLast() (1) that returns a non-const reference. */

	T& getLast()
	{
		return const_cast<T&>(std::as_const(*this).getLast());
	}

	/* getIf
	Returns a vector of item pointers that satisfy the callback 'f'. */

	std::vector<T*> getIf(std::function<bool(const T&)> f)
	{
		std::vector<T*> out;
		for (T& item : m_items)
			if (f(item))
				out.push_back(&item);
		return out;
	}

	/* add
	Add a new item to this container by moving it. */

	void add(T&& item)
	{
		m_items.push_back(std::move(item));
	}

	/* insert
	Inserts a new item to this container at the given position by moving it. */

	void insert(T&& item, int position)
	{
		m_items.insert(m_items.begin() + position, std::move(item));
	}

	void removeById(ID id)
	{
		u::vector::removeIf(m_items, [id](const T& item) { return item.id == id; });
	}

	void removeByIndex(int index)
	{
		m_items.erase(m_items.begin() + index);
	}

private:
	std::vector<T> m_items;
};
} // namespace giada::u

#endif
