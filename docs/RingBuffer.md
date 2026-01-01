# RingBuffer (FIFO based)

## 1. What a Ring Buffer Is

A **ring buffer** (also called a circular buffer) is a fixed-capacity data structure that:

* Stores elements in a contiguous block of memory
* Uses wrap-around indexing instead of shifting data
* Supports efficient FIFO (first-in, first-out) access
* Avoids reallocations and data movement once created

The implementation is *supposed* a **thread-safe, fixed-capacity FIFO ring buffer**.

## 2. Data Members and Their Roles

```cpp
std::vector<T>     m_buffer;
size_t             m_capacity;
size_t             m_readPos{0};
size_t             m_writePos{};
size_t             m_size{0};
mutable std::mutex m_mutex;
```

### `m_buffer`

* Backing storage of fixed size
* Allocated once during construction
* Never resized

### `m_capacity`

* Maximum number of elements the buffer can hold
* Equal to `m_buffer.size()`

### `m_readPos`

* Index of the next element to be read
* Advances on reads
* Wraps around using modulo arithmetic

### `m_writePos`

* Index of the next element to be written
* Advances on writes
* Wraps around using modulo arithmetic

### `m_size`

* Number of valid elements currently in the buffer
* Prevents ambiguity between empty and full states

### `m_mutex`

* Protects all shared state
* Ensures thread safety for concurrent reads/writes

## 3. Construction

```cpp
explicit RingBuffer(size_t capacity)
  : m_buffer(capacity), m_capacity(capacity) {}
```

* Allocates fixed storage upfront
* No further memory allocations occur during use
* Capacity cannot change after construction

## 4. Write Operation

```cpp
auto write(const T* data, size_t count) -> size_t
```

### Purpose

Attempts to write up to `count` elements into the buffer.

### Steps

1. Acquire mutex lock
2. Compute available free space:

   ```cpp
   space = m_capacity - m_size;
   ```
3. Clamp write size:

   ```cpp
   toWrite = min(count, space);
   ```
4. Copy elements sequentially into the buffer
5. Advance `m_writePos` with wrap-around
6. Increase `m_size`
7. Return number of elements written

### Important Properties

* Never overwrites unread data
* Partial writes are allowed
* Zero allocation
* Time complexity: **O(n)** where `n = toWrite`

## 5. Read Operation

```cpp
auto read(T* data, size_t count) -> size_t
```

### Purpose

Attempts to read up to `count` elements from the buffer.

### Steps

1. Acquire mutex lock
2. Clamp read size:

   ```cpp
   toRead = min(count, m_size);
   ```
3. Copy elements sequentially from the buffer
4. Advance `m_readPos` with wrap-around
5. Decrease `m_size`
6. Return number of elements read

### Important Properties

* Never reads invalid data
* Partial reads are allowed
* FIFO ordering preserved
* Time complexity: **O(n)** where `n = toRead`

## 6. Index Wrap-Around Logic

```cpp
pos = (pos + 1) % m_capacity;
```

This is the core ring buffer mechanism:

* When the index reaches the end of the array
* It wraps back to index `0`
* No shifting or reallocation occurs

## 7. Capacity and State Queries

### `capacity()`

Returns total capacity of the buffer.

### `available()`

Returns number of elements currently stored.

### `space()`

Returns remaining free space:

```cpp
capacity - available
```

All of these:

* Acquire the mutex
* Are thread-safe
* Run in **O(1)** time

## 8. Clear Operation

```cpp
void clear()
```

### Effect

* Resets read and write positions
* Sets size to zero
* Does not deallocate memory

### Use Cases

* Reset audio pipelines
* Flush buffers on state changes
* Recover from underruns

## 9. Thread Safety Model

* All public operations are mutex-protected
* Safe for:
  * Single producer + single consumer
  * Multiple producers + multiple consumers
* Lock granularity is coarse but safe

This prioritizes **correctness and simplicity** over lock-free complexity.

## 10. What This Ring Buffer Guarantees

* Fixed memory usage
* FIFO ordering
* No data overwrites
* No invalid reads
* Deterministic behavior (hopefully)
* Thread safety
* No dynamic allocations after construction

## 11. What This Ring Buffer Does Not Do

* No blocking or waiting semantics
* No condition variables
* No overwrite-on-full behavior
* No lock-free optimizations
* No iterator access

Most of it are deliberate design choices, tho some can be thanks to my idiocy
