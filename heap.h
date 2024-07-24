#ifndef __STL_HEAP_H
#define __STL_HEAP_H

#include "stl_iterator.h"

/**
 *      堆算法，堆算法的底层容器要求为RandmoAccessIterator迭代器，且内存要是可变化的内存，
 *      堆算法底层迭代器最好选用Vector<>容器
 *      且使用大根堆，堆头为堆最大元素值
*/

/**
 *  push_heap算法系列
 *  在调用中push_heap函数时，要加入元素的
*/
template<class RandomAccessIterator, class Distance, class T>
void __push_heap(RandomAccessIterator first, Distance holeIndex, Distance topIndex, T value) {
    // 找出父节点
    Distance parent = (holeIndex - 1) / 2;
    // 一直循环调整堆，父节点值小于value则父节点下方
    while(holeIndex > topIndex && *(first + parent) < value ) {
        // 堆没有循环完成且当前节点小于value,需要将当前节点下方
        *(first + holeIndex) = *(first + parent);
        // 继续向上调整
        holeIndex = parent;
        parent = (holeIndex - 1) / 2;
    }
    // 循环完成最后的holeIndex位置就是value在堆中位置
    *(first + holeIndex) = value;
}

template<class RandomAccessItera, class Distance, class T>
inline void __push_heap_aux(RandomAccessItera first, RandomAccessItera last, Distance, T) {
    // 再处理后转交一层工作
    __push_heap(first, Distance((last - first) - 1), Distance(0), T(*(last - 1)));
}

template <class RandomAccessIterator>
inline void push_heap(RandomAccessIterator first, RandomAccessIterator last) {
    // 且需要加入的堆的元素已经被压入堆中，真正的工作交给aux底层函数
    __push_heap_aux(first, last, distance_type(first), value_type(first));
}

/**
 *    pop_heap算法
 *    最次调整堆，但堆元素并没有出堆，而是在堆末尾，最后调用容器例如（pop_bakc)释放堆末尾元素
*/
template<class RandomAccessIterator, class Distance, class T>
void __adjust_heap(RandomAccessIterator first, Distance holdIndex, Distance len, T value) {
    Distance topIndex = holdIndex;
    Distance secondChild = 2 * holdIndex + 2;     // holdIndex节点的右孩子索引
    while(secondChild < len) {
        // 比较hold节点的两个孩子值，选出较大的一个
        if(*(first + secondChild) < *(first + secondChild - 1))
            --secondChild;
        *(first + holdIndex) = *(first + secondChild);
        // 调整holdIndex继续循环
        holdIndex = secondChild;
        secondChild = (holdIndex + 1) * 2; 
    }
    // 堆有节点没有右孩子，则让左节点向上调整
    if(secondChild == len){
        --secondChild;
        *(first + holdIndex) = *(first + secondChild);
        holdIndex = secondChild;
    }
    // 此时可能没有满足堆的规则，则调整一次堆
    __push_heap(first, holdIndex, topIndex, value);
}

template<class RandomAccessIterator, class Distance, class T>
inline void __pop_heap(RandomAccessIterator first, RandomAccessIterator last, RandomAccessIterator result, T value, Distance) {
    // 将堆头值设置为尾部值
    *result = *first;
    // 调整堆尾值
    __adjust_heap(first, Distance(0), Distance(last - first), value);
}

template<class RandomAccessIterator, class T>
inline void __pop_heap_aux(RandomAccessIterator first, RandomAccessIterator last, T) {
    __pop_heap(first, last -1, last -1, T(*(last -1)), distance_type(first));
}

template<class RandomAccessIterator>
inline void pop_heap(RandomAccessIterator first, RandomAccessIterator last) {
    __pop_heap_aux(first, last, value_type(first));
}


/**
 *   sort_heao排序算法
 *   每次获取堆中的最大值（堆头元素），持续对堆执行pop_heao操作。
*/
template<class RandomAccessIterator>
void sort_heap(RandomAccessIterator first, RandomAccessIterator last) {
    // 每次执行pop_heap操作，堆中极值就被放在堆尾，则减少堆元素在执行一次pop_heao，知道堆中只剩下一个元素
    // 但执行完成后堆的规则被破坏，原容器就不在是一个堆
    while(last - first > 1)
        pop_heap(first, last--);   // 执行一次pop_heap，堆的范围就减小一
}


/**
 *  make_heap算法系列，将迭代器[first, last)排列成一个heap
*/
template<class RandomAccessIterator, class T, class Distance>
void __make_heap(RandomAccessIterator first, RandomAccessIterator last, T, Distance ) {
    if(last - first < 2) return;    // 堆长度小于2则直接返回
    // 由于叶节点不需要重新排列，则找出第一个不为的子树，进行重新排序，然后从下到上进行重新调整，最后就是一个堆
    Distance len = last - first;
    Distance holdIndex = (len - 2) / 2;
    while(true) {
        // 重排holdIndex为根节点的子树
        __adjust_heap(first, holdIndex, len, T(*(first + holdIndex)));
        if(holdIndex == 0) return;
        holdIndex--;
    }
}

template<class RandomAccessIterator>
inline void make_heap(RandomAccessIterator first, RandomAccessIterator last) {
    __make_heap(first, last, value_type(first), distance_type(first));
}


#endif
