#ifndef __LIST_H
#define __LIST_H

#include "stl_alloc.h"
#include "stl_construct.h"
#include "algo.h"

/**
 *  list_node 用于list的底层保存list节点的数据和连接其他的list_node，list_node是一个双向链表
*/
template <class T>
struct __list_node {
    typedef __list_node<T>* pointer;
    pointer prev;
    pointer next;
    T data;
};

/**
 *  list_iterator list的迭代器，由于普通指针不能完全完成访问list节点的功能，因此需要设计一个iterator专用于list的访问
*/
template <class T, class Ref, class Ptr>
struct __list_iterator {
    typedef __list_iterator<T, T&, T*>      iterator;
    typedef __list_iterator<T, Ref, Ptr>    self;
    
    typedef bidirectional_iterator_tag      iterator_category;
    typedef T       value_type;
    typedef Ptr     pointer;
    typedef Ref     reference;
    typedef __list_node<T>*     link_type;
    typedef size_t  size_type;
    typedef ptrdiff_t   difference_type;

    link_type node;     //指向list的节点

    //construct
    __list_iterator(link_type x) : node(x) {}
    __list_iterator() {}
    __list_iterator(const iterator& x) : node(x.node) {}

    bool operator==(const self& x) const { return node == x.node; }
    bool operator!=(const iterator& x) const { return node != x.node; }
    // 以下对迭代器取值
    reference operator*() const { return (*node).data; }
    pointer operator->() const { return &(operator*()); }

    // 迭代器的运算操作w
    self& operator++() {
        node = (link_type)((*node).next);
        return *this;
    }
    self operator++(int) {
        self tmp = *this;
        ++*this;
        return tmp;
    }

    self& operator--() {
        node = (link_type)((*node).prev);
        return *this;
    }
    self operator--(int) {
        self tmp = *this;
        --*this;
        return tmp;
    }
};

/**
 *  list 通过 list_iterator进行操作
 *  为了满足iterator迭代器的要求（对数组元素实行左开右闭区间），则令list指向一个node节点，该节点为空
*/
template <class T, class Alloc = __default_alloc_template<0>>
class list 
{
protected:
    typedef __list_node<T> list_node;

public:
    typedef T   value_type;
    typedef T*  pointer;
    typedef const T* const_pointer;
    typedef T&  reference;
    typedef const T& const_reference;
    typedef list_node* link_type;
    typedef __list_iterator<T, T&, T*> iterator;
    typedef size_t      size_type;
    typedef ptrdiff_t   difference_type;

protected:
    link_type node;     // 指向一个list_node
    typedef simple_alloc<list_node, Alloc> list__node_allocator;

protected:
    // 配置一个节点空间
    link_type get_node() { return list__node_allocator::allocate(); }
    // 释放一个节点
    void put_node(link_type p) { list__node_allocator::deallocate(p); }
    // 配置并构造一个节点
    link_type create_node(const value_type& x) {
        link_type p = get_node();
        construct(&p->data, x);
        return p;
    }

    // 销毁一个节点
    void destroy_node(link_type p) {
        destroy(&p->data);
        put_node(p);
    }

protected:
    void empty_initialize() {
        node = get_node();
        node->next = node;
        node->prev = node;
    }

    // 将[first, last) 内的所有元素移动到position之前
    // 内部接口用内部的成员函数调用
    void transfer(iterator position, iterator first, iterator last) {
        if(position != last) {
            (*(link_type((*last.node).prev))).next = position.node;
            (*(link_type((*first.node).prev))).next = last.node;
            (*(link_type((*position.node).prev))).next = first.node;
            link_type tmp = link_type((*position.node).prev);
            (*position.node).prev = (*last.node).prev;
            (*last.node).prev = (*first.node).prev;
            (*first.node).prev = tmp;
        }
    }

    // 与链表x交换数据
    void swap(list& x) 
    {
        list_node tmp = node;
        node = x.node;
        x.node = node;
    }

public:
    iterator begin() const { return iterator((link_type)((*node).next)); }
    iterator end()  const { return iterator(node); }

    bool empty() const { return node->next == node; }

    size_type size() const 
    {
        return distance(begin(), end());
    }

    reference front() { return *begin(); }
    reference back()  { return *(--end()); }

    // 加入node操作
    iterator insert(iterator position, const T& x) {
        // 产生一个节点
        link_type tmp = create_node(x);
        // 调整指针加入节点
        tmp->next = position.node;
        tmp->prev = position.node->prev;
        (link_type(position.node->prev))->next = tmp;
        position.node->prev = tmp;
        return iterator(tmp);
    }

    void push_back(const T& x) { insert(end(), x); }
    void push_front(const T& x) { insert(begin(), x); }

    // 移除Iterator 所指节点
    iterator erase(iterator position);

    // 移除节点
    void pop_back() { iterator tmp = end(); erase(--tmp); }
    void pop_front() { erase(begin()); }

