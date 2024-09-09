#ifndef __STL_RB_TREE_H
#define __STL_RB_TREE_H

/**
 *      红黑树 也是一种二叉搜索树，其结点也是左孩子小于该结点，右孩子大于该结点，且每个结点被赋予一种颜色（红或者黑）
 *      且红黑树满足以下四种规则
 *      1. 根结点必须为黑色
 *      2. 一个结点颜色为红色，其父节点必须为黑色
 *      3. 任一结点至叶结点的任何路径，其所含之黑结点必须相同
 * 
*/

#include "stl_alloc.h"
#include "stl_construct.h"
#include "stl_iterator.h"
#include <utility>

typedef bool __rb_tree_color_type;
const __rb_tree_color_type __rb_tree_red = false;
const __rb_tree_color_type __rb_tree_black = true;

struct __rb_tree_base_node 
{
    typedef __rb_tree_color_type color_type;
    typedef __rb_tree_base_node* base_ptr;

    color_type color;
    base_ptr parent;
    base_ptr left;
    base_ptr right;
    
    static base_ptr minimum(base_ptr x)
    {
        while(x->left != nullptr)
            x = x->left;
        return x;
    }

    static base_ptr maximum(base_ptr x)
    {
        while(x->right != nullptr)
            x = x->right;
        return x;
    }
};

template <class Value>
struct __rb_tree_node : public __rb_tree_base_node
{
    typedef __rb_tree_node<Value>* link_type;
    Value value_field; 
};

struct __rb_tree_base_iterator
{
    typedef __rb_tree_base_node::base_ptr base_ptr;
    typedef bidirectional_iterator_tag iterator_category;
    typedef ptrdiff_t difference_type;

    base_ptr node;      // 连接一个容器节点

    // 符合二叉搜索树的 increament, 被用于operator++ 调用
    void increment()
    {
        if(node->right != nullptr)
        {
            // 节点有右孩子，则找到右孩子子树最左节点
            node = node->right;
            while(node->left != nullptr)
                node = node->left;
        }else {
            // 节点没有右孩子，父节点的右孩子且该父节点右孩子不等于node节点
            base_ptr y = node->parent;
            while(node == y->right)
            {
                node = y;
                y = y->parent;
            }

            // 判断root没有右孩子或者该节点已经是最大的节点的情况
            // 由于rb_tree 的 root节点的特殊性，因此出现这个判断，root的parten 的right指向 maximum;
            if(node->right != y)
                node = y;
        }
    }

    // 符合二叉搜索树的 decreament, 被用于operator-- 调用
    void decrement()
    {   

        if(node->color == __rb_tree_red && node->parent->parent == node) {
            // 由于rb_tree的特殊性质,对于header节点，执行decrement出现一些不一样的状况,head->right 是maximum，header->left是 minimum, decrment 因此需要指向maximum
            node = node->right;
        }else if(node->left != nullptr)
        {
            // 该node节点有左孩子，因此找到左孩子最右边的节点则是上一个
            node = node->left;
            while(node->right != 0)
                node = node->right;
        }else {
            // 没有左孩子，则找父节点且该父节点的左孩子不等于该节点
            base_ptr y = node->parent;
            while(y->left == node)
            {
                node = y;
                y = y->parent;
            }
            node = y;
        }
    }

    bool operator==(const __rb_tree_base_iterator& x) const {
        return node == x.node;
    }

    bool operator!=(const __rb_tree_base_iterator& x) const {
        return node != x.node;
    }
};

template <class Value, class Ref, class Ptr>
struct __rb_tree_iterator : public __rb_tree_base_iterator
{
    typedef Value value_type;
    typedef Ref reference;
    typedef Ptr pointer;
    typedef __rb_tree_iterator<Value, Value&, Value*> iterator;
    typedef __rb_tree_iterator<Value, const Value&, const Value*> const_iterator; 
    typedef __rb_tree_iterator<Value, Ref, Ptr> self;
    typedef __rb_tree_node<Value>* link_type;

    __rb_tree_iterator() {}
    __rb_tree_iterator(link_type x) { node = x; }
    __rb_tree_iterator(const iterator& it) { node = it.node; }

    reference operator* () const {
        return ((link_type)(node))->value_field;
    }
    // 使用 obj->member 时，编译器转转换为 (obj.operator->())->member), 因此方便编译器层层解析，使用必须放回一个指针。
    /**
     *    一般常用于智能指针的封装，如果pointer指向一个类，
     *     则迭代器re_tree_iterator可以指针使用重载的operator->()访问pointer所指向类的成员函数和成员变量。
     *     迭代器被重载成一个pointer指针去访问pointer的类成员。
    */
    pointer operator-> () const {
        return &(operator*());
    }

