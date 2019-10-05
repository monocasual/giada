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


#ifndef G_RCU_LIST_H
#define G_RCU_LIST_H


#include <cassert>
#include <thread>
#include <atomic>
#include <iterator>


namespace giada {
namespace m
{
template<typename T>
class RCUList
{
public:

	/* Lock
	Scoped lock structure. */

	struct Lock
	{
		Lock(RCUList<T>& r) : rcu(r) 
		{ 
			rcu.lock(); 
		}

		~Lock()	
		{ 
			rcu.unlock();
		}

		RCUList<T>& rcu;
	};

	/* Node
	Element of the linked list. */
	
	struct Node
	{
		std::unique_ptr<T> data;
		std::atomic<Node*> next;

		Node(std::unique_ptr<T> data, Node* next=nullptr)
		: data(std::move(data)), 
		  next(next) 
		{}
	};

	/* Iterator (const)
	This is based on simple, non-atomic pointers: you must always lock the RCU
	list before looping over it! */

	class Iterator : public std::iterator<std::forward_iterator_tag, Node*>
	{
	public:

		Iterator(Node* n) : m_curr(n) {}

		bool operator!= (const Iterator& o) const
		{
			return m_curr != o.m_curr;
		}

		bool operator== (const Iterator& o) const
		{
			return m_curr == o.m_curr;
		}

		const T* operator* () const
		{
			return m_curr->data.get();
		}

		T* operator* ()
		{
			return m_curr->data.get();
		}

		const Iterator& operator++ ()  // Prefix operator (++x)
		{
			if (m_curr != nullptr)
				m_curr = m_curr->next;
			return *this;
		}
	
	private:
	
		const Node* m_curr;
	};

	/* RCUList
	Singly linked list protected by a Read-Copy-Update (RCU) mechanism. */

	RCUList() 
		: m_grace  (0), 
		  m_size   (0), 
		  m_writing(false),
		  m_head   (nullptr),
		  m_tail   (nullptr)
	{
		m_readers[0].store(0);
		m_readers[1].store(0);
	}

	RCUList(std::unique_ptr<T> data) : RCUList()
	{
		push(std::move(data));
	}

	RCUList(const RCUList&) = delete;
	RCUList(RCUList&&)      = delete;

	~RCUList()
	{
		clear();
	}

	Iterator begin()
	{ 
		assert(m_readers[t_grace].load() > 0 && "Forgot lock before reading");
		return Iterator(m_head.load());
	}

	Iterator end()
	{ 
		assert(m_readers[t_grace].load() > 0 && "Forgot lock before reading");
		return Iterator(nullptr);
	}

	/* unlock
	Increases current readers count. Always call lock()/unlock() when reading
	data from the list. Or use the scoped version Lock above. */

	void lock()
	{
		t_grace = m_grace.load();
		m_readers[t_grace]++;
	}

	/* unlock
	Releases current readers count. */

	void unlock()
	{
		m_readers[t_grace]--;
	}

	/* get
	Returns a reference to the data held by node 'i'. */

	T* get(size_t i=0) const
	{
		assert(i < size() && "Index overflow");
		assert(m_readers[t_grace].load() > 0 && "Forgot lock before reading");
		return getNode(i)->data.get();
	}

	/* Subscript operator []
	Same as above for the [] syntax. */

	T* operator[] (size_t i) const
	{
    	return get(i);
    }

	/* back
	Return data held by the last node. */

	T* back() const
	{
		assert(m_readers[t_grace].load() > 0 && "Forgot lock before reading");
		return m_tail.load()->data.get();
	}

	/* clone
	Returns a new copy of the data held by node 'i'. The template machinery
	is required for when you declare a RCUList<Base> and later on want to clone
	a derived object. Usage:
	
	RCUList<Base> list;
	...
	std::unique_ptr<Derived> d = list.clone<Derived>(i);
	*/

	template<typename C=T>
	std::unique_ptr<C> clone(size_t i=0) const
    {
		return std::make_unique<C>(*static_cast<C*>(getNode(i)->data.get()));
    }

	/* swap
	Exchanges data contained in node 'i' with new data 'data'. New data must
	always come from a call to clone(). There is a natural protection against 
	multiple calls to swap() made by the same thread: the caller is blocked by 
	the spinlock below: no progress is made until m_readers[oldgrace] > 0. */

	void swap(std::unique_ptr<T> data, size_t i=0)
	{
		/* Never start two overlapping writing sessions. */

		if (m_writing.load() == true)
			return;
		
		/* Begin of writing session. */

		m_writing.store(true);

		/* Flip the current grace bit. Now we have entered a new grace period
		with a different number from the previous one. */

		std::int8_t oldgrace = m_grace.fetch_xor(1);

		/* Prepare useful node pointers: current, next and previous. Fetching
		from the current list with getNode() is safe here: we are just reading. */

		Node* curr = getNode(i);
		Node* prev = curr == m_head.load() ? nullptr : getNode(i - 1);
		Node* next = curr == m_tail.load() ? nullptr : getNode(i + 1);
		
		/* Prepare a new node holding the new data in input. */

		Node* n = new Node(std::move(data), next);
	
		/* Make the previous node point to the new one just created. New
		readers will read the new one from now on. The only write operation 
		performed here is the atomic store. */

		if (prev != nullptr)
			prev->next.store(n);
		else
			m_head.store(n);

		if (next == nullptr)
			m_tail.store(n);

		/* Wait until no readers from the previous grace period are reading the 
		list. Avoid brutal spinlock with a tiny sleep. */

		while (m_readers[oldgrace] > 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(50));

		/* Delete old node. Node destructor makes sure data is deleted. */

		delete curr;

		/* End of writing session. */

		m_writing.store(false);
		changed.store(true);
	}

