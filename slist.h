#ifndef __SLIST_H
#define __SLIST_H

#include "stl_alloc.h"
#include "stl_construct.h"

// 单向链表的节点基础节点
struct __slist_node_base
{
    __slist_node_base *next;
};

// 单向链表节点
template< class T>
struct __slist_node : public __slist_node_base
{
    T data;
};

// 全局函数：已知某一节点，向改节点后面插入函数
inline __slist_node_base* __slist_node_link(
    __slist_node_base* prve, __slist_node_base* newnode)
{
    newnode->next = prve->next;
    prve->next = newnode;
    return newnode;
}

// 全局函数：单向链表的大小
inline size_t __slist_size(__slist_node_base* node)
{
    size_t result = 0;
    for(; node != 0; node = node->next)
        ++result;
    return result;
}


// 单向链表的迭代器基本结构
struct __slist_iterator_base
{
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef forward_iterator_tag    iterator_category;  // 单项链表tag

    // 指向节点基本结构
    __slist_node_base* node;

    __slist_iterator_base(__slist_node_base* x) : node(x) {}

    void incr() { node = node->next; }  // 前进一个节点

    bool operator==(const __slist_iterator_base& x) const {
        return node == x.node;
    }

    bool operator!=(const __slist_iterator_base& x) const {
        return node != x.node;
    }
};

// 单项链接的迭代器结构
template<class T, class Ref, class Ptr>
struct __slist_iterator : public __slist_iterator_base
{
    typedef __slist_iterator<T, T&, T*>     iterator;
    typedef __slist_iterator<T, const T&, const T*>    const_iterator;
    typedef __slist_iterator<T, Ref, Ptr>   self;

    typedef T   value_type;
    typedef Ptr pointer;
    typedef Ref reference;
    typedef __slist_node<T> list_node;

    __slist_iterator(list_node* x) : __slist_iterator_base(x) {}
    __slist_iterator() : __slist_iterator_base(0) {}
    __slist_iterator(const iterator& x) : __slist_iterator_base(x.node) {}

    reference operator* () const {
        return ((list_node*) node) ->data;
    }

    pointer operator-> () const {
        return &(operator*());
    }

    self& operator++ () {
        incr();
        return *this;
    }

    self operator++ (int) {
        self tmp = *this;
        incr();
        return tmp;
    }
};

template<class T, class Alloc = __default_alloc_template<0>>
class slist {
public:
    typedef  T     value_type;
    typedef  T* pointer;
    typedef  const T* const_pointer;
    typedef  T&  reference;
    typedef const T&  const_reference;
    typedef ptrdiff_t difference_type;

    typedef __slist_iterator<T, reference, pointer> iterator;
    typedef __slist_iterator<const T, const T&, const T*> const_iterator;

private:
    typedef __slist_node<T> list_node;
    typedef __slist_node_base list_node_base;
    typedef __slist_iterator_base iterator_base;
    typedef simple_alloc<list_node, Alloc> list_node_allocator;

    static list_node* create_node(const value_type& x) {
        list_node* new_node = list_node_allocator::allocate();
        construct( (&new_node->data), x);
        new_node->next = 0;
        return new_node;
    }

    static void destroy_node(const list_node* node) {
        destroy(&(node->data));
        list_node_allocator::deallocate(node);
    }

private:
    list_node_base head;    // 头部节点，不是指针
    list_node* __inset_after(list_node_base* pre, const value_type& x) {
        return (list_node*) __slist_node_link(pre, create_node(x));
    }

public:
    slist() { head.next = 0; }
public:
    iterator begin() { return iterator((list_node*)head.next); }
    iterator end() { return iterator(0); }   // 空节点
    size_t size() { return __slist_size(head.next); }
    bool empty() { return head.next == 0; }
    iterator insert_after(iterator pos, const value_type& x) {
        return iterator(__inset_after(pos.node, x));
    }
    void swap(slist& L) {
        list_node_base* tmp = head.next;
        head.next = L.head.next;
        L.head.next = tmp;
    }

public:
    reference front() { return ((list_node*)head.next)->data; }

    // 从头部加入节点
    void push_front(const value_type& x) {
        __slist_node_link(&head, create_node(x));
    }

    // 从头部取出节点
    void pop_front() {
        list_node_base* node = head.next;
        head.next = node->next;
        destroy_node((list_node *)node);
    }
};








#endif