    // 清楚所有节点
    void clear();

    // 移除数值为value的节点
    void remove(const T& x);

    // 移除数值相同的连续元素
    void unique();

    // construct
    list() { empty_initialize(); }      // 产生一个空链表

    // 各种split接口
    // 将x接合与position所指位置之前
    void splice(iterator position, list& x) {
        if(!x.empty())
            transfer(position, x.begin(), x.end());
    }

    // 将i所指元素接合于position所指位置之前，position和i可指向同一个list
    void splice(iterator position, list&, iterator i) {
        iterator j = i;
        ++j;
        if(position == i || position == j) return;
        transfer(position, i, j);
    }

    // 将[first, last)内的元素结合于position之前
    // position和[first, last)可指向同一个list
    // 但position不能位于[first, last)之类
    void splice(iterator position, iterator first, iterator last) {
        if(first != last)
            transfer(position, first, last);
    }

    // merge() 将 x 合并到 *this 上面，两个list的内容都必须是递增排序的
    void merge(list<T, Alloc>& x);

    // reverse() 将*this的内容逆向重置
    void reverse();

    // 由于list的特殊性，不能使用STL的sort算法，需要自己定义一个sort算法
    void sort();

};

template<class T, class Alloc>
typename list<T,Alloc>::iterator list<T, Alloc>::erase(iterator position) {
    link_type next_node = link_type(position.node->next);
    link_type prev_node = link_type(position.node->prev);
    prev_node->next = next_node;
    next_node->prev = prev_node;
    destroy_node(position.node);
    return iterator(next_node);
}

template<class T, class Alloc>
void list<T, Alloc>::clear() {
    link_type cur = (link_type)node->next;
    while(cur != node) {
        link_type tmp = cur;
        cur = (link_type)cur->next;
        destroy_node(tmp);
    }
    // 恢复只有一个空node的list初始状态
    node->next = node;
    node->prev = node;
}

template<class T, class Alloc>
void list<T, Alloc>::remove(const T& x) {
    iterator first = begin();
    iterator last = end();
    while(first != last) {
        iterator next = first;
        ++next;
        if(*first == x) erase(first);
        first = next;
    }
}

template<class T, class Alloc>
void list<T, Alloc>::unique() {
    iterator first = begin();
    iterator last  = end();
    if(first == node) return;
    iterator next = first;
    while(++next != last) {
        if(*first == *next)
            erase(next);
        else
            first = next;
        next = first;
    }
}

template<class T, class Alloc>
void list<T, Alloc>::merge(list<T, Alloc>& x) {
    iterator first1 = begin();
    iterator end1 = end();
    iterator first2 = x.begin();
    iterator end2 = x.end();

    // x 与 this 必须已经是递增排序排好的
    while(first1 != end1 && first2 != end2) {
        if(*first2 < *first1) {
            iterator next = first2;
            ++next;
            transfer(first1, first2, next);
            first2 = next;
        }else 
            ++first1;
    }
    if(first2 != end2) transfer(end1, first2, end2);
}

template<class T, class Alloc>
void list<T, Alloc>::reverse() {
    // 空list或者只有一个元素则不进行操作
    if(node->next == node || link_type(node->next)->next = node) return;
    iterator first = begin();
    ++first;
    iterator last = end();
    while(first != last) {
        iterator old = first;
        ++first;
        transfer(begin(), old, first);
    }
}

/**
 *  排序算法为创建64个桶每个桶的index 为 1 - 64 , 容量为 2^(index)
 *  每次取出原list 头元素放入桶中，每个桶中的元素已经按递增排好序
 *  如果当前桶容量满就归并所有桶元素并放入下一个编号容量更大的桶中
 *  最后归并所有桶中的元素
*/
template<class T, class Alloc>
void list<T, Alloc>::sort() {
    // 空list或者只有一个元素则直接返回
    if(node->next == node || link_type(node->next)->next == node) return;

    // 临时list用于临时存放数据
    list<T, Alloc> carry;           // 每次取原list头元素放到
    list<T, Alloc> bucket[64];     // 64个桶，每个桶的大小为2^i
    int fill = 0;
    while(!empty()) {
        carry.splice(carry.begin(), *this, begin());    // 取出原list头元素
        int i = 0;
        // i < fill 的桶未满，不断向桶中运输元素，并保持桶中元素递增排序
        // i < fill 的桶元素满， 归并所有桶中元素并放入容量更大的桶中.
        while(i < fill && !bucket[i].empty() ) {
            carry.merge(bucket[i]);
            ++i;
        }
        bucket[i].swap(carry);
        // bucket[fill]已满，增加下一个桶
        if(i == fill) ++fill;
    }
    // 归并所有桶
    for(int i = 1; i < fill; i++)
        bucket[i].merge(bucket[i - 1]);
    swap(bucket[fill - 1]);
}

#endif