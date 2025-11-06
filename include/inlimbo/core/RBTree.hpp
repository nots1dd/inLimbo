#pragma once

#include "Config.hpp"
#include "toml/Parser.hpp"
#include <functional>
#include <utility>

namespace dirsort
{

// ============================================================
// Constants
// ============================================================
#define RED   'R'
#define BLACK 'B'

string DIRECTORY_FIELD = string(parser::parseTOMLField(
  PARENT_LIB, PARENT_LIB_FIELD_DIR)); // fetches the directory from the example config.toml for now
string DEBUG_LOG_PARSE = string(parser::parseTOMLField(PARENT_DBG, PARENT_DBG_FIELD_TAGLIB_PARSER_LOG));

// ============================================================
// Generic Node structure for the Red-Black Tree
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
// Generic Red-Black Tree (templated)
// ============================================================
// - Key: must be comparable (operator<)
// - Value: optional payload type
// - Supports callback traversal for metadata or user actions
// ============================================================

template <typename Key, typename Value = void>
class INLIMBO_API_CPP RedBlackTree
{
private:
    using NodeT = Node<Key, Value>;
    NodeT* root;
    NodeT* NIL;

    // Optional: user-defined callback for in-order traversal
    std::function<void(const Key&, Value&)> onVisit;

    // ========================================================
    // Helper rotations
    // ========================================================
    FORCE_INLINE void leftRotate(NodeT* x)
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

    FORCE_INLINE void rightRotate(NodeT* x)
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

    // ========================================================
    // Fixing insertion rule violations
    // ========================================================
    void fixInsert(NodeT* k)
    {
        while (UNLIKELY(k != root && k->parent->color == RED))
        {
            if (k->parent == k->parent->parent->left)
            {
                NodeT* u = k->parent->parent->right; // uncle
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
                NodeT* u = k->parent->parent->left; // uncle
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

    // ========================================================
    // Recursive traversal helper
    // ========================================================
    void inorderHelper(NodeT* node)
    {
        if (node == NIL) return;

        inorderHelper(node->left);
        if (onVisit)
            onVisit(node->key, node->value);
        inorderHelper(node->right);
    }

public:
    // ========================================================
    // Constructor / Destructor
    // ========================================================
    explicit RedBlackTree(std::function<void(const Key&, Value&)> visitCallback = nullptr)
        : onVisit(std::move(visitCallback))
    {
        NIL = new NodeT(Key{});
        NIL->color = BLACK;
        NIL->left = NIL->right = NIL;
        root = NIL;
    }

    ~RedBlackTree()
    {
        // TODO: add cleanup (delete all nodes)
    }

    // ========================================================
    // Core insert
    // ========================================================
    void insert(const Key& key, const Value& value = Value{})
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

    // ========================================================
    // Traversal
    // ========================================================
    void inorder() { inorderHelper(root); }

    // ========================================================
    // Configurable callback setter
    // ========================================================
    void setVisitCallback(std::function<void(const Key&, Value&)> cb)
    {
        onVisit = std::move(cb);
    }

    // ========================================================
    // Accessors
    // ========================================================
    [[nodiscard]] auto isEmpty() const noexcept -> bool { return root == NIL; }
    [[nodiscard]] auto getRoot() const noexcept -> NodeT* { return root; }
};

} // namespace dirsort
