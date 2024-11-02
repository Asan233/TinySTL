#ifndef __STL_HASH_H
#define __STL_HASH_H

#include "stl_iterator.h"
#include "stl_alloc.h"
#include "vector.h"
#include "algo.h"
#include "stl_pair.h"

//实践表明，如果hash表的大小为质数，hash碰撞的概率会极大地减少
static const int __stl_num_primes = 28;
static const unsigned long __stl_prime_list[__stl_num_primes] =
{
  53ul,         97ul,         193ul,       389ul,       769ul,
  1543ul,       3079ul,       6151ul,      12289ul,     24593ul,
  49157ul,      98317ul,      196613ul,    393241ul,    786433ul,
  1572869ul,    3145739ul,    6291469ul,   12582917ul,  25165843ul,
  50331653ul,   100663319ul,  201326611ul, 402653189ul, 805306457ul, 
  1610612741ul, 3221225473ul, 4294967291ul
};
// 选出下一个质数
inline unsigned long __stl_next_prime(unsigned long n){
    const unsigned long* first = __stl_prime_list;
    const unsigned long* last = __stl_prime_list + __stl_num_primes;
    const unsigned long* pos = MYSTL::lower_bound(first, last, n);
    return pos == last ? *(last - 1) : *pos;
}


template<class Value>
struct __hashtable_node
{
    __hashtable_node* next;
    Value val;
};

template<class Value, class Key, class HashFcn, class ExtractKey, class EqualKey, class Alloc>
class hashtable;

template<class Value, class Key, class HashFcn, class ExtractKey, class EqualKey, class Alloc>
struct __hashtable_iterator{
    typedef hashtable<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc> hashtable;
    typedef __hashtable_iterator<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc> iterator;
    typedef __hashtable_node<Value> node;

    typedef forward_iterator_tag    iterator_category;
    typedef Value   value_type;
    typedef ptrdiff_t   difference_type;
    typedef size_t  size_type;
    typedef Value& reference;
    typedef Value* pointer;

    node* cur;      //迭代器目前保存的指针
    hashtable* ht;  //建立迭代器与hashtable的联系

    __hashtable_iterator(node* n, hashtable* tab) : cur(n), ht(tab) {}
    __hashtable_iterator() {}

    reference operator*() { return (cur->val); }

    // 返回一个Value类型的指针，如果Value类型的指针定义了->, 则C++编译器则会递归调用Value类型的operator，知道得到一个真正的指针后执行->操作
    pointer operator->() { return &(operator*()); }

    bool operator==(const iterator& it) const { return cur == it.cur; }
    bool operator!=(const iterator& it) const { return cur != it.cur; }
    iterator& operator++();
    iterator operator++(int);
};

template<class Value, class Key, class HashFcn, class ExtrackKey, class EqualKey, class Alloc>
typename __hashtable_iterator<Value, Key, HashFcn, ExtrackKey, EqualKey, Alloc>::iterator&
__hashtable_iterator<Value, Key, HashFcn, ExtrackKey, EqualKey, Alloc>::operator++() {
    const node* old = cur;
    cur = cur->next;
    if(!cur) {
        // 根据元素值算出当前的bucket
        size_type bucket = ht->bkt_num(old->val);
        while(!cur && ++bucket < ht->buckets.size())
            cur = ht->buckets[bucket];
    }
    return *this;
}

template<class Value, class Key, class HashFcn, class ExtrackKey, class EqualKey, class Alloc>
typename __hashtable_iterator<Value, Key, HashFcn, ExtrackKey, EqualKey, Alloc>::iterator
__hashtable_iterator<Value, Key, HashFcn, ExtrackKey, EqualKey, Alloc>::operator++(int) {
    iterator tmp = *this;
    ++*this;    // 调用operator++()
    return tmp;
}


template<class Value, class Key, class HashFcn, class ExtracKey,  class EqualKey, class Alloc=__default_alloc_template<0>>
class hashtable{
public:
    typedef HashFcn hasher;
    typedef EqualKey key_equal;
    typedef size_t size_type;

    typedef Value       value_type;
    typedef Key         key_type;
    typedef ptrdiff_t   difference_type;
    typedef value_type* pointer;
    typedef const value_type*   const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;

// 迭代器
public:
    typedef __hashtable_iterator<Value, Key, HashFcn, ExtracKey, EqualKey, Alloc> iterator;
    // 声明迭代器为友元，让其能够访问hashtable内部成员
    friend struct __hashtable_iterator<Value, Key, HashFcn, ExtracKey, EqualKey, Alloc>;
private:
    // 一般都为可调用对象
    hasher hash;
    key_equal equals;
    ExtracKey get_key;

