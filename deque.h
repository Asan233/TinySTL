#ifndef __DEQUE_H
#define __DEQUE_H

#define __STL_TRY try

#include "stl_iterator.h"
#include "stl_construct.h"
#include "stl_alloc.h"
#include "stl_unintialized.h"
#include "algo.h"
#include <iostream>

/**
 *      Deque 采用将离散的每块一小段内存链接起来的方式
 *      因此Deque的内存管理方式比起Vector的内存管理方式更加复杂
 *      为了使Deque有一种使用起来连续存储的假象，即迭代器能Ramdom Access，则迭代器比Vector, list更加复杂
*/

// 全局函数用来返回缓冲区的大小
// n != 0，回传n表示buffer size由用户定义
// n == 0, 表示buffer size使用默认值，且
//      sz < 512, 返回 512/sz
//      sz >= 512, 返回1
inline size_t __deque_buf_size(size_t n, size_t sz)
{
    return n != 0 ? n : ( sz < 512 ? size_t (512/sz) : size_t(1) ); 
}

// Deque iterator 继承自 iterator 符合STL::iterator标准
template <typename T, typename Ref, typename Ptr, size_t BufSiz>
struct __deque_iterator : public iterator<random_iterator_tag, T> {
    typedef __deque_iterator<T, T&, T*, BufSiz>     __iterator;
    typedef __deque_iterator<T, const T&, const T*, BufSiz>     __const_iterator;
    static size_t buffer_size() { return __deque_buf_size(BufSiz, sizeof(T)); }
    
    // 除了STL::iterator的标准外，还需要定义一些Deque自己
    typedef size_t size_type;
    typedef T** map_pointer;
    typedef __deque_iterator self;
    typedef typename iterator<random_iterator_tag, T>::difference_type difference_type;
    typedef typename iterator<random_iterator_tag, T>::reference reference;
    typedef typename iterator<random_iterator_tag, T>::pointer  pointer;

    // 将Iterator与迭代器进行链接
    T*  cur;        // 该指针指向所指之缓冲区的现行元素
    T*  frist;      // 该指针指向所指之缓冲区的头
    T*  last;       // 该指针指向所指之缓冲区的尾
    map_pointer node;   // 指向管控中心

    // construct
    __deque_iterator(T* x, map_pointer y) noexcept : cur(x), frist(*y), last(frist + buffer_size()), node(y) { }
    __deque_iterator( ) noexcept : cur(), frist(), last(), node() { }
    __deque_iterator(const self& x) noexcept : cur(x.cur), frist(x.frist), last(x.last), node(x.node) { } 

public:
    /**
     *   由于 Ddeque 迭代器的特殊方式，因此需要在迭代器内部封装一些方法
     *   和重载操作符号使该迭代器对Deque能够像 Ramdom Access 一样使用
    */
    void set_node(map_pointer new_node){
        node = new_node;
        frist = *node;
        last = frist + (buffer_size());
    }
    reference operator*() const { return *cur; }
    pointer operator->() const { return &(operator*()); }
    // 计算两个迭代器之间的距离
    difference_type operator-(const self&x) const {
        return ( buffer_size() ) * ( this->node - x.node - 1) + (this->cur - this->frist) + (x.last - x.cur);
    }
    // 后置与前置加法
    self& operator++() {
        ++cur;              // 切换到下一个元素
        if(cur == last) {   // 到达当前缓冲区尾部
            set_node(node + 1); // 切换到下一个缓冲区
            // 重新设置指针
            cur = frist;    // 切换到第一个元素
        }
        return *this;
    }

    self operator++(int) {
        self tmp = *this;
        ++*this;
        return tmp;
    }

    // 后置与前置减法
    self& operator--() {
        if(cur == frist) {
            set_node(node - 1);
            cur = last;
        }
        --cur;
        return *this;
    }

