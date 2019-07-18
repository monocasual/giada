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
 * will be useful, but WITHOUT ANY WARRANTY without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------- */


#ifndef G_RCU_H
#define G_RCU_H


#include <atomic>


namespace giada {
namespace m
{
template<typename T>
class RCU
{
public:

	struct Lock
	{
		Lock(RCU<T>& r) : rcu(r) 
		{
			rcu.lock(); 
		}

		~Lock()
		{ 
			rcu.unlock();
		}

		RCU<T>& rcu;
	};

	RCU()
	: curr   (new T),
	  prev   (nullptr),
	  readers(0),
	  changed(false)
	{
	}

	~RCU()
	{
		delete curr;
		delete prev;
	}

	void lock()
	{
		readers++;
	}

	void unlock()
	{
		readers--;
	}

	const T* get() const
	{
		return curr.load();
	}

	T* getw()
	{
		return curr.load();
	}
	
	T* clone() const
	{
		return new T(*curr.load());
	}

	bool swap(T* t)
	{
		/* If prev is full, a swap is still pending. Clean up memory and
		abort the operation. */

		if (prev.load() != nullptr) {
			delete t;
			return false;
		}

		/* Store the current pointer into prev. Readers that are still reading
		will fetch this from now on. */

		prev.store(curr);

		/* Curr contains the new proposed value. No one is reading from this
		right now. */

		curr.store(t);

		/* Spin until there are no more readers to *prev. When so, free the
		memory it points to and say goodbye. */

		while (readers.load() > 0);
		
		T* old = prev.load();
		prev.store(nullptr);
		delete old;

		changed.store(true);

		return true;
	}

	std::atomic<bool> changed;

private:

	std::atomic<T*>   curr;
	std::atomic<T*>   prev;
	std::atomic<int>  readers;
};
}} // giada::m::

#endif