    typedef __hashtable_node<Value> node;
    typedef simple_alloc<node, Alloc> node_allocator;

    vector<node*, Alloc> buckets;
    size_type num_elements;

    void resize(size_type num_element_hint);    // 检查buckets的大小，剩余空间不够则重新分配buckets的大小

    node* new_node(const value_type& obj)
    {
        node *n = node_allocator::allocate();
        n->next = 0;
        construct(&n->val, obj);
        return n;
    }

    void delete_node(node *n)
    {
        destroy(&n->val);
        node_allocator::deallocate(n);
    }

private:
    // 初始化哈希表的buckets
    void initialized_buckets(size_type n)
    {
        // 找到buckets最合适大小
        size_type n_buckets = next_size(n);
        //std::cout << n_buckets << std::endl;
        // 预留buckets大小避免vector重复分配内存,浪费系统时间
        buckets.reserve(n_buckets);
        buckets.insert(buckets.end(), n_buckets, (node *) 0);
        num_elements = 0;
    }

protected:
    // 插入元素，不允许重复
    pair<iterator, bool> insert_unique_noresize(const value_type& obj);
    // 插入元素，允许重复
    iterator insert_equal_noresize(const value_type& obj);

    // hash出元素在buckets中的位置
    size_type bkt_num_key(const value_type& obj, size_t n) const
    {
        return hash(obj) % n;
    }
public:
    size_type bucket_count() const { return buckets.size(); }
    // buckets中的最多可以用于多少bucket
    size_type max_bucket_count() const { return __stl_prime_list[__stl_num_primes - 1]; }
    // 返回最接近n并大于等于n的质数
    size_type next_size(size_type n) const { return __stl_next_prime(n); }

public:
    // construct
    // 传入初始化哈希表大小，哈希函数，Key值比较函数
    hashtable(size_type n, const HashFcn& hf, const EqualKey& eql) : hash(hf), equals(eql), get_key(ExtracKey()), num_elements(0) 
    {
        // 初始化bucket大小
        initialized_buckets(n);
    }

    // 插入元素，不允许重复
    pair<iterator, bool> insert_unique(const value_type& obj){
        resize(num_elements + 1);
        return insert_unique_noresize(obj);
    }
    // 插入元素，允许重复
    iterator insert_equal(const value_type& obj){
        resize(num_elements + 1);
        return insert_equal_noresize(obj);
    }

    /**
     *  Hash出元素在buckets中的位置
    */
    // 接受一个n作为buckets的size
    size_type bkt_num(const value_type& obj, size_t n) const
    {
        return bkt_num_key(get_key(obj), n);
    }
    // 只接受一个value_type
    size_type bkt_num(const value_type& obj) const
    {
        return bkt_num_key(get_key(obj), buckets.size());
    }

    // 情况buckets中的元素
    void clear();

    size_type size() const {
        return num_elements;
    }

    // 返回buckets中第一个bucket
    iterator begin()
    {
        for(size_type n = 0; n < buckets.size(); ++n)
            if(buckets[n] != 0)
                return iterator(buckets[n], this);
        
        return end();
    }

    iterator end()
    {
        return iterator(0, this);
    }

    void copy_from(const hashtable<Value, Key, HashFcn, ExtracKey, EqualKey, Alloc>& ht);

    // 通过key找到对应element
    iterator find(const key_type& key);

    // 找出键值为key的元素数量
    size_type count(const key_type& key) const;
};

template<class V, class K, class HF, class Ex, class Eq, class A>
void hashtable<V, K, HF, Ex, Eq, A>::clear()
{
    // 对一个bucket
    for(size_type i = 0; i < buckets.size(); ++i)
    {
        node *first = buckets[i];
        while(first != 0)
        {
            buckets[i] = first->next;
            delete_node(first);
            first = buckets[i];
        }
        buckets[i] = 0;
    }
    num_elements = 0;
}

 template<class V, class K, class HF, class Ex, class Eq, class A>
 void hashtable<V, K, HF, Ex, Eq, A>::resize(size_type num_element_hint)
 {
    // 判断是否重新分配buckets的条件有点奇特
    // 元素个数是否大于buckets.size
    const size_type old_size = buckets.size();
    if(num_element_hint > old_size){
        const size_type n = next_size(num_element_hint);    // 找出下一个质数
        if(n > old_size){   // 防止buckets已经到最大内存分配后，没有增长后又重新分配
            vector<node*, A> tmp(n, (node *)0);
            // 递归处理原来buckets中的node *
            for(size_type bucket = 0; bucket < buckets.size(); ++bucket) {
                node *first = buckets[bucket];
                while(first){
                    // hash出在新buckets中的位置
                    size_type new_bucket = bkt_num(first->val, n);
                    buckets[bucket] = first->next;
                    first->next = tmp[new_bucket];
                    tmp[new_bucket] = first;
                    first = buckets[bucket];
                }
            }
            buckets.swap(tmp);
        }
    }
 }