    self operator--(int) {
        self tmp = *this;
        --*this;
        return tmp;
    }
    // 为了实现RomdAccess迭代器随机存取功能
    self& operator+=(difference_type n) {
        // 计算偏置距离
        const difference_type offset = n + (cur - frist);
        if(offset >= 0 && offset < difference_type(buffer_size()))
            // 目标位置在同一缓冲区内
            cur += n;
        else {
            // 不在同一缓冲区中
            difference_type node_offset = offset > 0 ? offset / difference_type(buffer_size()) : -difference_type((-offset - 1) / buffer_size()) - 1;
            // 使node节点切换到正确的缓冲区
            set_node(node + node_offset);
            // 切换到正确的元素
            cur = frist + (offset - node_offset * difference_type(buffer_size()));
        }
        return *this;
    }

    self& operator-=(difference_type n) {
        return *this += -n;
    }

    self operator+(difference_type n) const {
        self tmp = *this;
        return tmp += n;
    }

    self operator-(difference_type n) const {
        self tmp = *this;
        return tmp -= n;
    }

    // 实现随机存取
    reference operator[] (difference_type n) const { return *(*this + n); }
    bool operator==(const self& x) const { return cur == x.cur; }
    bool operator!=(const self& x) const { return cur != x.cur; }
    bool operator<(const self& x) const {
        return (node == x.node) ? (cur < x.cur) : (node < x.node);
    }
};

template <class T, class Alloc = simple_alloc<T, __default_alloc_template<0>>, size_t Bufsize = 0>
class deque {
public:
    typedef T   value_type;
    typedef T*  pointer;
    typedef const T*    const_pointer;
    typedef T&  reference;
    typedef const T& const_reference;
    typedef size_t  size_type;
    typedef ptrdiff_t   difference_type;

    // deque iterator
    typedef __deque_iterator<T, T&, T*, Bufsize> iterator;

protected:
    // 专属空间配置器, 每次配置一个元素大小
    typedef simple_alloc<value_type, __default_alloc_template<0>> data_allocator;
    // 专属空间配置器，每次配置一个指针大小
    typedef simple_alloc<pointer, __default_alloc_template<0>> map_allocator;

    enum { initial_map_size = 8 };
protected:
    // Ddeque 采用 map_pointer类型 来管理各个离散的连续内存块
    typedef pointer *map_pointer;

protected:
    iterator start;     // 表示第一个节点
    iterator finish;    // 表示最后一个节点

    map_pointer map;        // 中控管理各个离散连续的内存块,其中每个元素都是指针，指向一个缓冲区
    size_type map_size;     // map内可容纳多少指针（多少个连续内存块

protected:
    void create_map_and_nodes(size_type num_elements);
    void fill_initialize(size_type n, const value_type& value);     // 赋值Deque中所有元素

protected:
    // 内存分配函数
    pointer allocate_node() { return data_allocator::allocate( iterator::buffer_size()); }
    // 内存节点释放
    void deallocate_node(pointer p) { data_allocator::deallocate(p, iterator::buffer_size()); }
    // 重新分配map节点
    void reallocater_map(size_type nodes_to_add, bool add_at_front);
    
    // push_back只剩一个空间时调用
    void push_back_aux(const value_type& t);
    // push_front前已经没有空间时调用
    void push_front_aux(const value_type& t);
    // pop_back 最后迭代器没有元素
    void pop_back_aux();
    // pop_front 最前迭代器没有元素
    void pop_front_aux();
    // 完成insert的实际插入函数
    iterator insert_aux(iterator, const value_type& );

    //根据map前端剩余空间，重新分配map
    void reserve_map_at_back (size_type nodes_to_add = 1) {
        if( nodes_to_add + 1 > map_size - (finish.node - map) ){
            // 符合则map尾端备用空间不足
            // 需要重新置换分配一个更大的map
            reallocater_map(nodes_to_add, false);
        }
    }

