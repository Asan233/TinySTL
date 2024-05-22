#ifndef _STL_ALLOC_H
#define _STL_ALLOC_H

#include <stdlib.h>
#include <iostream>


// 构造器前置声明
template<int inst> class __malloc_alloc_template;
template<int inst> class __default_alloc_template;

/**
 *  SGI采用二级配置器对内存分配进行管理，为了适配STL的内存分配接口，使用simple_alloc进行一层接口包装
 *  simple_alloc调用底层 Alloc 进行内存配置。
*/
template<class T, class Alloc>
class simple_alloc
{
public:
    static T* allocate(size_t n)
    {   return 0 == n ? 0 : (T*) Alloc::allocate(n * sizeof(T));  }

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


/**
 *  第二级配置器
 *  二级内存配置器只配置小于128 betys的空间，超过128 betys的空间交给一级配置分配
 *  为了便于管理分配的空间都为8的倍数例如：8，16，24，.....，128;
 *  用一个free_list_link连接各个大小的空间
 *  
*/
enum { __ALIGN = 8 };
enum { __MAX_BYTES = 128 };
enum { __NFREELISTS = __MAX_BYTES / __ALIGN };

//与第一级配置器一样模板参数完全没有发挥作用
template<int inst>
class __default_alloc_template {
private:
    //将bytes 上调至8的倍数
    static size_t ROUND_UP(size_t bytes) {
        return ( (bytes + __ALIGN - 1) & (~(__ALIGN - 1)) );
    }

    // 使用union数据类型用作于内存地址
    // 这样可以不浪费内存空间
    union obj {
        union obj * free_list_link;
        char client_data[1];
    };

private:
    // 16 free-lists
    static obj * volatile free_list[__NFREELISTS];
    // 根据bytes选择合适的内存块
    static size_t FREELIST_INDEX(size_t bytes) {
        return ((bytes + __ALIGN - 1) & ~(__ALIGN - 1)) / __ALIGN - 1;
    }

    //返回一个大小为n的对象，并可能加入大小为 n 的其他区块到 free_list 中
    static void * refill(size_t n);

    //配置一大块空间，可容纳nobjs 个大小为 "size" 的区块
    //如果内存空间紧张，则配置的数量可能小于 nobjs 
    static char *chunk_alloc(size_t size, int &nobjs);