// 插入元素，允许重复
template<class V, class K, class HF, class Ex, class Eq, class A>
pair<typename hashtable<V, K, HF, Ex, Eq, A>::iterator, bool> 
hashtable<V, K, HF, Ex, Eq, A>::insert_unique_noresize(const value_type& obj)
{
    // Hash得到num bucket
    const size_type n = bkt_num(obj);
    node *first = buckets[n];   //  得到buckets头部
    
    // 如果有元素则进入以下循环
    for(node *cur = first; cur; cur = cur->next)
    {
        if(equals(get_key(cur->val), get_key(obj)))
            return pair<iterator, bool>(iterator(cur, this), false);
    }

    // 离开循环或者没有进入，则说明没有相同的元素
    node *tmp = new_node(obj);
    tmp->next = first;
    buckets[n] = tmp;
    ++num_elements;
    return pair<iterator, bool>(iterator(tmp, this), false);
}

// 插入元素，不允许重复
template<class V, class K, class HF, class Ex, class Eq, class A>
typename hashtable<V, K, HF, Ex, Eq, A>::iterator
hashtable<V, K, HF, Ex, Eq, A>::insert_equal_noresize(const value_type& obj)
{
    const size_type n = bkt_num(obj);
    node *first = buckets[n];   // 得到buckets头部
    
    for(node *cur = first; cur; cur = cur->next)
    {
        if(equals(get_key(cur->val), get_key(obj)))
        {
            node *tmp = new_node(obj);
            tmp->next = cur->next;
            cur->next = tmp;
            ++num_elements;
            return iterator(tmp, this);
        }
    }

    // 程序运行到这，说明没有key相等的元素
    node *tmp = new_node(obj);
    tmp->next = buckets[n];
    buckets[n] = tmp;
    ++num_elements;
    return iterator(tmp, this);
}

template<class V, class K, class HF, class Ex, class Eq, class A>
void hashtable<V, K, HF, Ex, Eq, A>::copy_from(const hashtable<V, K, HF, Ex, Eq, A>& ht){
    clear();
    // 对buckets进行resize()与ht.buckets一样的尺寸，容量大于则保留，容量小于则扩充
    resize(ht.size());
    // buckets已被clear(),buckets.end()就是开头
    //buckets.insert(buckets.end(), ht.buckets.size(), (node *)0);
    // 复制hashtable就是复制buckets以及buckets中的里面的链表元素
    for(size_type i = 0; i < ht.buckets.size(); ++i){
        if(const node *cur = ht.buckets[i]){
            node *copy = new_node(cur->val);
            buckets[i] = copy;
            
            for(node *next = cur->next; next;cur = next, next = cur->next){
                copy->next = new_node(next->val);
                copy = copy->next;
            }
        }
    }
    num_elements = ht.num_elements;     // 重新登录节点数量
}

template<class V, class K, class HF, class Ex, class Eq, class A>
typename hashtable<V, K, HF, Ex, Eq, A>::iterator
hashtable<V, K, HF, Ex, Eq, A>::find(const key_type& key){
    size_type n = bkt_num(key);
    node *first = buckets[n];
    for(; first && !equals(get_key(first->val), key); first = first->next) {}
    return iterator(first, this);
}

template<class V, class K, class HF, class Ex, class Eq, class A>
typename hashtable<V, K, HF, Ex, Eq, A>::size_type
hashtable<V, K, HF, Ex, Eq, A>::count(const key_type& key) const {
    size_type n = bkt_num(key);
    size_type result = 0;
    for(const node* cur = buckets[n]; cur; cur = cur->next){
        if(equals(get_key(cur->val), key))
            ++result;
    }
    return result;
}

#endif