    //根据map后端剩余空间，重新分配map 
    void reserve_map_at_front (size_type nodes_to_add = 1) {
        if( nodes_to_add > (start.node - map) ) {
            // 符合则map前端备用空间不足
            // 需要重新置换分配一个更大的map
            reallocater_map(nodes_to_add, true);
        }
    }
    
// 构造与析构函数
public:
    deque(int n, const value_type& value) : start(), finish(), map(0), map_size(0)
    {
        fill_initialize(n, value);
    }
public:
    // basic access
    iterator begin() { return start; }
    iterator end() {  return finish; }

    reference operator[] (size_type n) {
        // 调用__deque_iterator<>::operator[]
        return start[ typename iterator::difference_type(n) ];
    }

    reference front() { return *start; }
    reference back() {
        iterator tmp = finish;
        --tmp;
        return *tmp;
    }

    size_type size() const { return finish - start; }
    size_type max_size() const { return size_type(-1); }
    bool empty() const { return finish == start; }


    // 清空队列
    void clear();
    // erase重载函数组
    iterator erase(iterator pos);      // 清除pos所指向迭代器
    iterator erase(iterator first, iterator last);  // 清除[first, last)内所有元素
    
    // insert插入函数
    iterator insert(iterator position, const value_type& x) {
        // 插入点在最前面
        if(position == start.cur) {
            push_front(x);
            return start;
        }else if (position == finish.cur) {
            push_back(x);
            iterator tmp = finish;
            --tmp;
            return tmp;
        }else {
            return insert_aux(position, x);    // 交给insert_aux去做
        }
    }

    // 公共接口函数供调用
    void push_back(const value_type& t) {
        if(finish.cur != finish.last - 1) {
            // 最后缓冲区有至少两个备用空间，则直接构造在上面
            construct(finish.cur, t);
            ++finish.cur;
        } else {
            // 缓冲区只剩最后一个空间
            push_back_aux(t);
        }
    }

    void push_front(const value_type& t) {
        if(start.cur != start.frist) {
            construct(start.cur - 1, t);
            --start.cur;
        }else {
            // 缓冲区front没有空间了
            push_front_aux(t);
        }
    }

    void pop_back() {
        if(finish.cur != finish.frist) {
            --finish.cur;
            destroy(finish.cur);
        }else {
            // 最后缓冲区迭代器没有任何元素，工作交由aux处理
            pop_back_aux();
        }
    }

    void pop_front() {
        if(start.cur != start.last - 1) {
            destroy(start.cur);
            ++start.cur;
        }else {
            // 最前缓冲区迭代器没有任何元素，工作交由aux处理
            pop_front_aux();
        }
    }
};

/**
 *      Deque function 定义
*/
template <class T, class Alloc, size_t Bufsize>
void deque<T, Alloc, Bufsize>::fill_initialize(size_type n, const value_type& value)
{
    // 为Deque产生合适的内存结构
    create_map_and_nodes(n);
    map_pointer cur;
    for(cur = start.node; cur < finish.node; ++cur)
        uninitialized_fill(*cur, *cur + iterator::buffer_size(), value);

    // 由于尾端nodes可能不需要填满，则需要特殊设置初值
    uninitialized_fill(finish.frist, finish.cur, value);
}

template <class T, class Alloc, size_t Bufsize>
void deque<T, Alloc, Bufsize>::create_map_and_nodes(size_type num_elements)
{
    // 需要节点数 = (元素个数 / 每个缓冲区可容纳节点数量个数) + 1
    // 如果刚好整除则，多分配一个节点数
    size_type num_nodes = num_elements / iterator::buffer_size() + 1;

    // 前后各预留一个节点，最少配置节点数量为8个
    map_size = initial_map_size > (num_nodes + 2) ? initial_map_size : (num_nodes + 2);
    map = map_allocator::allocate(map_size);
    // 以上配置 map_size 个节点的map

    // 令指针合适得指向所有节点
    // 令nstart和nfinish指向map所指的最中央区域
    map_pointer nstart = map + (map_size - num_nodes) / 2;
    map_pointer nfinish = nstart + num_nodes - 1;

    map_pointer cur;

    // 为map的每个节点指向一个缓冲区，为每个节点配置缓冲区
    // map中所有节点加起来就是map的空间
    for(cur = nstart; cur <= nfinish; ++cur)
        *cur = allocate_node();

    // 设置deque迭代器
    start.set_node(nstart);
    finish.set_node(nfinish);
    start.cur = start.frist;
    finish.cur = finish.frist + num_elements % iterator::buffer_size();
} 

