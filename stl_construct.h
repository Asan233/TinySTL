#ifndef __STL_CONSTRUCT_H
#define __STL_CONSTRUCT_H
/**
 *  该文件是空间配置器中的构造/析构头文件，负责对内存空间使用高效的construct()和destroy()
 *  使用__type_true 和 __type_false 判别是否对该类型有特化版本。
 *  对特化版本使用高效的construct()/destroy()。
*/
#include <new>
#include "type_traits.h"
#include "stl_iterator.h"
#include <iostream>

template<typename T1, typename T2>
inline void construct(T1* p, const T2& value)
{
    // 调用placement new 在p地址上构造T1以T2为初值
    new (p) T1(value);
}

template<typename T>
inline void destroy(T* pointer)
{
    pointer->~T();      //调用dtor ~T()
}


template<class ForwardIterator, class T>
inline void __destroy(ForwardIterator first, ForwardIterator last, T)
{
    typedef typename __type_traits<T>::has_trivial_destructor trivial_destructor;
    __destroy_aux(first, last, trivial_destructor());
}

/**
 *  接受两个迭代器并对特定类型进行优化dstr()
*/
template<class ForwardIterator>
inline void destroy(ForwardIterator first, ForwardIterator last)
{
    __destroy( (ForwardIterator)first, (ForwardIterator)last, value_type(first) );
}

//value type 如果有 non-trivial destructor
template<typename ForwardIterator>
inline void __destroy_aux(ForwardIterator first, ForwardIterator last, __false_type)
{
    for( ; first < last; ++first)
    {
        destroy(&(*first));
    }
}

//value type 如果有 trivial destructor
template<typename ForwardIterator>
inline void __destroy_aux(ForwardIterator first, ForwardIterator last, __true_type)
{
    // 什么都不用做
}


#endif