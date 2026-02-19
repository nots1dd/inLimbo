# RingBuffer (FIFO based)

## 1. What a Ring Buffer Is

A **ring buffer** (also called a circular buffer) is a fixed-capacity data structure that:

* Stores elements in a contiguous block of memory
* Uses wrap-around indexing instead of shifting data
* Supports efficient FIFO (first-in, first-out) access
* Avoids reallocations and data movement once created

The implementation is a **single-producer single-consumer (SPSC), lock-free, fixed-capacity FIFO ring buffer** optimized for high-frequency streaming use cases (audio).

## 2. Data Members and Their Roles

```cpp
std::unique_ptr<T[]> m_data;

size_t m_capacity{};
size_t m_mask{};

std::atomic<size_t> m_read{0};
std::atomic<size_t> m_write{0};
```

### `m_data`

* Backing storage of fixed size
* Allocated once during construction
* Never resized

### `m_capacity`

* Maximum number of elements the buffer can hold
* Rounded up to the next power of two for fast wrap-around

### `m_mask`

* Equals `m_capacity - 1`
* Used for wrap-around via bitwise masking:

  ```cpp
  index = counter & m_mask;
  ```

### `m_read`

* Monotonic read counter (not a direct index)
* Owned primarily by the consumer thread
* Converted to a physical index using `& m_mask`

### `m_write`

* Monotonic write counter (not a direct index)
* Owned primarily by the producer thread
* Converted to a physical index using `& m_mask`

## 3. Construction

```cpp
explicit RingBufferSPSC(size_t capacity)
{
  m_capacity = next_pow2(capacity);
  m_mask     = m_capacity - 1;
  m_data     = std::make_unique<T[]>(m_capacity);
}
```

* Rounds capacity up to a power of two
* Allocates storage upfront
* Capacity cannot change after construction
* No further allocations occur during use

## 4. Write Operation

```cpp
auto write(const T* src, size_t count) noexcept -> size_t
```

### Purpose

Attempts to write up to `count` elements into the buffer without overwriting unread data.

### Steps

1. Load current read counter (`acquire`) and write counter (`relaxed`)
2. Compute free space:

   ```cpp
   freeSpace = m_capacity - (w - r);
   ```
3. Clamp write size:

   ```cpp
   toWrite = min(count, freeSpace);
   ```
4. Compute physical write position:

   ```cpp
   wpos = w & m_mask;
   ```
5. Perform at most two contiguous memory copies (wrap-aware)

   * First chunk: from `wpos` to end of storage
   * Second chunk: from beginning of storage if wrap is needed
6. Publish updated write counter (`release`)

### Important Properties

* Never overwrites unread data
* Partial writes are allowed
* Lock-free: no mutex, no blocking
* Uses block copy (`memcpy`) instead of element-by-element writes
* Time complexity: **O(n)** where `n = toWrite`, with constant-time wrap math

## 5. Read Operation

```cpp
auto read(T* dst, size_t count) noexcept -> size_t
```

### Purpose

Attempts to read up to `count` elements from the buffer in FIFO order.

### Steps

1. Load current read counter (`relaxed`) and write counter (`acquire`)
2. Compute available elements:

   ```cpp
   avail = w - r;
   ```
3. Clamp read size:

   ```cpp
   toRead = min(count, avail);
   ```
4. Compute physical read position:

   ```cpp
   rpos = r & m_mask;
   ```
5. Perform at most two contiguous memory copies (wrap-aware)

   * First chunk: from `rpos` to end of storage
   * Second chunk: from beginning of storage if wrap is needed
6. Publish updated read counter (`release`)

### Important Properties

* Never reads invalid data
* Partial reads are allowed
* FIFO ordering preserved
* Lock-free: no mutex, no blocking
* Uses block copy (`memcpy`) instead of element-by-element reads
* Time complexity: **O(n)** where `n = toRead`, with constant-time wrap math

## 6. Index Wrap-Around Logic

Wrap-around is done using bitwise masking rather than modulo:

```cpp
pos = counter & (capacity - 1);
```

This requires:

* `capacity` must be a power of two
* `mask = capacity - 1`

This avoids modulo operations in hot paths and enables fast wrap-around.

## 7. Capacity and State Queries

### `capacity()`

Returns total capacity of the buffer.

### `available()`

Returns number of elements currently stored:

```cpp
available = write - read
```

### `space()`

Returns remaining free space:

```cpp
space = capacity - available
```

All of these:

* Are lock-free
* Run in **O(1)** time
* Use atomic loads (`acquire`) to avoid stale counters across threads

## 8. Clear Operation

```cpp
auto clear() noexcept -> void
{
  m_read.store(0, std::memory_order_release);
  m_write.store(0, std::memory_order_release);
}
```

### Effect

* Resets producer and consumer counters
* Effectively discards all buffered data
* Does not deallocate memory

### Use Cases

* Reset audio pipelines
* Flush buffers on state changes
* Recover from underruns

## 9. Thread Safety Model

* Designed strictly for:

  * One producer thread calling `write()`
  * One consumer thread calling `read()`
* Thread safety is achieved through:

  * Atomic monotonic counters
  * Acquire/release ordering for cross-thread visibility
  * Ownership discipline (producer advances `m_write`, consumer advances `m_read`)

This design prioritizes **low latency, deterministic throughput, and no lock contention**.

## 10. What This Ring Buffer Guarantees

* Fixed memory usage
* FIFO ordering
* No data overwrites
* No invalid reads
* No dynamic allocations after construction
* Very low overhead per call (no mutex, no per-element loops)
* Correct cross-thread visibility under the SPSC usage model

## 11. What This Ring Buffer Does Not Do

* Not safe for multiple producers or multiple consumers
* No blocking or waiting semantics
* No condition variables
* No overwrite-on-full behavior
* No support for non-trivially-copyable element types (by design)
* No iterator access