// 只有当尾部只剩一个剩余空间时才会调用该函数 finish.cur == finish.last -1
template <class T, class Alloc, size_t Bufsize>
void deque<T, Alloc, Bufsize>::push_back_aux(const value_type& t) {
    value_type copy = t;
    // 若满足某种条件重新换一个map
    reserve_map_at_back();
    *(finish.node + 1) = allocate_node();
    // 构造初值
    __STL_TRY {
        construct(finish.cur, copy);
        finish.set_node(finish.node + 1);
        finish.cur = finish.frist;
    }
    catch(...) {
        finish.set_node(finish.node - 1);
        finish.cur = finish.last - 1;
        deallocate_node( *(finish.node + 1) );
        throw;   
    }
}

// 只有当前部没有剩余空间时才会调用该函数
template <class T, class Alloc, size_t Bufsize>
void deque<T, Alloc, Bufsize>::push_front_aux(const value_type& t) {
    value_type copy = t;
    // 若满足某种条件重新换一个map
    reserve_map_at_front();
    *(start.node - 1) = allocate_node();
    __STL_TRY {
        start.set_node(start.node - 1);
        start.cur = start.last - 1;
        construct(start.cur, copy);
    }
    catch(...) {
        // 出现错误, 则回滚到以前状态
        start.set_node(start.node + 1);
        start.cur = start.frist;
        deallocate_node(*(start.node - 1));
        throw;
    }
}

template <class T, class Alloc, size_t Bufsize>
void deque<T, Alloc, Bufsize>::pop_back_aux() {
    // 释放最后的一个缓冲区
    deallocate_node(*finish.node);
    // 调整finish迭代器
    finish.set_node(finish.node - 1);
    finish.cur = finish.last - 1;
    destroy(finish.cur);
}

template <class T, class Alloc, size_t Bufsize>
void deque<T, Alloc, Bufsize>::pop_front_aux() {
    destroy(start.cur);
    // 释放最前的一个缓冲区
    deallocate_node(*start.node);
    // 调整start迭代器
    start.set_node(start.node + 1);
    start.cur = start.frist;
}

// 重新分配map空间函数
template <class T, class Alloc , size_t Bufsize>
void deque<T, Alloc, Bufsize>::reallocater_map(size_type node_to_add, bool add_at_front) {
    size_type old_num_nodes = finish.node - start.node + 1;
    size_type new_num_nodes = old_num_nodes + node_to_add;

    map_pointer new_start;
    // 原map有足够的大小
    if(map_size > 2 * new_num_nodes) {
        new_start = map + (map_size - new_num_nodes) / 2 + (add_at_front ? node_to_add : 0);
        if(new_start < start.node)
            MYSTL::copy(start.node, finish.node + 1, new_start);
        else
            MYSTL::copy_backward(start.node, finish.node + 1, new_start);
    }else {
        // 原map没有足够的大小，需要重新分配一个更大的map
        size_type new_map_size = map_size + (map_size > node_to_add ? map_size : node_to_add) + 2;
        // 配置一块空间，准备给新map
        map_pointer new_map = map_allocator::allocate(new_map_size);
        new_start = new_map + (new_map_size - new_num_nodes) / 2 + (add_at_front ? node_to_add : 0);
        // 拷贝原map内容
        MYSTL::copy(start.node, finish.node, new_start);
        // 释放原map
        map_allocator::deallocate(map, map_size);
        // 设置新map大小和指针
        map = new_map;
        map_size = new_map_size;
    }
    // 重新设置迭代器 
    start.set_node(new_start);
    finish.set_node(new_start + old_num_nodes - 1);
}