	/* push
	Adds a new element to the list containing 'data'. */

	void push(std::unique_ptr<T> data)
	{
		/* Never start two overlapping writing sessions. */

		if (m_writing.load() == true)
			return;

		/* Begin of writing session. */

		m_writing.store(true);	
		
		/* Create new node. */

		Node* n = new Node(std::move(data));
		
		/* Update the current tail->next pointer to this node, if a tail exists.
		I.e., grab the current last node and append it the new one. */

		if (m_tail.load() != nullptr)
			m_tail.load()->next.store(n);
		
		/* Update tail pointer to point to the new node. */

		m_tail.store(n);

		/* Head is null when the list is empty. If so, set head to this new
		node too. A list with only one node has both head and tail pointing
		to the same node. */

		if (m_head.load() == nullptr)
			m_head.store(n);

		/* Upgrade static size. Last thing to do, so that other threads won't
		read a false size. */

		m_size++;

		/* End of writing session. Data has changed, set the flag. */

		m_writing.store(false);
		changed.store(true);
	}

	/* pop
	Removes the i-th element. There is a natural protection against multiple
	calls to pop() made by the same thread: the caller is blocked by the
	spinlock below: no progress is made while m_readers[oldgrace] > 0. */

	void pop(size_t i)
	{
		/* Never start two overlapping writing sessions. */

		if (m_writing.load() == true)
			return;

		/* Begin of writing session. */

		m_writing.store(true);

		/* Flip the current grace bit. Now we have entered a new grace period
		with a different number from the previous one. */

		std::int8_t oldgrace = m_grace.fetch_xor(1);
		
		/* Prepare useful node pointers: current, next and previous. Fetching
		from the current list with getNode() is safe here: we are just reading. */

		Node* curr = getNode(i);
		Node* prev = curr == m_head.load() ? nullptr : getNode(i - 1);
		Node* next = curr == m_tail.load() ? nullptr : getNode(i + 1);
		
		/* Disconnect the previous node from the current one to be deleted:
		prev->curr->next becomes prev->next, with (curr) pending in the void 
		for the remaining readers. Special care is needed if we are removing an 
		edge node. */

		if (prev != nullptr)
			prev->next.store(next);
		else 
			m_head.store(next);

		if (next == nullptr)
			m_tail.store(prev);

		/* Size can be updated at this point. */

		m_size--;
		
		/* Wait until no readers from the previous grace period are reading the 
		list. Avoid brutal spinlock with a tiny sleep. */

		while (m_readers[oldgrace] > 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(50));

		/* Delete old node. Node destructor makes sure data is deleted. */

		delete curr;
		
		/* End of writing session. Data has changed, set the flag. */

		m_writing.store(false);
		changed.store(true);
	}

	/* clear
	Removes all nodes. */

	void clear()
	{
		/* Never start two overlapping writing sessions. */

		if (m_writing.load() == true)
			return;

		/* Begin of writing session. */

		m_writing.store(true);

		/* Flip the current grace bit. Now we have entered a new grace period
		with a different number from the previous one. */

		std::int8_t oldgrace = m_grace.fetch_xor(1);
	
		/* Store the first node locally. We will need it later on. */

		Node* current = m_head.load();

		/* Block any other reader by setting the size to 0 in advance and
		cleaning up head/tail pointers. */

		m_size.store(0);
		m_head.store(nullptr);
		m_tail.store(nullptr);

		while (current != nullptr) {

			/* Wait until no readers from the previous grace period are reading the 
			list. Avoid brutal spinlock with a tiny sleep. */

			while (m_readers[oldgrace] > 0)
				std::this_thread::sleep_for(std::chrono::milliseconds(50));

			/* Delete old node. Node destructor makes sure data is deleted. */

    		Node* next = current->next;
    		delete current;
    		current = next;
		}

		/* End of writing session. Data has changed, set the flag. */

		m_writing.store(false);
		changed.store(true);
	}

	/* size
	Returns the number of nodes in the list. */

	size_t size() const
	{
		return m_size.load();
	}

	/* changed
	Tells whether the list has been altered with a swap, a push or a pop. */

	std::atomic<bool> changed;
	
	/* value_type
	A variable that holds the type of data contained in the list. Used for
	metaprogramming stuff. */

	using value_type = T;

private:

	Node* getNode(size_t i) const
	{
		size_t p    = 0;
		Node*  curr = m_head.load();
		
		while (curr != nullptr && p < i) {
			p++;
			curr = curr->next.load();
		}
		assert(curr != nullptr);
		return curr;
	}

	std::array<std::atomic<int>, 2> m_readers;
	std::atomic<std::int8_t>        m_grace;
	std::atomic<size_t>             m_size;
	std::atomic<bool>               m_writing;

	/* m_head
	Pointer to the first node. Used when reading and writing: always update
	this one first, as it is read by reader threads. */

	std::atomic<Node*> m_head;

	/* m_tail
	Pointer to the last node. Used only when reading: unlike m_head, updating it
	the right time is not that critical. */

	std::atomic<Node*> m_tail;

	/* t_grace
	Current grace flag. Each thread has its own copy of it (thread_local). */

	thread_local static int t_grace;
};


template<typename T>
thread_local int RCUList<T>::t_grace = 0;
}} // giada::m::


#endif
