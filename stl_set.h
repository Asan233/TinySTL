#ifndef __STL_SET_H
#define __STL_SET_H

#include "stl_rb_tree.h"
#include "stl_function.h"

template<class Key, class Compare = std::less<Key>, class Alloc = __default_alloc_template<0>>
class set 
{
// 一些关于set的定义
public:
    typedef Key key_type;
    typedef Key value_type;
    // 由于set的值和键一样，因此值和键的比较函数也一致
    typedef Compare key_compare;
    typedef Compare value_compare;

private:
    /**
     *  普通set的底层使用rb_tree来做容器
    */
    typedef rb_tree<key_type, value_type, identity<key_type>, key_compare, Alloc> rep_type;
    rep_type t;  // set内部定义的红黑树，set的底层操作全用rb_tree操作

public:
    /**
     *      由于set的特性无法更改其中的元素值，因此选用const_iterator作为set的迭代器
     *      且set中的所有point, ref都是const
    */
    typedef typename rep_type::const_pointer pointer;
    typedef typename rep_type::const_pointer const_pointer;
    typedef typename rep_type::const_reference reference;
    typedef typename rep_type::const_reference const_reference;
    typedef typename rep_type::const_iterator  iterator;
    typedef typename rep_type::const_iterator  const_iterator;
    typedef typename rep_type::size_type  size_type;
    typedef typename rep_type::difference_type difference_type;

    // 由于set的性质，不允许重复的元素，且Key = Value
    // RB_tree 作为set的底层容器，则set调用RB_tree的底层函数insert_unique()，不允许插入重复元素
    set() : t(Compare()) { }
    explicit set(const Compare& comp) : t(comp) { }

    // 输入两个迭代器，将[first, last)插入set之中
    template<class InputIterator>
    set(InputIterator first, InputIterator last) : t(Compare()) { t.insert_uniqual(first, last); }
    // 迭代器之间插入，并输入一个比较函数
    template<class InputIterator>
    set(InputIterator first, InputIterator last, const Compare& comp) : t(comp) { t.insert_uniqual(first, last); }
    // 复制构造函数
    set(const set<Key, Compare, Alloc> &x) : t(x.t) { }

    // set<Key, Compare, Alloc>& operator=(const set<key, Compare, Alloc> &x) {
    //     t = x.t;
    //     return *this;
    // }

public:
    // 以下操作完全在rb_tree t 中已经提供
    key_compare key_comp() const { return t.key_comp(); }
    value_compare value_comp() const { return t.key_comp(); }
    iterator begin() const { return t.begin(); }
    iterator end() const { return t.end(); }
    bool empty() const { return t.empty(); }
    size_type size() const { return t.size(); }
    // void swap(set<Key, Compare, Alloc>& x) { t.swap(x.t); }

    void clear() { t.clear(); }
    // set operator
    iterator find(const value_type& x) { return t.find(x); }

    // // 友元函数定义
    // friend bool operator== (const set&, const set&);
    // friend bool operator< (const set&, const set&);
    
};

#endif
