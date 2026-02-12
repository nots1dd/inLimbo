#pragma once

#include <cstddef>

// ============================================================
// Project and Version Info
// ============================================================
#define INLIMBO_VERSION_MAJOR                       0
#define INLIMBO_VERSION_MINOR                       0
#define INLIMBO_VERSION_PATCH                       0
#define INLIMBO_VERSION_STR                         "0.0.1"
#define INLIMBO_VERSION_ENCODE(major, minor, patch) (((major) * 10000) + ((minor) * 100) + (patch))
#define INLIMBO_VERSION \
  INLIMBO_VERSION_ENCODE(INLIMBO_VERSION_MAJOR, INLIMBO_VERSION_MINOR, INLIMBO_VERSION_PATCH)

// ============================================================
// Git Commit Info (provided by cmake)
// ============================================================

#ifndef INLIMBO_GIT_COMMIT_HASH
#define INLIMBO_GIT_COMMIT_HASH "unknown"
#endif

#ifndef INLIMBO_GIT_COMMIT_SHORT
#define INLIMBO_GIT_COMMIT_SHORT "unknown"
#endif

#ifndef INLIMBO_GIT_BRANCH
#define INLIMBO_GIT_BRANCH "unknown"
#endif

#ifndef INLIMBO_GIT_DIRTY
#define INLIMBO_GIT_DIRTY 0
#endif

#define INLIMBO_BUILD_ID INLIMBO_VERSION_STR "-" INLIMBO_GIT_COMMIT_SHORT

// Global defaults
#define INLIMBO_DEFAULT_CONFIG_FILE_NAME            "config.toml"
#define INLIMBO_DEFAULT_LOCKFILE_PATH               "/tmp/inLimbo.lock"
#define INLIMBO_DEFAULT_CACHE_BIN_NAME              "lib.bin"
#define INLIMBO_DEFAULT_TELEMETRY_BIN_NAME          "telemetry.bin"
#define INLIMBO_DEFAULT_TELEMETRY_REGISTRY_BIN_NAME "telemetry_registry.bin"

// ============================================================
// Compiler / Standard / Platform Detection
// ============================================================
#if defined(_MSVC_LANG)
#define CPP_STD _MSVC_LANG
#else
#define CPP_STD __cplusplus
#endif

#if CPP_STD >= 202302L
#define CPP_VER 23
#elif CPP_STD >= 202002L
#define CPP_VER 20
#elif CPP_STD >= 201703L
#else
#error "C++20 or higher required."
#endif

// ============================================================
// Compiler Detection
// ============================================================

#if defined(_MSC_VER)

#define COMPILER_MSVC 1
#define INLIMBO_COMPILER_ID_MSVC 1
#define INLIMBO_COMPILER_NAME "MSVC"
#define INLIMBO_COMPILER_VERSION_MAJOR (_MSC_VER / 100)
#define INLIMBO_COMPILER_VERSION_MINOR (_MSC_VER % 100)

#elif defined(__clang__)

#define COMPILER_CLANG 1
#define INLIMBO_COMPILER_ID_CLANG 2
#define INLIMBO_COMPILER_NAME "Clang"
#define INLIMBO_COMPILER_VERSION_MAJOR __clang_major__
#define INLIMBO_COMPILER_VERSION_MINOR __clang_minor__
#define INLIMBO_COMPILER_VERSION_PATCH __clang_patchlevel__

#elif defined(__GNUC__)

#define COMPILER_GCC 1
#define INLIMBO_COMPILER_ID_GCC 3
#define INLIMBO_COMPILER_NAME "GCC"
#define INLIMBO_COMPILER_VERSION_MAJOR __GNUC__
#define INLIMBO_COMPILER_VERSION_MINOR __GNUC_MINOR__
#define INLIMBO_COMPILER_VERSION_PATCH __GNUC_PATCHLEVEL__

#else

