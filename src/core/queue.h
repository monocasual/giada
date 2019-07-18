#ifndef FIFO_H
#define FIFO_H


#include <array>
#include <atomic>


namespace giada {
namespace m
{
template<typename T, size_t size>
class Queue
{
public:

    Queue() : m_head(0), m_tail(0)
    {
    }


    Queue(const Queue&) = delete;


    bool pop(T& item)
    {
        size_t curr = m_head.load();
        if (curr == m_tail.load())  // Queue empty, nothing to do
            return false;

        item = m_data[curr];
        m_head.store(increment(curr));
        return true;
    }


    bool push(const T& item)
    {
        size_t curr = m_tail.load();
        size_t next = increment(curr);

        if (next == m_head.load()) // Queue full, nothing to do
            return false;

        m_data[curr] = item;
        m_tail.store(next);
        return true;
    }

private:

    size_t increment(size_t i) const
    {
        return (i + 1) % size;
    }


    std::array<T, size> m_data;
    std::atomic<size_t> m_head;
    std::atomic<size_t> m_tail;
};
}} // giada::m::


#endif
