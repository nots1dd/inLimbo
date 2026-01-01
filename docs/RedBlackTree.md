# RedBlackTree<Key, Value>

This implementation is a **fully self-contained, intrusive, pointer-based Red-Black Tree** with:

* Explicit NIL sentinel node
* Parent pointers
* Zero-allocation iterators
* Key-only and key–value iteration
* Insert and erase with full rebalancing
* Deterministic memory behavior

## 1. What a Red-Black Tree Is

A **Red-Black Tree (RBT)** is a self-balancing binary search tree with these guarantees:

* Every node is either **red** or **black**
* The root is always black
* Red nodes cannot have red children
* Every path from root to NIL has the same number of black nodes
* Tree height is bounded by `O(log n)`

This guarantees:

* `insert`, `erase`, `find`: **O(log n)**
* Predictable performance
* No degeneration into a linked list

## 2. Sentinel NIL Node Design

```cpp
NodeT* m_NIL;
```

Instead of using `nullptr` for missing children, the tree uses a **single shared sentinel node**:

* Color: BLACK
* Left/right pointers point to itself
* Simplifies balancing logic
* Avoids repeated null checks

All leaf children always point to `m_NIL`.

## 3. Node Structure

```cpp
template <typename Key, typename Value>
struct Node
{
  Key   key;
  Value value;
  char  color;
  Node* left;
  Node* right;
  Node* parent;
};
```

### Properties

* Stores both key and value directly
* Uses parent pointers (enables upward traversal)
* Color stored as a `char` (`'R'` / `'B'`)
* No virtual functions, no heap indirection

This keeps nodes:

* Compact
* Cache-friendly
* Deterministic

## 4. Core Tree State

```cpp
NodeT* m_root;
NodeT* m_NIL;
size_t m_size;
```

* `m_root` → root of the tree (never null)
* `m_NIL`  → sentinel leaf node
* `m_size` → number of stored elements

## 5. Rotations

### Left Rotation

```cpp
leftRotate(x)
```

Transforms:

```
    x              y
     \            / \
      y    ->    x   c
     / \          \
    b   c          b
```

Purpose:

* Rebalance the tree after insert/erase
* Preserve in-order traversal
* Update parent and child pointers carefully

### Right Rotation

Mirror of left rotation.

Both rotations:

* Run in **O(1)**
* Do not allocate memory
* Only rearrange pointers

## 6. Insertion Logic

### 6.1 Binary Search Tree Insert

```cpp
while (curr != m_NIL)
```

* Walk down the tree
* Maintain ordering: `left < parent < right`
* If key already exists → overwrite value
* New node is inserted as RED

This stage does **not** balance the tree.

### 6.2 Insert Fixup (`fixInsert`)

Handles all red-black violations.

Main cases:

1. **Red uncle** → recolor and move up
2. **Black uncle + triangle** → rotate to line
3. **Black uncle + line** → rotate and recolor

Loop continues until:

* Root reached, or
* Parent becomes black

Finally:

```cpp
m_root->color = BLACK;
```

Guarantees all RB invariants.

## 7. Erase Logic

### 7.1 Search Target Node

```cpp
NodeT* z = m_root;
```

Standard BST lookup.

### 7.2 Transplant Operation

```cpp
transplant(u, v)
```

Purpose:

* Replace subtree rooted at `u` with subtree rooted at `v`
* Adjust parent pointers correctly
* Central building block for erase

This avoids duplicating pointer logic in erase cases.

### 7.3 Erase Cases

1. **No left child**
2. **No right child**
3. **Two children**

In case 3:

* Find successor (`minimum(z->right)`)
* Swap structure, not values
* Preserve RB properties

### 7.4 Erase Fixup (`fixErase`)

Triggered only if a **black node is removed**.

Handles:

* Double-black propagation
* Red sibling
* Black sibling with red children
* Black sibling with black children

## 8. Iterators (Zero Allocation)

### 8.1 Design Goals

* No recursion
* No stack allocation
* No heap allocation
* Parent-pointer based traversal
* In-order traversal

### 8.2 Iterator Views

#### Key-only view

```cpp
KeyView<Key>
```

Used for:

```cpp
for (auto k : tree)
```

#### Key–value view

```cpp
KeyValueView<Key, Value>
```

Used for:

```cpp
for (auto [k, v] : tree.begin_kv())
```

### 8.3 Increment Logic

```cpp
operator++()
```

Algorithm:

1. If right child exists → go to leftmost of right subtree
2. Otherwise:
   * Move up using parent pointers
   * Stop when coming from a left child
3. If root passed → iterator becomes `end()`

This is **O(1)** per increment (gradually I believe but still neat nonetheless).

## 9. Destruction

```cpp
destroy(NodeT* n)
```

* Post-order traversal
* Deletes all nodes except `m_NIL`
* No memory leaks
* Deterministic cleanup

## 10. Complexity Guarantees

| Operation | Time Complexity |
| --------- | --------------- |
| insert    | O(log n)        |
| erase     | O(log n)        |
| lookup    | O(log n)        |
| iterate   | O(n)            |
| rotate    | O(1)            |

Memory:

* One allocation per inserted node
* No reallocations
* No hidden allocations

## 11. What This Implementation Does Well

* Iterator support without overhead (so it is damn easy to use, super hard to understand)
* Predictable performance

## 12. Known Trade-offs

* Recursive destroy (could be iterative if stack depth is a concern)

Other *niche* problems are not of our concern, but this one could possibly hurt if our tree is very big. Can cause stack implosion but so far it holds up pretty well (with ~1k nodes).

Open to having a safe guarded stack implementation should the need arise for it. (But in a realistic scenario it seems unlikely to me.)

## FAQ

1. Why does `NilNode` exist?

In case of a singular "type" (key) we want to explictly state to the RBT that we want to generate a red black tree with only nodes of type T (key's type)

Now why cant we just set `Value = void` in the template? As far as I understand it, it is bad practice (in previous commits I was doing this and was working as expected) but void is not an object type in C++ and tends to break generics so it is best to avoid it.

If we do use `void` we would have to further go through SFINAE hell and branch for `std::is_void_v<>` and shit. I am not a big fan of such verbose code that leads me nowhere. I was trying something similar in my previous commits and although it was working, my implementation for it was not satisfactory due to its verbose code that is not required as RBT is a small part of the project.

Unnecessary callbacks, very specific names (like inorder) that the user doesnt require so now SFINAE is out of the picture mostly and the iteration is much more C++ intuitive (even if the impl is ugly the outcome justifies it imo)

Finally, it is nice to **EXPLICITLY** mention that the value is `Nil` and that we only care about the `Key` section of it. It forces every function call using the RBT to understand that the second argument in the template is `NilNode` making it more code aware I suppose.
