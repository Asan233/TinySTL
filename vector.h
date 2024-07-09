#ifndef _VECTOR_H
#define _VECTOR_H

#include "stl_alloc.h"
#include "stl_unintialized.h"
#include "algo.h"

/**
 *  vector 内存配置为如果当前内存空间不足，则重新配置当前内存空间为当前空间的两倍
*/


template <class T, class Alloc = __default_alloc_template<0>>
class vector {
public:
 // SGI 容器类型标准
    using value_type = T;
    using iterator   = T*;
    using const_iterator = const T*;    //由于对vector容器的操作，使用内嵌指针就能够很好的满足
    using pointer    = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

protected:
    // vector的内存配置器
    using data_allocator    =   simple_alloc<T, Alloc>;
    iterator start;             // 表示目前使用空间开始地址
    iterator finish;            // 表示目前使用空间的尾部
    iterator end_of_storage;    // 表示目前已分配空间的尾部

    // 在制定位置后面插入一个元素
    void insert_aux(iterator position, const value_type& x);

    // 内存重分配
    void deallocate() {
        if(start)
          data_allocator::deallocate(start, end_of_storage - start);
    }

    void fill_initialized(size_type n, const T& value)
    {
        start = allocate_and_fill(n, value);
        finish = start + n;
        end_of_storage = finish;
    }

    iterator allocate_and_fill(size_type n, const T& x) {
        iterator result = data_allocator::allocate(n);
        uninitialized_fill_n(result, n, x);
        return result;
    }

public:
    // vecotr 容器对外开放的访问接口
    iterator begin() { return start; }
    iterator end()   { return finish;  }
    size_type size() const {  return size_type( finish - start ); }
    size_type capacity()  const   { return size_type(end_of_storage - start ); }
    bool empty() const { return begin() == end(); }
    reference operator[] (size_type n) { return *( begin() + n ); }

    vector() : start(nullptr), finish(nullptr), end_of_storage(nullptr) {}
    vector( size_type n, const T& value ) { fill_initialized(n, value); }
    vector(int n, const T& value) { fill_initialized(n, value); }
    vector(long n, const T& value) { fill_initialized(n, value); }
    explicit vector(size_type n) { fill_initialize(n, T()); }

    ~vector() {
        destroy(start, finish);
        deallocate();
    }
    
    reference front() { return *begin(); }
    reference back()  { return *(end() - 1); }

    void push_back(const value_type& x) {
        if(finish != end_of_storage) {
            construct(finish, x);
            ++finish;
        }
        else
            insert_aux(end(), x);
    }

/*
    // 右值完美转发
    void push_back(value_type&& x) {
        if(finish != end_of_storage) {
            construct(finish, std::forward<value_type>(x) );
        } else {
            insert_aux(end(), std::forward<value_type>(x));
        }
    }
*/

    void pop_back() {
        --finish;
        destroy(finish);
    }
    iterator erase(iterator position) {
        if (position + 1 != end())
            copy(position + 1, finish, position);
        --finish;
        destroy(finish);
        return position;
    }
    void resize(size_type new_size, const T& x) {
        if (new_size < size() )
            erase(begin() + new_size, end());
        else
            insert(end(), new_size - size(), x);
    }
    void resize(size_type new_size) { resize(new_size, T()); }
    void clear() { erase(begin(), end()); }
    void insert(iterator position, size_type n, const T& x);
};

template<typename T, typename Alloc>
void vector<T, Alloc>::insert(iterator position, size_type n, const T& x) {
    if(n != 0) {    // 当 n != 0 才进行一下的操作
        if (size_type(end_of_storage - finish) >= n) {  // 当前剩余内容容量能够满足要求
            T x_copy = x;
            // 以下计算插入点之后的现有元素个数
            const size_type elems_after = finish - position;
            iterator old_finish = finish;

            if(elems_after > n) {   // 插入点之后元素个数多余插入元素个数
                uninitialized_copy(finish - n, finish, finish);
                finish += n;
                copy_backward(position, old_finish - n, old_finish);
                // 从插入点开始插入元素
                fill(position, n, x_copy);
            } else {    // 插入点之后元素个数少于插入元素个数
                uninitialized_fill_n(finish, n - elems_after, x_copy);
                finish = finish + n - elems_after;
                uninitialized_copy(position, old_finish, finish);
                finish += elems_after;
                fill(position, old_finish, x_copy);
            }
        } else {    // 容器剩余空间容量不能够放入元素
            // 决定新空间的容量
            const size_type old_size = size();
            const size_type len = old_size + (old_size > n ? old_size : n);
            // 配置新vector空间
            iterator new_start = data_allocator::allocate(len);
            iterator new_finish = new_start;
            try {
                new_finish = uninitialized_copy(start, position, new_start);
                new_finish = uninitialized_fill_n(new_finish, n, x);
                new_finish = uninitialized_copy(position, finish, new_finish);
            } catch(...) {
                // 发生异常则全部回滚
                destroy(new_start, new_finish);
                data_allocator::deallocate(new_start, len);
                throw;
            }
            // 释放以前旧的vector
            destroy(start, finish);
            deallocate();
            // 更新vector参数
            start = new_start;
            finish = new_finish;
            end_of_storage = new_start + len;
        }
    }
}

template<typename T, typename Alloc>
void vector<T, Alloc>::insert_aux(iterator position, const T & x) {
    if(finish != end_of_storage) {  //备用空间还有剩余
        construct(finish, *(finish - 1));
        ++finish;
        T x_copy = x;
        MYSTL::copy_backward(position, finish - 2, finish - 1);
        *position = x_copy;
    }
    else {  //备用空间已经用完重新申请备用空间
        const size_type old_size = size();
        const size_type len = old_size != 0 ? 2 * old_size : 1;

        // 申请新的内存空间
        iterator new_start = data_allocator::allocate(len);
        iterator new_finish = new_start;
        try {
            // 拷贝原内容到新的内存空间上
            new_finish = uninitialized_copy(start, position, new_start);
            // 插入新的元素
            construct(new_finish, x);
            ++new_finish;
            // 插入后半元素
            new_finish = uninitialized_copy(position, finish, new_finish);
        } catch(...) {
            // "commit or rollback"
            destroy(new_start, new_finish);
            data_allocator::deallocate(new_start, len);
            throw;
        }
        // 释放原地址空间
        destroy(begin(), end());
        deallocate();

        // 调整迭代器，使其指向新vector地址
        start = new_start;
        finish = new_finish;
        end_of_storage = new_start + len;
    }
}

#endif
