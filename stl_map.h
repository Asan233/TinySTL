#ifndef __STL_MAP_H
#define __STL_MAP_H

#include "stl_rb_tree.h"
#include "stl_pair.h"

template<class Key, class T, class Compare = std::less<Key>, class Alloc = __default_alloc_template<0>>
class map {
public:
    typedef Key key_type;       //key type
    typedef T   data_type;      //data type
    typedef T   mapped_type;    // map type
    typedef pair<const Key, T> value_type;  // value type(key / value)
    typedef Compare key_compare;    // compare function

    // 以下定义一个function，其作用调用 “元素比较函数”
    template<class Value = value_type>
    class value_compare {
    protected:
        Compare comp;
        value_compare(Compare c) : comp(c) {}
    public:
        bool operator() (const Value& x, const Value& y) const {
            return comp(x.first, y.first);
        }
    };

private:
    typedef rb_tree<key_type, value_type, std::_Select1st<value_type>, key_compare, Alloc> rep_type;
    rep_type t;
    
public:
    typedef typename rep_type::pointer pointer;
    typedef typename rep_type::const_pointer const_pointer;
    typedef typename rep_type::reference reference;
    typedef typename rep_type::const_reference const_reference;
    typedef typename rep_type::iterator iterator;       // map不能像set一样将iterator定义为const，因为map可能会修改元素的实值
    typedef typename rep_type::const_iterator   const_iterator;
    typedef typename rep_type::size_type    size_type;
    typedef typename rep_type::difference_type  difference_type;

    map() : t(Compare()) {}
    explicit map(const Compare& comp) : t(comp) {}

    template<class InputerIterator>
    map(InputerIterator first, InputerIterator last) : t(Compare()) { t.insert_uniqual(first, last); }

    template<class InputerIterator>
    map(InputerIterator first, InputerIterator last, const Compare& comp) : t(comp) { t.insert_uniqual(first, last); }

    map(const map<Key, T, Compare, Alloc>& x) : t(x.t) { }
    map<Key, T, Compare, Alloc>& operator=(const map<Key, T, Compare, Alloc>& x) {
        t = x.t;
        return *this; 
    }

    // 以下方法均调用rb_tree的接口
    key_compare key_comp() { return t.key_comp(); }

    //  value_compare value_comp() { return value_compare(t.key_comp()); }
    // 报错 value_compare 为模板类，编译器不允许函数返回类型中自动推导类型（即函数返回模板类时不能省略模板参数）
    // 正确应该如下
    value_compare<value_type> value_comp() { return value_compare(t.key_comp()); }
    iterator begin() { return t.begin(); }
    const_iterator begin() const { return t.begin(); }
    iterator end() { return t.end(); }
    const_iterator end() const { return t.end(); }
    bool empty() const { return t.empty(); }
    size_type size() const { return t.size(); }
    
    // []操作符
    T& operator[](const key_type& x) {
        return (*(( insert(value_type(x, T())) ).first)).second;
    }

    // 返回一个pair<iterator, bool> ： <插入元素的迭代器，是否真的插入>
    pair<iterator, bool> insert(const value_type& x) {
        return t.insert_uniqual(x);
    }

    template<class InputIterator>
    void insert(InputIterator first, InputIterator last) {
        t.insert_uniqual(first, last);
    }

    void clear() { t.clear(); }

    iterator find(const key_type& x) { return t.find(x); }

};


#endif
