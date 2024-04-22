#ifndef _TYPE_TRAITS_H
#define _TYPE_TRAITS_H

/**
 *   利用 traits 技法改善对类型复制，构造，析构等效率
*/

// 两个空的结构体，不会带来额外的负担，却又能利用参数推到选择合适的重载函数
struct __true_type { };
struct __false_type { };

/**
 *  SGI默认将所有内嵌类型定义为 false_type
 *  默认定义最保守的值
*/
template <typename type>
struct __type_traits {
    typedef __true_type     this_dummy_member_must_be_first;

    typedef __false_type    has_trivial_default_constructor;
    typedef __false_type    has_trivial_copy_constructor;
    typedef __false_type    has_trivial_assignment_operator;
    typedef __false_type    has_trivial_destructor;
    typedef __false_type    is_POD_type;
};

/**
 *  利用 template 特例化对内置类型特例化
*/
// char
struct __type_traits<char> {
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_operator;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
};

//signed char
struct __type_traits<signed char> {
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_operator;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
};

//unsigned char
struct __type_traits<unsigned char> {
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_operator;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
};

//short
struct __type_traits<short> {
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_operator;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
};

//unsigned short
struct __type_traits<unsigned short> {
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_operator;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
};

// int
struct __type_traits<int> {
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_operator;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
};

// unsignde int
struct __type_traits<unsigned int> {
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_operator;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
};

// long
struct __type_traits<long> {
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_operator;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
};

// unsigned long
struct __type_traits<unsigned long> {
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_operator;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
};

// float
struct __type_traits<float> {
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_operator;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
};

// double
struct __type_traits<double> {
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_operator;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
};

// long double
struct __type_traits<long double> {
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_operator;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
};

// C 原生指针
template <typename T>
struct __type_traits<T *> {
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_operator;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
};


#endif
