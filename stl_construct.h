#ifndef _STL_CONSTRUCT_H
#define _STL_CONSTRUCT_H
/**
 *  该文件是空间配置器中的构造/析构头文件，负责对内存空间使用高效的construct()和destroy()
 *  使用__type_true 和 __type_false 判别是否对该类型有特化版本。
 *  对特化版本使用高效的construct()/destroy()。
*/
#include<new.h>


template<typename T1, typename T2>
inline void construct(T1* p, const T2& value)
{
    // 调用placement new 在p地址上构造T1以T2为初值
    new (p) T1(value);
}

template<typename T>
inline void destroy(T* pointer)
{
    pointer->~T();      //调用dtor ~T()
}



#endif