    self& operator++() {
        increment();
        return *this;
    }
    self operator++(int) {
        self tmp = *this;
        decrement();
        return tmp;
    }
    self& operator--() {
        decrement();
        return *this;
    }
    self operator--(int) {
        self tmp = *this;
        decrement();
        return tmp;
    }
};

/**
 * 全局函数
 * 新节点必须为红色节点，如果父节点为红色节点则不符合RB-tree的性质,需要重新调整树形
 * 旋转树形，左旋转
 * 由于可能会选择根节点，因此根节点需要使用引用传值
 * x 为旋转点
*/
inline void __rb_tree_rotate_left(__rb_tree_base_node* x, __rb_tree_base_node*& root)
{
    // x为旋转点
    // 令y为x的右孩子
    __rb_tree_base_node* y = x->right;
    // 将y的左孩子设为x的右孩子
    x->right = y->left;
    if(y->left != nullptr){
        y->left->parent = x;
    }

    y->parent = x->parent;
    // x为根节点则y代替称为根节点
    // 由于root为引用传值，则直接使用root形参即可更新参数
    if(x == root)
        root = y;
    // 将x的父节点设为y的父节点
    if(x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;
    // 将x的父节点设置成y, x为y的左孩子
    y->left = x;
    x->parent = y;
}

/**
 *  全局函数
 *  旋转树形，右旋转
 *  x 为旋转点
*/
inline void __rb_tree_rotate_right(__rb_tree_base_node* x, __rb_tree_base_node*& root)
{
    // x为旋转点
    // 令y为x的左孩子
    __rb_tree_base_node* y = x->left;
    
    // 令x的左孩子为y的右孩子
    x->left = y->right;
    if(y->right != nullptr)
        y->right->parent = x;
    
    // 令x的父节点作为y的父节点
    y->parent = x->parent;
    // 如果x为root更新root节点
    // 由于root为引用传值，则直接使用root形参即可更新参数
    if(x == root)
        root = y;
    if(x == x->parent->left)
        x->parent->left = y;
    else 
        x->parent->right = y;
    // 令x的父亲为y
    y->right = x;
    x->parent = y;
}

/**
 *  全局函数
 *  重新令树形符合RB-tree的性质
 *  参数一为新插入的节点，参数二为root节点
*/
inline void __rb_tree_reblance(__rb_tree_base_node* x, __rb_tree_base_node*& root)
{
    // 新节点必为红色
    x->color = __rb_tree_red;
    // 如果父节点为红且没有遍历到根节点
    while(x != root && x->parent->color == __rb_tree_red)
    {
        if(x->parent == x->parent->left)
        {
            // 父节点为祖父节点的左节点
            // 令y节点为伯父节点
            __rb_tree_base_node* y = x->parent->right;
            if(y != nullptr && y->color == __rb_tree_red)
            {
                // 伯父节点和父节点都是红色，则将黑色节点下放
                x->parent->color = __rb_tree_black;
                y->color = __rb_tree_black;
                x->parent->parent->color = __rb_tree_red;
                x = x->parent->parent;
            }else 
            {
                // 没有伯父节点或者伯父节点为黑色
                if(x == x->parent->right)
                {
                    // x为父节点的右孩子，则需要先左旋
                    __rb_tree_rotate_left(x->parent, root);
                }

                x->parent->color = __rb_tree_black;
                x->parent->parent->color = __rb_tree_red;
                // 右旋转
                __rb_tree_rotate_right(x->parent->parent, root);
            }
        }else 
        {
            // 父节点为祖父节点的右节点
            // 令y节点为伯父节点
            __rb_tree_base_node* y = x->parent->left;
            if(y != nullptr && y->color == __rb_tree_red)
            {
                // x的父亲节点和伯父节点都为红色，则将黑色节点下放
                x->parent->color = __rb_tree_black;
                y->color = __rb_tree_black;
                x->parent->parent->color = __rb_tree_red;
                x = x->parent->parent;
            }else
            {
                // 伯父节点为黑色或者不存在
                if(x == x->parent->left)
                {
                    // x为右节点需要先右旋
                    __rb_tree_rotate_right(x->parent, root);
                }
                // 左旋
                x->parent->color = __rb_tree_black;
                x->parent->parent->color = __rb_tree_red;
                __rb_tree_rotate_left(x->parent->parent, root);
            }
        }
    } //while 循环调整结束，可能遍历到root使root为红色
    // 根节点调整为黑色
    x->color = __rb_tree_black;
}

template<class Key, class Value, class KeyOfValue, class Compare, class Alloc = __default_alloc_template<0> >
class rb_tree
{
protected:
    typedef void* void_pointer;
    typedef __rb_tree_base_node* base_ptr;
    typedef __rb_tree_node<Value> rb_tree_node;
    typedef simple_alloc<rb_tree_node, Alloc> rb_tree_node_allocator;
    typedef __rb_tree_color_type color_type;

public:
    typedef Value   value_type;
    typedef Key     key_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef rb_tree_node* link_type;
    typedef size_t size_type;
    typedef ptrdiff_t   difference_type;

protected:
    link_type get_node() { return rb_tree_node_allocator::allocate(); }
    void put_node(link_type p) { rb_tree_node_allocator::deallocate(p); }

    link_type create_node(const Value& x) {
        // 分配一个结点空间
        link_type tmp = get_node();
        // 构造结点值 
        construct(&tmp->value_field, x);
        return tmp;
    }

    // 复制一个结点的 值 和 色
    link_type clone_node(link_type x) {
        link_type tmp = create_node(x->value_field);
        tmp->color = x->color;
        tmp->left = 0;
        tmp->right = 0;
        return tmp;
    }

    // 销毁结点
    void destroy_node(link_type p) {
        destroy(&p->value_field);
        put_node(p);
    }

protected:
    // RB-tree存储header的指针, 并只用三个数据表示整颗RB-tree
    size_type node_count;
    link_type header;
    Compare key_compare;

    // 一下三个成员函数方便取得header的数据
    link_type& root() const { return (link_type&)header->parent; }
    link_type& leftmost() const { return (link_type&)header->left; }
    link_type& rightmost() const { return (link_type&)header->right; }

    // 六个函数用于方便取得x结点的信息
    static link_type& left(link_type x){ return (link_type&)(x->left); }
    static link_type& right(link_type x) { return (link_type&)(x->right); }
    static link_type& parent(link_type x) { return (link_type&)(x->parent); }
    static reference value(link_type x) { return x->value_field; }
    static const Key& key(link_type x) { return KeyOfValue()(value(x)); }
    static color_type& color(link_type x) { return (color_type&)(x->color); }

    static link_type& left(base_ptr x){ return (link_type)(x->left); }
    static link_type& right(base_ptr x) { return (link_type)(x->right); }
    static link_type& parent(base_ptr x) { return (link_type)(x->parent); }
    static reference value(base_ptr x) { return ((link_type)(x))->value_field; }
    static const Key& key(base_ptr x) { return KeyOfValue()(value(x)); }
    static color_type& color(base_ptr x) { return (color_type&)(x->color); }

    static link_type minimum(link_type x) {
        return (link_type) __rb_tree_base_node::minimum(x);
    }
    static link_type maximum(link_type x) {
        return (link_type) __rb_tree_base_node::maximum(x);
    }

public:
    typedef __rb_tree_iterator<value_type, reference, pointer> iterator;

private:
    iterator __insert(base_ptr x, base_ptr y, const value_type& v);
    link_type __copy(link_type x, link_type p);
    // void __erase(link_type x);

    void init() {
        header = get_node();
        color(header) = __rb_tree_red;

        root() = 0;
        leftmost() = header;
        rightmost() = header;
    }

public:
    rb_tree(const Compare& comp = Compare() ) : node_count(0), key_compare(comp) { init(); }
    rb_tree(const rb_tree<Key, Value, KeyOfValue, Compare, Alloc>& rb) : node_count(0), key_compare(Compare()) {
        init();
        if(rb.root() == nullptr) return;
        else {
            root() = __copy(rb.root(), header);
            leftmost() = minimum(root());
            rightmost() = minimum(root());
        }
        node_count = rb.node_count;
    }

    ~rb_tree() { 
        clear();
        put_node(header);
    }

    rb_tree<Key, Value, KeyOfValue, Compare, Alloc>& operator=(const rb_tree<Key, Value, KeyOfValue, Compare, Alloc>& x) {
        if(this != &x) {
            clear(); // 以前的树
            key_compare = x.key_compare;
            if(x.root()) {
                header = x.header;
                node_count = x.node_count;
            }else {
                init();
            }
        }
        return *this;
    }

public:
    Compare key_comp() const { return key_compare; }
    iterator begin() { return leftmost(); }
    iterator end() { return header; }
    bool empty() const { return node_count == 0; }
    size_type size() const { return node_count; }
    void clear();   // 清空整个树

    iterator insert_equal(const value_type& v);
    std::pair<iterator, bool> insert_uniqual(const value_type& x);

    // 二叉搜索的查找方式去其他不同，红黑树自己的二叉搜索查找方式
    iterator find(const Key& k);
};

template<class Key, class Value, class KeyofValue, class Compare, class Alloc>
void rb_tree<Key, Value, KeyofValue, Compare, Alloc>::clear() {

}

/**
 *    二叉搜索树的查找方式
 *  遇到Key大于的向 "左"走，并更新y
 *  遇到Key小于向 "右"走，
 *  找到是否是在大于k的所有节点中，Key最小的一个，如果Key也不大于K则Key == K
*/
template<class Key, class Value, class KeyOfValue, class Compare, class Alloc>
typename rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::iterator
rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::find(const Key& k) {
    link_type y = header;
    link_type x = root();
    while(x != nullptr) {
        if(!key_compare( key(x), k ) ) {
            // key(x) 大于或等于K，向树左边进行搜索
            y = x;
            x = left(x);
        }else {
            // x 小于 K, 向树右边进行搜索
            x = right(x);
        }
    }
    iterator j = iterator(y);
    return ( (j == end()) || key_compare(k, key(j.node)) ) ? end() : j;
}

template<class Key, class Value, class KeyofValue, class Compare, class Alloc>
typename rb_tree<Key, Value, KeyofValue, Compare, Alloc>::iterator
rb_tree<Key, Value, KeyofValue, Compare, Alloc>::insert_equal(const value_type& v) {
    link_type y = header;
    link_type x = root();
    while(x != 0)
    {
        y = x;
        x = key_compare(KeyofValue()(v), key(x)) ? y->left : y->right;
    }
    return __insert(x, y, v);
}

template<class Key, class Value, class  KeyofValue, class Compare, class Alloc>
std::pair< typename rb_tree<Key, Value, KeyofValue, Compare, Alloc>::iterator, bool >
rb_tree<Key, Value, KeyofValue, Compare, Alloc>::insert_uniqual(const value_type& v)
{
    link_type y = header;
    link_type x = root();
    bool comp = true;
    while(x != 0)
    {
        y = x;
        comp = key_compare( KeyofValue()(v), key(x) );
        x = comp ? left(x) : right(x);
    }

    // 离开while, y所指结点则为x的父节点
    iterator j = iterator(y);
    if(comp)
        if(j == begin())
            return std::pair<iterator, bool>(__insert(x, y, v), true);
        else
            --j;

    if( key_compare(key(j.node), KeyofValue()(v)) )
        return std::pair<iterator, bool>(__insert(x, y, v), true);
    
    return std::pair<iterator, bool>(j, false);
}

template<class Key, class Value, class KeyofValue, class Compare, class Alloc>
typename rb_tree<Key, Value, KeyofValue, Compare, Alloc>::iterator
rb_tree<Key, Value, KeyofValue, Compare, Alloc>::__insert(base_ptr x__, base_ptr y__, const value_type& v)
{
    link_type x = (link_type)x__;
    link_type y = (link_type)y__;
    link_type z;

    // key_compare 是键值大小比较函数，应该是个function object
    if( y == header || x != 0 || key_compare(KeyofValue()(v), key(y)) ) {
        z = create_node(v);
        left(y) = z;
        if(y == header) {
            root() = z;
            rightmost() = z;
        }

        if( leftmost() == y ) {
            leftmost() = z;
        }
    }else {
        z = create_node(v);
        right(y) = z;
        if(rightmost() == y) {
            rightmost() = z;
        } 
    }

    parent(z) = y;      // 设置新结点的父结点
    left(z) = 0;        // 设置新结点的左孩子
    right(z) = 0;       // 设置新结点的右孩子
    __rb_tree_reblance(z, header->parent);      // 调整红黑树使其符合规则
    ++node_count;       // 结点数增加
    return iterator(z);
}


/**
 *      将树中的节点从x开始递归地拷贝到新树中，并连接到p节点上
*/
template<class Key, class Value, class KeyOfValue, class Compar, class Alloc>
typename rb_tree<Key, Value, KeyOfValue, Compar, Alloc>::link_type
rb_tree<Key, Value, KeyOfValue, Compar, Alloc>::__copy(link_type x, link_type p) {
    link_type top = clone_node(x);
    top->parent = p;

    // 节点创建错误防止内存泄漏
    try {
        // 由于左节点深度一般都比右节点深度大，因此为了性能优化，对右节点使用递归处理
        // 对左节点使用循环处理
        if( x->right )
            top->right = __copy(right(x), top);

        p = top;
        x = left(x);
        // 对左节点循环处理
        while(x != nullptr) {
            link_type y = clone_node(x);
            p->left = y;
            y->parent = p;
            if( x->right )
                y->right = __copy(right(x), y);
            p = y;
            x = left(x);
        }
    } catch(...) {
        destroy_node(top);
        throw;
    }
    return top;
}

#endif
