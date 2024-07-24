#ifndef __PRIORITY_QUEUE_H
#define __PRIORITY_QUEUE_H

#include <functional>
#include "vector.h"

/**
 *      priority_queue container adapter 配接器
 *      以vector作为底层容器
 * 
*/
template <class T, class Sequence = vector<T>, class Compare = less<typename Sequence::value_type> >
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
    
}



#endif
