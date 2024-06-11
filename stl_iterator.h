#ifndef __STL_ITERATOR_H
#define __STL_ITERATOR_H

#include <cstddef>

// SGI_STL五种迭代器的类型，每种迭代器允许的操作方式都不同
struct input_iterator_tag {};   //迭代器所指对象不允许外界改变，只读
struct output_iterator_tag {};  //只写
struct forward_iterator_tag : public input_iterator_tag {}; //允许读写迭代器
struct bidirectional_iterator_tag : public forward_iterator_tag {};  // 允许双向移动的迭代器
struct random_iterator_tag : public bidirectional_iterator_tag {}; // 支持随机访问迭代器


/**
 *   为了连接 容器 与 算法，使用 Iterator 迭代器来
 *  由于每个迭代器的类型不同，因此需要设计一种通用方式能够从 Iterator 中提取出特征
 *  type_traits编程方法可以很好地解决这个问题
*/

// 为了避免疏忽，自己定义的迭代器最好都继承这个迭代器标准
// 定义了 SGI_STL Iterator 应该定义的类型
template<typename Category, typename T, typename Distance = ptrdiff_t,
         typename Pointer = T*, typename Reference = T&>
struct iterator {
    typedef Category    iterator_category;
    typedef T           value_type;
    typedef Distance    difference_type;
    typedef Pointer     pointer;
    typedef Reference   reference;
};        


/**
 *  由于 C 原生指针类型不能定义在其中定义 value_type 等数据类型
 *  因此需要用 traits 无论将 Iterator 或者 C 指针 都能提出其特征例如
 *  value_type pointier 等
 * 
*/
template<typename Iterator>
struct iterator_traits {
    typedef typename Iterator::iterator_category    iterator_category;
    typedef typename Iterator::value_type           value_type;
    typedef typename Iterator::difference_type      difference_type;
    typedef typename Iterator::pointer              pointer;
    typedef typename Iterator::reference            reference;     
};

// 针对 C 原生指针特化
template<typename T>
struct iterator_traits<T *> {
    typedef random_iterator_tag     iterator_category;
    typedef T                       value_type;
    typedef ptrdiff_t               difference_type;
    typedef T*                      pointer;
    typedef T&                      reference;
};

// 针对 C const to pointer 特化
template<typename T>
struct iterator_traits<const T*> {
    typedef random_iterator_tag     iterator_category;
    typedef T                       value_type;
    typedef ptrdiff_t               difference_type;
    typedef T*                      pointer;
    typedef T&                      reference;
};


// 调用以下函数可以很好得到 SGI_STL of Iterator 的 category
template <typename Iterator>
inline typename iterator_traits<Iterator>::iterator_category
iterator_category(const Iterator&) {
    typedef typename iterator_traits<Iterator>::iterator_category category;
    return category();
}

// 调用以下函数可以很好得到 SGI_STL of Iterator 的 value type
template <typename Iterator>
inline typename iterator_traits<Iterator>::value_type
value_type(const Iterator &) {
    typedef typename iterator_traits<Iterator>::value_type value_type;
    return value_type();
}

template <typename Iterator>
inline typename iterator_traits<Iterator>::difference_type
distance_type(const Iterator&) {
    typedef typename iterator_traits<Iterator>::difference_type difference_type;
    return difference_type();
}

/**
 *   以下为distance()函数组，用于计算两个迭代器之间的距离
*/
// InputIterator的特例
template<class InputIterator>
inline typename iterator_traits<InputIterator>::difference_type
_distance(InputIterator first, InputIterator last, input_iterator_tag)
{
    typename iterator_traits<InputIterator>::difference_type n = 0;
    while(first != last) {
        ++first;
        ++n;
    }
    return n;
}

template<class RandomAcessIterator>
inline typename iterator_traits<RandomAcessIterator>::difference_type
_distance(RandomAcessIterator first, RandomAcessIterator last, random_iterator_tag)
{
    return last - first;
}

template<class InputIterator>
inline typename iterator_traits<InputIterator>::difference_type
distance(InputIterator first, InputIterator last)
{
    using category = typename iterator_traits<InputIterator>::iterator_category;
    return _distance(first, last, category());
}


#endif
