#ifndef _STL_ALLOC_H
#define _STL_ALLOC_H

#include <stdlib.h>
#include <iostream>

/**
 *  SGI采用二级配置器对内存分配进行管理，为了适配STL的内存分配接口，使用simple_alloc进行一层接口包装
 *  simple_alloc调用底层 Alloc 进行内存配置。
*/
template<typename T, typename Alloc>
class simple_alloc
{
    static T* allocate(size_t n)
    { return 0 == n ? 0 : (T*) Alloc::allocate(n * sizeof(T)); }

    static T* allocate(void)
    { return (T*) Alloc::allocate(sizeof(T)); }

    static void deallocate(T* p, size_t n)
    { if(0 != n) Alloc::deallocate(p, n * sizeof(T)); }

    static void deallocate(T* p)
    { Alloc::deallocate(p, sizeof(T)); }
};

/**
 *  第一级内存管理器
 *  对于超过128bytes分配请求都算做大内存
 *  直接调用placement new 分配大内存
 *  一般而言不需要模板参数，inst没有派上用场
*/

#define __THROW_BAD_ALLOC   std::cerr << "out of memory" << std::endl; exit(1)
template <int inst>
class __malloc_alloc_template
{
private:
    // oom : out of memory
    // 在内存不足时调用
    static void * oom_malloc(size_t);
    static void * oom_realloc(void *, size_t);
    // 内存分配失败时处理函数指针，指向执行的处理函数
    static void (* __malloc_alloc_oom_handler) ();
public:
    // 第一级配置器直接调用malloc()申请内存
    static void * allocate(size_t n)
    {
        void * result = malloc(n);
        if(0 == result) result = oom_malloc(n);
        return result;
    }

    static void deallocate(void* p, size_t n)
    {
        free(p);
    }

    static void * reallocate(void *p, size_t /* old size*/, size_t new_size)
    {
        p = realloc(p, new_size);
        if(0 == p) p = oom_realloc(p, new_size);
        return p;
    }

    /**
     *  基于set_new_handler().
     *  out_of_memory_handler()
     *  参数：传递一个void ()的函数指针f，将其设置为oom_handler
     *  返回: 返回以前的oom_handler
     * 
    */
   static void (*set_malloc_handler(void (*f)())) ()
   {
        void (*old) () = __malloc_alloc_oom_handler;
        __malloc_alloc_oom_handler = f;
        return old;
   }
};

// 初始化oom_handler为NULL
template<int inst>
void (* __malloc_alloc_template<inst>::__malloc_alloc_oom_handler)() = 0;

/**
 *  malloc_alloc_template
 *  oom_alloc等函数定义
*/
template<int inst>
void * __malloc_alloc_template<inst>::oom_malloc(size_t n)
{
    void (* my_malloc_handler) ();
    void * result;

    for(;;) {   //不断尝试释放、配置、再释放、再配置
        my_malloc_handler = __malloc_alloc_oom_handler;
        if(0 == my_malloc_handler) { __THROW_BAD_ALLOC; }
        (*my_malloc_handler)();     //调用处理程序释放内存
        result = malloc(n);
        if(result) return result;
    }
}

template<int inst>
void * __malloc_alloc_template<inst>::oom_realloc(void* p, size_t n)
{
    void (* my_malloc_handler) ();
    void * result;

    for(;;) {
        my_malloc_handler = __malloc_alloc_oom_handler;
        if(0 == my_malloc_handler) { __THROW_BAD_ALLOC; }
        (*my_malloc_handler)();
        result = realloc(p, n);
        if(result) return result;
    }
}

// 实例化一个inst为0的版本
typedef __malloc_alloc_template<0> malloc_alloc;

#endif