#pragma once

#include "Config.hpp"
#include <functional>
#include <utility>
#include <string>

namespace dirsort
{

// ============================================================
// Constants
// ============================================================
#define RED   'R'
#define BLACK 'B'

extern std::string DIRECTORY_FIELD;
extern std::string DEBUG_LOG_PARSE;

// ============================================================
// Node structure (templated)
// ============================================================

template <typename Key, typename Value = void>
struct Node
{
    Key   key;
    Value value;
    char  color{RED};
    Node* left{nullptr};
    Node* right{nullptr};
    Node* parent{nullptr};

    explicit Node(const Key& k, const Value& v = Value{}) 
        : key(k), value(v) {}
};

// ============================================================
// Red-Black Tree (templated)
// ============================================================

template <typename Key, typename Value = void>
class INLIMBO_API_CPP RedBlackTree
{
private:
    using NodeT = Node<Key, Value>;
    NodeT* root;
    NodeT* NIL;

    std::function<void(const Key&, Value&)> onVisit;

    // Helper rotations
    FORCE_INLINE void leftRotate(NodeT* x);
    FORCE_INLINE void rightRotate(NodeT* x);
    void fixInsert(NodeT* k);
    void inorderHelper(NodeT* node);

public:
    explicit RedBlackTree(std::function<void(const Key&, Value&)> visitCallback = nullptr);
    ~RedBlackTree();

    void insert(const Key& key, const Value& value = Value{});
    void inorder();

    void setVisitCallback(std::function<void(const Key&, Value&)> cb);

    [[nodiscard]] auto isEmpty() const noexcept -> bool;
    [[nodiscard]] auto getRoot() const noexcept -> NodeT*;
};

// ============================================================
// Template Implementations
// (must be in header due to templates)
// ============================================================

template <typename Key, typename Value>
FORCE_INLINE void RedBlackTree<Key, Value>::leftRotate(NodeT* x)
{
    NodeT* y  = x->right;
    x->right = y->left;
    if (LIKELY(y->left != NIL))
        y->left->parent = x;

    y->parent = x->parent;
    if (x->parent == nullptr)
        root = y;
    else if (x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;

    y->left   = x;
    x->parent = y;
}

template <typename Key, typename Value>
FORCE_INLINE void RedBlackTree<Key, Value>::rightRotate(NodeT* x)
{
    NodeT* y  = x->left;
    x->left  = y->right;
    if (LIKELY(y->right != NIL))
        y->right->parent = x;

    y->parent = x->parent;
    if (x->parent == nullptr)
        root = y;
    else if (x == x->parent->right)
        x->parent->right = y;
    else
        x->parent->left = y;

    y->right  = x;
    x->parent = y;
}

template <typename Key, typename Value>
void RedBlackTree<Key, Value>::fixInsert(NodeT* k)
{
    while (UNLIKELY(k != root && k->parent->color == RED))
    {
        if (k->parent == k->parent->parent->left)
        {
            NodeT* u = k->parent->parent->right;
            if (u->color == RED)
            {
                k->parent->color = BLACK;
                u->color = BLACK;
                k->parent->parent->color = RED;
                k = k->parent->parent;
            }
            else
            {
                if (k == k->parent->right)
                {
                    k = k->parent;
                    leftRotate(k);
                }
                k->parent->color = BLACK;
                k->parent->parent->color = RED;
                rightRotate(k->parent->parent);
            }
        }
        else
        {
            NodeT* u = k->parent->parent->left;
            if (u->color == RED)
            {
                k->parent->color = BLACK;
                u->color = BLACK;
                k->parent->parent->color = RED;
                k = k->parent->parent;
            }
            else
            {
                if (k == k->parent->left)
                {
                    k = k->parent;
                    rightRotate(k);
                }
                k->parent->color = BLACK;
                k->parent->parent->color = RED;
                leftRotate(k->parent->parent);
            }
        }
    }
    root->color = BLACK;
}

template <typename Key, typename Value>
void RedBlackTree<Key, Value>::inorderHelper(NodeT* node)
{
    if (node == NIL) return;
    inorderHelper(node->left);
    if (onVisit)
        onVisit(node->key, node->value);
    inorderHelper(node->right);
}

template <typename Key, typename Value>
RedBlackTree<Key, Value>::RedBlackTree(std::function<void(const Key&, Value&)> visitCallback)
    : onVisit(std::move(visitCallback))
{
    NIL = new NodeT(Key{});
    NIL->color = BLACK;
    NIL->left = NIL->right = NIL;
    root = NIL;
}

template <typename Key, typename Value>
RedBlackTree<Key, Value>::~RedBlackTree()
{
    // TODO: recursive cleanup
}

template <typename Key, typename Value>
void RedBlackTree<Key, Value>::insert(const Key& key, const Value& value)
{
    auto* new_node = new NodeT(key, value);
    new_node->left = new_node->right = NIL;

    NodeT* parent = nullptr;
    NodeT* current = root;

    while (current != NIL)
    {
        parent = current;
        current = (new_node->key < current->key) ? current->left : current->right;
    }

    new_node->parent = parent;

    if (parent == nullptr)
        root = new_node;
    else if (new_node->key < parent->key)
        parent->left = new_node;
    else
        parent->right = new_node;

    if (new_node->parent == nullptr)
    {
        new_node->color = BLACK;
        return;
    }

    if (new_node->parent->parent == nullptr)
        return;

    fixInsert(new_node);
}

template <typename Key, typename Value>
void RedBlackTree<Key, Value>::inorder()
{
    inorderHelper(root);
}

template <typename Key, typename Value>
void RedBlackTree<Key, Value>::setVisitCallback(std::function<void(const Key&, Value&)> cb)
{
    onVisit = std::move(cb);
}

template <typename Key, typename Value>
auto RedBlackTree<Key, Value>::isEmpty() const noexcept -> bool
{
    return root == NIL;
}

template <typename Key, typename Value>
auto RedBlackTree<Key, Value>::getRoot() const noexcept -> typename RedBlackTree<Key, Value>::NodeT*
{
    return root;
}

} // namespace dirsort