#define COMPILER_UNKNOWN 1
#define INLIMBO_COMPILER_ID_UNKNOWN 0
#define INLIMBO_COMPILER_NAME "Unknown"

#endif

#if defined(INLIMBO_COMPILER_ID_MSVC)
#define INLIMBO_COMPILER_ID INLIMBO_COMPILER_ID_MSVC
#elif defined(INLIMBO_COMPILER_ID_CLANG)
#define INLIMBO_COMPILER_ID INLIMBO_COMPILER_ID_CLANG
#elif defined(INLIMBO_COMPILER_ID_GCC)
#define INLIMBO_COMPILER_ID INLIMBO_COMPILER_ID_GCC
#else
#define INLIMBO_COMPILER_ID INLIMBO_COMPILER_ID_UNKNOWN
#endif

#define INLIMBO_COMPILER_VERSION_STR \
    STRINGIFY(INLIMBO_COMPILER_VERSION_MAJOR) "." \
    STRINGIFY(INLIMBO_COMPILER_VERSION_MINOR)

#if defined(INLIMBO_COMPILER_VERSION_PATCH)
#define INLIMBO_COMPILER_VERSION_FULL_STR \
    STRINGIFY(INLIMBO_COMPILER_VERSION_MAJOR) "." \
    STRINGIFY(INLIMBO_COMPILER_VERSION_MINOR) "." \
    STRINGIFY(INLIMBO_COMPILER_VERSION_PATCH)
#else
#define INLIMBO_COMPILER_VERSION_FULL_STR INLIMBO_COMPILER_VERSION_STR
#endif

// Final BUILD version string
#define INLIMBO_BUILD_VERSION_STRING \
  std::string("Version  : ") + INLIMBO_VERSION_STR + "\n" \
+ "Commit   : " + INLIMBO_GIT_COMMIT_HASH + "\n" \
+ "Build ID : " + INLIMBO_BUILD_ID + "\n" \
+ "Branch   : " + INLIMBO_GIT_BRANCH + "\n" \
+ "Dirty    : " + std::string(INLIMBO_GIT_DIRTY ? "yes" : "no") + "\n" \
+ "Compiler : " + std::string(INLIMBO_COMPILER_NAME) + " " + INLIMBO_COMPILER_VERSION_FULL_STR + "\n"

#if defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS 1
#elif defined(__APPLE__) && defined(__MACH__)
#define PLATFORM_MACOS 1
#elif defined(__linux__)
#define PLATFORM_LINUX 1
#elif defined(__unix__)
#define PLATFORM_UNIX 1
#else
#define PLATFORM_UNKNOWN 1
#endif

#if defined(_M_X64) || defined(__x86_64__)
#define ARCH_X64 1
#elif defined(_M_IX86) || defined(__i386__)
#define ARCH_X86 1
#elif defined(__aarch64__)
#define ARCH_ARM64 1
#elif defined(__arm__)
#define ARCH_ARM 1
#else
#define ARCH_UNKNOWN 1
#endif

// ============================================================
// API Visibility and Calling Conventions
// ============================================================

// --- DLL Export/Import ---
#if PLATFORM_WINDOWS
#ifdef INLIMBO_BUILD
#define INLIMBO_API __declspec(dllexport)
#else
#define INLIMBO_API __declspec(dllimport)
#endif
#else
#define INLIMBO_API __attribute__((visibility("default")))
#endif

// --- Calling Conventions ---
#if PLATFORM_WINDOWS
#define INLIMBO_STDCALL    __stdcall
#define INLIMBO_CDECL      __cdecl
#define INLIMBO_FASTCALL   __fastcall
#define INLIMBO_VECTORCALL __vectorcall
#else
// On Linux/macOS, use GCC attributes or empty definitions for clarity
#define INLIMBO_STDCALL    __attribute__((stdcall))
#define INLIMBO_CDECL      __attribute__((cdecl))
#define INLIMBO_FASTCALL   __attribute__((fastcall))
#define INLIMBO_VECTORCALL /* not supported on GCC */
#endif

