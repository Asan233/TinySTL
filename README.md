# TinySTL

[![Finshing](https://img.shields.io/badge/Finshing-50%25%20Complete-brightgreen})]()
[![Release](https://img.shields.io/github/release/Asan233/TinySTL.svg)](https://github.com/Alinshans/MyTinySTL/releases) 
[![License](https://img.shields.io/badge/License-MIT%20License-blue.svg)](https://opensource.org/licenses/MIT) 
[![start](https://img.shields.io/github/stars/Asan233/TinySTL)]()

## 简介
基于`C++11` 的 `TinySTL`，为了深入`STL`原理的学习，结合`SIG STL` 的源码，实现了`STL` 容器的大部分接口功能。由于本项目作为新手练习用途，如有不规范的地方还请海涵。如有路过的大佬发现错处，还请在`Issues`中指出或者`Pull Requests` 改善代码，谢谢！

## 环境
* ubuntu20.04
* gcc 13.1.0

## 运行
1.克隆仓库
```
$ git clone git@github.com:Asan233/TinySTL.git
```

2.引入头文件

在项目工程文件中，引入需要容器的 `.h `文件即可。

## 目录
* 配置器（√）
    * allocator
        * allocate
        * deallocate
    * construct
        * construct
        * destroy

* 迭代器（√）
    * iterator

* 仿函数（暂时未完成）
    * functional
    * hash_funcional

* 容器 
    * vector（√）
    * list（√）
    * deque（√）
    * map（√）
    * set（√）
    * unordered_map
    * unordered_set
    * basic_string

* 配接器
    * stack
    * queue
    * priority_queue

* 算法与基本数据结构
    * 基本算法（√）
    * 数字算法（√）
    * set算法（√）
    * heap算法（√）
    * 红黑树（√）

## 测试
每个容器使用了`.cpp` 文件进行测试，可编译对应的`test_容器名.cpp` 文件进行容器测试。
