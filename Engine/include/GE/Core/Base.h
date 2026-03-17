#pragma once

#include <memory>
#include <cstdint>

// ─── Platform detection ──────────────────────────────────────────────────────
#if defined(GE_PLATFORM_WINDOWS)
    #define GE_API __declspec(dllexport)
#else
    #define GE_API __attribute__((visibility("default")))
#endif

// ─── Assert ──────────────────────────────────────────────────────────────────
#ifdef GE_DEBUG
    #if defined(GE_PLATFORM_WINDOWS)
        #define GE_DEBUGBREAK() __debugbreak()
    #else
        #include <signal.h>
        #define GE_DEBUGBREAK() raise(SIGTRAP)
    #endif

    #define GE_ASSERT(x, ...) \
        if (!(x)) { GE_ERROR("Assertion failed: {0}", __VA_ARGS__); GE_DEBUGBREAK(); }
#else
    #define GE_DEBUGBREAK()
    #define GE_ASSERT(x, ...)
#endif

// ─── Primitives ──────────────────────────────────────────────────────────────
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using f32 = float;
using f64 = double;

// ─── Smart pointer aliases ───────────────────────────────────────────────────
template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T>
using Scope = std::unique_ptr<T>;

template<typename T, typename... Args>
constexpr Ref<T> MakeRef(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
constexpr Scope<T> MakeScope(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}
