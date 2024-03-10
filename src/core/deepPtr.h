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

#ifndef G_DEEP_PTR_H
#define G_DEEP_PTR_H

#include <memory>

namespace giada::m
{
template <typename T>
class DeepPtr
{
public:
	/* swap for copy-and-swap idiom. */
	friend void swap(DeepPtr& first, DeepPtr& second)
	{
		using std::swap; // enable ADL (not necessary here, but good practice)
		swap(first.m_ptr, second.m_ptr);
	}

	/* Constructor (empty). You can construct it with an explicit nullptr parameter, if 
	needed, like so: DeepPtr ptr(nullptr). Or simply DeepPtr ptr; */
	DeepPtr(std::nullptr_t value = nullptr)
	: m_ptr(value)
	{
	}

	/* Constructor (with value). This pointer will own 'value', e.g.: DeepPtr ptr(new Foo);.
	Use makeDeep() below to avoid raw 'new' calls. */
	DeepPtr(T* value)
	: m_ptr(value)
	{
	}

	/* Copy constructor. */
	DeepPtr(const DeepPtr& other)
	: m_ptr(nullptr)
	{
		if (other)
			m_ptr = std::make_unique<T>(*other.m_ptr);
	}

	/* Move constructor. */
	DeepPtr(DeepPtr&& other) noexcept
	: DeepPtr()
	{
		swap(*this, other);
	}

	~DeepPtr() = default;

	/* Copy assignment operator. */
	DeepPtr& operator=(DeepPtr other)
	{
		swap(*this, other);
		return *this;
	}

	/* Move assignment operator. */
	DeepPtr& operator=(DeepPtr&& other) noexcept
	{
		m_ptr       = std::move(other.m_ptr);
		other.m_ptr = nullptr;
		return *this;
	}

	T&             operator*() { return *m_ptr; }
	const T&       operator*() const { return *m_ptr; }
	T* const       operator->() { return m_ptr.operator->(); }
	const T* const operator->() const { return m_ptr.operator->(); }
	explicit       operator bool() const { return m_ptr != nullptr; }

	const T* const get() const { return m_ptr.get(); }

private:
	std::unique_ptr<T> m_ptr;
};

template <typename T, typename... Args>
DeepPtr<T> makeDeep(Args&&... args)
{
	return DeepPtr<T>(new T(std::forward<Args>(args)...));
}
} // namespace giada::m

#endif
