#ifndef __PRIORITY_QUEUE_H
#define __PRIORITY_QUEUE_H

#include <functional>
#include <iostream>
#include "vector.h"
#include "heap.h"

/**
 *      priority_queue container adapter 配接器
 *      以vector作为底层容器
 * 
*/
template <class T, class Sequence = vector<T>, class Compare = std::less<typename Sequence::value_type> >
class priority_queue {
public:
    typedef typename Sequence::value_type       value_type;
    typedef typename Sequence::size_type        size_type;
    typedef typename Sequence::reference        reference;
    typedef typename Sequence::const_reference  const_reference;

protected:
    Sequence    c;      // 底层容器
    Compare     comp;   // 元素大小比较标准

public:
    // priority_queue Construct 函数
    priority_queue() : c() {}
    explicit priority_queue(const Compare& x) : c(), comp(x) {}

    template<class InputIterator>
    priority_queue(InputIterator first, InputIterator last, const Compare& x) : c(first, last), comp(x) {
        make_heap(first, last, comp);
    }

    template<class InputIterator>
    priority_queue(InputIterator first, InputIterator last) : c(first, last), comp() {
        make_heap(c.begin(), c.end(), comp);
    }

    // priority 接口函数
    bool empty() { return c.empty(); }
    size_type size() { return c.size(); }
    const_reference top() const { return c.front(); }
    reference top() { return c.front(); }

    void push(const value_type& x) {
        c.push_back(x);
        push_heap(c.begin(), c.end(), comp);
    }

    void pop() {
        pop_heap(c.begin(), c.end(), comp);
        c.pop_back();
    }
};


#endif
