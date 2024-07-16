#ifndef _STL_UNINTIALIZED_H
#define _STL_UNINTIALIZED_H

#include "stl_construct.h"
#include "algo.h"

/**
 * unintialized_copy()  fill()  full_n()类型特例化
 * 对于 POD 类型拥有 cont()/dcot()/assigment()任务调用 STL 的高级函数
 * 使用taite_type()
*/

/**
 *  使用重载对POD类型自动调用 true_type()
*/
template<typename InputIterator, typename ForwardIterator>
inline ForwardIterator __uninitialized_copy_aux(InputIterator first, InputIterator last, ForwardIterator result, __true_type)
{
    // POD类型调用STL高级函数
    return copy(first, last, result);
}

// __false_type
template<typename InputIterator, typename ForwardIterator>
inline ForwardIterator __uninitialized_copy_aux(InputIterator first, InputIterator last, ForwardIterator result, __false_type)
{
    ForwardIterator cur = result;
    for(; first != last; ++first, ++cur)
    {
        construct(&*(cur), *first);
    }
    return cur;
}

template<typename InputIterator, typename ForwardIterator, typename T>
inline ForwardIterator __uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result, T )
{
    // trait_typed()实现分辨是否为POD类型
    typedef typename __type_traits<T>::is_POD_type is_POD;
    //__aux实现实际操作，并针对is_POD进行特殊处理
    return __uninitialized_copy_aux(first, last, result, is_POD());
}

template<typename InputIterator, typename ForwardIterator>
inline ForwardIterator uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result)
{
    return __uninitialized_copy(first, last, result, value_type(result));
}


/**
 *  uninitialized_fill()
*/
template<typename ForwardIterator, typename T>
inline void __uninitialized_fill_aux(ForwardIterator first, ForwardIterator last, const T& x, __true_type)
{
    MYSTL::fill(first, last, x);
}

template<typename ForwardIterator, typename T>
inline void __uninitialized_fill_aux(ForwardIterator first, ForwardIterator last, const T& x, __false_type)
{
    ForwardIterator cur = first;
    for(; cur != last; ++cur)
        construct(&(*cur), x);
}

template<typename ForwardIterator, typename T, typename T1>
inline void __uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& x, T1)
{
    typedef typename __type_traits<T1>::is_POD_type is_POD;
    __uninitialized_fill_aux(first, last, x, is_POD());
}

template<typename ForwardIterator, typename T>
inline void uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& x)
{
    __uninitialized_fill(first, last, x, value_type(first) );
}

/**
 *  uninitialzed_fill_n()
 *  
*/
template<typename ForwardIterator, typename Size, typename T>
inline ForwardIterator uninitialized_fill_n(ForwardIterator first, Size n, const T& x)
{
    return __uninitialized_fill_n(first, n, x, value_type(first));
}

template<typename ForwardIterator, typename Size, typename T, typename T1>
inline ForwardIterator __uninitialized_fill_n(ForwardIterator first, Size n, const T& x, T1 *)
{
    typedef typename __type_traits<T1>::is_POD_type is_POD;
    return __uninitialized_fill_n_aux(first, n, x, is_POD());
}

template<typename ForwardIterator, typename Size, typename T>
inline ForwardIterator __uninitialized_fill_n_aux(ForwardIterator first, Size n, const T& x, __true_type)
{
    return fill_n(first, n, x);
}

template<typename ForwardIterator, typename Size, typename T>
inline ForwardIterator __uninitialized_fill_n_aux(ForwardIterator first, Size n, const T& x, __false_type)
{
    ForwardIterator cur = first;
    for(; n > 0; --n, ++cur)
        construct(&*cur, x);
    return cur;
}

#endif
