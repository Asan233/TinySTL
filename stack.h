#ifndef __STL_STACK_H
#define __STL_STACK_H

#include "deque.h"

/**
 *  STL stack栈配接器(adapter)，其规则为先进后出
 *  stack底层容器使用deque，对deque的进行上一层包装，使其接口规则符合stack
 *  因此stack没有迭代器(iterator)
*/

template <typename T, typename Sequence = deque<T> >
class stack {
    template<class U, class S>
    friend bool operator==(const stack<U, S>&, const stack<U, S>& );
    template<class U, class S>
    friend bool operator<(const stack<U, S>&, const stack<U, S>& );
public:
    typedef typename Sequence::value_type value_type;
    typedef typename Sequence::size_type size_type;
    typedef typename Sequence::reference reference;
    typedef typename Sequence::const_reference  const_reference;

protected:
    Sequence c;     //底层容器
public:
    // Stack自己的外部接口，利用底层容器完成
    bool empty() const { return c.empty(); }
    size_type size() const { return c.size(); }
    reference top() { return c.back(); }
    const_reference top() const { return c.back(); }
    // deque两个可进出，但stack只有头能进出
    void push(const value_type& x) { c.push_back(x); }
    void pop() { c.pop_back(); }
};

template<class T, class Sequence>
bool operator==(const stack<T, Sequence>& x, const stack<T, Sequence>& y)
{
    return x.c == y.c;
}

template<class T, class Sequence>
bool operator<(const stack<T, Sequence>& x, const stack<T, Sequence>& y)
{
    return x.c < y.c;
}
#endif