    //Chunk allocation state
    static char* start_free;    // 内存池起始位置
    static char* end_free;      // 内存池结束位置
    static size_t heap_size;

public:
    static void* allocate(size_t n);
    static void deallocate(void *p, size_t n);
    static void* reallocate(void *p, size_t, size_t new_size);
};

//__default_alloc初始值设置
template<int inst>
char *__default_alloc_template<inst>::start_free = 0;

template<int inst>
char *__default_alloc_template<inst>::end_free = 0;

template<int inst>
size_t __default_alloc_template<inst>::heap_size = 0;

template<int inst>
typename __default_alloc_template<inst>::obj * volatile
__default_alloc_template<inst>::free_list[__NFREELISTS] = 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

template<int inst>
void * __default_alloc_template<inst>::allocate(size_t n)
{
    obj * volatile * my_free_list;
    obj * result;

    //大于128 bytes调用第一级配置器
    if(n > (size_t) __MAX_BYTES ) {
        return (malloc_alloc::allocate(n));
    }

    //找到最合适的空间大小
    my_free_list = free_list + FREELIST_INDEX(n);
    result = *my_free_list;
    if(0 == result) {
        // 没找到可用的 free list，准备重新填充 free list;
        void *r = refill(ROUND_UP(n));
        return r;
    }
    // 取下free list 并调整
    *my_free_list = result->free_list_link;
    return result;
}

template<int inst>
void __default_alloc_template<inst>::deallocate(void *p, size_t n)
{
    obj *q = (obj *)p;
    obj * volatile *my_free_list;
    if(n > (size_t) 128) {
        malloc_alloc::deallocate(p, n);
        return;
    }
    // 寻找对应的free_list
    my_free_list = free_list + FREELIST_INDEX(n);
    // 调整free list
    q->free_list_link = (*my_free_list);
    *my_free_list = q;
}

/**
 *  假如 free list 中已经没有可用空间时
 *  refill 重新填充 free list
 *  新的空间将取自内存池( 由chunk_alloc()维护 )，从chunk_alloc()中取出的是一块连续的内存空间
 *  取出后自己进行将空间连接成 free list
*/
// 假设 n 已经被上调至8的倍数
template<int inst>
void * __default_alloc_template<inst>::refill(size_t n) {
    // 默认重新填充20个，可能应该内存紧张原因而不足
    int nobjs = 20;
    // 调用chunk_alloc()向内存池中请求内存，其中 nobjs 传递引用，返回实际分配的个数
    char * chunk = chunk_alloc(n, nobjs);
    obj * volatile * my_free_list;
    obj * result;
    obj * current_obj, * next_obj;
    int i;
    
    // nobj == 1 直接返回给调用者
    if(1 == nobjs) return chunk;
    // 否则将其他节点纳入free list
    my_free_list = free_list + FREELIST_INDEX(n);

    // 在chunk串联起free_list
    result = (obj * )chunk;     //返回给调用者
    // 指向下一块内存
    *my_free_list = next_obj = (obj *)(chunk + n);
    //串联chunk空间成为free_list
    for(i = 1; ;++i)
    {
        current_obj = next_obj;
        next_obj = (obj *)((char *)next_obj + n);
        if(nobjs - 1 == i) {
            // 将最后一块next指针设置为0
            current_obj->free_list_link = 0;
            break;
        }else {
            current_obj->free_list_link = next_obj;
        }
    }
    return result;
}

/**
 *  内存池
 *  从内存池中取出空间给 free list 是 chunk_alloc() 工作
 *  size已经是8的倍数，nbojs 通过引用传递，表示实际分配的空间数量
*/
template<int inst>
char * __default_alloc_template<inst>::chunk_alloc(size_t size, int & nbojs)
{
    char * result;
    size_t total_bytes = size * nbojs;
    size_t bytes_left = end_free - start_free;

    if(bytes_left >= total_bytes) {
        // 如果内存池剩余空间满足要求
        result = start_free;
        start_free += total_bytes;
        return (result);
    } else if (bytes_left >= size) {
        // 内存池容量不能完全满足要求，但是至少能配出一个size大小空间
        nbojs = bytes_left / size;
        total_bytes = nbojs * size;
        result = start_free;
        start_free += total_bytes;
        return (result);
    }else {
        // 内存池中空间连一块 size 大小空间都分配不出来
        // 需要向系统申请空间，申请大小一般为 2 * total_bytes + heap_size >> 4 大小
        size_t bytes_to_get = 2 * total_bytes + ROUND_UP((heap_size >> 4));
        // 先将内存池中剩余的碎片空间加入适当 free list 中
        if(bytes_left > 0) {
            // 寻找适当free list
            obj * volatile * my_free_list = free_list + FREELIST_INDEX(bytes_left);
            // 调整free list
            ((obj *)start_free)->free_list_link = *my_free_list;
            *my_free_list = (obj *)start_free;
        }

        // 配置heap空间，补充内存池空间
        start_free = (char *)malloc(bytes_to_get);
        if(0 == start_free) {
            // malloc() 申请空间失败
            int i;
            obj * volatile * my_free_list, *p;
            // 检查 free list 有没有大的空间分配
            for(i = size; i <= __MAX_BYTES; i += __ALIGN)
            {
                my_free_list = free_list + FREELIST_INDEX(i);
                p = *my_free_list;
                if(0 != p) {
                    // free list 中由内存块未被利用
                    // 将free list 中的空间归还给内存池
                    *my_free_list = p->free_list_link;
                    start_free = (char *) p;
                    end_free = start_free + i;
                    // 递归调用自己，调整nbojs分配空间
                    return (chunk_alloc(size, nbojs));
                }
            }
            // free_list 中也没有内存可用，malloc() 也没有内存能分配
            end_free = 0;
            // 调用第一级配置器，使用 out of memory 能不能分配出内存
            start_free = (char *)malloc_alloc::allocate(bytes_to_get);
            // malloc_alloc 不能分配时会抛出异常
        }
        // 更新内存池大小
        heap_size += bytes_to_get;
        end_free = start_free + bytes_to_get;
        return (chunk_alloc(size, nbojs));
    }
}


#endif