// --- C vs C++ API exposure ---
#if PLATFORM_WINDOWS
#define INLIMBO_API_C   extern "C" INLIMBO_API INLIMBO_CDECL
#define INLIMBO_API_CPP INLIMBO_API
#else
#define INLIMBO_API_C   extern "C" INLIMBO_API INLIMBO_CDECL
#define INLIMBO_API_CPP INLIMBO_API
#endif

// Example usage:
// INLIMBO_API_C void INLIMBO_STDCALL api_function(int x);
// INLIMBO_API_CPP class INLIMBO_API Foo {};

// ============================================================
// Deprecation, Hints, Inlining, Alignment, Assertions
// ============================================================
#if COMPILER_MSVC
#define DEPRECATED(msg) __declspec(deprecated(msg))
#elif defined(__has_cpp_attribute)
#if __has_cpp_attribute(deprecated)
#define DEPRECATED(msg) [[deprecated(msg)]]
#else
#define DEPRECATED(msg)
#endif
#else
#define DEPRECATED(msg)
#endif

#define UNUSED(x)         (void)(x)
#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x)      STRINGIFY_IMPL(x)
#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b)      CONCAT_IMPL(a, b)

#if COMPILER_GCC || COMPILER_CLANG
#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define LIKELY(x)   (x)
#define UNLIKELY(x) (x)
#endif

#if COMPILER_MSVC
#define FORCE_INLINE __forceinline
#elif COMPILER_GCC || COMPILER_CLANG
#define FORCE_INLINE inline __attribute__((always_inline))
#else
#define FORCE_INLINE inline
#endif

#if COMPILER_GCC || COMPILER_CLANG
#define ALIGNAS(x) __attribute__((aligned(x)))
#define NORETURN   __attribute__((noreturn))
#elif COMPILER_MSVC
#define ALIGNAS(x) __declspec(align(x))
#define NORETURN   __declspec(noreturn)
#else
#define ALIGNAS(x)
#define NORETURN
#endif

#define STATIC_ASSERT(expr, msg) static_assert((expr), msg)

#ifdef NDEBUG
#define ASSERT_MSG(expr, msg) ((void)0)
#else
#define ASSERT_MSG(expr, msg)                                          \
  do                                                                   \
  {                                                                    \
    if (!(expr))                                                       \
    {                                                                  \
      std::ostringstream _oss;                                         \
      _oss << "\n🚨 Assertion failed!\n"                               \
           << "  Expression : " << #expr << "\n"                       \
           << "  Message    : " << msg << "\n"                         \
           << "  Function   : " << __func__ << "\n"                    \
           << "  Location   : " << __FILE__ << ":" << __LINE__ << "\n" \
           << "----------- IGNORE BACKTRACE -----------\n\n";          \
      std::cerr << _oss.str() << std::flush;                           \
      std::abort();                                                    \
    }                                                                  \
  } while (0)
#endif

// ============================================================
// Build Config
// ============================================================
#if defined(NDEBUG)
#define BUILD_RELEASE 1
#else
#define BUILD_DEBUG 1
#endif

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define ENDIAN_BIG 1
#else
#define ENDIAN_LITTLE 1
#endif

// ============================================================
// ABI & Binary Compatibility
// ============================================================
#define ABI_VERSION_MAJOR 1
#define ABI_VERSION_MINOR 0
#define ABI_VERSION_PATCH 0
#define ABI_VERSION       INLIMBO_VERSION_ENCODE(ABI_VERSION_MAJOR, ABI_VERSION_MINOR, ABI_VERSION_PATCH)
#define ABI_TAG_STRING    "ABI" STRINGIFY(ABI_VERSION_MAJOR) "." STRINGIFY(ABI_VERSION_MINOR)

