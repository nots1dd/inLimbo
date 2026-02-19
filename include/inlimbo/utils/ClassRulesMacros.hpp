#pragma once

// Some macros for me to not write ugly cpp
// syntax for move and copy semantics of class objects
// and lifetimes.

// --------------------------------------------
// Constructors / destructor
// --------------------------------------------
#define DEFAULT_CTOR(Class) Class() = default

#define NO_CTOR(Class) Class() = delete

#define DEFAULT_DTOR(Class) ~Class() = default

#define VIRTUAL_DTOR(Class) virtual ~Class() = default

#define NO_DTOR(Class) ~Class() = delete

// --------------------------------------------
// Copy semantics
// --------------------------------------------
#define ENABLE_COPY(Class)                  \
  Class(const Class&)            = default; \
  Class& operator=(const Class&) = default

#define DISABLE_COPY(Class)                \
  Class(const Class&)            = delete; \
  Class& operator=(const Class&) = delete

// --------------------------------------------
// Move semantics
// --------------------------------------------
#define ENABLE_MOVE(Class)             \
  Class(Class&&)            = default; \
  Class& operator=(Class&&) = default

#define DISABLE_MOVE(Class)           \
  Class(Class&&)            = delete; \
  Class& operator=(Class&&) = delete

// --------------------------------------------
// Full presets
// --------------------------------------------

// Non copyable - Non Movable
#define IMMUTABLE(Class) \
  DISABLE_COPY(Class);   \
  DISABLE_MOVE(Class)

// Non copyable - Movable
#define MOVE_ONLY(Class) \
  DISABLE_COPY(Class);   \
  ENABLE_MOVE(Class)

// Copyable - None Movable
#define NON_MOVABLE(Class) \
  ENABLE_COPY(Class);      \
  DISABLE_MOVE(Class)

// Non Copyable - Movable
#define NON_COPYABLE(Class) \
  DISABLE_COPY(Class);      \
  ENABLE_MOVE(Class)
