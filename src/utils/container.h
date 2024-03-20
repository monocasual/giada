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

namespace giada::u
{
// clang-format off
template <typename T>
concept HasId = requires(T t) {
	{ T::id };
};

template <typename T>
concept HasIndex = requires(T t) {
	{ T::index };
};
// clang-format on

template <typename T, bool Identifiable = false, bool Sortable = false>
class Container
{
	static_assert(!Identifiable || HasId<T>, "T must have a 'id' member if Identifiable is true");
	static_assert(!Sortable || HasIndex<T>, "T must have a 'index' member if Sortable is true");

public:
	const int size() const
	{
		return m_items.size();
	}

	/* contains
	Returns whether this container contains item with the given id. */

	const bool contains(ID id) const
	    requires Identifiable
	{
		return findById(id) != nullptr;
	}

	/* getById (1)
	Returns a const reference of item with given ID. Assumes that the item is
	present in the container (raises assertion otherwise). */

	const T& getById(ID id) const
	    requires Identifiable
	{
		const T* item = findById(id);
		assert(item != nullptr);
		return *item;
	}

	/* getById (2)
	Alternate version of getById() (1) that returns a non-const reference. */

	T& getById(ID id)
	    requires Identifiable
	{
		return const_cast<T&>(std::as_const(*this).getById(id));
	}

	/* getByIndex (1)
	Returns a const reference of item at index 'index'. */

	const T& getByIndex(std::size_t index) const
	{
		assert(index < m_items.size());
		if constexpr (Sortable)
			assert(static_cast<std::size_t>(m_items[index].index) == index); // Make sure indexing is right

		return m_items[index];
	}

	/* getByIndex (2)
	Alternate version of getByIndex() (1) that returns a non-const reference. */

	T& getByIndex(std::size_t index)
	{
		return const_cast<T&>(std::as_const(*this).getByIndex(index));
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

	/* findById (1)
	Finds an element with the given ID. Unlike getById(), the item might not be
	present. If so, returns nullptr. */

	const T* findById(ID id) const
	    requires Identifiable
	{
		for (const T& item : m_items)
			if (item.id == id)
				return &item;
		return nullptr;
	}

	/* findById (2)
	Alternate version of findById() (1) that returns a non-const reference. */

	T* findById(ID id)
	    requires Identifiable
	{
		return const_cast<T*>(std::as_const(*this).findById(id));
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
	    requires Identifiable
	{
		if constexpr (Sortable)
		{
			return getById(id).index;
		}
		else
		{
			for (int i = 0; const T& item : m_items)
			{
				if (item.id == id)
					return i;
				i++;
			}
			assert(false);
			return -1;
		}
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
	Adds a new item to this container by moving it, and returns a reference. */

	T& add(T&& item)
	{
		if constexpr (Identifiable)
			assert(isUniqueId(item.id));

		m_items.push_back(std::move(item));

		if constexpr (Sortable)
			rebuildIndexes();

		return m_items.back();
	}

	/* insert
	Inserts a new item to this container at the given index by moving it, and
	returns a reference. If the container is empty, index is ignored and the
	item will be added as first element. */

	T& insert(T&& item, std::size_t index)
	    requires Sortable
	{
		if constexpr (Identifiable)
			assert(isUniqueId(item.id));

		if (m_items.empty())
			return add(std::move(item));

		assert(index <= m_items.size());

		m_items.insert(m_items.begin() + index, std::move(item));
		rebuildIndexes();
		return m_items[index];
	}

	/* moveByIndex
	Moves the element at 'oldIndex' to 'newIndex'. 'newIndex' can be greater than
	the last element index. */

	void moveByIndex(std::size_t oldIndex, std::size_t newIndex)
	    requires Sortable
	{
		if (oldIndex == newIndex)
			return;

		assert(oldIndex < m_items.size());

		newIndex = std::min(newIndex, m_items.size() - 1);
		auto it  = m_items.begin() + oldIndex;

		if (oldIndex < newIndex) // Forward
			std::rotate(it, it + 1, m_items.begin() + newIndex + 1);
		else // Backward
			std::rotate(m_items.begin() + newIndex, it, it + 1);

		rebuildIndexes();
	}

	/* moveById
	Moves the element with the given id to 'newIndex'. */

	void moveById(ID id, std::size_t newIndex)
	    requires Identifiable && Sortable
	{
		moveByIndex(getById(id).index, newIndex);
	}

	void removeById(ID id)
	    requires Identifiable
	{
		const auto f = [id](const T& item)
		{ return item.id == id; };
		m_items.erase(std::remove_if(m_items.begin(), m_items.end(), f), m_items.end());

		if constexpr (Sortable)
			rebuildIndexes();
	}

	void removeByIndex(std::size_t index)
	{
		assert(index < m_items.size());

		m_items.erase(m_items.begin() + index);

		if constexpr (Sortable)
			rebuildIndexes();
	}

	void clear()
	{
		m_items.clear();
	}

private:
	bool isUniqueId(ID id) const
	    requires Identifiable
	{
		const auto f = [id](const T& item)
		{ return item.id == id; };
		return std::find_if(m_items.begin(), m_items.end(), f) == m_items.end();
	}

	void rebuildIndexes()
	    requires Sortable
	{
		for (std::size_t i = 0; T & item : m_items)
			item.index = i++;
	}

	std::vector<T> m_items;
};
} // namespace giada::u

#endif
