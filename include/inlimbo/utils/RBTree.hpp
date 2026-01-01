#pragma once

#include "Config.hpp"
#include <cstddef>

namespace utils
{

// ============================================================
// Constants
// ============================================================
static constexpr char RED   = 'R';
static constexpr char BLACK = 'B';

// RBT specific structs
namespace rbt
{

template <typename Key> struct KeyView
{
  const Key& key;
};

template <typename Key, typename Value> struct KeyValueView
{
  const Key& key;
  Value&     value;
};

// ============================================================
// Node structure
// ============================================================
template <typename Key, typename Value> struct Node
{
  Key   key;
  Value value;
  char  color{RED};
  Node* left{nullptr};
  Node* right{nullptr};
  Node* parent{nullptr};

  Node(const Key& k, const Value& v) : key(k), value(v) {}
};

struct NilNode
{
};

} // namespace rbt

// ============================================================
// Red-Black Tree
// ============================================================
template <typename Key, typename Value> class RedBlackTree
{
private:
  using NodeT = rbt::Node<Key, Value>;

  NodeT* m_root{nullptr};
  NodeT* m_NIL{nullptr};
  size_t m_size{0};

  // ----------------------------------------------------------
  // Rotations
  // ----------------------------------------------------------
  FORCE_INLINE void leftRotate(NodeT* x)
  {
    NodeT* y = x->right;
    x->right = y->left;

    if (y->left != m_NIL)
      y->left->parent = x;

    y->parent = x->parent;

    if (!x->parent)
      m_root = y;
    else if (x == x->parent->left)
      x->parent->left = y;
    else
      x->parent->right = y;

    y->left   = x;
    x->parent = y;
  }

  FORCE_INLINE void rightRotate(NodeT* x)
  {
    NodeT* y = x->left;
    x->left  = y->right;

    if (y->right != m_NIL)
      y->right->parent = x;

    y->parent = x->parent;

    if (!x->parent)
      m_root = y;
    else if (x == x->parent->right)
      x->parent->right = y;
    else
      x->parent->left = y;

    y->right  = x;
    x->parent = y;
  }

  // ----------------------------------------------------------
  // Fix insert
  // ----------------------------------------------------------
  void fixInsert(NodeT* k)
  {
    while (k->parent && k->parent->color == RED)
    {
      NodeT* gp = k->parent->parent;
      if (!gp)
        break;

      if (k->parent == gp->left)
      {
        NodeT* u = gp->right;
        if (u->color == RED)
        {
          k->parent->color = BLACK;
          u->color         = BLACK;
          gp->color        = RED;
          k                = gp;
        }
        else
        {
          if (k == k->parent->right)
          {
            k = k->parent;
            leftRotate(k);
          }
          k->parent->color = BLACK;
          gp->color        = RED;
          rightRotate(gp);
        }
      }
      else
      {
        NodeT* u = gp->left;
        if (u->color == RED)
        {
          k->parent->color = BLACK;
          u->color         = BLACK;
          gp->color        = RED;
          k                = gp;
        }
        else
        {
          if (k == k->parent->left)
          {
            k = k->parent;
            rightRotate(k);
          }
          k->parent->color = BLACK;
          gp->color        = RED;
          leftRotate(gp);
        }
      }
    }
    m_root->color = BLACK;
  }

  // ----------------------------------------------------------
  // Helpers
  // ----------------------------------------------------------
  auto minimum(NodeT* n) const -> NodeT*
  {
    while (n->left != m_NIL)
      n = n->left;
    return n;
  }

  // Detach u and glue v in its place
  //
  // transplant is used in the erase operation to replace one subtree with another.
  // It updates the parent pointers to ensure the tree remains connected correctly.
  void transplant(NodeT* u, NodeT* v)
  {
    if (!u->parent)
      m_root = v;
    else if (u == u->parent->left)
      u->parent->left = v;
    else
      u->parent->right = v;

    v->parent = u->parent;
  }

  void fixErase(NodeT* x)
  {
    while (x != m_root && x->color == BLACK)
    {
      if (x == x->parent->left)
      {
        NodeT* w = x->parent->right;
        if (w->color == RED)
        {
          w->color         = BLACK;
          x->parent->color = RED;
          leftRotate(x->parent);
          w = x->parent->right;
        }

        if (w->left->color == BLACK && w->right->color == BLACK)
        {
          w->color = RED;
          x        = x->parent;
        }
        else
        {
          if (w->right->color == BLACK)
          {
            w->left->color = BLACK;
            w->color       = RED;
            rightRotate(w);
            w = x->parent->right;
          }
          w->color         = x->parent->color;
          x->parent->color = BLACK;
          w->right->color  = BLACK;
          leftRotate(x->parent);
          x = m_root;
        }
      }
      else
      {
        NodeT* w = x->parent->left;
        if (w->color == RED)
        {
          w->color         = BLACK;
          x->parent->color = RED;
          rightRotate(x->parent);
          w = x->parent->left;
        }

        if (w->right->color == BLACK && w->left->color == BLACK)
        {
          w->color = RED;
          x        = x->parent;
        }
        else
        {
          if (w->left->color == BLACK)
          {
            w->right->color = BLACK;
            w->color        = RED;
            leftRotate(w);
            w = x->parent->left;
          }
          w->color         = x->parent->color;
          x->parent->color = BLACK;
          w->left->color   = BLACK;
          rightRotate(x->parent);
          x = m_root;
        }
      }
    }
    x->color = BLACK;
  }

  void destroy(NodeT* n)
  {
    if (n == m_NIL)
      return;
    destroy(n->left);
    destroy(n->right);
    delete n;
  }

public:
  // ==========================================================
  // Iterator (zero-allocation, parent-based)
  // ==========================================================
  template <typename View> class iterator_base
  {
    NodeT* node{nullptr};
    NodeT* nil{nullptr};

    static auto leftmost(NodeT* n, NodeT* nil) -> NodeT*
    {
      while (n->left != nil)
        n = n->left;
      return n;
    }

  public:
    using value_type = View;
    using reference  = View;

    iterator_base() = default;
    iterator_base(NodeT* n, NodeT* nil) : node(n), nil(nil) {}

    auto operator*() const -> reference
    {
      if constexpr (std::is_same_v<View, rbt::KeyView<Key>>)
        return {node->key};
      else
        return {node->key, node->value};
    }

    auto operator++() -> iterator_base&
    {
      if (node->right != nil)
      {
        node = leftmost(node->right, nil);
        return *this;
      }

      NodeT* p = node->parent;
      while (p && node == p->right)
      {
        node = p;
        p    = p->parent;
      }

      node = (p ? p : nil);
      return *this;
    }

    auto operator==(const iterator_base& o) const -> bool { return node == o.node; }
    auto operator!=(const iterator_base& o) const -> bool { return node != o.node; }
  };

  using key_iterator = iterator_base<rbt::KeyView<Key>>;
  using kv_iterator  = iterator_base<rbt::KeyValueView<Key, Value>>;

  auto begin() -> key_iterator { return key_iterator(minimum(m_root), m_NIL); }

  auto end() -> key_iterator { return key_iterator(m_NIL, m_NIL); }

  auto begin_kv() -> kv_iterator { return kv_iterator(minimum(m_root), m_NIL); }

  auto end_kv() -> kv_iterator { return kv_iterator(m_NIL, m_NIL); }

  // ==========================================================
  // Public API
  // ==========================================================
  RedBlackTree()
  {
    m_NIL        = new NodeT(Key{}, Value{});
    m_NIL->color = BLACK;
    m_NIL->left = m_NIL->right = m_NIL;
    m_root                     = m_NIL;
  }

  ~RedBlackTree()
  {
    destroy(m_root);
    delete m_NIL;
  }

  RedBlackTree(const RedBlackTree&)                    = delete;
  auto operator=(const RedBlackTree&) -> RedBlackTree& = delete;

  void insert(const Key& key, const Value& value)
  {
    NodeT* parent = nullptr;
    NodeT* curr   = m_root;

    while (curr != m_NIL)
    {
      parent = curr;
      if (key == curr->key)
      {
        curr->value = value;
        return;
      }
      curr = (key < curr->key) ? curr->left : curr->right;
    }

    auto* node = new NodeT(key, value);
    node->left = node->right = m_NIL;
    node->parent             = parent;

    if (!parent)
      m_root = node;
    else if (key < parent->key)
      parent->left = node;
    else
      parent->right = node;

    ++m_size;

    if (!node->parent)
      node->color = BLACK;
    else if (node->parent->parent)
      fixInsert(node);
  }

  auto erase(const Key& key) -> bool
  {
    NodeT* z = m_root;
    while (z != m_NIL && z->key != key)
      z = (key < z->key) ? z->left : z->right;

    if (z == m_NIL)
      return false;

    NodeT* y = z;
    NodeT* x;
    char   y_color = y->color;

    if (z->left == m_NIL)
    {
      x = z->right;
      transplant(z, z->right);
    }
    else if (z->right == m_NIL)
    {
      x = z->left;
      transplant(z, z->left);
    }
    else
    {
      y       = minimum(z->right);
      y_color = y->color;
      x       = y->right;

      if (y->parent == z)
        x->parent = y;
      else
      {
        transplant(y, y->right);
        y->right         = z->right;
        y->right->parent = y;
      }

      transplant(z, y);
      y->left         = z->left;
      y->left->parent = y;
      y->color        = z->color;
    }

    delete z;
    --m_size;

    if (y_color == BLACK)
      fixErase(x);

    return true;
  }

  [[nodiscard]] auto isEmpty() const noexcept -> bool { return m_root == m_NIL; }
  [[nodiscard]] auto size() const noexcept -> size_t { return m_size; }
};

} // namespace utils