#if COMPILER_GCC || COMPILER_CLANG
#define ABI_EXPORT              extern "C" __attribute__((visibility("default")))
#define ABI_HIDDEN              __attribute__((visibility("hidden")))
#define ABI_ALIAS(sym)          __attribute__((alias(#sym)))
#define ABI_VERSIONED(sym, ver) __asm__(#sym "@" ver)
#elif COMPILER_MSVC
#define ABI_EXPORT extern "C" __declspec(dllexport)
#define ABI_HIDDEN
#define ABI_ALIAS(sym)
#define ABI_VERSIONED(sym, ver)
#else
#define ABI_EXPORT extern "C"
#define ABI_HIDDEN
#define ABI_ALIAS(sym)
#define ABI_VERSIONED(sym, ver)
#endif

#define PACK_BEGIN(name, align)                           \
  _Pragma("pack(push, align)") struct ALIGNAS(align) name \
  {
#define PACK_END() \
  }                \
  _Pragma("pack(pop)")

#define ABI_NAMESPACE_BEGIN(ver) \
  namespace ABI_v##ver           \
  {
#define ABI_NAMESPACE_END(ver) \
  }                            \
  using namespace ABI_v##ver;
#define ABI_LOCK(name, version)                                  \
  namespace name                                                 \
  {                                                              \
  static constexpr int ABI_MAJOR = version;                      \
  static_assert(ABI_MAJOR == ABI_VERSION_MAJOR, "ABI mismatch"); \
  }

#define STABLE_STRUCT(name) struct name final
#define STABLE_ENUM(name)   enum class name : int

#define ABI_INLINE                inline __attribute__((__always_inline__, __gnu_inline__, __artificial__))
#define ABI_NO_INLINE             __attribute__((noinline))
#define EXPORT_C                  ABI_EXPORT INLIMBO_API_C
#define EXPORT_CPP                INLIMBO_API_CPP
#define ABI_STABILITY_CHECK(expr) static_assert((expr), "ABI stability check failed")

// ============================================================
// Structure Layout & Binary Safety Guards
// ============================================================
#define VERIFY_SIZE(type, expected) \
  static_assert(sizeof(type) == (expected), "Size mismatch in " #type)

#define VERIFY_ALIGN(type, expected) \
  static_assert(alignof(type) == (expected), "Alignment mismatch in " #type)

#define VERIFY_OFFSET(type, field, expected)                                               \
  static_assert(offsetof(type, field) == (expected), "Field offset mismatch in " #type ":" \
                                                     ":" #field)

#define VERIFY_LAYOUT(type, size, align) \
  VERIFY_SIZE(type, size);               \
  VERIFY_ALIGN(type, align)

// ============================================================
// Optional ABI/Feature Macros
// ============================================================
#define CHECK_ABI_COMPAT(old, newtype)                                              \
  static_assert(sizeof(old) == sizeof(newtype) && alignof(old) == alignof(newtype), \
                "ABI layout mismatch")

#define SYMBOL_GUARD(sym) extern "C" void sym() __attribute__((weak, alias("__abi_guard_" #sym)))

#define FEATURE_LOCK(name, state)                       \
  static constexpr bool CONCAT(feature_, name) = state; \
  static_assert(CONCAT(feature_, name), "Feature " #name " must remain enabled")

#define ABI_FEATURE_FLAG(name) static constexpr const char* CONCAT(abi_feature_, name) = #name

#define ABI_NAMESPACE(ver) namespace abi_##ver

#define ABI_EXPORT_STRUCT(name, ver)    \
  ABI_NAMESPACE_BEGIN(ver) struct name; \
  ABI_NAMESPACE_END(ver)

// ============================================================
// Type & Struct ID Tags
// ============================================================
#define ABI_TYPE_TAG(name, id)                       \
  static constexpr const char* ABI_TypeName = #name; \
  static constexpr uint32_t    ABI_TypeId   = id

#define ABI_STRUCT_ID(name, ver) \
  static constexpr uint64_t ABI_StructHash = ((uint64_t)ver << 32) ^ sizeof(name)