// 清空队列
/**
 *   用来清空整个队列，但会保留一个缓冲区
*/
template <class T, class Alloc, size_t Bufsize>
void deque<T, Alloc, Bufsize>::clear() {
    // 头尾的迭代器一定是满元素，则全部回收
    for(map_pointer node = start.node + 1; node < finish.node; ++node)
    {
        // 释放元素
        destroy(*node, *node + iterator::buffer_size());
        // 释放内存缓冲区
        data_allocator::deallocate(*node, iterator::buffer_size());
    }
    
    // 检查头尾迭代器是否是一个
    if(start.node != finish.node) {
        // 头尾缓冲器不为同一个则分别释放头尾迭代器中内存
        destroy(start.frist, start.last);
        destroy(finish.frist, finish.last);
        // 释放尾部迭代器缓冲区，保留头缓冲区
        data_allocator::deallocate(*finish.node, iterator::buffer_size());
    }else {
        destroy(start.frist, start.last);
    }
    // 调整尾部迭代器
    finish = start;
}

// 清除pos所指元素
template<class T, class Alloc, size_t BufSize>
typename deque<T, Alloc, BufSize>::iterator deque<T, Alloc, BufSize>::erase(iterator pos) {
    iterator next = pos;
    ++next;
    difference_type index = pos - start;
    if(index < (size() >> 1) ) {
        // 移动元素小于一般，则移动前面元素
        copy_backward(start, pos, next);
        pop_front();
    }else {
        copy(next, finish, pos);
        pop_back();
    }
    return start + index;
}

// 清除[first, last)所指元素
template<class T, class Alloc, size_t BufSize>
typename deque<T, Alloc, BufSize>::iterator deque<T, Alloc, BufSize>::erase(iterator first, iterator last) {
    // 如果清除整个区域则调用clear
    if(first == start && last == finish) {
        clear();
        return finish;
    } else {
        difference_type n = last - first;   // 需要清除的缓冲区长度
        difference_type elems_before = first - start;   // 清楚起始开头位置
        if( elems_before < (size() - n) / 2) {
            MYSTL::copy_backward(start, first, last);
            iterator new_start = start + n;
            destroy(start, first);
            // 将map缓冲区回收
            for(map_pointer cur = start.node ; cur < new_start.node; ++cur)
                data_allocator::deallocate(*cur, iterator::buffer_size());
            start = new_start;
        } else {    // 需要清除区域后后方元素较少
            MYSTL::copy(last, finish, first);
            iterator new_finish = finish - n;
            destroy(new_finish, finish);
            // 将map缓冲区回收
            for(map_pointer cur = new_finish.node + 1; cur <= finish.node; ++cur)
                data_allocator::deallocate(*cur, iterator::buffer_size());
            finish = new_finish;
        }
        return start + elems_before;
    }
}

template<class T, class Alloc, size_t BufSize>
typename deque<T, Alloc, BufSize>::iterator deque<T, Alloc, BufSize>::insert_aux(iterator position, const value_type& x) 
{
    difference_type index = position - start;   //插入点之前的个数
    value_type x_copy = x;
    if(index < (size() >> 1)) { // 插入点之前元素较少，移动先前元素
        push_back(front());     // 最前端加入一个与队头相同元素
        iterator front1 = start;
        ++front1;
        iterator front2 = front1;
        ++front2;
        position = start + index;
        iterator pos = position;
        ++pos;
        copy(front2, pos, front1); 
    }else { // 插入点之后元素较少，移动后面的元素
        push_back(back());
        iterator back1 = finish;
        --back1;
        iterator back2 = back1;
        --back2;
        position = start + index;
        copy_backward(position, back2, back1);
    }
    *position = x_copy;
    return position;
}

#endif
