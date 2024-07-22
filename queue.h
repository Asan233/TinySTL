#ifndef __QUEUE_H
#define __QUEUE_H

#include "deque.h"
template <typename T, typename Sequence = deque<T> >
class queue
{
    friend bool operator== (const queue& x, const queue& y);
    friend bool operator< (const queue& x, const queue& y);

public:
    typedef typename Sequence::value_type value_type;
    typedef typename Sequence::size_type size_type;
    typedef typename Sequence::reference reference;
    typedef typename Sequence::const_reference const_reference;

protected:
    Sequence c;

public:
    bool empty() const { return c.empty(); }
    size_type size() const { return c.size(); }
    reference front() { return c.front(); }
    const_reference front() const { return c.front(); }
    reference back() { return c.back(); }
    const_reference back() const { return c.back(); }
    // queue头出脚进
    void push(const value_type& x) { c.push_back(x); }
    void pop() { c.pop_front(); }
};

template <typename T, typename Sequence>
bool operator== (const queue<T, Sequence>& x, const queue<T, Sequence>& y)
{
    return x.c == y.c;
}

template <typename T, typename Sequence>
bool operator< (const queue<T, Sequence>& x, const queue<T, Sequence>& y)
{
    return x.c < y.c;
}
#endif